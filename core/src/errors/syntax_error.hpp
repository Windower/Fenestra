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

#ifndef WINDOWER_ERRORS_SYNTAX_ERROR_HPP
#define WINDOWER_ERRORS_SYNTAX_ERROR_HPP

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{

class syntax_error : public windower_error
{
public:
    syntax_error(std::u8string_view error_code);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index, std::size_t begin_index, std::size_t end_index);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index, std::size_t begin_index, std::size_t end_index,
        std::vector<std::u8string> options);

    std::u8string const& source() const noexcept;
    std::size_t mark_index() const noexcept;
    std::size_t begin_index() const noexcept;
    std::size_t end_index() const noexcept;
    std::vector<std::u8string> const& options() const noexcept;

private:
    std::shared_ptr<std::u8string const> m_source;
    std::shared_ptr<std::vector<std::u8string> const> m_options;
    std::size_t m_mark_index;
    std::size_t m_begin_index;
    std::size_t m_end_index;
};

}

#endif
