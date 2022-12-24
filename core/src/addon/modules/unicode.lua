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

local bit = require('bit')
local ffi = require('ffi')
local string = require('string')

local serializer = require('core.serializer')

local ffi_copy = ffi.copy
local ffi_gc = ffi.gc
local ffi_istype = ffi.istype
local ffi_sizeof = ffi.sizeof
local ffi_string = ffi.string

local unicode = {}

local delete_u8string = ffi.new('void(*)(void const*)', (select(1, ...)))
local u8string_data = ffi.new('void const*(*)(void const*)', (select(2, ...)))
local u8string_length = ffi.new('size_t(*)(void const*)', (select(3, ...)))

local delete_wstring = ffi.new('void(*)(void const*)', (select(4, ...)))
local wstring_data = ffi.new('void const*(*)(void const*)', (select(5, ...)))
local wstring_length = ffi.new('size_t(*)(void const*)', (select(6, ...)))

local delete_sjis_string = ffi.new('void(*)(void const*)', (select(7, ...)))
local sjis_string_data = ffi.new('void const*(*)(void const*)', (select(8, ...)))
local sjis_string_length = ffi.new('size_t(*)(void const*)', (select(9, ...)))

local to_wstring_native = ffi.new('void const*(*)(char const*, size_t)', (select(10, ...)))
local from_wstring_native = ffi.new('void const*(*)(wchar_t const*, size_t)', (select(11, ...)))

local to_sjis_string_native = ffi.new('void const*(*)(char const*, size_t)', (select(12, ...)))
local from_sjis_string_native = ffi.new('void const*(*)(char const*, size_t)', (select(13, ...)))

local lookup_autotranslate = ffi.new('void const*(*)(uint32_t)', (select(14, ...)))

local wchar_array = ffi.typeof('wchar_t[?]')
local wchar_ptr = ffi.typeof('wchar_t*')

unicode.symbol = {
    element = {
        fire = '\u{E000}',
        ice = '\u{E001}',
        wind = '\u{E002}',
        earth = '\u{E003}',
        lightning = '\u{E004}',
        water = '\u{E005}',
        light = '\u{E006}',
        dark = '\u{E007}'
    },
    autotranslate = {start_mark = '\u{E008}', end_mark = '\u{E009}'}
}

unicode.to_utf16 = function(utf8_string)
    if type(utf8_string) ~= 'string' then
        error('bad argument #1 to \'to_utf16\' (string expected, got ' ..
                  type(utf8_string) .. ')', 2)
    end

    local utf16_ptr = ffi_gc(to_wstring_native(utf8_string, #utf8_string),
                             delete_wstring)
    local utf16_length = wstring_length(utf16_ptr)
    local result = wchar_array(utf16_length + 1)
    ffi_copy(result, wstring_data(utf16_ptr), utf16_length * 2)
    delete_wstring(ffi_gc(utf16_ptr, nil))
    return result, utf16_length
end

unicode.from_utf16 = function(utf16_string, utf16_length)
    if ffi_istype(utf16_string, wchar_array) then
        if utf16_length == nil then
            utf16_length = ffi_sizeof(utf16_string) / 2
        end
    elseif ffi_istype(utf16_string, wchar_ptr) then
        if utf16_length == nil then
            utf16_length = 0
            while utf16_string[utf16_length] ~= 0 do
                utf16_length = utf16_length + 1
            end
        end
    else
        error(
            'bad argument #1 to \'from_utf16\' (wchar_t[] or wchar_t* expected, got ' ..
                type(utf16_string) .. ')', 2)
    end

    if type(utf16_length) ~= 'number' then
        error('bad argument #2 to \'from_utf16\' (number expected, got ' ..
                  type(utf16_length) .. ')', 2)
    end

    local utf8_ptr = ffi_gc(from_wstring_native(utf16_string, utf16_length),
                            delete_u8string)
    local utf8_length = u8string_length(utf8_ptr)
    local result = ffi_string(u8string_data(utf8_ptr), utf8_length)
    delete_u8string(ffi_gc(utf8_ptr, nil))
    return result, utf8_length
end

unicode.to_shift_jis = function(utf8_string)
    if type(utf8_string) ~= 'string' then
        error('bad argument #1 to \'to_shift_jis\' (string expected, got ' ..
                  type(utf8_string) .. ')', 2)
    end

    local shift_jis_ptr = ffi_gc(to_sjis_string_native(utf8_string, #utf8_string),
                                 delete_u8string)
    local shift_jis_length = u8string_length(shift_jis_ptr)
    local result = ffi_string(u8string_data(shift_jis_ptr), shift_jis_length)
    delete_u8string(ffi_gc(shift_jis_ptr, nil))
    return result, shift_jis_length
end

unicode.from_shift_jis = function(shift_jis_string)
    if type(shift_jis_string) ~= 'string' then
        error('bad argument #1 to \'from_shift_jis\' (string expected, got ' ..
                  type(shift_jis_string) .. ')', 2)
    end

    local utf8_ptr = ffi_gc(from_sjis_string_native(shift_jis_string,
                                                  #shift_jis_string),
                            delete_u8string)
    local utf8_length = u8string_length(utf8_ptr)
    local result = ffi_string(u8string_data(utf8_ptr), utf8_length)
    delete_u8string(ffi_gc(utf8_ptr, nil))
    return result, utf8_length
end

unicode.length = function(string)
    if ffi_istype(string, wchar_array) then
        return ffi_sizeof(string) / 2
    elseif ffi_istype(string, wchar_ptr) then
        local length = 0
        while string[length] ~= 0 do length = length + 1 end
        return length
    elseif type(string) == 'string' then
        return #string
    else
        error(
            'bad argument #1 to \'length\' (wchar_t[], wchar_t* or string expected, got ' ..
                type(string) .. ')', 2)
    end
end

do
    local bit_band = bit.band
    local bit_bor = bit.bor
    local bit_lshift = bit.lshift
    local string_byte = string.byte
    local string_gsub = string.gsub

    local start_mark = unicode.symbol.autotranslate.start_mark
    local end_mark = unicode.symbol.autotranslate.end_mark

    unicode.expand_autotranslate = function(text, prefix, postfix)
        if type(text) ~= 'string' then
            error(
                'bad argument #1 to \'expand_autotranslate\' (string expected, got ' ..
                    type(text) .. ')', 2)
        end

        if prefix == nil then
            prefix = start_mark
        elseif type(prefix) ~= 'string' then
            error(
                'bad argument #2 to \'expand_autotranslate\' (string expected, got ' ..
                    type(prefix) .. ')', 2)
        end

        if postfix == nil then
            postfix = end_mark
        elseif type(postfix) ~= 'string' then
            error(
                'bad argument #3 to \'expand_autotranslate\' (string expected, got ' ..
                    type(postfix) .. ')', 2)
        end

        return (string_gsub(text, '[\xF3\xF4][\x80-\xBF][\x80-\xBF][\x80-\xBD]',
                            function(utf8)
            local code_point = bit_bor(bit_lshift(
                                           bit_band(string_byte(utf8, 1), 0x07),
                                           18), bit_lshift(
                                           bit_band(string_byte(utf8, 2), 0x3F),
                                           12), bit_lshift(
                                           bit_band(string_byte(utf8, 3), 0x3F),
                                           06),
                                       bit_band(string_byte(utf8, 4), 0x3F))
            if code_point >= 0xF0000 and code_point <= 0x10FFFF then
                local ptr = ffi_gc(lookup_autotranslate(code_point),
                                   delete_u8string)
                local length = u8string_length(ptr)
                if length > 0 then
                    return prefix .. ffi_string(u8string_data(ptr), length) ..
                               postfix
                end
            end
            return utf8
        end))
    end
end

serializer.register('__unicode', unicode, false)
serializer.register('__unicode.to_utf16', unicode.to_utf16)
serializer.register('__unicode.from_utf16', unicode.from_utf16)
serializer.register('__unicode.to_shift_jis', unicode.to_shift_jis)
serializer.register('__unicode.from_shift_jis', unicode.from_shift_jis)
serializer.register('__unicode.length', unicode.length)
serializer.register('__unicode.expand_autotranslate',
                    unicode.expand_autotranslate)
serializer.register('__unicode.symbol', unicode.symbol, false)
serializer.register('__unicode.symbol.element', unicode.symbol.element, false)
serializer.register('__unicode.symbol.autotranslate',
                    unicode.symbol.autotranslate, false)

return unicode
