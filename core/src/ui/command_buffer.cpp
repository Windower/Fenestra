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

#include "ui/command_buffer.hpp"

#include <d3d8.h>

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <cstdint>

namespace windower::ui
{

void command_buffer::wrapped_command::execute(
    ::IDirect3DDevice8* d3d_device) const noexcept
{
    m_execute(m_data, d3d_device);
}

std::uintptr_t command_buffer::state(state_id id) const noexcept
{
    return gsl::at(m_state, gsl::narrow_cast<std::size_t>(id));
}

bool command_buffer::has_state(state_id id, std::uintptr_t value) const noexcept
{
    return state(id) == value;
}

bool command_buffer::has_state(state_id id, void const* value) const noexcept
{
    return has_state(id, std::bit_cast<std::uintptr_t>(value));
}

void command_buffer::state(state_id id, std::uintptr_t value) noexcept
{
    gsl::at(m_state, gsl::narrow_cast<std::size_t>(id)) = value;
}

void command_buffer::state(state_id id, void const* value) noexcept
{
    state(id, std::bit_cast<std::uintptr_t>(value));
}

void command_buffer::execute(::IDirect3DDevice8* device) const noexcept
{
    for (auto const& c : m_commands)
    {
        c.execute(device);
    }
}

void command_buffer::clear() noexcept
{
    namespace range = std::ranges;

    m_commands.clear();
    range::fill(m_state, 0);
}

}
