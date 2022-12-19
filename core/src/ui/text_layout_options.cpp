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

#include "ui/text_layout_options.hpp"

#include <gsl/gsl>

namespace windower::ui
{

std::strong_ordering strong_order(
    text_layout_options const& lhs, text_layout_options const& rhs) noexcept
{
    if (auto const result = lhs.alignment <=> rhs.alignment; result != 0)
    {
        return result;
    }

    if (auto const result = lhs.vertical_alignment <=> rhs.vertical_alignment;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.word_wrapping <=> rhs.word_wrapping;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.trimming_string <=> rhs.trimming_string;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.trimming_delimiter <=> rhs.trimming_delimiter;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            lhs.trimming_delimiter_count <=> rhs.trimming_delimiter_count;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            lhs.trimming_granularity <=> rhs.trimming_granularity;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.left, rhs.padding.left);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = std::strong_order(lhs.padding.top, rhs.padding.top);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.right, rhs.padding.right);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.bottom, rhs.padding.bottom);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    return lhs.underline <=> rhs.underline;
}

}
