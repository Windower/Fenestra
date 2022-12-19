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

#ifndef WINDOWER_UI_PATCH_HPP
#define WINDOWER_UI_PATCH_HPP

#include "ui/dimension.hpp"
#include "ui/rectangle.hpp"
#include "ui/thickness.hpp"

namespace windower::ui
{

class patch
{
public:
    dimension texture_size;
    rectangle bounds;
    thickness overdraw;

    constexpr patch() noexcept = default;

    constexpr patch(
        rectangle const& bounds, thickness const& overdraw = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, overdraw{overdraw}
    {}
};

class nine_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    thickness slice;
    thickness overdraw;

    constexpr nine_patch() noexcept = default;

    constexpr nine_patch(
        rectangle const& bounds, thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

class h_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    h_thickness slice;
    thickness overdraw;

    constexpr h_patch() noexcept = default;

    constexpr h_patch(
        rectangle const& bounds, h_thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

class v_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    v_thickness slice;
    thickness overdraw;

    constexpr v_patch() noexcept = default;

    constexpr v_patch(
        rectangle const& bounds, v_thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

}

#endif
