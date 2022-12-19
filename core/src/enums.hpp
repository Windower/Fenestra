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

#ifndef WINDOWER_ENUMS_HPP
#define WINDOWER_ENUMS_HPP

#include <cstdint>

namespace windower
{
    enum class window_type : std::int32_t
    {
        borderless = 0,
        window = 1,
        full_screen = 2,
    };

    enum class texture_compression : std::int32_t
    {
        high = 0,
        low = 1,
        uncompressed = 2,
    };

    enum class environment_animation : std::int32_t
    {
        off = 0,
        normal = 1,
        smooth = 2,
    };

    enum class font_type : std::int32_t
    {
        compressed = 0,
        uncompressed = 1,
        high_quality = 2,
    };

    enum class mouse_button
    {
        left,
        right,
        middle,
    };
}

#endif
