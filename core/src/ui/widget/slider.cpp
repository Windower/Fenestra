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

#include "ui/widget/slider.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"
#include "ui/widget/image_button.hpp"
#include "utility.hpp"

namespace windower::ui::widget
{
namespace
{

constexpr auto thumb_normal   = patch{{11, 159, 25, 173}};
constexpr auto thumb_hot      = patch{{27, 159, 41, 173}};
constexpr auto thumb_disabled = patch{{43, 159, 57, 173}};

constexpr auto h_track_enabled  = h_patch{{59, 159, 65, 165}, {3}, {-3}};
constexpr auto h_track_disabled = h_patch{{67, 159, 73, 165}, {3}, {-3}};
constexpr auto h_fill_enabled   = h_patch{{59, 167, 65, 173}, {3}, {-3}};
constexpr auto h_fill_disabled  = h_patch{{67, 167, 73, 173}, {3}, {-3}};

constexpr auto v_track_enabled  = v_patch{{59, 159, 65, 165}, {3}, {-3}};
constexpr auto v_track_disabled = v_patch{{67, 159, 73, 165}, {3}, {-3}};
constexpr auto v_fill_enabled   = v_patch{{59, 167, 65, 173}, {3}, {-3}};
constexpr auto v_fill_disabled  = v_patch{{67, 167, 73, 173}, {3}, {-3}};

float slider_h(
    context& ctx, id id, float value, float min, float max,
    color fill_color) noexcept
{
    static constexpr auto thumb_size = 12.f;

    auto true_min = std::min(min, max);
    auto true_max = std::max(min, max);

    value = std::clamp(value, true_min, true_max);

    auto bounds = ctx.bounds();
    bounds.x1   = bounds.x0 + std::max(bounds.width(), 12.f);
    bounds.y1   = bounds.y0 + 12;

    auto enabled        = ctx.enabled();
    auto force_disabled = false;

    if (min == max && enabled)
    {
        ctx.push_enabled(false);
        force_disabled = true;
        enabled        = false;
    }

    auto const x0 = bounds.x0;
    auto const x3 = bounds.x1;

    auto const x1 = map_value(value, min, max, x0, x3 - thumb_size);
    auto const x2 = x1 + thumb_size;

    auto const y0 = bounds.y0;
    auto const y1 = bounds.y1;

    ctx.bounds({x0, y0, x1, y1});
    auto const dn_track_state = basic_button(ctx, id.part(1));
    if (dn_track_state.pressed)
    {
        value -= (max - min) * .1f;
    }

    ctx.bounds({x2, y0, x3, y1});
    auto const up_track_state = basic_button(ctx, id.part(2));
    if (up_track_state.pressed)
    {
        value += (max - min) * .1f;
    }

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(
        ctx, {x0, y0, x3, y1}, enabled ? h_track_enabled : h_track_disabled);
    primitive::rectangle(
        ctx, min > max ? rectangle{x1, y0, x3, y1} : rectangle{x0, y0, x2, y1},
        enabled ? h_fill_enabled : h_fill_disabled,
        enabled ? fill_color : colors::white);

    ctx.bounds({x1, y0, x2, y1});
    auto const thumb_state = basic_button(ctx, id.part(3));
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), thumb_disabled);
    }
    else if (thumb_state.active && thumb_state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), thumb_hot);
        auto const x = thumb_state.drag_position.x;
        value        = map_value(x, x0, x3 - thumb_size, min, max);
    }
    else if (thumb_state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), thumb_hot);
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), thumb_normal);
    }

    if (force_disabled)
    {
        ctx.pop_enabled();
    }

    return std::clamp(value, true_min, true_max);
}

float slider_v(
    context& ctx, id id, float value, float min, float max,
    color fill_color) noexcept
{
    static constexpr auto thumb_size = 12.f;

    auto true_min = std::min(min, max);
    auto true_max = std::max(min, max);

    value = std::clamp(value, true_min, true_max);

    auto bounds = ctx.bounds();
    bounds.x1   = bounds.x0 + 12;
    bounds.y1   = bounds.y0 + std::max(bounds.height(), 12.f);

    auto enabled        = ctx.enabled();
    auto force_disabled = false;

    if (min == max && enabled)
    {
        ctx.push_enabled(false);
        force_disabled = true;
        enabled        = false;
    }

    auto const x0 = bounds.x0;
    auto const x1 = bounds.x1;

    auto const y0 = bounds.y0;
    auto const y3 = bounds.y1;

    auto const y1 = map_value(value, min, max, y3 - thumb_size, y0);
    auto const y2 = y1 + thumb_size;

    ctx.bounds({x0, y2, x1, y3});
    auto const dn_track_state = basic_button(ctx, id.part(1));
    if (dn_track_state.pressed)
    {
        value -= (max - min) * .1f;
    }

    ctx.bounds({x0, y0, x1, y1});
    auto const up_track_state = basic_button(ctx, id.part(2));
    if (up_track_state.pressed)
    {
        value += (max - min) * .1f;
    }

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(
        ctx, {x0, y0, x1, y3}, enabled ? v_track_enabled : v_track_disabled);
    primitive::rectangle(
        ctx, min > max ? rectangle{x0, y0, x1, y2} : rectangle{x0, y1, x1, y3},
        enabled ? v_fill_enabled : v_fill_disabled,
        enabled ? fill_color : colors::white);

    ctx.bounds({x0, y1, x1, y2});
    auto const thumb_state = basic_button(ctx, id.part(3));
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), thumb_disabled);
    }
    else if (thumb_state.active && thumb_state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), thumb_hot);
        auto const y = thumb_state.drag_position.y;
        value        = map_value(y, y3 - thumb_size, y0, min, max);
    }
    else if (thumb_state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), thumb_hot);
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), thumb_normal);
    }

    if (force_disabled)
    {
        ctx.pop_enabled();
    }

    return std::clamp(value, true_min, true_max);
}

}

float slider(
    context& ctx, id id, float value, float min, float max,
    direction direction) noexcept
{
    return slider(
        ctx, id, value, min, max, ctx.system_color(system_color::accent),
        direction);
}

float slider(
    context& ctx, id id, float value, float min, float max, color fill_color,
    direction direction) noexcept
{
    switch (direction)
    {
    case direction::left_to_right:
        return slider_h(ctx, id, value, min, max, fill_color);
    case direction::right_to_left:
        return slider_h(ctx, id, value, max, min, fill_color);
    case direction::bottom_to_top:
        return slider_v(ctx, id, value, min, max, fill_color);
    case direction::top_to_bottom:
        return slider_v(ctx, id, value, max, min, fill_color);
    default: fail_fast();
    }
}

}
