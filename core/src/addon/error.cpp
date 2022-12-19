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

#include "addon/error.hpp"

#include "addon/lua.hpp"
#include "addon/unsafe.hpp"
#include "utility.hpp"

#include <lua.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

windower::lua::error::error(state s) :
    std::runtime_error{windower::to_string(lua::get<std::u8string>(s, -1))},
    m_stack_trace{unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(std::string const& message, state s) :
    std::runtime_error{message}, m_stack_trace{
                                     unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(char const* message, state s) :
    std::runtime_error{message}, m_stack_trace{
                                     unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(std::string const& message) :
    std::runtime_error{message}
{}

windower::lua::error::error(char const* message) : std::runtime_error{message}
{}

bool windower::lua::error::has_stack_trace() const noexcept
{
    return m_stack_trace != nullptr;
}

std::vector<windower::lua::stack_frame> const&
windower::lua::error::stack_trace() const noexcept
{
    return *m_stack_trace;
}
