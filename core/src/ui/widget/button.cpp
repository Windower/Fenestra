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

#include "ui/widget/button.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{
namespace
{

constexpr auto off_normal = nine_patch{{11, 51, 25, 77}, {4}};
constexpr auto off_hot    = nine_patch{{27, 51, 41, 77}, {4}};
constexpr auto on_normal  = nine_patch{{43, 51, 57, 77}, {4}};
constexpr auto on_hot     = nine_patch{{59, 51, 73, 77}, {4}};
constexpr auto active     = nine_patch{{75, 51, 89, 77}, {4}};
constexpr auto disabled   = nine_patch{{91, 51, 105, 77}, {4}};

}

button_state
button(context& ctx, id id, std::u8string_view text, bool checked) noexcept
{
    auto const enabled = ctx.enabled();

    auto const state = basic_button(ctx, id);

    primitive::set_texture(ctx, ctx.skin());
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), disabled);
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        if (state.hot)
        {
            primitive::rectangle(ctx, ctx.bounds(), active);
        }
        else
        {
            primitive::rectangle(ctx, ctx.bounds(), checked ? on_hot : off_hot);
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), checked ? on_hot : off_hot);
    }
    else
    {
        primitive::rectangle(
            ctx, ctx.bounds(), checked ? on_normal : off_normal);
    }

    auto const text_bounds = contract(ctx.bounds(), {1});
    primitive::text(
        ctx,
        checked || (state.active && state.hot)
            ? contract(ctx.bounds(), {1, 2, 1, 0})
            : contract(ctx.bounds(), {1}),
        text,
        {.fill_color = enabled
                           ? ctx.system_color(system_color::button)
                           : ctx.system_color(system_color::button_disabled),
         .flags      = text_rasterization_flags::clip_to_bounds},
        {.alignment          = text_alignment::center,
         .vertical_alignment = text_vertical_alignment::middle,
         .padding            = {1, 1, 1, 1}});

    return state;
}

}
