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

#ifndef WINDOWER_UI_SIZE_HPP
#define WINDOWER_UI_SIZE_HPP

#include "ui/vector.hpp"

#include <limits>

namespace windower::ui
{

class dimension
{
public:
    static constexpr float unbounded = std::numeric_limits<float>::infinity();

    float width  = 0.f;
    float height = 0.f;

    constexpr dimension() noexcept = default;

    constexpr dimension(float width, float height) noexcept :
        width{width}, height{height}
    {}

    constexpr explicit dimension(vector const& vector) noexcept :
        dimension{vector.x, vector.y}
    {}

    constexpr bool operator==(dimension const&) const noexcept = default;

    constexpr explicit operator vector() const noexcept
    {
        return {width, height};
    }
};

constexpr std::strong_ordering
strong_order(dimension const& lhs, dimension const& rhs) noexcept
{
    if (auto const result = std::strong_order(lhs.width, rhs.width);
        result != std::strong_ordering::equal)
    {
        return result;
    }
    return std::strong_order(lhs.height, rhs.height);
}

}

#endif
