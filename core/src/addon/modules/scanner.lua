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

local ffi = require('ffi')

local serializer = require('core.serializer')

local error = error
local type = type

local args = {...}

-- LuaFormatter off
local scan_native = ffi.new(
    'void*(*)(char const*,size_t,char const*,size_t)',
    args[1])
-- LuaFormatter on

local scan = function(signature, module)
    if type(signature) ~= 'string' then
        error('bad argument #1 to \'scan\' (string expected, got ' ..
                  type(signature) .. ')', 2)
    end
    if module == nil then
        module = 'ffximain.dll'
    elseif type(module) ~= 'string' then
        error('bad argument #2 to \'scan\' (string expected, got ' ..
                  type(module) .. ')', 2)
    end

    local result = scan_native(module, #module, signature, #signature)
    if result == nil then -- coalesce nullptr to nil
        result = nil
    end
    return result
end

local scanner = {scan = scan}

serializer.register('__scanner', scanner, false)
serializer.register('__scanner.scan', scanner.scan)

return scanner
