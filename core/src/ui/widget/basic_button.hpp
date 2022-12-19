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

#ifndef WINDOWER_UI_WIDGET_BASIC_BUTTON_HPP
#define WINDOWER_UI_WIDGET_BASIC_BUTTON_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/mouse_button.hpp"
#include "ui/vector.hpp"

#include <cstdint>

namespace windower::ui::widget
{

class button_state
{
public:
    bool hot     = false;
    bool active  = false;
    bool pressed = false;
    bool clicked = false;
    vector drag_offset;
    vector drag_position;
    std::uint64_t repeat_count = 0;
    mouse_button button        = mouse_button::left;

    constexpr operator bool() const noexcept
    {
        return clicked && button == mouse_button::left;
    }
};

button_state basic_button(context& ctx, id id) noexcept;

}

#endif
