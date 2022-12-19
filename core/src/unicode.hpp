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

#ifndef WINDOWER_UNICODE_BASE_HPP
#define WINDOWER_UNICODE_BASE_HPP

#include <gsl/gsl>

#include <string>
#include <string_view>

namespace windower
{

enum class client_language : std::uint8_t
{
    japanese = 1,
    english  = 2,
};

using sjis_char = unsigned char;

using sjis_string      = std::basic_string<sjis_char>;
using sjis_string_view = std::basic_string_view<sjis_char>;

std::u8string to_u8string(std::wstring_view str) noexcept;
std::u8string to_u8string(sjis_string_view str) noexcept;
std::wstring to_wstring(std::u8string_view str) noexcept;
std::wstring to_wstring(sjis_string_view str) noexcept;
sjis_string to_sjis_string(
    std::u8string_view str,
    client_language client_language = client_language::english) noexcept;
sjis_string to_sjis_string(
    std::wstring_view str,
    client_language client_language = client_language::english) noexcept;

char32_t next_code_point(std::u8string_view str, std::size_t& offset) noexcept;
char32_t next_code_point(std::wstring_view str, std::size_t& offset) noexcept;
char32_t next_code_point(sjis_string_view str, std::size_t& offset) noexcept;

void append(std::u8string& str, char32_t code_point) noexcept;
void append(std::wstring& str, char32_t code_point) noexcept;
void append(
    sjis_string& str, char32_t code_point,
    client_language client_language = client_language::english) noexcept;

std::u8string nfkc_fold_case(std::u8string_view) noexcept;

std::uint32_t to_autotranslate_id(
    char32_t code_point,
    client_language client_language = client_language::english) noexcept;

bool is_whitespace(char32_t code_point) noexcept;

constexpr bool is_autotranslate(char32_t code_point) noexcept
{
    return code_point >= U'\U000F0000' && code_point <= U'\U000FFFFD' ||
           code_point >= U'\U00100000' && code_point <= U'\U0010FFFD';
}

}

#endif
