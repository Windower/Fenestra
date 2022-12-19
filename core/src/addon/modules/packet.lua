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
    trigger_key,
    inject_incoming_native_ptr,
    inject_outgoing_native_ptr = ...
-- LuaFormatter on

local bit = require('bit')
local ffi = require('ffi')
local string = require('string')

local event = require('core.event')
local serializer = require('core.serializer')
local windower = require('core.windower')

local error = error
local next = next
local setmetatable = setmetatable
local type = type
local string_len = string.len

local event_trigger = event.trigger
local package_name = windower.package_name or '<script>'

-- LuaFormatter off
local inject_incoming_native = ffi.cast(
    'void(*)(uint16_t, char const*, size_t, char const*, size_t)',
    inject_incoming_native_ptr)
local inject_outgoing_native = ffi.cast(
    'void(*)(uint16_t, char const*, size_t, char const*, size_t)',
    inject_outgoing_native_ptr)
-- LuaFormatter on

local incoming_event = event.new(false)
local outgoing_event = event.new(false)

local blocked_key = {}

local new_impl
do
    local sequence_counter_key = {}
    local timestamp_key = {}
    local original_id_key = {}
    local original_size_key = {}
    local original_data_key = {}
    local injected_by_key = {}

    local keys = {
        id = 'id',
        size = function(p) return string_len(rawget(p, 'data')) end,
        sequence_counter = sequence_counter_key,
        timestamp = timestamp_key,
        data = 'data',
        original_id = original_id_key,
        original_size = original_size_key,
        original_data = original_data_key,
        blocked = blocked_key,
        modified = function(p)
            return rawget(p, 'id') ~= rawget(p, original_id_key) or
                       rawget(p, 'data') ~= rawget(p, original_data_key)
        end,
        injected = function(p) return rawget(p, injected_by_key) ~= '' end,
        injected_by = injected_by_key
    }

    local next_impl = function(p, k)
        local v
        k, v = next(keys, k)
        if type(v) == 'function' then return k, v(p) end
        return k, rawget(p, v)
    end

    local metatable = {
        __index = function(p, k)
            local key = keys[k]
            if type(key) == 'function' then return key(p) end
            return rawget(p, key)
        end,
        __newindex = function(p, k, v)
            if k == 'blocked' then
                if v ~= true then
                    error('cannot set field \'blocked\' to value ' ..
                              tostring(v))
                end
                rawset(p, blocked_key, true)
                return
            end

            if keys[k] then
                error('cannot modify field \'' .. tostring(k) ..
                          '\' on packet object')
            end

            error('cannot add keys to a packet object')
        end,
        __pairs = function(p) return next_impl, p, nil end,
        __tostring = function(_) return 'core.packet' end,
        __metatable = '__packet'
    }

    new_impl = function(original_id, original_data, id, data, sequence_counter,
                        timestamp, blocked, injected_by)
        return setmetatable({
            ['id'] = id,
            [sequence_counter_key] = sequence_counter,
            [timestamp_key] = timestamp,
            ['data'] = data,
            [original_id_key] = original_id,
            [original_size_key] = string_len(original_data or ''),
            [original_data_key] = original_data,
            [blocked_key] = blocked,
            [injected_by_key] = injected_by
        }, metatable)
    end
end

local new = function(id, data)
    return new_impl(nil, nil, id, data, nil, nil, false, '')
end

local verify_packet
do
    local band = bit.band
    local bnot = bit.bnot
    verify_packet = function(packet, function_name)
        if type(packet) ~= 'table' then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (packet expected, got ' .. type(packet) .. ')')
        end

        local blocked = rawget(packet, blocked_key)
        if blocked == nil then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (packet expected, got table)')
        end

        local id = rawget(packet, 'id')
        if type(id) ~= 'number' then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet id; number expected, got' .. type(id) ..
                      ')')
        elseif id <= 0 then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet id; expected a positive integer, got' ..
                      id .. ')')
        elseif id >= 512 then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet id; expected an integer less than 512, got' ..
                      id .. ')')
        elseif (id + 2 ^ 52) - 2 ^ 52 ~= id then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet id; expected an integer, got' .. id ..
                      ')')
        end

        local data = rawget(packet, 'data')
        if type(data) ~= 'string' then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet data; string expected, got' ..
                      type(data) .. ')')
        end

        local size = string_len(data)
        if size >= 509 then
            error('bad argument #1 to \'' .. function_name ..
                      '\' (invalid packet data; expected a size less than 509, got' ..
                      size .. ')')
        end

        local padded_size = band(size + 3, bnot(3))
        data = data .. '\0\0\0'

        return blocked, id, data, padded_size
    end
end

local inject_incoming = function(packet)
    local blocked, id, data, size = verify_packet(packet, 'inject_incoming')
    if blocked then return end
    inject_incoming_native(id, data, size, package_name, #package_name)
end

local inject_outgoing = function(packet)
    local blocked, id, data, size = verify_packet(packet, 'inject_outgoing')
    if blocked then return end
    inject_outgoing_native(id, data, size, package_name, #package_name)
end

local trigger = function(incoming, original_id, original_data, id, data,
                         sequence_counter, timestamp, blocked, injected_by)
    local packet_object = new_impl(original_id, original_data, id, data,
                                   sequence_counter, timestamp, blocked,
                                   injected_by)
    event_trigger(incoming and incoming_event or outgoing_event, packet_object)
    if rawget(packet_object, blocked_key) then return true end
    local result_id = rawget(packet_object, 'id')
    local result_data = rawget(packet_object, 'data')
    if result_id == id and result_data == data then return nil end
    return result_id, result_data
end

local packet = {
    new = new,
    inject_incoming = inject_incoming,
    inject_outgoing = inject_outgoing,
    incoming = incoming_event:client(),
    outgoing = outgoing_event:client()
}

rawset(registry, trigger_key, trigger)

serializer.register('__packet', packet, false)
serializer.register('__packet.new', packet.new, false)
serializer.register('__packet.inject_incoming', packet.inject_incoming, false)
serializer.register('__packet.inject_outgoing', packet.inject_outgoing, false)
serializer.register('__packet.incoming', packet.incoming, false)
serializer.register('__packet.outgoing', packet.outgoing, false)

serializer.register_class('__packet', serializer.disable, serializer.disable)

return packet
