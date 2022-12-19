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

#ifndef WINDOWER_GEOMETRY_HPP
#define WINDOWER_GEOMETRY_HPP

#include <algorithm>
#include <cstdint>

namespace windower
{
    struct point
    {
        std::int32_t x;
        std::int32_t y;
    };

    struct dimension
    {
        std::int32_t width;
        std::int32_t height;
    };

    struct rectangle
    {
        point location;
        dimension size;
    };

    constexpr rectangle intersect(rectangle lhs, rectangle rhs)
    {
        auto l = std::max(lhs.location.x, rhs.location.x);
        auto t = std::max(lhs.location.y, rhs.location.y);
        auto r = std::min(lhs.location.x + lhs.size.width, rhs.location.x + rhs.size.width);
        auto b = std::min(lhs.location.y + lhs.size.height, rhs.location.y + rhs.size.height);
        return {{l, t}, {r <= l ? 0 : r - l, b <= t ? 0 : b - t}};
    }
}

#endif
