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

#include "ui/widget/scroll_panel.hpp"

#include "ui/direction.hpp"
#include "ui/primitives.hpp"
#include "ui/transform.hpp"
#include "ui/widget/image_button.hpp"
#include "utility.hpp"

namespace windower::ui::widget
{
namespace
{

constexpr auto h_down_button = image_button_descriptor{
    u8":skin",
    {{11, 191, 24, 205}, {1, 1, 0, 1}},
    {{11, 207, 24, 221}, {1, 1, 0, 1}},
    {{11, 223, 24, 237}, {1, 1, 0, 1}},
    {{11, 239, 24, 253}, {1, 1, 0, 1}}};
constexpr auto h_up_button = image_button_descriptor{
    u8":skin",
    {{60, 191, 73, 205}, {0, 1, 1, 1}},
    {{60, 207, 73, 221}, {0, 1, 1, 1}},
    {{60, 223, 73, 237}, {0, 1, 1, 1}},
    {{60, 239, 73, 253}, {0, 1, 1, 1}}};

constexpr auto h_thumb_normal = h_patch{{28, 191, 56, 205}, {3}, {0, 1}};
constexpr auto h_thumb_hot    = h_patch{{28, 207, 56, 221}, {3}, {0, 1}};
constexpr auto h_thumb_active = h_patch{{28, 223, 56, 237}, {3}, {0, 1}};
constexpr auto h_thumb_off    = h_patch{{28, 239, 56, 253}, {3}, {0, 1}};

constexpr auto h_track     = patch{{26, 191, 26, 205}, {0, 1}};
constexpr auto h_track_off = patch{{26, 239, 26, 253}, {0, 1}};

constexpr auto v_down_button = image_button_descriptor{
    u8":skin",
    {{75, 191, 89, 204}, {1, 1, 1, 0}},
    {{91, 191, 105, 204}, {1, 1, 1, 0}},
    {{107, 191, 121, 204}, {1, 1, 1, 0}},
    {{123, 191, 137, 204}, {1, 1, 1, 0}}};
constexpr auto v_up_button = image_button_descriptor{
    u8":skin",
    {{75, 240, 89, 253}, {1, 0, 1, 1}},
    {{91, 240, 105, 253}, {1, 0, 1, 1}},
    {{107, 240, 121, 253}, {1, 0, 1, 1}},
    {{123, 240, 137, 253}, {1, 0, 1, 1}}};

constexpr auto v_thumb_normal = v_patch{{75, 208, 89, 236}, {3}, {1, 0}};
constexpr auto v_thumb_hot    = v_patch{{91, 208, 105, 236}, {3}, {1, 0}};
constexpr auto v_thumb_active = v_patch{{107, 208, 121, 236}, {3}, {1, 0}};
constexpr auto v_thumb_off    = v_patch{{123, 208, 137, 236}, {3}, {1, 0}};

constexpr auto v_track     = patch{{75, 206, 89, 206}, {1, 0}};
constexpr auto v_track_off = patch{{123, 206, 137, 206}, {1, 0}};

float scroll_bar_h(
    context& ctx, id id, float value, float min, float max,
    float thumb_size) noexcept
{
    using std::swap;

    static constexpr auto min_thumb_width = 8.f;

    if (min > max)
    {
        swap(min, max);
    }

    auto enabled        = ctx.enabled();
    auto force_disabled = false;

    thumb_size = std::clamp(thumb_size, 0.f, max - min);
    value      = std::clamp(value, min, max - thumb_size);
    if (value == min && value + thumb_size >= max)
    {
        ctx.push_enabled(false);
        force_disabled = true;
        enabled        = false;
    }

    auto bounds = ctx.bounds();
    bounds.x1   = bounds.x0 + std::max(bounds.width(), 35.f);
    bounds.y1   = bounds.y0 + 12;

    auto const track_width = bounds.width() - 26;
    auto const thumb_width = track_width * thumb_size / (max - min);
    auto const padding     = std::max(0.f, min_thumb_width - thumb_width);

    auto const x0 = bounds.x0;
    auto const x5 = bounds.x1;

    auto const x1 = x0 + 12;
    auto const x2 = map_value(value, min, max, x0 + 13, x5 - padding - 13);
    auto const x3 = x2 + thumb_width + padding;
    auto const x4 = x5 - 12;

    auto const y0 = bounds.y0;
    auto const y1 = bounds.y1;

    ctx.bounds({x0, y0, x1, y1});
    auto const down_state = image_button(ctx, id.part(1), h_down_button);
    if (down_state.pressed)
    {
        value -= 1;
    }

    ctx.bounds({x4, y0, x5, y1});
    auto const up_state = image_button(ctx, id.part(2), h_up_button);
    if (up_state.pressed)
    {
        value += 1;
    }

    ctx.bounds({x1, y0, x2, y1});
    auto const down_track_state = basic_button(ctx, id.part(3));
    if (down_track_state.pressed)
    {
        value -= std::max(thumb_size, 1.f);
    }

    ctx.bounds({x3, y0, x4, y1});
    auto const up_track_state = basic_button(ctx, id.part(4));
    if (up_track_state.pressed)
    {
        value += std::max(thumb_size, 1.f);
    }

    primitive::set_texture(ctx, ctx.skin());
    ctx.bounds({x2, y0, x3, y1});
    auto const thumb_state = basic_button(ctx, id.part(5));
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), h_thumb_off);
    }
    else if (thumb_state.active && thumb_state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), h_thumb_active);
        auto const x = thumb_state.drag_position.x;
        value        = map_value(x, x0 + 13, x5 - padding - 13, min, max);
    }
    else if (thumb_state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), h_thumb_hot);
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), h_thumb_normal);
    }

    if (enabled)
    {
        primitive::rectangle(ctx, {x1, y0, x2, y1}, h_track);
        primitive::rectangle(ctx, {x3, y0, x4, y1}, h_track);
    }
    else
    {
        primitive::rectangle(ctx, {x1, y0, x2, y1}, h_track_off);
        primitive::rectangle(ctx, {x3, y0, x4, y1}, h_track_off);
    }

    if (force_disabled)
    {
        ctx.pop_enabled();
    }

    return std::clamp(value, min, max - thumb_size);
}

float scroll_bar_v(
    context& ctx, id id, float value, float min, float max,
    float thumb_size) noexcept
{
    using std::swap;

    static constexpr auto min_thumb_width = 8.f;

    if (min > max)
    {
        swap(min, max);
    }

    auto enabled        = ctx.enabled();
    auto force_disabled = false;

    thumb_size = std::clamp(thumb_size, 0.f, max - min);
    value      = std::clamp(value, min, max - thumb_size);
    if (value == min && value + thumb_size >= max && enabled)
    {
        ctx.push_enabled(false);
        force_disabled = true;
        enabled        = false;
    }

    auto bounds = ctx.bounds();
    bounds.x1   = bounds.x0 + 12;
    bounds.y1   = bounds.y0 + std::max(bounds.height(), 35.f);

    auto const track_width = bounds.height() - 26;
    auto const thumb_width = track_width * thumb_size / (max - min);
    auto const padding     = std::max(0.f, min_thumb_width - thumb_width);

    auto const x0 = bounds.x0;
    auto const x1 = bounds.x1;

    auto const y0 = bounds.y0;
    auto const y5 = bounds.y1;

    auto const y1 = y0 + 12;
    auto const y2 = map_value(value, min, max, y0 + 13, y5 - padding - 13);
    auto const y3 = y2 + thumb_width + padding;
    auto const y4 = y5 - 12;

    ctx.bounds({x0, y0, x1, y1});
    auto const down_state = image_button(ctx, id.part(6), v_down_button);
    if (down_state.pressed)
    {
        value -= 1;
    }

    ctx.bounds({x0, y4, x1, y5});
    auto const up_state = image_button(ctx, id.part(7), v_up_button);
    if (up_state.pressed)
    {
        value += 1;
    }

    ctx.bounds({x0, y1, x1, y2});
    auto const down_track_state = basic_button(ctx, id.part(8));
    if (down_track_state.pressed)
    {
        value -= std::max(thumb_size, 1.f);
    }

    ctx.bounds({x0, y3, x1, y4});
    auto const up_track_state = basic_button(ctx, id.part(9));
    if (up_track_state.pressed)
    {
        value += std::max(thumb_size, 1.f);
    }

    primitive::set_texture(ctx, ctx.skin());
    ctx.bounds({x0, y2, x1, y3});
    auto const thumb_state = basic_button(ctx, id.part(10));
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), v_thumb_off);
    }
    else if (thumb_state.active && thumb_state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), v_thumb_active);
        auto const y = thumb_state.drag_position.y;
        value        = map_value(y, y0 + 13, y5 - padding - 13, min, max);
    }
    else if (thumb_state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(ctx, ctx.bounds(), v_thumb_hot);
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), v_thumb_normal);
    }

    if (enabled)
    {
        primitive::rectangle(ctx, {x0, y1, x1, y2}, v_track);
        primitive::rectangle(ctx, {x0, y3, x1, y4}, v_track);
    }
    else
    {
        primitive::rectangle(ctx, {x0, y1, x1, y2}, v_track_off);
        primitive::rectangle(ctx, {x0, y3, x1, y4}, v_track_off);
    }

    if (force_disabled)
    {
        ctx.pop_enabled();
    }

    return std::clamp(value, min, max - thumb_size);
}

}

void begin_scroll_panel(context& ctx, id id, scroll_panel_state& state) noexcept
{
    static constexpr auto scroll_bar_size = 12.f;

    auto visibility_vertical   = state.visibility_vertical;
    auto visibility_horizontal = state.visibility_horizontal;

    auto bounds = ctx.bounds();
    if (visibility_horizontal != scroll_bar_visibility::hidden)
    {
        if (visibility_vertical != scroll_bar_visibility::hidden)
        {
            bounds.min_width(47);
            bounds.min_height(47);
        }
        else
        {
            bounds.min_width(35);
            bounds.min_height(12);
        }
    }
    else if (visibility_vertical != scroll_bar_visibility::hidden)
    {
        bounds.min_width(12);
        bounds.min_height(35);
    }

    auto viewport = bounds;
    if (visibility_vertical == scroll_bar_visibility::automatic &&
        state.canvas_size.height > viewport.height())
    {
        visibility_vertical = scroll_bar_visibility::visible;
    }
    if (visibility_horizontal == scroll_bar_visibility::automatic &&
        state.canvas_size.width > viewport.width())
    {
        visibility_horizontal = scroll_bar_visibility::visible;
    }
    if (visibility_vertical == scroll_bar_visibility::automatic &&
        state.canvas_size.height > viewport.height())
    {
        visibility_vertical = scroll_bar_visibility::visible;
    }

    if (visibility_vertical == scroll_bar_visibility::visible)
    {
        viewport.x1 -= scroll_bar_size;
    }
    if (visibility_horizontal == scroll_bar_visibility::visible)
    {
        viewport.y1 -= scroll_bar_size;
    }

    ctx.push_transform(transform::identity());

    state.offset += ctx.get_scroll(id) * state.line_height;
    state.offset.x = std::clamp(
        state.offset.x, 0.f,
        std::clamp(
            state.canvas_size.width - viewport.width(), 0.f,
            state.canvas_size.width));
    state.offset.y = std::clamp(
        state.offset.y, 0.f,
        std::clamp(
            state.canvas_size.height - viewport.height(), 0.f,
            state.canvas_size.height));

    if (visibility_vertical == scroll_bar_visibility::visible)
    {
        ctx.bounds({viewport.x1, viewport.y0, bounds.x1, viewport.y1});

        auto const offset_y   = state.offset.y / state.line_height;
        auto const canvas_h   = state.canvas_size.height / state.line_height;
        auto const viewport_h = viewport.height() / state.line_height;

        state.offset.y =
            scroll_bar_v(ctx, id, offset_y, 0, canvas_h, viewport_h) *
            state.line_height;
    }

    if (visibility_horizontal == scroll_bar_visibility::visible)
    {
        ctx.bounds({viewport.x0, viewport.y1, viewport.x1, bounds.y1});

        auto const offset_x   = state.offset.x / state.line_height;
        auto const canvas_w   = state.canvas_size.width / state.line_height;
        auto const viewport_w = viewport.width() / state.line_height;

        state.offset.x =
            scroll_bar_h(ctx, id, offset_x, 0, canvas_w, viewport_w) *
            state.line_height;
    }

    ctx.push_clip(viewport);
    ctx.push_offset(bounds.position() - state.offset);
}

void end_scroll_panel(context& ctx) noexcept
{
    ctx.pop_offset();
    ctx.pop_clip();
    ctx.pop_transform();
}

}
