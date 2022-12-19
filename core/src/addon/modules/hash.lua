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
local string = require('string')

local serializer = require('core.serializer')

local band = bit.band
local bor = bit.bor
local bxor = bit.bxor
local lshift = bit.lshift
local rol = bit.rol
local rshift = bit.rshift
local tobit = bit.tobit

local byte = string.byte

local hash
do
    local mul = function(a, b)
        return band(a, 0xFFFF) * b + lshift(rshift(a, 16) * b, 16)
    end

    hash = function(value, seed)
        if type(value) ~= 'string' then value = tostring(value) end

        if seed == nil then
            seed = 0
        elseif type(seed) ~= 'number' then
            error('bad argument #2 to \'hash\' (number expected, got ' ..
                      type(seed) .. ')')
        end

        local h = tobit(seed)

        local length = #value
        for i = 1, length - 3, 4 do
            local a, b, c, d = byte(value, i, i + 3)
            local block = bor(a, lshift(b, 8), lshift(c, 16), lshift(d, 24))
            h = bxor(h, mul(rol(mul(block, 0xCC9E2D51), 15), 0x1B873593))
            h = mul(rol(h, 13), 5) + 0xE6546B64
        end

        local tail = length % 4
        if tail > 0 then
            local a, b, c = byte(value, length - tail + 1, length)
            local block = bor(a, lshift(b or 0, 8), lshift(c or 0, 16))
            h = bxor(h, mul(rol(mul(block, 0xCC9E2D51), 15), 0x1B873593))
        end

        h = bxor(h, length)
        h = mul(bxor(h, rshift(h, 16)), 0x85EBCA6B)
        h = mul(bxor(h, rshift(h, 13)), 0xC2B2AE35)
        h = bxor(h, rshift(h, 16))
        if h < 0 then h = h + 0x100000000 end
        return h
    end
end

serializer.register('__hash', hash)

return hash
