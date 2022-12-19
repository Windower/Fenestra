--[[
This serialization library is based on the bitser library by Robin
Wellner. It has been modified to suit the needs of the Windower project.
===========================================================================
Copyright © 2016 Robin Wellner
Copyright © 2019 Windower Dev Team

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
]]

local bit = require('bit')
local debug = require('debug')
local ffi = require('ffi')
local io = require('io')
local math = require('math')
local os = require('os')
local string = require('string')

local class = require('core.class')

---@class __windower_coroutinelib : coroutinelib
---@field schedule fun(function: fun())
---@field sleep fun(delay: number)
---@field sleep_frame fun(delay?: number)
local coroutine = coroutine

---@class __windower_jitlib_opt
---@field start fun(...)
---@class __windower_jitlib : jitlib
---@field opt __windower_jitlib_opt
local jit = require('jit')

---@class __windower_tablelib : tablelib
---@field move fun(...)
local table = require('table')

local getmetatable = getmetatable
local pairs = pairs
local setmetatable = setmetatable
local type = type

local debug_getinfo = debug.getinfo
local debug_getupvalue = debug.getupvalue
local ffi_cast = ffi.cast
local ffi_copy = ffi.copy
local ffi_new = ffi.new
local ffi_string = ffi.string
local string_dump = string.dump
local table_insert = table.insert

local seen_length_key = {}

local buf_pos = 0
local buf_size = -1
local buf = nil
local writable_buf = nil
local writable_buf_size = nil
local serialize_upvalues = false

local uint8_t_array = ffi.typeof('uint8_t[?]')
local uint8_t_ptr = ffi.typeof('uint8_t*')
local int16_t_ref = ffi.typeof('int16_t[1]')
local int32_t_ref = ffi.typeof('int32_t[1]')
local double_ref = ffi.typeof('double[1]')

local function buffer_prereserve(min_size)
    if buf_size < min_size then
        buf_size = min_size
        buf = uint8_t_array(buf_size)
    end
end

local function buffer_clear()
    buf_size = -1
    buf = nil
    writable_buf = nil
    writable_buf_size = nil
end

local function buffer_make_buffer(size)
    if writable_buf then
        buf = writable_buf
        buf_size = writable_buf_size
        writable_buf = nil
        writable_buf_size = nil
    end
    buf_pos = 0
    buffer_prereserve(size)
end

local function buffer_new_reader(str)
    local size = #str
    buffer_make_buffer(size)
    ffi_copy(buf, str, size)
end

local function buffer_new_data_reader(data, size)
    writable_buf = buf
    writable_buf_size = buf_size
    buf_pos = 0
    buf_size = size
    buf = ffi_cast(uint8_t_ptr, data)
end

local function buffer_reserve(additional_size)
    while buf_pos + additional_size > buf_size do
        buf_size = buf_size * 2
        local oldbuf = buf
        buf = uint8_t_array(buf_size)
        ffi_copy(buf, oldbuf, buf_pos)
    end
end

local function buffer_write_byte(value)
    buffer_reserve(1)
    buf[buf_pos] = value
    buf_pos = buf_pos + 1
end

local function buffer_write_string(value)
    local size = #value
    buffer_reserve(size)
    ffi_copy(buf + buf_pos, value, size)
    buf_pos = buf_pos + size
end

local function buffer_write_data(ct, size, ...)
    buffer_reserve(size)
    ffi_copy(buf + buf_pos, ffi_new(ct, ...), size)
    buf_pos = buf_pos + size
end

local function buffer_ensure(size)
    if buf_pos + size > buf_size then error('malformed serialized data') end
end

local function buffer_read_byte()
    buffer_ensure(1)
    local x = buf[buf_pos]
    buf_pos = buf_pos + 1
    return x
end

local function buffer_read_string(size)
    buffer_ensure(size)
    local x = ffi_string(buf + buf_pos, size)
    buf_pos = buf_pos + size
    return x
end

local function buffer_read_data(ct, size)
    buffer_ensure(size)
    local x = ffi_new(ct)
    ffi_copy(x, buf + buf_pos, size)
    buf_pos = buf_pos + size
    return x
end

local resource_registry = {}
local resource_name_registry = {}
local resource_safe_registry = {}

local class_serializer_registry = {}
local class_deserializer_registry = {}

local serialize_value

local function write_number(value, seen)
    if (value + 2 ^ 52) - 2 ^ 52 == value and value >= -2147483648 and value <=
        2147483647 then
        if value >= -27 and value <= 100 then
            -- small int
            buffer_write_byte(value + 27)
        elseif value >= -32768 and value <= 32767 then
            -- int16_t
            buffer_write_byte(250)
            buffer_write_data(int16_t_ref, 2, value)
        else
            -- int13_t
            buffer_write_byte(245)
            buffer_write_data(int32_t_ref, 4, value)
        end
    else
        -- double
        buffer_write_byte(246)
        buffer_write_data(double_ref, 8, value)
    end
end

local function write_string(value, seen)
    local size = #value
    if size <= 31 then
        -- small string
        buffer_write_byte(size + 192)
    else
        -- large string
        buffer_write_byte(244)
        write_number(size - 58)
    end
    buffer_write_string(value)
end

local function write_nil(value, seen) buffer_write_byte(247) end

local function write_boolean(value, seen) buffer_write_byte(value and 249 or 248) end

local function write_table(value, seen)
    local class_name = class(value)
    if class_name then
        buffer_write_byte(242)
        serialize_value(class_name, seen)
        local serializer = class_serializer_registry[class_name]
        value = serializer(value)
    else
        buffer_write_byte(240)
    end
    local len = #value
    write_number(len, seen)
    for i = 1, len do serialize_value(value[i], seen) end
    for k, v in pairs(value) do
        if type(k) ~= 'number' or (k + 2 ^ 52) - 2 ^ 52 ~= k or k > len or k < 1 then
            serialize_value(k, seen)
            serialize_value(v, seen)
        end
    end
    write_nil(nil, seen)
end

local function write_function(value, seen)
    if serialize_upvalues then
        local info = debug_getinfo(value, 'u')
        if info.nups ~= 0 then
            buffer_write_byte(252)
            write_string(string_dump(value))
            write_number(info.nups)
            for i = 1, info.nups do
                local _, upvalue = debug_getupvalue(value, i)
                if resource_safe_registry[upvalue] == false then
                    error('attempt to serialize an unsafe upvalue')
                end
                serialize_value(upvalue, seen)
            end
            return
        end
    end
    buffer_write_byte(251)
    write_string(string_dump(value))
end

local types = {
    ['number'] = write_number,
    ['string'] = write_string,
    ['table'] = write_table,
    ['boolean'] = write_boolean,
    ['nil'] = write_nil,
    ['function'] = write_function
}

serialize_value = function(value, seen)
    if seen[value] then
        local ref = seen[value]
        if ref <= 63 then
            -- small reference
            buffer_write_byte(ref + 127)
        else
            -- large reference
            buffer_write_byte(243)
            write_number(ref - 91, seen)
        end
        return
    end
    local t = type(value)
    if t ~= 'number' and t ~= 'boolean' and t ~= 'nil' then
        local next = seen[seen_length_key] + 1
        seen[value] = next
        seen[seen_length_key] = next
    end
    if resource_name_registry[value] then
        local name = resource_name_registry[value]
        local size = #name
        if size < 16 then
            -- small resource
            buffer_write_byte(size + 224)
            buffer_write_string(name)
        else
            -- large resource
            buffer_write_byte(241)
            write_string(name, seen)
        end
        return
    end
    (types[t] or error('cannot serialize type ' .. t))(value, seen)
end

local function serialize_impl(value)
    buffer_make_buffer(4096)
    local seen = {[seen_length_key] = 0}
    serialize_value(value, seen)
end

local function add_to_seen(value, seen)
    table_insert(seen, value)
    return value
end

local function reserve_seen(seen)
    table_insert(seen, 0)
    return #seen
end

local function deserialize_value(seen)
    local t = buffer_read_byte()
    if t < 128 then
        -- small int
        return t - 27
    elseif t < 192 then
        -- small reference
        return seen[t - 127]
    elseif t < 224 then
        -- small string
        return add_to_seen(buffer_read_string(t - 192), seen)
    elseif t < 240 then
        -- small resource
        return add_to_seen(resource_registry[buffer_read_string(t - 224)], seen)
    elseif t == 240 then
        -- table
        local v = add_to_seen({}, seen)
        local len = deserialize_value(seen)
        for i = 1, len do v[i] = deserialize_value(seen) end
        local key = deserialize_value(seen)
        while key ~= nil do
            v[key] = deserialize_value(seen)
            key = deserialize_value(seen)
        end
        return v
    elseif t == 241 then
        -- large resource
        local idx = reserve_seen(seen)
        local value = resource_registry[deserialize_value(seen)]
        seen[idx] = value
        return value
    elseif t == 242 then
        -- instance
        local instance = add_to_seen({}, seen)
        local class_name = deserialize_value(seen)
        local deserializer = class_deserializer_registry[class_name]
        local len = deserialize_value(seen)
        for i = 1, len do instance[i] = deserialize_value(seen) end
        local key = deserialize_value(seen)
        while key ~= nil do
            instance[key] = deserialize_value(seen)
            key = deserialize_value(seen)
        end
        return deserializer(instance, class)
    elseif t == 243 then
        -- large reference
        return seen[deserialize_value(seen) + 91]
    elseif t == 244 then
        -- large string
        local size = deserialize_value(seen) + 58
        local value = buffer_read_string(size)
        return add_to_seen(value, seen)
    elseif t == 245 then
        -- int32_t
        return buffer_read_data(int32_t_ref, 4)[0]
    elseif t == 246 then
        -- double
        return buffer_read_data(double_ref, 8)[0]
    elseif t == 247 then
        -- nil
        return nil
    elseif t == 248 then
        -- false
        return false
    elseif t == 249 then
        -- true
        return true
    elseif t == 250 then
        -- int16_t
        return buffer_read_data(int16_t_ref, 2)[0]
    elseif t == 251 then
        -- function
        return add_to_seen(loadstring(deserialize_value({})), seen)
    elseif t == 252 then
        -- function + upvalues
        local idx = reserve_seen(seen)
        local value = loadstring(deserialize_value({}))
        local upvalue_count = deserialize_value({})
        for i = 1, upvalue_count do
            local upvalue = deserialize_value(seen)
            if resource_safe_registry[upvalue] == false then
                error('attempt to deserialize an unsafe upvalue')
            end
            debug.setupvalue(value, i, upvalue)
        end
        seen[idx] = value
        return value
    else
        error('unsupported serialized type ' .. t)
    end
end

local serialize_buffer = function(value, preserve_upvalues)
    serialize_upvalues = preserve_upvalues or false
    serialize_impl(value)
    return buf, buf_pos
end

local deserialize_buffer = function(data, size, preserve_upvalues)
    serialize_upvalues = preserve_upvalues or false
    buffer_new_data_reader(data, size)
    return deserialize_value({})
end

local serialize = function(value, preserve_upvalues)
    return ffi_string(serialize_buffer(value, preserve_upvalues))
end

local deserialize = function(str, preserve_upvalues)
    serialize_upvalues = preserve_upvalues or false
    buffer_new_reader(str)
    return deserialize_value({})
end

local register = function(name, resource, safe)
    if type(name) ~= 'string' then error() end

    if safe == nil then
        safe = true
    elseif type(safe) ~= 'boolean' then
        error()
    end

    if resource_registry[name] ~= nil then
        error('\'' .. name .. '\' already registered with value ' ..
                  tostring(resource_registry[name]))
    end

    if resource_name_registry[name] ~= nil then
        error(tostring(resource) .. ' already registered with name \'' ..
                  resource_name_registry[name] .. '\'')
    end

    resource_registry[name] = resource
    resource_name_registry[resource] = name
    resource_safe_registry[resource] = safe
    return resource
end

local unregister = function(name)
    if type(name) ~= 'string' then error() end

    local resource = resource_registry[name]
    if resource ~= nil then
        resource_registry[name] = nil
        resource_name_registry[resource] = nil
        resource_safe_registry[resource] = nil
    end
end

local register_class = function(name, serializer, deserializer)
    if type(name) ~= 'string' then error() end

    class_serializer_registry[name] = serializer
    class_deserializer_registry[name] = deserializer
end

local unregister_class = function(name)
    if type(name) ~= 'string' then error() end

    class_serializer_registry[name] = nil
    class_deserializer_registry[name] = nil
end

local reserve_buffer = buffer_prereserve
local clear_buffer = buffer_clear

local serializer = {
    serialize_buffer = serialize_buffer,
    deserialize_buffer = deserialize_buffer,
    serialize = serialize,
    deserialize = deserialize,
    register = register,
    unregister = unregister,
    register_class = register_class,
    unregister_class = unregister_class,
    reserve_buffer = buffer_prereserve,
    clear_buffer = buffer_clear
}

-- built-ins
serializer.register('___G', _G, false)
serializer.register('__assert', assert)
serializer.register('__collectgarbage', collectgarbage, false)
serializer.register('__dofile', dofile)
serializer.register('__error', error)
serializer.register('__getfenv', getfenv, false)
serializer.register('__getmetatable', getmetatable)
serializer.register('__ipairs', ipairs)
serializer.register('__load', load)
serializer.register('__loadfile', loadfile)
serializer.register('__loadstring', loadstring)
serializer.register('__next', next)
serializer.register('__pairs', pairs)
serializer.register('__pcall', pcall)
serializer.register('__print', print)
serializer.register('__rawequal', rawequal)
serializer.register('__rawget', rawget)
serializer.register('__rawlen', rawlen)
serializer.register('__rawset', rawset)
serializer.register('__require', require, false)
serializer.register('__select', select)
serializer.register('__setfenv', setfenv, false)
serializer.register('__setmetatable', setmetatable)
serializer.register('__tonumber', tonumber)
serializer.register('__tostring', tostring)
serializer.register('__type', type)
serializer.register('__unpack', unpack)
serializer.register('__xpcall', xpcall)

-- bit
serializer.register('__bit', bit, false)
serializer.register('__bit.arshift', bit.arshift)
serializer.register('__bit.band', bit.band)
serializer.register('__bit.bnot', bit.bnot)
serializer.register('__bit.bor', bit.bor)
serializer.register('__bit.bswap', bit.bswap)
serializer.register('__bit.bxor', bit.bxor)
serializer.register('__bit.lshift', bit.lshift)
serializer.register('__bit.rol', bit.rol)
serializer.register('__bit.ror', bit.ror)
serializer.register('__bit.rshift', bit.rshift)
serializer.register('__bit.tobit', bit.tobit)
serializer.register('__bit.tohex', bit.tohex)

-- coroutine
serializer.register('__coroutine', coroutine, false)
serializer.register('__coroutine.create', coroutine.create, false)
serializer.register('__coroutine.isyieldable', coroutine.isyieldable)
serializer.register('__coroutine.resume', coroutine.resume, false)
serializer.register('__coroutine.running', coroutine.running, false)
serializer.register('__coroutine.status', coroutine.status)
serializer.register('__coroutine.yield', coroutine.yield, false)
serializer.register('__coroutine.sleep', coroutine.sleep, false)
serializer.register('__coroutine.sleep_frame', coroutine.sleep_frame, false)
serializer.register('__coroutine.schedule', coroutine.schedule, false)

-- debug
serializer.register('__debug', debug, false)
serializer.register('__debug.debug', debug.debug, false)
serializer.register('__debug.getfenv', debug.getfenv, false)
serializer.register('__debug.gethook', debug.gethook, false)
serializer.register('__debug.getinfo', debug.getinfo, false)
serializer.register('__debug.getlocal', debug.getlocal, false)
serializer.register('__debug.getmetatable', debug.getmetatable, false)
serializer.register('__debug.getregistry', debug.getregistry, false)
serializer.register('__debug.getupvalue', debug.getupvalue, false)
serializer.register('__debug.getuservalue', debug.getuservalue, false)
serializer.register('__debug.setfenv', debug.setfenv, false)
serializer.register('__debug.setlocal', debug.setlocal, false)
serializer.register('__debug.setmetatable', debug.setmetatable, false)
serializer.register('__debug.setupvalue', debug.setupvalue, false)
serializer.register('__debug.setuservalue', debug.setuservalue, false)
serializer.register('__debug.traceback', debug.traceback)
serializer.register('__debug.upvalueid', debug.upvalueid, false)
serializer.register('__debug.upvaluejoin', debug.upvaluejoin, false)

-- ffi
serializer.register('__ffi', ffi, false)
serializer.register('__ffi.abi', ffi.abi)
serializer.register('__ffi.alignof', ffi.alignof, false)
serializer.register('__ffi.cast', ffi.cast, false)
serializer.register('__ffi.cdef', ffi.cdef, false)
serializer.register('__ffi.copy', ffi.copy)
serializer.register('__ffi.errno', ffi.errno, false)
serializer.register('__ffi.fill', ffi.fill)
serializer.register('__ffi.gc', ffi.gc, false)
serializer.register('__ffi.istype', ffi.istype, false)
serializer.register('__ffi.load', ffi.load, false)
serializer.register('__ffi.metatype', ffi.metatype, false)
serializer.register('__ffi.new', ffi.new, false)
serializer.register('__ffi.offsetof', ffi.offsetof, false)
serializer.register('__ffi.sizeof', ffi.sizeof, false)
serializer.register('__ffi.string', ffi.string)
serializer.register('__ffi.typeof', ffi.typeof, false)

-- io
serializer.register('__io', io, false)
serializer.register('__io.close', io.close, false)
serializer.register('__io.flush', io.flush, false)
serializer.register('__io.input', io.input, false)
serializer.register('__io.lines', io.lines, false)
serializer.register('__io.open', io.open, false)
serializer.register('__io.output', io.output, false)
serializer.register('__io.popen', io.popen, false)
serializer.register('__io.read', io.read, false)
serializer.register('__io.tmpfile', io.tmpfile, false)
serializer.register('__io.type', io.type, false)
serializer.register('__io.write', io.write, false)

-- jit
serializer.register('__jit', jit, false)
serializer.register('__jit.flush', jit.flush, false)
serializer.register('__jit.off', jit.off, false)
serializer.register('__jit.on', jit.on, false)
serializer.register('__jit.opt', jit.opt, false)
serializer.register('__jit.opt.start', jit.opt.start, false)
serializer.register('__jit.status', jit.status, false)
-- serializer.register('__jit.util', jit.util, false)

-- math
serializer.register('__math', math, false)
serializer.register('__math.abs', math.abs)
serializer.register('__math.acos', math.acos)
serializer.register('__math.asin', math.asin)
serializer.register('__math.atan', math.atan)
serializer.register('__math.atan2', math.atan2)
serializer.register('__math.ceil', math.ceil)
serializer.register('__math.cos', math.cos)
serializer.register('__math.cosh', math.cosh)
serializer.register('__math.deg', math.deg)
serializer.register('__math.exp', math.exp)
serializer.register('__math.floor', math.floor)
serializer.register('__math.fmod', math.fmod)
serializer.register('__math.frexp', math.frexp)
serializer.register('__math.ldexp', math.ldexp)
serializer.register('__math.log', math.log)
serializer.register('__math.log10', math.log10)
serializer.register('__math.max', math.max)
serializer.register('__math.min', math.min)
serializer.register('__math.modf', math.modf)
serializer.register('__math.pow', math.pow)
serializer.register('__math.rad', math.rad)
serializer.register('__math.random', math.random, false)
serializer.register('__math.randomseed', math.randomseed, false)
serializer.register('__math.sin', math.sin)
serializer.register('__math.sinh', math.sinh)
serializer.register('__math.sqrt', math.sqrt)
serializer.register('__math.tan', math.tan)
serializer.register('__math.tanh', math.tanh)

-- os
serializer.register('__os', os, false)
serializer.register('__os.clock', os.clock)
serializer.register('__os.date', os.date)
serializer.register('__os.difftime', os.difftime)
serializer.register('__os.execute', os.execute, false)
serializer.register('__os.exit', os.exit, false)
serializer.register('__os.getenv', os.getenv)
serializer.register('__os.remove', os.remove, false)
serializer.register('__os.rename', os.rename, false)
serializer.register('__os.setlocale', os.setlocale, false)
serializer.register('__os.time', os.time)
serializer.register('__os.tmpname', os.tmpname)

-- package
serializer.register('__package', package, false)
serializer.register('__package.loaded', package.loaded, false)
serializer.register('__package.loaders', package.loaders, false)
serializer.register('__package.loadlib', package.loadlib, false)
serializer.register('__package.searchers', package.searchers)
serializer.register('__package.searchpath', package.searchpath)

-- string
serializer.register('__string', string, false)
serializer.register('__string.byte', string.byte)
serializer.register('__string.char', string.char)
serializer.register('__string.dump', string.dump)
serializer.register('__string.find', string.find)
serializer.register('__string.format', string.format)
serializer.register('__string.gmatch', string.gmatch)
serializer.register('__string.gsub', string.gsub)
serializer.register('__string.len', string.len)
serializer.register('__string.lower', string.lower)
serializer.register('__string.match', string.match)
serializer.register('__string.rep', string.rep)
serializer.register('__string.reverse', string.reverse)
serializer.register('__string.sub', string.sub)
serializer.register('__string.upper', string.upper)

-- table
serializer.register('__table', table, false)
-- serializer.register('__table.clear', table.clear)
serializer.register('__table.concat', table.concat)
serializer.register('__table.insert', table.insert)
serializer.register('__table.maxn', table.maxn)
serializer.register('__table.move', table.move)
-- serializer.register('__table.new', table.new)
serializer.register('__table.remove', table.remove)
serializer.register('__table.pack', table.pack)
serializer.register('__table.sort', table.sort)
-- serializer.register('__table.unpack', table.unpack) -- alias of global unpack

-- core.class
serializer.register('__class', class)

-- core.serializer
serializer.register('__serializer', serializer, false)
serializer.register('__serializer.clear_buffer', clear_buffer)
serializer.register('__serializer.deserialize_buffer', deserialize_buffer)
serializer.register('__serializer.deserialize', deserialize)
serializer.register('__serializer.register_class', register_class)
serializer.register('__serializer.register', register)
serializer.register('__serializer.reserve_buffer', reserve_buffer)
serializer.register('__serializer.serialize', serialize)
serializer.register('__serializer.unregister_class', unregister_class)
serializer.register('__serializer.unregister', unregister)

return serializer
