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

#ifndef WINDOWER_UI_MARKDOWN_HPP
#define WINDOWER_UI_MARKDOWN_HPP

#include "ui/color.hpp"

#include <dwrite.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace windower::ui
{

enum class size_unit
{
    absolute,
    relative,
    pixel,
    point,
    pica,
};

struct format
{};

struct typeface
{
    std::wstring value;
};

struct font_size
{
    size_unit unit;
    float value;
};

struct stretch
{
    ::DWRITE_FONT_STRETCH value;
};

struct weight
{
    ::DWRITE_FONT_WEIGHT value;
};

struct style
{
    ::DWRITE_FONT_STYLE value;
};

struct strikethrough
{
    bool value;
};

struct underline
{
    bool value;
};

struct stroke
{
    std::optional<font_size> width;
    std::optional<color> color;
};

using option = std::variant<
    format, typeface, font_size, stretch, weight, style, strikethrough,
    underline, color, stroke>;

struct fragment
{
    std::size_t start;
    std::size_t length;
    option data;
};

std::vector<fragment> parse_markdown(std::wstring_view text) noexcept;

}

#endif
