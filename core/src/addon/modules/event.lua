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
    save_stack,
    error_addon = ...
-- LuaFormatter on

---@type __windower_coroutinelib
local coroutine = coroutine

local os = require('os')
local string = require('string')
local table = require('table')

local channel = require('core.channel')
local hash = require('core.hash')
local pin = require('core.pin')
local serializer = require('core.serializer')
local windower = require('core.windower')

local package_name = windower.package_name or '<script>'

local error = error
local next = next
local rawget = rawget
local setmetatable = setmetatable

local os_time = os.time
local os_clock = os.clock

local string_format = string.format

local table_remove = table.remove

local channel_get = channel.get

local package_name_key = {}
local name_key = {}
local local_handlers_key = {}
local remote_handlers_key = {}
local remote_handlers_lookup_key = {}
local server_key = {}
local serializable_key = {}

local servers = setmetatable({}, {__mode = 'v'})
local remote_clients = setmetatable({}, {__mode = 'v'})

local new_server
local new_local_client
local new_remote_client
local trigger
local register
local unregister
local client

local remote_trigger
local remote_register
local remote_unregister

local trigger_local_handlers = function(e, ...)
    local handlers = rawget(e, local_handlers_key)
    for i = 1, handlers.count do handlers[i](...) end
    return true
end

local trigger_remote_handlers = function(e, ...)
    local handlers = rawget(e, remote_handlers_key)
    if handlers == nil then return true end
    local slot = 1
    local last = handlers.count
    for i = 1, last do
        local handler = handlers[i]
        local ok, result = pcall(handler, ...)
        if ok and result ~= false then
            if i ~= slot then handlers[slot] = handler end
            slot = slot + 1
        end
    end
    while last >= slot do
        table_remove(handlers, last)
        last = last - 1
    end
    handlers.count = last
    return last > 0
end

local trigger_handlers = function(e, ...)
    local remote_result = trigger_remote_handlers(e, ...)
    trigger_local_handlers(e, ...)
    return remote_result
end

trigger = function(e, ...)
    if not rawget(e, server_key) then
        error('cannot call trigger on an event client object')
    end
    trigger_handlers(e, ...)
end

local register_handler = function(e, handler, handlers_key)
    local handlers = rawget(e, handlers_key)
    local count = handlers.count
    if count == 0 then
        local event_package_name = rawget(e, package_name_key)
        if event_package_name ~= package_name then
            local remote_server = channel_get(event_package_name, '__event')
            local event_name = rawget(e, name_key)
            local ok, result = remote_server:pcall(remote_register, event_name,
                                                   package_name)
            if not ok or not result then
                error('failed to register remote event handler')
            end
        end
    end
    for i = 1, count do if handlers[i] == handler then return end end
    count = count + 1
    handlers[count] = handler
    handlers.count = count
end

register =
    function(e, handler) register_handler(e, handler, local_handlers_key) end

local unregister_handler = function(e, handler, handlers_key)
    local handlers = rawget(e, handlers_key)
    local count = handlers.count
    for i = 1, count do
        if handlers[i] == handler then
            table_remove(handlers, i)
            count = count - 1
            handlers.count = count
            if count == 0 then
                local event_package_name = rawget(e, package_name_key)
                if event_package_name ~= package_name then
                    local remote_server =
                        channel_get(event_package_name, '__event')
                    local event_name = rawget(e, name_key)
                    local ok, result = remote_server:pcall(remote_unregister,
                                                           event_name,
                                                           package_name)
                    if not ok or not result then
                        error('failed to unregister remote event handler')
                    end
                end
            end
            return
        end
    end
end

unregister = function(e, handler)
    unregister_handler(e, handler, local_handlers_key)
end

client = function(e) return new_local_client(e) end

local write_error = function() error('cannot modify an event object') end

local make_metatable = function(index)
    local next_impl = function(_, k)
        local v
        k, v = next(index)
        if k ~= nil then return k, v end
    end

    local metatable = {
        __index = index,
        __newindex = write_error,
        -- __pairs = function(t)
        --     return next_impl, t, nil
        -- end,
        __tostring = function(e)
            return 'core.event:' .. rawget(e, package_name_key) .. ':' ..
                       rawget(e, name_key)
        end,
        __metatable = '__event'
    }

    return metatable
end

do
    local metatable = make_metatable({
        trigger = trigger,
        register = register,
        unregister = unregister,
        client = client
    })

    local generate_name
    do
        local count = 0
        local seed = package_name .. ':event:'
        generate_name = function()
            count = count + 1
            local time = os_time() + os_clock() % 1
            seed = string_format('%02X:%08X', count, hash(seed .. ':' .. time))
            return seed
        end
    end

    new_server = function(serializable)
        if serializable == nil then serializable = true end
        local name = generate_name()
        local e = setmetatable({
            [package_name_key] = package_name,
            [name_key] = name,
            [local_handlers_key] = {count = 0},
            [remote_handlers_key] = {count = 0},
            [server_key] = true,
            [serializable_key] = serializable,
            [remote_handlers_lookup_key] = setmetatable({}, {__mode = 'v'})
        }, metatable)
        servers[name] = e
        return e
    end
end

do
    local metatable = make_metatable({
        register = register,
        unregister = unregister,
        client = client
    })

    new_local_client = function(e)
        if not rawget(e, server_key) then return e end
        local event_package_name = rawget(e, package_name_key)
        local event_name = rawget(e, name_key)
        local event_local_handlers = rawget(e, local_handlers_key)
        local event_serializable = rawget(e, serializable_key)
        e = setmetatable({
            [package_name_key] = event_package_name,
            [name_key] = event_name,
            [local_handlers_key] = event_local_handlers,
            [server_key] = false,
            [serializable_key] = event_serializable
        }, metatable)
        return e
    end

    new_remote_client = function(remote_package_name, remote_event_name)
        if remote_package_name == package_name then
            local e = servers[remote_event_name]
            if e == nil then return nil end
            return new_local_client(e)
        end
        local name = remote_package_name .. ':' .. remote_event_name
        local e = remote_clients[name]
        if e == nil then
            e = setmetatable({
                [package_name_key] = remote_package_name,
                [name_key] = remote_event_name,
                [local_handlers_key] = {count = 0},
                [server_key] = false
            }, metatable)
            remote_clients[name] = e
        end
        return e
    end
end

local event = {
    new = new_server,
    client = client,
    trigger = trigger,
    register = register,
    unregister = unregister
}

local trigger_by_name = function(event_name, ...)
    local e = remote_clients[event_name]
    if not e then return false end
    return xpcall(trigger_handlers, save_stack, e, ...)
end

local register_by_name = function(event_name, remote_package_name)
    local server = servers[event_name]
    if server == nil then return false end
    local name = package_name .. ':' .. event_name
    coroutine.schedule(function()
        local remote_server = channel_get(remote_package_name, '__event')
        local remote_handler = function(...)
            local ok, result, message = remote_server:pcall(remote_trigger,
                                                            name, ...)
            if ok and not result then
                error_addon(remote_package_name, message)
            end
            return ok and result
        end
        register_handler(server, remote_handler, remote_handlers_key)
        rawget(server, remote_handlers_lookup_key)[remote_package_name] =
            remote_handler
    end)
    return true
end

local unregister_by_name = function(event_name, remote_package_name)
    local server = servers[event_name]
    if server == nil then return false end
    local fn = rawget(server, remote_handlers_lookup_key)[remote_package_name]
    local handlers = rawget(server, remote_handlers_key)
    for i = 1, #handlers do
        if handlers[i] == fn then
            table_remove(handlers, i)
            return true
        end
    end
    return false
end

remote_trigger = function(_, event_name, ...)
    return trigger_by_name(event_name, ...)
end

remote_register = function(_, event_name, remote_package_name)
    return register_by_name(event_name, remote_package_name)
end

remote_unregister = function(_, event_name, remote_package_name)
    return unregister_by_name(event_name, remote_package_name)
end

pin(channel.new('__event')).env = {
    trigger_by_name = trigger_by_name,
    register_by_name = register_by_name,
    unregister_by_name = unregister_by_name
}

serializer.register('__event', event, false)
serializer.register('__event.new', event.new, false)
serializer.register('__event.client', event.client, false)
serializer.register('__event.trigger', event.trigger, false)
serializer.register('__event.register', event.register, false)
serializer.register('__event.unregister', event.unregister, false)
serializer.register('__event.remote_register', remote_register, false)
serializer.register('__event.remote_unregister', remote_unregister, false)
serializer.register('__event.remote_trigger', remote_trigger, false)

local serialize = function(event)
    local serializable = rawget(event, serializable_key)
    if not serializable then
        error('attempt to serialize an unserializable event')
    end
    local event_package_name = rawget(event, package_name_key)
    local event_name = rawget(event, name_key)
    return {event_package_name, event_name}
end

local deserialize = function(name, class)
    return new_remote_client(name[1], name[2])
end

serializer.register_class('__event', serialize, deserialize)

return event
