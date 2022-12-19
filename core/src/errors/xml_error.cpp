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

#include "errors/xml_error.hpp"

#include <memory>
#include <string>
#include <string_view>

windower::xml_error::xml_error(std::u8string_view description) :
    xml_error{description, {}, {}, 0, 0}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::filesystem::path path) :
    xml_error{description, std::move(path), {}, 0, 0}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::u8string_view line_source,
    std::size_t line, std::size_t column) :
    xml_error{description, {}, line_source, line, column}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::filesystem::path path,
    std::u8string_view line_source, std::size_t line, std::size_t column) :
    syntax_error{u8"XML", line_source, column},
    m_path{std::make_shared<std::filesystem::path>(std::move(path))},
    m_description{std::make_shared<std::u8string>(description)}, m_line{line}
{}

std::filesystem::path const& windower::xml_error::path() const noexcept
{
    return *m_path;
}

std::u8string const& windower::xml_error::description() const noexcept
{
    return *m_description;
}

std::size_t windower::xml_error::line() const noexcept { return m_line; }
