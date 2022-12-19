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

#ifndef WINDOWER_ERRORS_XML_ERROR_HPP
#define WINDOWER_ERRORS_XML_ERROR_HPP

#include "errors/syntax_error.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace windower
{

class xml_error : public syntax_error
{
public:
    xml_error(std::u8string_view description);
    xml_error(std::u8string_view description, std::filesystem::path path);
    xml_error(
        std::u8string_view description, std::u8string_view line_source,
        std::size_t line, std::size_t column);
    xml_error(
        std::u8string_view description, std::filesystem::path path,
        std::u8string_view line_source, std::size_t line, std::size_t column);

    std::filesystem::path const& path() const noexcept;
    std::u8string const& description() const noexcept;
    std::size_t line() const noexcept;

private:
    std::shared_ptr<std::filesystem::path> m_path;
    std::shared_ptr<std::u8string> m_description;
    std::size_t m_line;
};

}

#endif
