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

#include "ui/widget/check.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/patch.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{
namespace
{

constexpr auto off_normal   = patch{{11, 79, 25, 93}};
constexpr auto off_hot      = patch{{27, 79, 41, 93}};
constexpr auto off_active   = patch{{43, 79, 57, 93}};
constexpr auto off_disabled = patch{{59, 79, 73, 93}};

constexpr auto on_normal   = patch{{11, 95, 25, 109}};
constexpr auto on_hot      = patch{{27, 95, 41, 109}};
constexpr auto on_active   = patch{{43, 95, 57, 109}};
constexpr auto on_disabled = patch{{59, 95, 73, 109}};

constexpr auto tri_normal   = patch{{11, 111, 25, 125}};
constexpr auto tri_hot      = patch{{27, 111, 41, 125}};
constexpr auto tri_active   = patch{{43, 111, 57, 125}};
constexpr auto tri_disabled = patch{{59, 111, 73, 125}};

}

button_state check(
    context& ctx, id id, std::u8string_view text,
    std::optional<bool> checked) noexcept
{
    auto const enabled = ctx.enabled();

    auto const state = basic_button(ctx, id);

    auto const& bounds      = ctx.bounds();
    auto const position     = bounds.position();
    auto const text_bounds  = expand(contract(bounds, {16, 0, 0, 0}), {0, 3});
    auto const check_bounds = rectangle{position, position + vector{12, 12}};

    primitive::text(
        ctx, text_bounds, text,
        {.fill_color = ctx.system_color(
             enabled ? system_color::label : system_color::label_disabled)});

    primitive::set_texture(ctx, ctx.skin());
    if (!ctx.enabled())
    {
        primitive::rectangle(
            ctx, check_bounds,
            checked ? *checked ? on_disabled : off_disabled : tri_disabled);
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        if (state.hot)
        {
            primitive::rectangle(
                ctx, check_bounds,
                checked ? *checked ? on_active : off_active : tri_active);
        }
        else
        {
            primitive::rectangle(
                ctx, check_bounds,
                checked ? *checked ? on_hot : off_hot : tri_hot);
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(
            ctx, check_bounds, checked ? *checked ? on_hot : off_hot : tri_hot);
    }
    else
    {
        primitive::rectangle(
            ctx, check_bounds,
            checked ? *checked ? on_normal : off_normal : tri_normal);
    }

    return state;
}

}
