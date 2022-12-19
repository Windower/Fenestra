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

#include "ui/widget/window.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "ui/primitives.hpp"
#include "ui/rectangle.hpp"
#include "ui/text_layout_engine.hpp"
#include "ui/widget/basic_button.hpp"
#include "utility.hpp"

#include <array>
#include <iostream>

namespace windower::ui::widget
{
namespace
{

static constexpr auto standard_resize_handles = thickness{5};

constexpr auto standard_a_title =
    nine_patch{{11, 3, 49, 25}, {16, 16, 16, 6}, {9, 9, 9, 0}};
constexpr auto standard_a_frame =
    nine_patch{{11, 25, 49, 49}, {16, 0, 16, 16}, {9, 0, 9, 9}};

constexpr auto standard_a_resize_l = patch{{92, 14, 101, 14}, {}};
constexpr auto standard_a_resize_t = patch{{102, 4, 102, 13}, {}};
constexpr auto standard_a_resize_r = patch{{103, 14, 112, 14}, {}};
constexpr auto standard_a_resize_b = patch{{102, 15, 102, 24}, {}};

constexpr auto standard_a_resize_tl =
    nine_patch{{92, 4, 102, 14}, {10, 10, 0, 0}, {}};
constexpr auto standard_a_resize_tr =
    nine_patch{{102, 4, 112, 14}, {0, 10, 10, 0}, {}};
constexpr auto standard_a_resize_bl =
    nine_patch{{92, 14, 102, 24}, {10, 0, 0, 10}, {}};
constexpr auto standard_a_resize_br =
    nine_patch{{102, 14, 112, 24}, {0, 0, 10, 10}, {}};

constexpr auto standard_a_close        = patch{{114, 2, 130, 18}, {}};
constexpr auto standard_a_close_hot    = patch{{114, 18, 130, 34}, {}};
constexpr auto standard_a_close_active = patch{{114, 34, 130, 50}, {}};

constexpr auto standard_i_title =
    nine_patch{{51, 3, 89, 25}, {16, 16, 16, 6}, {9, 9, 9, 0}};
constexpr auto standard_i_frame =
    nine_patch{{51, 25, 89, 49}, {16, 0, 16, 16}, {9, 0, 9, 9}};

constexpr auto standard_i_resize_l = patch{{92, 38, 101, 38}, {}};
constexpr auto standard_i_resize_t = patch{{102, 28, 102, 37}, {}};
constexpr auto standard_i_resize_r = patch{{103, 38, 112, 38}, {}};
constexpr auto standard_i_resize_b = patch{{102, 39, 102, 48}, {}};

constexpr auto standard_i_resize_tl =
    nine_patch{{92, 28, 102, 38}, {10, 10, 0, 0}, {}};
constexpr auto standard_i_resize_tr =
    nine_patch{{102, 28, 112, 38}, {0, 10, 10, 0}, {}};
constexpr auto standard_i_resize_bl =
    nine_patch{{92, 38, 102, 48}, {10, 0, 0, 10}, {}};
constexpr auto standard_i_resize_br =
    nine_patch{{102, 38, 112, 48}, {0, 0, 10, 10}, {}};

constexpr auto standard_i_close        = patch{{130, 2, 146, 18}, {}};
constexpr auto standard_i_close_hot    = patch{{130, 18, 146, 34}, {}};
constexpr auto standard_i_close_active = patch{{130, 34, 146, 50}, {}};

constexpr auto tooltip_frame = nine_patch{{147, 3, 177, 33}, {3}};

constexpr auto layout_a_frame = nine_patch{{11, 3, 41, 33}, {3}};

constexpr auto layout_a_resize_l = patch{{11, 38, 17, 38}, {1, 0}};
constexpr auto layout_a_resize_t = patch{{22, 35, 22, 41}, {0, 1}};
constexpr auto layout_a_resize_r = patch{{27, 38, 33, 38}, {1, 0}};
constexpr auto layout_a_resize_b = patch{{38, 35, 38, 41}, {0, 1}};

constexpr auto layout_a_resize_l_hot = patch{{11, 46, 17, 46}, {1, 0}};
constexpr auto layout_a_resize_t_hot = patch{{22, 43, 22, 49}, {0, 1}};
constexpr auto layout_a_resize_r_hot = patch{{27, 46, 33, 46}, {1, 0}};
constexpr auto layout_a_resize_b_hot = patch{{38, 43, 38, 49}, {0, 1}};

constexpr auto layout_a_resize_tl = patch{{11, 51, 25, 65}};
constexpr auto layout_a_resize_tr = patch{{27, 51, 41, 65}};
constexpr auto layout_a_resize_bl = patch{{11, 67, 25, 81}};
constexpr auto layout_a_resize_br = patch{{27, 67, 41, 81}};

constexpr auto layout_ah_frame = nine_patch{{43, 3, 73, 33}, {3}};

constexpr auto layout_ah_resize_l = patch{{43, 38, 49, 38}, {1, 0}};
constexpr auto layout_ah_resize_t = patch{{54, 35, 54, 41}, {0, 1}};
constexpr auto layout_ah_resize_r = patch{{59, 38, 65, 38}, {1, 0}};
constexpr auto layout_ah_resize_b = patch{{70, 35, 70, 41}, {0, 1}};

constexpr auto layout_ah_resize_l_hot = patch{{43, 46, 49, 46}, {1, 0}};
constexpr auto layout_ah_resize_t_hot = patch{{54, 43, 54, 49}, {0, 1}};
constexpr auto layout_ah_resize_r_hot = patch{{59, 46, 65, 46}, {1, 0}};
constexpr auto layout_ah_resize_b_hot = patch{{70, 43, 70, 49}, {0, 1}};

constexpr auto layout_ah_resize_tl = patch{{43, 51, 57, 65}};
constexpr auto layout_ah_resize_tr = patch{{59, 51, 73, 65}};
constexpr auto layout_ah_resize_bl = patch{{43, 67, 57, 81}};
constexpr auto layout_ah_resize_br = patch{{59, 67, 73, 81}};

constexpr auto layout_i_frame = nine_patch{{75, 3, 105, 33}, {3}};

constexpr auto layout_i_resize_l = patch{{75, 38, 81, 38}, {1, 0}};
constexpr auto layout_i_resize_t = patch{{86, 35, 86, 41}, {0, 1}};
constexpr auto layout_i_resize_r = patch{{91, 38, 97, 38}, {1, 0}};
constexpr auto layout_i_resize_b = patch{{102, 35, 102, 41}, {0, 1}};

constexpr auto layout_i_resize_l_hot = patch{{75, 46, 81, 46}, {1, 0}};
constexpr auto layout_i_resize_t_hot = patch{{86, 43, 86, 49}, {0, 1}};
constexpr auto layout_i_resize_r_hot = patch{{91, 46, 97, 46}, {1, 0}};
constexpr auto layout_i_resize_b_hot = patch{{102, 43, 102, 49}, {0, 1}};

constexpr auto layout_i_resize_tl = patch{{75, 51, 89, 65}};
constexpr auto layout_i_resize_tr = patch{{91, 51, 105, 65}};
constexpr auto layout_i_resize_bl = patch{{75, 67, 89, 81}};
constexpr auto layout_i_resize_br = patch{{91, 67, 105, 81}};

constexpr auto layout_ih_frame = nine_patch{{107, 3, 137, 33}, {3}};

constexpr auto layout_ih_resize_l = patch{{107, 38, 113, 38}, {1, 0}};
constexpr auto layout_ih_resize_t = patch{{118, 35, 118, 41}, {0, 1}};
constexpr auto layout_ih_resize_r = patch{{123, 38, 129, 38}, {1, 0}};
constexpr auto layout_ih_resize_b = patch{{134, 35, 134, 41}, {0, 1}};

constexpr auto layout_ih_resize_l_hot = patch{{107, 46, 113, 46}, {1, 0}};
constexpr auto layout_ih_resize_t_hot = patch{{118, 43, 118, 49}, {0, 1}};
constexpr auto layout_ih_resize_r_hot = patch{{123, 46, 129, 46}, {1, 0}};
constexpr auto layout_ih_resize_b_hot = patch{{134, 43, 134, 49}, {0, 1}};

constexpr auto layout_ih_resize_tl = patch{{107, 51, 121, 65}};
constexpr auto layout_ih_resize_tr = patch{{123, 51, 137, 65}};
constexpr auto layout_ih_resize_bl = patch{{107, 67, 121, 81}};
constexpr auto layout_ih_resize_br = patch{{123, 67, 137, 81}};

std::tuple<bool, vector, button_state>
drag_target(context& ctx, id id, rectangle bounds) noexcept
{
    ctx.bounds(bounds);
    auto const state = basic_button(ctx, id);
    return {
        state.active ? state.button == mouse_button::left : state.hot,
        state.drag_position - bounds.position(), state};
}

void resize_l(window_state& state, vector const& delta) noexcept
{
    auto const zoom = state.zoom_factor();

    auto const x0 = state.bounds().x0;
    auto const x1 = (state.bounds().x1 - x0) * zoom + x0;

    auto const min_w = std::max(state.min_size().width, 0.f) * zoom;
    auto const max_w = std::max(state.max_size().width, min_w) * zoom;
    auto const min_x = x1 - max_w;
    auto const max_x = x1 - min_w;

    auto const delta_x = delta.x * zoom;

    state.bounds().x0 = std::clamp(x0 + delta_x, min_x, max_x);
    state.bounds().x1 = (x1 - state.bounds().x0) / zoom + state.bounds().x0;
}

void resize_t(window_state& state, vector const& delta) noexcept
{
    auto const zoom = state.zoom_factor();

    auto const y0 = state.bounds().y0;
    auto const y1 = (state.bounds().y1 - y0) * zoom + y0;

    auto const min_h = std::max(state.min_size().height, 0.f) * zoom;
    auto const max_h = std::max(state.max_size().height, min_h) * zoom;
    auto const min_y = y1 - max_h;
    auto const max_y = y1 - min_h;

    auto const delta_y = delta.y * zoom;

    state.bounds().y0 = std::clamp(y0 + delta_y, min_y, max_y);
    state.bounds().y1 = (y1 - state.bounds().y0) / zoom + state.bounds().y0;
}

void resize_r(window_state& state, vector const& delta) noexcept
{
    auto const min_w  = std::max(state.min_size().width, 0.f);
    auto const max_w  = std::max(state.max_size().width, min_w);
    auto const min_x  = state.bounds().x0 + min_w;
    auto const max_x  = state.bounds().x0 + max_w;
    state.bounds().x1 = std::clamp(state.bounds().x1 + delta.x, min_x, max_x);
}

void resize_b(window_state& state, vector const& delta) noexcept
{
    auto const min_h  = std::max(state.min_size().height, 0.f);
    auto const max_h  = std::max(state.max_size().height, min_h);
    auto const min_y  = state.bounds().y0 + min_h;
    auto const max_y  = state.bounds().y0 + max_h;
    state.bounds().y1 = std::clamp(state.bounds().y1 + delta.y, min_y, max_y);
}

enum class mouse_target
{
    none      = 0,
    resize_l  = 1,
    resize_tl = 2,
    resize_t  = 3,
    resize_tr = 4,
    resize_r  = 5,
    resize_br = 6,
    resize_b  = 7,
    resize_bl = 8,
    move      = 9,
    close     = 10,
};

std::tuple<mouse_target, button_state> mouse_targets(
    context& ctx, window const& window, window_state& state,
    rectangle const& frame, rectangle const& move, rectangle const& close,
    thickness const& border) noexcept
{
    auto const scope = std::bit_cast<std::uintptr_t>(&window) + 1;

    auto const center = contract(frame, border);

    auto const closeable = has_flag(state.flags(), window_flags::closeable);
    auto const movable   = has_flag(state.flags(), window_flags::movable);
    auto const resizable = has_flag(state.flags(), window_flags::resizable);

    if (resizable)
    {
        if (movable)
        {
            if (auto const [active, delta, button_state] = drag_target(
                    ctx, id{scope, 0},
                    {frame.x0, center.y0, center.x0, center.y1});
                active)
            {
                resize_l(state, delta);
                ctx.set_cursor(system_cursor::west);
                return {mouse_target::resize_l, button_state};
            }

            if (auto const [active, delta, button_state] = drag_target(
                    ctx, id{scope, 1},
                    {center.x0, frame.y0, center.x1, center.y0});
                active)
            {
                resize_t(state, delta);
                ctx.set_cursor(system_cursor::north);
                return {mouse_target::resize_t, button_state};
            }

            if (auto const [active, delta, button_state] = drag_target(
                    ctx, id{scope, 2},
                    {frame.x0, frame.y0, center.x0, center.y0});
                active)
            {
                resize_l(state, delta);
                resize_t(state, delta);
                ctx.set_cursor(system_cursor::north_west);
                return {mouse_target::resize_tl, button_state};
            }

            if (auto const [active, delta, button_state] = drag_target(
                    ctx, id{scope, 3},
                    {center.x1, frame.y0, frame.x1, center.y0});
                active)
            {
                resize_r(state, delta);
                resize_t(state, delta);
                ctx.set_cursor(system_cursor::north_east);
                return {mouse_target::resize_tr, button_state};
            }

            if (auto const [active, delta, button_state] = drag_target(
                    ctx, id{scope, 4},
                    {frame.x0, center.y1, center.x0, frame.y1});
                active)
            {
                resize_l(state, delta);
                resize_b(state, delta);
                ctx.set_cursor(system_cursor::south_west);
                return {mouse_target::resize_bl, button_state};
            }
        }

        if (auto const [active, delta, button_state] = drag_target(
                ctx, id{scope, 5}, {center.x1, center.y0, frame.x1, center.y1});
            active)
        {
            resize_r(state, delta);
            ctx.set_cursor(system_cursor::east);
            return {mouse_target::resize_r, button_state};
        }

        if (auto const [active, delta, button_state] = drag_target(
                ctx, id{scope, 6}, {center.x0, center.y1, center.x1, frame.y1});
            active)
        {
            resize_b(state, delta);
            ctx.set_cursor(system_cursor::south);
            return {mouse_target::resize_b, button_state};
        }

        if (auto const [active, delta, button_state] = drag_target(
                ctx, id{scope, 7}, {center.x1, center.y1, frame.x1, frame.y1});
            active)
        {
            resize_r(state, delta);
            resize_b(state, delta);
            ctx.set_cursor(system_cursor::south_east);
            return {mouse_target::resize_br, button_state};
        }
    }

    if (closeable)
    {
        ctx.bounds(intersection(close, movable ? center : frame));
        if (auto const button_state = basic_button(ctx, id{scope, 8});
            button_state || button_state.hot)
        {
            if (button_state)
            {
                state.flags(state.flags() | window_flags::hidden);
            }
            return {mouse_target::close, button_state};
        }
    }

    if (movable)
    {
        if (auto const [active, delta, button_state] = drag_target(
                ctx, id{scope, 9},
                intersection(move, movable ? center : frame));
            active)
        {
            auto const zoom = state.zoom_factor();

            state.bounds().x0 += delta.x * zoom;
            state.bounds().y0 += delta.y * zoom;
            state.bounds().x1 += delta.x * zoom;
            state.bounds().y1 += delta.y * zoom;

            return {mouse_target::move, button_state};
        }
    }

    return {mouse_target::none, {}};
}

void layout_window(
    context& ctx, window_state& state, rectangle const& frame) noexcept
{
    if (!ctx.layout_mode() ||
        !has_flag(state.flags(), window_flags::layout_enabled))
    {
        return;
    }

    auto const movable   = has_flag(state.flags(), window_flags::movable);
    auto const resizable = has_flag(state.flags(), window_flags::resizable);
    auto const hidden    = has_flag(state.flags(), window_flags::hidden);

    auto const id = std::bit_cast<std::uintptr_t>(&state) + 1;

    auto& wnd = ctx.begin_window(id, layer::layout, 0);

    auto const scaled = rectangle{
        frame.x0 * state.zoom_factor(), frame.y0 * state.zoom_factor(),
        frame.x1 * state.zoom_factor(), frame.y1 * state.zoom_factor()};

    wnd.bounds(scaled);
    wnd.zoom_factor(state.zoom_factor());
    wnd.interactable(movable || resizable);

    auto const [target, button_state] = mouse_targets(
        ctx, wnd, state, frame, frame, {}, {8.f / state.zoom_factor()});

    wnd.zoom_factor(1);

    auto const active = ctx.is_active(wnd) && (movable || resizable);

    auto const w = scaled.width();
    auto const h = scaled.height();

    primitive::set_texture(ctx, u8":system");

    primitive::rectangle(
        ctx, scaled,
        active ? (hidden ? layout_ah_frame : layout_a_frame)
               : (hidden ? layout_ih_frame : layout_i_frame));

    ctx.push_clip(contract(scaled, {1}));

    if (resizable)
    {
        if (movable)
        {
            if (h >= 110)
            {
                primitive::rectangle(
                    ctx,
                    {scaled.x0 + 3, scaled.y0 + 50, scaled.x0 + 7,
                     scaled.y1 - 50},
                    target == mouse_target::resize_l
                        ? (active ? (hidden ? layout_ah_resize_l_hot
                                            : layout_a_resize_l_hot)
                                  : (hidden ? layout_ih_resize_l_hot
                                            : layout_i_resize_l_hot))
                        : (active ? (hidden ? layout_ah_resize_l
                                            : layout_a_resize_l)
                                  : (hidden ? layout_ih_resize_l
                                            : layout_i_resize_l)));
            }

            if (w >= 110)
            {
                primitive::rectangle(
                    ctx,
                    {scaled.x0 + 50, scaled.y0 + 3, scaled.x1 - 50,
                     scaled.y0 + 7},
                    target == mouse_target::resize_t
                        ? (active ? (hidden ? layout_ah_resize_t_hot
                                            : layout_a_resize_t_hot)
                                  : (hidden ? layout_ih_resize_t_hot
                                            : layout_i_resize_t_hot))
                        : (active ? (hidden ? layout_ah_resize_t
                                            : layout_a_resize_t)
                                  : (hidden ? layout_ih_resize_t
                                            : layout_i_resize_t)));
            }
        }

        if (h >= 110)
        {
            primitive::rectangle(
                ctx,
                {scaled.x1 - 7, scaled.y0 + 50, scaled.x1 - 3, scaled.y1 - 50},
                target == mouse_target::resize_r
                    ? (active ? (hidden ? layout_ah_resize_r_hot
                                        : layout_a_resize_r_hot)
                              : (hidden ? layout_ih_resize_r_hot
                                        : layout_i_resize_r_hot))
                    : (active
                           ? (hidden ? layout_ah_resize_r : layout_a_resize_r)
                           : (hidden ? layout_ih_resize_r
                                     : layout_i_resize_r)));
        }
        if (w >= 110)
        {
            primitive::rectangle(
                ctx,
                {scaled.x0 + 50, scaled.y1 - 7, scaled.x1 - 50, scaled.y1 - 3},
                target == mouse_target::resize_b
                    ? (active ? (hidden ? layout_ah_resize_b_hot
                                        : layout_a_resize_b_hot)
                              : (hidden ? layout_ih_resize_b_hot
                                        : layout_i_resize_b_hot))
                    : (active
                           ? (hidden ? layout_ah_resize_b : layout_a_resize_b)
                           : (hidden ? layout_ih_resize_b
                                     : layout_i_resize_b)));
        }
    }

    switch (target)
    {
    case mouse_target::resize_tl:
        primitive::rectangle(
            ctx, {scaled.x0 + 2, scaled.y0 + 2, scaled.x0 + 14, scaled.y0 + 14},
            active ? (hidden ? layout_ah_resize_tl : layout_a_resize_tl)
                   : (hidden ? layout_ih_resize_tl : layout_i_resize_tl));
        break;

    case mouse_target::resize_tr:
        primitive::rectangle(
            ctx, {scaled.x1 - 14, scaled.y0 + 2, scaled.x1 - 2, scaled.y0 + 14},
            active ? (hidden ? layout_ah_resize_tr : layout_a_resize_tr)
                   : (hidden ? layout_ih_resize_tr : layout_i_resize_tr));
        break;

    case mouse_target::resize_bl:
        primitive::rectangle(
            ctx, {scaled.x0 + 2, scaled.y1 - 14, scaled.x0 + 14, scaled.y1 - 2},
            active ? (hidden ? layout_ah_resize_bl : layout_a_resize_bl)
                   : (hidden ? layout_ih_resize_bl : layout_i_resize_bl));
        break;

    case mouse_target::resize_br:
        primitive::rectangle(
            ctx, {scaled.x1 - 14, scaled.y1 - 14, scaled.x1 - 2, scaled.y1 - 2},
            active ? (hidden ? layout_ah_resize_br : layout_a_resize_br)
                   : (hidden ? layout_ih_resize_br : layout_i_resize_br));
        break;
    }

    primitive::text(
        ctx, contract(scaled, {7}), state.title(),
        {.fill_color = ctx.system_color(
             active ? (hidden ? system_color::layout_active_hidden_title
                              : system_color::layout_active_title)
                    : (hidden ? system_color::layout_inactive_hidden_title
                              : system_color::layout_inactive_title)),
         .stroke_color = ctx.system_color(
             active
                 ? (hidden ? system_color::layout_active_hidden_title_stroke
                           : system_color::layout_active_title_stroke)
                 : (hidden ? system_color::layout_inactive_hidden_title_stroke
                           : system_color::layout_inactive_title_stroke)),
         .stroke_width = 1,
         .flags        = text_rasterization_flags::clip_to_bounds},
        {.padding = 1});

    ctx.end_window();
}

bool standard_window(context& ctx, window_state& state) noexcept
{
    static constexpr auto close_w = 22.f;

    auto const min_w = std::max(state.min_size().width, 10.f);
    auto const min_h = std::max(state.min_size().height, 10.f);
    auto const max_w = std::max(state.max_size().width, min_w);
    auto const max_h = std::max(state.max_size().height, min_h);

    state.bounds().width(std::clamp(state.bounds().width(), min_w, max_w));
    state.bounds().height(std::clamp(state.bounds().height(), min_h, max_h));

    state.min_size().width  = min_w;
    state.min_size().height = min_h;
    state.max_size().width  = max_w;
    state.max_size().height = max_h;

    auto bounds       = state.bounds();
    auto const origin = bounds.position();

    auto const zoom_offset_x = bounds.x0 / state.zoom_factor() - bounds.x0;
    auto const zoom_offset_y = bounds.y0 / state.zoom_factor() - bounds.y0;

    bounds.x0 += zoom_offset_x;
    bounds.y0 += zoom_offset_y;
    bounds.x1 += zoom_offset_x;
    bounds.y1 += zoom_offset_y;

    auto const hidden    = has_flag(state.flags(), window_flags::hidden);
    auto const closeable = has_flag(state.flags(), window_flags::closeable);
    auto const click_through =
        has_flag(state.flags(), window_flags::click_through);

    auto const title_width =
        closeable ? bounds.width() - close_w + standard_resize_handles.right
                  : bounds.width();

    auto title_layout_options =
        text_layout_options{.word_wrapping = text_word_wrapping::no_wrap};
    title_layout_options.trimming_granularity =
        text_trimming_granularity::character;
    auto const title_layout = primitive::layout_text(
        ctx, {title_width, dimension::unbounded}, state.title(),
        title_layout_options);
    auto const title_h = std::ceil(title_layout.metric_bounds().height());

    auto frame = expand(
        bounds,
        {standard_resize_handles.left, standard_resize_handles.top + title_h,
         standard_resize_handles.right, standard_resize_handles.bottom});

    layout_window(ctx, state, frame);

    auto const id = std::bit_cast<std::uintptr_t>(&state);

    auto& wnd = ctx.begin_window(id, state.layer(), state.depth());
    wnd.zoom_factor(state.zoom_factor());
    wnd.interactable(!hidden && !click_through);
    wnd.origin(origin);
    wnd.bounds(
        {frame.x0 * state.zoom_factor(), frame.y0 * state.zoom_factor(),
         frame.x1 * state.zoom_factor(), frame.y1 * state.zoom_factor()});

    frame.x0 -= bounds.x0;
    frame.y0 -= bounds.y0;
    frame.x1 -= bounds.x0;
    frame.y1 -= bounds.y0;

    if (hidden)
    {
        ctx.end_window();
        return false;
    }

    auto const active = ctx.is_active(wnd);

    auto const color = fade(state.color(), 255);
    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(
        ctx, {frame.x0, frame.y0, frame.x1, 0},
        active ? standard_a_title : standard_i_title, color);
    primitive::rectangle(
        ctx, {frame.x0, 0, frame.x1, frame.y1},
        active ? standard_a_frame : standard_i_frame, color);
    primitive::text(
        ctx, {frame.x0 + 4, frame.y0 + 1}, title_layout,
        {.fill_color = modulate(
             color, ctx.system_color(
                        active ? system_color::window_title
                               : system_color::window_title_inactive)),
         .flags = text_rasterization_flags::clip_to_bounds});

    primitive::set_texture(ctx, ctx.skin());
    if (auto const [target, button_state] = mouse_targets(
            ctx, wnd, state, frame, {frame.x0, frame.y0, frame.x1, 0},
            {frame.x1 - close_w, frame.y0, frame.x1, 0},
            standard_resize_handles);
        target == mouse_target::close)
    {
        if (button_state.active)
        {
            primitive::rectangle(
                ctx, {frame.x1 - 18, frame.y0 + 2, frame.x1 - 2, frame.y0 + 18},
                active ? standard_a_close_active : standard_i_close_active,
                color);
        }
        else
        {
            primitive::rectangle(
                ctx, {frame.x1 - 18, frame.y0 + 2, frame.x1 - 2, frame.y0 + 18},
                active ? standard_a_close_hot : standard_i_close_hot, color);
        }
    }
    else
    {
        if (closeable)
        {
            primitive::rectangle(
                ctx, {frame.x1 - 18, frame.y0 + 2, frame.x1 - 2, frame.y0 + 18},
                active ? standard_a_close : standard_i_close, color);
        }

        switch (target)
        {
        case mouse_target::resize_l:
            primitive::rectangle(
                ctx, {frame.x0 - 4, frame.y0 + 4, frame.x0 + 5, frame.y1 - 4},
                active ? standard_a_resize_l : standard_i_resize_l);
            break;

        case mouse_target::resize_t:
            primitive::rectangle(
                ctx, {frame.x0 + 4, frame.y0 - 4, frame.x1 - 4, frame.y0 + 5},
                active ? standard_a_resize_t : standard_i_resize_t);
            break;

        case mouse_target::resize_r:
            primitive::rectangle(
                ctx, {frame.x1 - 5, frame.y0 + 4, frame.x1 + 4, frame.y1 - 4},
                active ? standard_a_resize_r : standard_i_resize_r);
            break;

        case mouse_target::resize_b:
            primitive::rectangle(
                ctx, {frame.x0 + 4, frame.y1 - 5, frame.x1 - 4, frame.y1 + 4},
                active ? standard_a_resize_b : standard_i_resize_b);
            break;

        case mouse_target::resize_tl:
            primitive::rectangle(
                ctx, {frame.x0 - 4, frame.y0 - 4, frame.x0 + 10, frame.y0 + 10},
                active ? standard_a_resize_tl : standard_i_resize_tl);
            break;

        case mouse_target::resize_tr:
            primitive::rectangle(
                ctx, {frame.x1 - 10, frame.y0 - 4, frame.x1 + 4, frame.y0 + 10},
                active ? standard_a_resize_tr : standard_i_resize_tr);
            break;

        case mouse_target::resize_bl:
            primitive::rectangle(
                ctx, {frame.x0 - 4, frame.y1 - 10, frame.x0 + 10, frame.y1 + 4},
                active ? standard_a_resize_bl : standard_i_resize_bl);
            break;

        case mouse_target::resize_br:
            primitive::rectangle(
                ctx, {frame.x1 - 10, frame.y1 - 10, frame.x1 + 4, frame.y1 + 4},
                active ? standard_a_resize_br : standard_i_resize_br);
            break;
        }
    }

    ctx.push_clip({{}, vector{bounds.size()}});

    return true;
}

bool tooltip_window(context& ctx, window_state& state) noexcept
{
    auto const min_w = std::max(state.min_size().width, 0.f);
    auto const min_h = std::max(state.min_size().height, 0.f);
    auto const max_w = std::max(state.max_size().width, min_w);
    auto const max_h = std::max(state.max_size().height, min_h);

    state.bounds().width(std::clamp(state.bounds().width(), min_w, max_w));
    state.bounds().height(std::clamp(state.bounds().height(), min_h, max_h));

    state.min_size().width  = min_w;
    state.min_size().height = min_h;
    state.max_size().width  = max_w;
    state.max_size().height = max_h;

    auto bounds       = state.bounds();
    auto const origin = bounds.position();

    auto const zoom_offset_x = bounds.x0 / state.zoom_factor() - bounds.x0;
    auto const zoom_offset_y = bounds.y0 / state.zoom_factor() - bounds.y0;

    bounds.x0 += zoom_offset_x;
    bounds.y0 += zoom_offset_y;
    bounds.x1 += zoom_offset_x;
    bounds.y1 += zoom_offset_y;

    auto const hidden = has_flag(state.flags(), window_flags::hidden);
    auto const click_through =
        has_flag(state.flags(), window_flags::click_through);

    layout_window(ctx, state, bounds);

    auto const id = std::bit_cast<std::uintptr_t>(&state);

    auto& wnd = ctx.begin_window(id, state.layer(), state.depth());
    wnd.zoom_factor(state.zoom_factor());
    wnd.interactable(!hidden && !click_through);
    wnd.origin(origin);
    wnd.bounds(
        {bounds.x0 * state.zoom_factor(), bounds.y0 * state.zoom_factor(),
         bounds.x1 * state.zoom_factor(), bounds.y1 * state.zoom_factor()});

    bounds.x1 -= bounds.x0;
    bounds.y1 -= bounds.y0;
    bounds.x0 = 0;
    bounds.y0 = 0;

    if (hidden)
    {
        ctx.end_window();
        return false;
    }

    auto const color = fade(state.color(), 255);

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(ctx, bounds, tooltip_frame, color);

    ctx.push_clip(bounds);

    return true;
}

bool chromeless_window(context& ctx, window_state& state) noexcept
{
    auto const min_w = std::max(state.min_size().width, 0.f);
    auto const min_h = std::max(state.min_size().height, 0.f);
    auto const max_w = std::max(state.max_size().width, min_w);
    auto const max_h = std::max(state.max_size().height, min_h);

    state.bounds().width(std::clamp(state.bounds().width(), min_w, max_w));
    state.bounds().height(std::clamp(state.bounds().height(), min_h, max_h));

    state.min_size().width  = min_w;
    state.min_size().height = min_h;
    state.max_size().width  = max_w;
    state.max_size().height = max_h;

    auto bounds       = state.bounds();
    auto const origin = bounds.position();

    auto const zoom_offset_x = bounds.x0 / state.zoom_factor() - bounds.x0;
    auto const zoom_offset_y = bounds.y0 / state.zoom_factor() - bounds.y0;

    bounds.x0 += zoom_offset_x;
    bounds.y0 += zoom_offset_y;
    bounds.x1 += zoom_offset_x;
    bounds.y1 += zoom_offset_y;

    auto const hidden = has_flag(state.flags(), window_flags::hidden);
    auto const click_through =
        has_flag(state.flags(), window_flags::click_through);

    layout_window(ctx, state, bounds);

    auto const id = std::bit_cast<std::uintptr_t>(&state);

    auto& wnd = ctx.begin_window(id, state.layer(), state.depth());
    wnd.zoom_factor(state.zoom_factor());
    wnd.interactable(!hidden && !click_through);
    wnd.origin(origin);
    wnd.bounds(
        {bounds.x0 * state.zoom_factor(), bounds.y0 * state.zoom_factor(),
         bounds.x1 * state.zoom_factor(), bounds.y1 * state.zoom_factor()});

    bounds.x1 -= bounds.x0;
    bounds.y1 -= bounds.y0;
    bounds.x0 = 0;
    bounds.y0 = 0;

    if (hidden)
    {
        ctx.end_window();
        return false;
    }

    if (state.color().a != 0)
    {
        primitive::set_texture(ctx, u8":system");
        primitive::rectangle(ctx, bounds, to_associated_alpha(state.color()));
    }

    ctx.push_clip(bounds);

    return true;
}

bool begin_window_impl(context& ctx, window_state& state) noexcept
{
    switch (state.style())
    {
    case window_style::standard: return standard_window(ctx, state);
    case window_style::tooltip: return tooltip_window(ctx, state);
    case window_style::chromeless: return chromeless_window(ctx, state);
    }
    fail_fast();
}

}

bool begin_window(context& ctx, window_state& state) noexcept
{
    auto zoom = state.zoom_factor();
    zoom      = std::max(zoom, 0.1f);
    state.zoom_factor(zoom);

    auto const result = begin_window_impl(ctx, state);

    return result;
}

void end_window(context& ctx) noexcept { ctx.end_window(); }

}
