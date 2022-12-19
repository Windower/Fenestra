/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hooklib/x86.hpp"

#include <gsl/gsl>

#include <bit>
#include <cstdint>
#include <iterator>
#include <stdexcept>

namespace
{

auto octal_split(gsl::not_null<std::uint8_t*>& it) noexcept
{
    std::uint8_t a = *it >> 6 & 3;
    std::uint8_t b = *it >> 3 & 7;
    std::uint8_t c = *it >> 0 & 7;

    it = std::next(it.get());

    return std::make_tuple(a, b, c);
}

std::uint8_t
decode_mod_sib(gsl::not_null<std::uint8_t*>& it, bool address_override) noexcept
{
    std::uint8_t mod;
    std::uint8_t reg;
    std::uint8_t r_m;

    std::tie(mod, reg, r_m) = octal_split(it);
    if (address_override)
    {
        if ((mod == 0 && r_m == 6) || mod == 2)
        {
            it = std::next(it.get(), 2);
        }
        else if (mod == 1)
        {
            it = std::next(it.get());
        }
    }
    else
    {
        if ((mod == 0 && r_m == 5) || mod == 2)
        {
            it = std::next(it.get(), 4);
        }
        else if (mod == 1)
        {
            it = std::next(it.get());
        }

        if (mod != 3 && r_m == 4)
        {
            it = std::next(it.get());
        }
    }

    return reg;
}

void decode_prefix(
    gsl::not_null<std::uint8_t*>& it, bool& operand_override,
    bool& address_override) noexcept
{
    while (true)
    {
        switch (*it)
        {
        default: return;

        case 0046:
        case 0056:
        case 0066:
        case 0076:
        case 0144:
        case 0145:
        case 0360:
        case 0362:
        case 0363: break;

        case 0146: operand_override = true; break;
        case 0147: address_override = true; break;
        }

        it = std::next(it.get());
    }
}

void decode_opcode_1(
    gsl::not_null<std::uint8_t*>& it, bool operand_override,
    bool address_override) noexcept
{
    std::uint8_t a;
    std::uint8_t b;
    std::uint8_t c;
    std::tie(a, b, c) = octal_split(it);
    switch (a)
    {
    default: break;
    case 0:
        switch (c)
        {
        default: break;
        case 0:
        case 1:
        case 2:
        case 3: decode_mod_sib(it, address_override); break;

        case 4: it = std::next(it.get()); break;

        case 5: it = std::next(it.get(), operand_override ? 2 : 4); break;
        }
        break;

    case 1:
        switch (b)
        {
        default: break;
        case 4:
            switch (c)
            {
            default: break;
            case 2:
            case 3: decode_mod_sib(it, address_override); break;
            }
            break;

        case 5:
            switch (c)
            {
            default: break;
            case 1: decode_mod_sib(it, address_override); [[fallthrough]];
            case 0: it = std::next(it.get(), operand_override ? 2 : 4); break;

            case 3: decode_mod_sib(it, address_override); [[fallthrough]];
            case 2: it = std::next(it.get()); break;
            }
            break;

        case 6:
        case 7: it = std::next(it.get()); break;
        }
        break;

    case 2:
        switch (b)
        {
        default: break;
        case 0:
            decode_mod_sib(it, address_override);
            switch (c)
            {
            default: break;
            case 0:
            case 2:
            case 3: it = std::next(it.get()); break;

            case 1: it = std::next(it.get(), operand_override ? 2 : 4); break;
            }
            break;
        case 1: decode_mod_sib(it, address_override); break;
        case 3:
            switch (c)
            {
            default: break;
            case 2: it = std::next(it.get(), address_override ? 4 : 6); break;
            }
            break;
        case 4:
            switch (c)
            {
            default: break;
            case 0:
            case 1:
            case 2:
            case 3: it = std::next(it.get(), 4); break;
            }
            break;
        case 5:
            switch (c)
            {
            default: break;
            case 0: it = std::next(it.get()); break;
            case 1: it = std::next(it.get(), operand_override ? 2 : 4); break;
            }
            break;
        case 6: it = std::next(it.get()); break;
        case 7: it = std::next(it.get(), operand_override ? 2 : 4); break;
        }
        break;

    case 3:
        switch (b << 3 | c)
        {
        default: break;
        case 000:
        case 001:
        case 006:
            decode_mod_sib(it, address_override);
            it = std::next(it.get());
            break;

        case 002:
        case 012: it = std::next(it.get(), 2); break;

        case 004:
        case 005:
        case 020:
        case 021:
        case 022:
        case 023:
        case 030:
        case 031:
        case 032:
        case 033:
        case 034:
        case 035:
        case 036:
        case 037:
        case 076:
        case 077: decode_mod_sib(it, address_override); break;

        case 007:
            decode_mod_sib(it, address_override);
            it = std::next(it.get(), operand_override ? 2 : 4);
            break;

        case 010: it = std::next(it.get(), 3); break;

        case 015:
        case 024:
        case 025:
        case 040:
        case 041:
        case 042:
        case 043:
        case 044:
        case 045:
        case 046:
        case 047:
        case 053: it = std::next(it.get()); break;

        case 050:
        case 051:
        case 052: it = std::next(it.get(), operand_override ? 2 : 4); break;

        case 066:
            switch (decode_mod_sib(it, address_override))
            {
            default: break;
            case 0:
            case 1: it = std::next(it.get());
            }
            break;

        case 067:
            switch (decode_mod_sib(it, address_override))
            {
            default: break;
            case 0:
            case 1: it = std::next(it.get(), operand_override ? 2 : 4);
            }
            break;
        }
        break;
    }
}

void decode_opcode_2(gsl::not_null<std::uint8_t*>&, bool, bool)
{
    throw std::runtime_error{"2 byte opcodes are not yet implemented"};
}

void decode_opcode_3(gsl::not_null<std::uint8_t*>&, bool, bool)
{
    throw std::runtime_error{"3 byte opcodes are not yet implemented"};
}

void decode_opcode_4(gsl::not_null<std::uint8_t*>&, bool, bool)
{
    throw std::runtime_error{"3 byte opcodes are not yet implemented"};
}

void decode(gsl::not_null<std::uint8_t*>& it)
{
    auto operand_override = false;
    auto address_override = false;

    decode_prefix(it, operand_override, address_override);

    switch (*it)
    {
    default: decode_opcode_1(it, operand_override, address_override); break;

    case 0017:
        it = std::next(it.get());
        switch (*it)
        {
        default: decode_opcode_2(it, operand_override, address_override); break;

        case 0070:
            it = std::next(it.get());
            decode_opcode_3(it, operand_override, address_override);
            break;

        case 0072:
            it = std::next(it.get());
            decode_opcode_4(it, operand_override, address_override);
            break;
        }
        break;
    }
}

}

windower::hooklib::x86::jump::jump(void* target)
{
    auto offset = std::bit_cast<std::intptr_t>(target) -
                  std::bit_cast<std::intptr_t>(m_raw.data()) - m_raw.size();
    gsl::at(m_raw, 0) = 0xE9;
    gsl::at(m_raw, 1) = offset >> 0x00 & 0xFF;
    gsl::at(m_raw, 2) = offset >> 0x08 & 0xFF;
    gsl::at(m_raw, 3) = offset >> 0x10 & 0xFF;
    gsl::at(m_raw, 4) = offset >> 0x18 & 0xFF;
}

static_assert(sizeof(windower::hooklib::x86::jump) == 5, "sizeof(jump) != 5");

windower::hooklib::x86::thiscall_thunk::thiscall_thunk(
    void* this_ptr, void* target) :
    m_jump{target}
{
    auto const ptr_value = std::bit_cast<std::intptr_t>(this_ptr);

    gsl::at(m_raw, 0) = 0xBA;
    gsl::at(m_raw, 1) = ptr_value >> 0x00 & 0xFF;
    gsl::at(m_raw, 2) = ptr_value >> 0x08 & 0xFF;
    gsl::at(m_raw, 3) = ptr_value >> 0x10 & 0xFF;
    gsl::at(m_raw, 4) = ptr_value >> 0x18 & 0xFF;
}

static_assert(
    sizeof(windower::hooklib::x86::thiscall_thunk) == 10,
    "sizeof(thiscall_thunk) != 10");

gsl::not_null<std::uint8_t*>
windower::hooklib::x86::next_instruction(gsl::not_null<std::uint8_t*> code_ptr)
{
    auto max = std::next(code_ptr.get(), max_instruction_size + 1);
    decode(code_ptr);
    if (code_ptr == max)
    {
        throw std::runtime_error{"invalid x86 opcode"};
    }
    return code_ptr;
}

bool windower::hooklib::x86::is_hotpatchable(
    gsl::not_null<std::uint8_t*> code_ptr)
{
    auto begin = code_ptr.get();

    return std::distance(begin, next_instruction(begin).get()) == 2 &&
           std::find_if(
               std::prev(begin, sizeof(jump)), begin,
               [](std::uint8_t x) { return x != 0x90; }) == begin;
}

gsl::not_null<std::uint8_t*> windower::hooklib::x86::follow_jumps(
    gsl::not_null<std::uint8_t*> code_ptr) noexcept
{
    while (true)
    {
        switch (*code_ptr & 0xFF)
        {
        case 0xE9: {
            auto const offset = *std::next(code_ptr.get(), 1) |
                                *std::next(code_ptr.get(), 2) << 8 |
                                *std::next(code_ptr.get(), 3) << 16 |
                                *std::next(code_ptr.get(), 4) << 24;
            code_ptr = std::next(code_ptr.get(), 5 + offset);
            break;
        }

        case 0xEB: {
            auto const offset = *std::next(code_ptr.get(), 1);
            code_ptr          = std::next(code_ptr.get(), 2 + offset);
            break;
        }

        default: return code_ptr;
        }
    }
}
