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

#include "errors/syntax_error.hpp"

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>

windower::syntax_error::syntax_error(std::u8string_view error_code) :
    syntax_error{error_code, u8"", 0, 0, 0}
{}

windower::syntax_error::syntax_error(
    std::u8string_view error_code, std::u8string_view source,
    std::size_t mark_index) :
    syntax_error{error_code, source, mark_index, mark_index, mark_index}
{}

windower::syntax_error::syntax_error(
    std::u8string_view error_code, std::u8string_view source,
    std::size_t mark_index, std::size_t begin_index, std::size_t end_index) :
    windower_error{error_code},
    m_source{std::make_shared<std::u8string>(source)},
    m_options{std::make_shared<std::vector<std::u8string>>()},
    m_mark_index{mark_index}, m_begin_index{begin_index}, m_end_index{end_index}
{}

windower::syntax_error::syntax_error(
    std::u8string_view error_code, std::u8string_view source,
    std::size_t mark_index, std::size_t begin_index, std::size_t end_index,
    std::vector<std::u8string> options) :
    windower_error{error_code},
    m_source{std::make_shared<std::u8string>(source)},
    m_options{std::make_shared<std::vector<std::u8string>>(std::move(options))},
    m_mark_index{mark_index}, m_begin_index{begin_index}, m_end_index{end_index}
{}

std::u8string const& windower::syntax_error::source() const noexcept
{
    return *m_source;
}

std::size_t windower::syntax_error::mark_index() const noexcept
{
    return m_mark_index;
}

std::size_t windower::syntax_error::begin_index() const noexcept
{
    return m_begin_index;
}

std::size_t windower::syntax_error::end_index() const noexcept
{
    return m_end_index;
}

std::vector<std::u8string> const&
windower::syntax_error::options() const noexcept
{
    return *m_options;
}
