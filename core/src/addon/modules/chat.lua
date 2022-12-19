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
    trigger_text_added_key,
    add_text_native_ptr = ...
-- LuaFormatter on

local ffi = require('ffi')

local event = require('core.event')
local serializer = require('core.serializer')

local rawget = rawget
local next = next
local type = type
local error = error
local setmetatable = setmetatable

local event_trigger = event.trigger

local add_text_native = ffi.cast('void(*)(char const*, size_t, uint8_t, bool)',
                                 add_text_native_ptr)

local text_added_event = event.new(false)

local trigger_text_added
do
    local original_text_key = {}
    local original_type_key = {}
    local original_indented_key = {}
    local blocked_key = {}

    local keys = {
        text = 'text',
        type = 'type',
        indented = 'indented',
        original_text = original_text_key,
        original_type = original_type_key,
        original_indented = original_indented_key,
        modified = function(t)
            return rawget(t, 'text') ~= rawget(t, original_text_key) or
                       rawget(t, 'type') ~= rawget(t, original_type_key) or
                       rawget(t, 'indented') ~= rawget(t, original_indented_key)
        end,
        blocked = blocked_key
    }

    local next_impl = function(t, k)
        local v
        k, v = next(keys, k)
        if type(v) == 'function' then return k, v(t) end
        return k, rawget(t, v)
    end

    local metatable = {
        __index = function(t, k)
            local key = keys[k]
            if type(key) == 'function' then return key(t) end
            return rawget(t, key)
        end,
        __newindex = function(t, k, v)
            if k == 'blocked' then
                if v ~= true then
                    error('cannot set field \'blocked\' to value ' ..
                              tostring(v))
                end
                rawset(t, blocked_key, true)
                return
            end

            if keys[k] then
                error('cannot modify field \'' .. tostring(k) ..
                          '\' on chat text object')
            end

            error('cannot add keys to a chat text object')
        end,
        __pairs = function(t) return next_impl, t, nil end,
        __tostring = function(_) return 'core.chat.text' end,
        __metatable = '__chat.text'
    }

    trigger_text_added = function(text, type, indented, original_text,
                                  original_type, original_indented, blocked)
        local text_object = setmetatable({
            text = text,
            type = type,
            indented = indented,
            [original_text_key] = original_text,
            [original_type_key] = original_type,
            [original_indented_key] = original_indented,
            [blocked_key] = blocked
        }, metatable)
        event_trigger(text_added_event, text_object)
        if rawget(text_object, blocked_key) then return true end
        local result_text = rawget(text_object, 'text')
        local result_type = rawget(text_object, 'type')
        local result_indented = rawget(text_object, 'indented')
        if result_text == text and result_type == type and result_indented ==
            indented then return nil end
        return result_text, result_type, result_indented
    end
end

local add_text = function(text, type_id, indented)
    if type(text) ~= 'string' then
        error('bad argument #1 to \'add_text\' (string expected, got ' ..
                  type(text) .. ')')
    end

    if type_id == nil then
        type_id = 206
    elseif type(type_id) ~= 'number' then
        error('bad argument #2 to \'add_text\' (number expected, got ' ..
                  type(type_id) .. ')')
    end

    if indented == nil then
        indented = false
    elseif type(indented) ~= 'boolean' then
        error('bad argument #3 to \'add_text\' (boolean expected, got ' ..
                  type(indented) .. ')')
    end

    add_text_native(text, #text, type_id, indented)
end

local chat = {add_text = add_text, text_added = text_added_event:client()}

rawset(registry, trigger_text_added_key, trigger_text_added)

serializer.register('__chat', chat, false)
serializer.register('__chat.add_text', chat.add_text, false)
serializer.register('__chat.text_added', chat.text_added, false)

serializer.register_class('__chat.text', serializer.disable, serializer.disable)

return chat
