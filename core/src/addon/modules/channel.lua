--[[
Copyright © Windower Dev Team

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"),to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

-- LuaFormatter off
local -- params
    registry,
    remote_pcall_key,
    get_remote_handle,
    remote_pcall_native_ptr = ...
-- LuaFormatter on

local ffi = require('ffi')

local serializer = require('core.serializer')
local windower = require('core.windower')

local getfenv = getfenv
local pcall = pcall
local select = select
local setfenv = setfenv
local setmetatable = setmetatable
local unpack = unpack

local ffi_cast = ffi.cast
local ffi_typeof = ffi.typeof

local serialize_buffer = serializer.serialize_buffer
local deserialize_buffer = serializer.deserialize_buffer

local package_name = windower.package_name or '<script>'

local package_name_key = {}
local name_key = {}
local handle_key = {}

local script_handle = {}

local servers = setmetatable({}, {__mode = 'v'})

local pcall_impl
do
    local void_ptr_ref = ffi_typeof('void*[1]')
    local size_t_ref = ffi_typeof('int32_t[1]')
    local remote_pcall_native = ffi_cast('int32_t(*)(void*, void*&, int32_t&)',
                                         remote_pcall_native_ptr)

    local restore = function(func, env, ...)
        setfenv(func, env)
        return ...
    end

    pcall_impl = function(channel, func, ...)
        local name = rawget(channel, name_key)
        local handle = rawget(channel, handle_key)
        if not handle then
            local server = servers[name]
            if server == nil then
                return false, 'channel \'' .. name ..
                           '\' not found or has been closed'
            end
            local env = server.env
            if type(env) ~= 'table' then env = {} end
            return restore(func, getfenv(func),
                           pcall(setfenv(func, env), server.data, ...))
        end
        local data, size = serialize_buffer({
            name = name,
            func = func,
            arg_count = select('#', ...),
            arg_table = {...}
        })
        if handle == script_handle then handle = nil end
        local data_ref = void_ptr_ref(data)
        local size_ref = size_t_ref(size)
        local result_code = remote_pcall_native(handle, data_ref, size_ref)
        if result_code == 0 then
            local result = deserialize_buffer(data_ref[0], size_ref[0])
            return unpack(result.result_table, 1, result.result_count)
        elseif result_code == 1 then
            return false, 'remote interpreter unloaded'
        elseif result_code == 2 then
            return false, 'error calling remote instance'
        else
            return false, 'unknown error'
        end
    end
end

local call_impl
do
    local results = function(ok, ...)
        if not ok then
            local message = ...
            error(message)
        end
        return ...
    end

    call_impl = function(channel, func, ...)
        return results(pcall_impl(channel, func, ...))
    end
end

local read_impl
local read_0_impl
local read_1_impl
local read_n_impl
do
    read_0_impl = function(data) return data end

    read_1_impl = function(data, key) return data[key] end

    read_n_impl = function(data, count, ...)
        local res = data
        for i = 1, count do res = res[select(i, ...)] end
        return res
    end

    read_impl = function(channel, ...)
        local count = select('#', ...)
        if count == 0 then
            return call_impl(channel, read_0_impl)
        elseif count == 1 then
            return call_impl(channel, read_1_impl, (...))
        else
            return call_impl(channel, read_n_impl, count, ...)
        end
    end
end

local new_server = function(name)
    local server = {}
    servers[name] = server
    return server
end

local get_client
do
    local metatable = {
        __index = {pcall = pcall_impl, call = call_impl, read = read_impl},
        __tostring = function(c)
            return 'core.channel:' .. rawget(c, package_name_key) .. ':' ..
                       rawget(c, name_key)
        end,
        __metatable = false
    }

    get_client = function(remote_package_name, remote_name)
        if remote_package_name == package_name then
            return setmetatable({
                [package_name_key] = remote_package_name,
                [name_key] = remote_name
            }, metatable)
        else
            local handle = script_handle
            if remote_package_name ~= '<script>' then
                handle = get_remote_handle(remote_package_name)
            end
            return setmetatable({
                [package_name_key] = remote_package_name,
                [name_key] = remote_name,
                [handle_key] = handle
            }, metatable)
        end
    end
end

local remote_pcall
do
    local intptr_t = ffi_typeof('intptr_t')

    local make_result = function(...)
        local ptr, size = serialize_buffer({
            result_count = select('#', ...),
            result_table = {...}
        })
        return tonumber(ffi_cast(intptr_t, ptr)), size
    end

    remote_pcall = function(data_ptr, data_size)
        local ok, data = pcall(deserialize_buffer, data_ptr, data_size)
        if not ok then return make_result(false, data) end
        if type(data) ~= 'table' or type(data.name) ~= 'string' or
            type(data.func) ~= 'function' or type(data.arg_count) ~= 'number' or
            type(data.arg_table) ~= 'table' then
            return make_result(false, 'invalid channel protocol')
        end
        local server = servers[data.name]
        if server == nil then
            return make_result(false, 'channel \'' .. data.name ..
                                   '\' not found or has been closed')
        end
        local env = server.env
        if type(env) ~= 'table' then env = {} end
        return make_result(pcall(setfenv(data.func, env), server.data,
                                 unpack(data.arg_table, 1, data.arg_count)))
    end
end

local channel = {
    new = new_server,
    get = get_client,
    pcall = pcall_impl,
    call = call_impl,
    read = read_impl
}

rawset(registry, remote_pcall_key, remote_pcall)

serializer.register('__channel', channel, false)
serializer.register('__channel.new', channel.new, false)
serializer.register('__channel.get', channel.get, false)
serializer.register('__channel.pcall', channel.pcall, false)
serializer.register('__channel.call', channel.call, false)
serializer.register('__channel.read', channel.read, false)
serializer.register('__channel.read_0', read_0_impl, false)
serializer.register('__channel.read_1', read_1_impl, false)
serializer.register('__channel.read_n', read_n_impl, false)

return channel
