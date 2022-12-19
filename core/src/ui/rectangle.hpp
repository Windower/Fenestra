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

#ifndef WINDOWER_UI_RECTANGLE_HPP
#define WINDOWER_UI_RECTANGLE_HPP

#include "ui/dimension.hpp"
#include "ui/thickness.hpp"
#include "ui/vector.hpp"

#include <algorithm>

namespace windower::ui
{

class rectangle
{
public:
    float x0 = 0.f;
    float y0 = 0.f;
    float x1 = 0.f;
    float y1 = 0.f;

    constexpr rectangle() noexcept = default;

    constexpr rectangle(float x0, float y0, float x1, float y1) noexcept :
        x0{x0}, y0{y0}, x1{x1}, y1{y1}
    {}

    constexpr rectangle(vector const& p1, vector const& p2) noexcept :
        x0{p1.x}, y0{p1.y}, x1{p2.x}, y1{p2.y}
    {}

    constexpr bool operator==(rectangle const&) const noexcept = default;

    constexpr vector position() const noexcept { return {x0, y0}; }
    constexpr void position(vector const& position) noexcept
    {
        auto const w = x1 - x0;
        auto const h = y1 - y0;
        x0           = position.x;
        y0           = position.y;
        width(w);
        height(h);
    }

    constexpr dimension size() const noexcept { return {x1 - x0, y1 - y0}; }
    constexpr void size(dimension const& size) noexcept
    {
        width(size.width);
        height(size.height);
    }

    constexpr float width() const noexcept { return x1 - x0; }
    constexpr void width(float value) noexcept { x1 = x0 + value; }
    void max_width(float max) noexcept { width(std::min(width(), max)); }
    void min_width(float min) noexcept { width(std::max(width(), min)); }
    void clamp_width(float min, float max) noexcept
    {
        width(std::clamp(width(), min, max));
    }

    constexpr float height() const noexcept { return y1 - y0; }
    constexpr void height(float value) noexcept { y1 = y0 + value; }
    void max_height(float max) noexcept { height(std::min(height(), max)); }
    void min_height(float min) noexcept { height(std::max(height(), min)); }
    void clamp_height(float min, float max) noexcept
    {
        height(std::clamp(height(), min, max));
    }
};

constexpr bool is_inside(vector const& p, rectangle const& r) noexcept
{
    return p.x >= r.x0 && p.y >= r.y0 && p.x < r.x1 && p.y < r.y1;
}

constexpr rectangle
intersection(rectangle const& a, rectangle const& b) noexcept
{
    auto const x0 = std::max(a.x0, b.x0);
    auto const y0 = std::max(a.y0, b.y0);
    auto const x1 = std::min(a.x1, b.x1);
    auto const y1 = std::min(a.y1, b.y1);
    return rectangle{x0, y0, x1, y1};
}

constexpr rectangle expand(rectangle const& r, thickness const& d) noexcept
{
    return {r.x0 - d.left, r.y0 - d.top, r.x1 + d.right, r.y1 + d.bottom};
}

constexpr rectangle contract(rectangle const& r, thickness const& d) noexcept
{
    return {r.x0 + d.left, r.y0 + d.top, r.x1 - d.right, r.y1 - d.bottom};
}

}

#endif
