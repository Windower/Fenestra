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

#ifndef WINDOWER_UI_VECTOR_HPP
#define WINDOWER_UI_VECTOR_HPP

#include <gsl/gsl>

#include <cmath>
#include <compare>

namespace windower::ui
{

struct vector
{
    float x = 0.f;
    float y = 0.f;

    constexpr vector() noexcept = default;
    constexpr vector(float x, float y) noexcept : x{x}, y{y} {}

    constexpr bool operator==(vector const&) const noexcept = default;
};

constexpr vector operator+(vector const& value) noexcept
{
    return {+value.x, +value.y};
}

constexpr vector operator-(vector const& value) noexcept
{
    return {-value.x, -value.y};
}

constexpr vector operator+(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

constexpr vector operator-(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

constexpr vector operator*(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x * rhs.x, lhs.y * rhs.y};
}

constexpr vector operator*(vector const& lhs, float rhs) noexcept
{
    return {lhs.x * rhs, lhs.y * rhs};
}

constexpr vector operator*(float lhs, vector const& rhs) noexcept
{
    return {lhs * rhs.x, lhs * rhs.y};
}

constexpr vector operator/(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x / rhs.x, lhs.y / rhs.y};
}

constexpr vector operator/(vector const& lhs, float rhs) noexcept
{
    return {lhs.x / rhs, lhs.y / rhs};
}

constexpr vector operator/(float lhs, vector const& rhs) noexcept
{
    return {lhs / rhs.x, lhs / rhs.y};
}

constexpr vector& operator+=(vector& lhs, vector const& rhs) noexcept
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

constexpr vector& operator-=(vector& lhs, vector const& rhs) noexcept
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

constexpr vector& operator*=(vector& lhs, float rhs) noexcept
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

constexpr vector& operator/=(vector& lhs, float rhs) noexcept
{
    lhs.x /= rhs;
    lhs.y /= rhs;
    return lhs;
}

}

#endif
