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

#ifndef WINDOWER_UI_THICKNESS_HPP
#define WINDOWER_UI_THICKNESS_HPP

namespace windower::ui
{

class thickness
{
public:
    float left   = 0.f;
    float top    = 0.f;
    float right  = 0.f;
    float bottom = 0.f;

    constexpr thickness() noexcept = default;

    constexpr thickness(float uniform) noexcept :
        top{uniform}, right{uniform}, bottom{uniform}, left{uniform}
    {}

    constexpr thickness(float left_right, float top_bottom) noexcept :
        left{left_right}, top{top_bottom}, right{left_right}, bottom{top_bottom}
    {}

    constexpr thickness(float left, float top_bottom, float right) noexcept :
        left{left}, top{top_bottom}, right{right}, bottom{top_bottom}
    {}

    constexpr thickness(
        float left, float top, float right, float bottom) noexcept :
        left{left},
        top{top}, right{right}, bottom{bottom}
    {}
};

class h_thickness
{
public:
    float left  = 0.f;
    float right = 0.f;

    constexpr h_thickness() noexcept = default;

    constexpr h_thickness(float uniform) noexcept :
        left{uniform}, right{uniform}
    {}

    constexpr h_thickness(float left, float right) noexcept :
        left{left}, right{right}
    {}

    constexpr operator thickness() const noexcept
    {
        return {left, 0, right, 0};
    }
};

class v_thickness
{
public:
    float top    = 0.f;
    float bottom = 0.f;

    constexpr v_thickness() noexcept = default;

    constexpr v_thickness(float uniform) noexcept :
        top{uniform}, bottom{uniform}
    {}

    constexpr v_thickness(float top, float bottom) noexcept :
        top{top}, bottom{bottom}
    {}

    constexpr operator thickness() const noexcept
    {
        return {0, top, 0, bottom};
    }
};

}

#endif
