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

#ifndef WINDOWER_UI_WIDGET_SCROLL_PANEL_HPP
#define WINDOWER_UI_WIDGET_SCROLL_PANEL_HPP

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/id.hpp"
#include "ui/vector.hpp"

#include <cstdint>

namespace windower::ui::widget
{

enum class scroll_bar_visibility : std::uint8_t
{
    hidden    = 0,
    visible   = 1,
    automatic = 2,
};

class scroll_panel_state
{
public:
    dimension canvas_size;
    scroll_bar_visibility visibility_horizontal =
        scroll_bar_visibility::automatic;
    scroll_bar_visibility visibility_vertical =
        scroll_bar_visibility::automatic;
    float line_height = 16;
    vector offset;
};

void begin_scroll_panel(
    context& ctx, id id, scroll_panel_state& state) noexcept;
void end_scroll_panel(context& ctx) noexcept;

}

#endif
