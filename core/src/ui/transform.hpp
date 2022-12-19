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

#ifndef WINDOWER_UI_TRANSFORM_HPP
#define WINDOWER_UI_TRANSFORM_HPP

#include "ui/vector.hpp"

#include <gsl/gsl>

#include <array>
#include <cmath>
#include <optional>

namespace windower::ui
{

class transform : public std::array<std::array<float, 3>, 2>
{
public:
    static consteval transform identity() noexcept { return {}; }

    static constexpr transform translation(vector v) noexcept
    {
        return translation(v.x, v.y);
    }

    static constexpr transform translation(float x, float y) noexcept
    {
        return {1.f, 0.f, x, 0.f, 1.f, y};
    }

    static transform rotation(float theta, vector v = {}) noexcept
    {
        return rotation(theta, v.x, v.y);
    }

    static transform rotation(float theta, float x, float y) noexcept
    {
        auto const sin = std::sin(theta);
        auto const cos = std::cos(theta);

        return {cos, -sin, x - x * cos + y * sin,
                sin, cos,  y - x * sin - y * cos};
    }

    static constexpr transform scale(vector v = {}) noexcept
    {
        return scale(v.x, v.y);
    }

    static constexpr transform scale(float x, float y) noexcept
    {
        return {x, 0.f, 0.f, 0.f, y, 0.f};
    }

    static constexpr transform shear(float x, float y) noexcept
    {
        return {1.f, x, 0.f, y, 1.f, 0.f};
    }

    constexpr transform() noexcept : transform{1.f, 0.f, 0.f, 0.f, 1.f, 0.f} {}
    constexpr transform(
        float e00, float e01, float e02, //
        float e10, float e11, float e12) noexcept :
        std::array<std::array<float, 3>, 2>{
            std::array<float, 3>{e00, e01, e02},
            std::array<float, 3>{e10, e11, e12}}
    {}
};

constexpr float determinant(transform const& t) noexcept
{
    auto const a = gsl::at(gsl::at(t, 0), 0);
    auto const b = gsl::at(gsl::at(t, 0), 1);
    auto const c = gsl::at(gsl::at(t, 1), 0);
    auto const d = gsl::at(gsl::at(t, 1), 1);
    return a * d - b * c;
}

constexpr std::optional<transform> inverse(transform const& t) noexcept
{
    auto const det = determinant(t);
    if (det == 0)
    {
        return std::nullopt;
    }

    auto const a = gsl::at(gsl::at(t, 0), 0);
    auto const b = gsl::at(gsl::at(t, 0), 1);
    auto const c = gsl::at(gsl::at(t, 0), 2);
    auto const d = gsl::at(gsl::at(t, 1), 0);
    auto const e = gsl::at(gsl::at(t, 1), 1);
    auto const f = gsl::at(gsl::at(t, 1), 2);

    return transform{e / det,  -b / det, (b * f - c * e) / det,
                     -d / det, a / det,  (d * c - a * f) / det};
}

constexpr transform
scale_correction(transform const& t, vector const& scale) noexcept
{
    auto const a = gsl::at(gsl::at(t, 0), 0);
    auto const b = gsl::at(gsl::at(t, 0), 1);
    auto const c = gsl::at(gsl::at(t, 0), 2);
    auto const d = gsl::at(gsl::at(t, 1), 0);
    auto const e = gsl::at(gsl::at(t, 1), 1);
    auto const f = gsl::at(gsl::at(t, 1), 2);

    auto const x = scale.x;
    auto const y = scale.y;

    return transform{a, b * x / y, c * x, d * y / x, e, f * y};
}

constexpr transform
operator*(transform const& lhs, transform const& rhs) noexcept
{
    auto const a = gsl::at(gsl::at(lhs, 0), 0);
    auto const b = gsl::at(gsl::at(lhs, 0), 1);
    auto const c = gsl::at(gsl::at(lhs, 0), 2);
    auto const d = gsl::at(gsl::at(lhs, 1), 0);
    auto const e = gsl::at(gsl::at(lhs, 1), 1);
    auto const f = gsl::at(gsl::at(lhs, 1), 2);

    auto const g = gsl::at(gsl::at(rhs, 0), 0);
    auto const h = gsl::at(gsl::at(rhs, 0), 1);
    auto const i = gsl::at(gsl::at(rhs, 0), 2);
    auto const j = gsl::at(gsl::at(rhs, 1), 0);
    auto const k = gsl::at(gsl::at(rhs, 1), 1);
    auto const l = gsl::at(gsl::at(rhs, 1), 2);

    return {a * g + b * j, a * h + b * k, a * i + b * l + c,
            d * g + e * j, d * h + e * k, d * i + e * l + f};
}

constexpr vector operator*(transform const& t, vector const& v) noexcept
{
    auto const a = gsl::at(gsl::at(t, 0), 0);
    auto const b = gsl::at(gsl::at(t, 0), 1);
    auto const c = gsl::at(gsl::at(t, 0), 2);
    auto const d = gsl::at(gsl::at(t, 1), 0);
    auto const e = gsl::at(gsl::at(t, 1), 1);
    auto const f = gsl::at(gsl::at(t, 1), 2);

    return {a * v.x + b * v.y + c, d * v.x + e * v.y + f};
}

}

#endif
