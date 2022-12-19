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

#ifndef WINDOWER_HOOKLIB_X86_HPP
#define WINDOWER_HOOKLIB_X86_HPP

#include <gsl/gsl>

#include <array>
#include <cstddef>
#include <cstdint>

namespace windower::hooklib
{

class x86
{
public:
    class jump
    {
    public:
        jump(void*);

    private:
        std::array<std::uint8_t, 5> m_raw;
    };

    class thiscall_thunk
    {
    public:
        thiscall_thunk(void*, void*);

    private:
        std::array<std::uint8_t, 5> m_raw;
        jump m_jump;
    };

    static constexpr std::size_t max_instruction_size         = 15;
    static constexpr std::uint8_t nop_instruction             = 0x90;
    static constexpr std::uint8_t trap_instruction            = 0xCC;
    static unsigned short int const hotpatch_jump_instruction = 0xF9EB;

    static gsl::not_null<std::uint8_t*>
        follow_jumps(gsl::not_null<std::uint8_t*>) noexcept;
    static gsl::not_null<std::uint8_t*>
        next_instruction(gsl::not_null<std::uint8_t*>);
    static bool is_hotpatchable(gsl::not_null<std::uint8_t*>);
};

}

#endif
