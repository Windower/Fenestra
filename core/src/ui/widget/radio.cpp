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

#include "ui/widget/radio.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/patch.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{

namespace
{

constexpr auto off_normal   = patch{{11, 127, 25, 141}};
constexpr auto off_hot      = patch{{27, 127, 41, 141}};
constexpr auto off_active   = patch{{43, 127, 57, 141}};
constexpr auto off_disabled = patch{{59, 127, 73, 141}};

constexpr auto on_normal   = patch{{11, 143, 25, 157}};
constexpr auto on_hot      = patch{{27, 143, 41, 157}};
constexpr auto on_active   = patch{{43, 143, 57, 157}};
constexpr auto on_disabled = patch{{59, 143, 73, 157}};

}

button_state
radio(context& ctx, id id, std::u8string_view text, bool checked) noexcept
{
    auto const enabled = ctx.enabled();

    auto const state = basic_button(ctx, id);

    auto const& bounds      = ctx.bounds();
    auto const position     = bounds.position();
    auto const text_bounds  = expand(contract(bounds, {16, 0, 0, 0}), {0, 3});
    auto const check_bounds = rectangle{position, position + vector{12, 12}};

    primitive::set_texture(ctx, ctx.skin());
    if (!ctx.enabled())
    {
        primitive::rectangle(
            ctx, check_bounds, checked ? on_disabled : off_disabled);
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        if (state.hot)
        {
            primitive::rectangle(
                ctx, check_bounds, checked ? on_active : off_active);
        }
        else
        {
            primitive::rectangle(ctx, check_bounds, checked ? on_hot : off_hot);
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, check_bounds, checked ? on_hot : off_hot);
    }
    else
    {
        primitive::rectangle(
            ctx, check_bounds, checked ? on_normal : off_normal);
    }

    primitive::text(
        ctx, text_bounds, text,
        {.fill_color = ctx.system_color(
             enabled ? system_color::label : system_color::label_disabled)});

    return state;
}

}
