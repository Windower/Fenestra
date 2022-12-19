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

#ifndef WINDOWER_UI_WIDGET_PROGRESS_HPP
#define WINDOWER_UI_WIDGET_PROGRESS_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/direction.hpp"

#include <span>

namespace windower::ui::widget
{

class progress_entry
{
public:
    float value;
    float max;
    color color;
};

void progress(
    context& ctx, float value, float max = 1.f,
    direction direction = direction::left_to_right) noexcept;

void progress(
    context& ctx, float value, float max, color color,
    direction direction = direction::left_to_right) noexcept;

void progress(
    context& ctx, std::span<progress_entry const> entries,
    direction direction = direction::left_to_right) noexcept;

}

#endif
