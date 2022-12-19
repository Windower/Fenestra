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

#include "ui/widget/progress.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/rectangle.hpp"

#include <algorithm>
#include <optional>
#include <span>

namespace windower::ui::widget
{
namespace
{

constexpr auto h_track = h_patch{{11, 175, 21, 185}, {5, 5}};
constexpr auto v_track = v_patch{{35, 175, 45, 185}, {5, 5}};

constexpr auto h_fill = h_patch{{23, 175, 33, 185}, {5, 5}};
constexpr auto v_fill = v_patch{{47, 175, 57, 185}, {5, 5}};

void fill_h(
    context& ctx, rectangle const& bounds, float x0, float x1,
    color color) noexcept
{
    x0 = std::clamp(bounds.x0 + x0, bounds.x0, bounds.x1);
    x1 = std::clamp(bounds.x0 + x1, bounds.x0, bounds.x1);

    auto const m = bounds.y0 + 4.f;
    auto const h = std::min(x1 - x0, 8.f) / 2.f;

    auto const y0 = m - h;
    auto const y1 = m + h;

    primitive::rectangle(ctx, {x0, y0, x1, y1}, h_fill, fade(color, 255));
}

void fill_v(
    context& ctx, rectangle const& bounds, float y0, float y1,
    color color) noexcept
{
    y0 = std::clamp(bounds.y0 + y0, bounds.y0, bounds.y1);
    y1 = std::clamp(bounds.y0 + y1, bounds.y0, bounds.y1);

    auto const m = bounds.x0 + 4.f;
    auto const w = std::min(y1 - y0, 8.f) / 2.f;

    auto const x0 = m - w;
    auto const x1 = m + w;

    primitive::rectangle(ctx, {x0, y0, x1, y1}, v_fill, fade(color, 255));
}

void progress_ltr(
    context& ctx, std::span<progress_entry const> entries) noexcept
{
    auto bounds = ctx.bounds();
    bounds.min_width(8);
    bounds.height(8);

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(ctx, bounds, h_track);

    if (entries.empty()) {}
    else
    {
        auto const width = bounds.width();

        auto prev = entries.begin();
        for (auto it = std::next(prev); it != entries.end(); prev = it, ++it)
        {
            auto const x0 = width * it->value / it->max;
            auto const x1 = width * prev->value / prev->max;
            if (x0 < x1)
            {
                fill_h(ctx, bounds, x0 - 8, x1, prev->color);
            }
        }
        auto const x = width * prev->value / prev->max;
        fill_h(ctx, bounds, 0, x, prev->color);
    }
}

void progress_rtl(
    context& ctx, std::span<progress_entry const> entries) noexcept
{
    auto bounds = ctx.bounds();
    bounds.min_width(8);
    bounds.height(8);

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(ctx, bounds, h_track);

    if (entries.empty()) {}
    else
    {
        auto const width = bounds.width();

        auto prev = entries.begin();
        for (auto it = std::next(prev); it != entries.end(); prev = it, ++it)
        {
            auto const x0 = width * it->value / it->max;
            auto const x1 = width * prev->value / prev->max;
            if (x0 < x1)
            {
                fill_h(ctx, bounds, width - x1, width - x0 + 8, prev->color);
            }
        }
        auto const x = width * prev->value / prev->max;
        fill_h(ctx, bounds, width - x, width, prev->color);
    }
}

void progress_btt(
    context& ctx, std::span<progress_entry const> entries) noexcept
{
    auto bounds = ctx.bounds();
    bounds.width(8);
    bounds.min_height(8);

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(ctx, bounds, v_track);

    if (entries.empty()) {}
    else
    {
        auto const height = bounds.height();

        auto prev = entries.begin();
        for (auto it = std::next(prev); it != entries.end(); prev = it, ++it)
        {
            auto const y0 = height * it->value / it->max;
            auto const y1 = height * prev->value / prev->max;
            if (y0 < y1)
            {
                fill_v(ctx, bounds, height - y1, height - y0 + 8, prev->color);
            }
        }
        auto const y = height * prev->value / prev->max;
        fill_v(ctx, bounds, height - y, height, prev->color);
    }
}

void progress_ttb(
    context& ctx, std::span<progress_entry const> entries) noexcept
{
    auto bounds = ctx.bounds();
    bounds.width(8);
    bounds.min_height(8);

    primitive::set_texture(ctx, ctx.skin());
    primitive::rectangle(ctx, bounds, v_track);

    if (entries.empty()) {}
    else
    {
        auto const height = bounds.height();

        auto prev = entries.begin();
        for (auto it = std::next(prev); it != entries.end(); prev = it, ++it)
        {
            auto const y0 = height * it->value / it->max;
            auto const y1 = height * prev->value / prev->max;
            if (y0 < y1)
            {
                fill_v(ctx, bounds, y0 - 8, y1, prev->color);
            }
        }
        auto const y = height * prev->value / prev->max;
        fill_v(ctx, bounds, 0, y, prev->color);
    }
}
}

void progress(
    context& ctx, float value, float max, direction direction) noexcept
{
    progress(
        ctx, value, max, ctx.system_color(system_color::accent), direction);
}

void progress(
    context& ctx, float value, float max, color color,
    direction direction) noexcept
{
    auto const entry = progress_entry{
        .value = value,
        .max   = max,
        .color = color,
    };
    progress(ctx, std::span<progress_entry const>{&entry, 1}, direction);
}

void progress(
    context& ctx, std::span<progress_entry const> entries,
    direction direction) noexcept
{
    switch (direction)
    {
    case direction::left_to_right: progress_ltr(ctx, entries); break;
    case direction::right_to_left: progress_rtl(ctx, entries); break;
    case direction::bottom_to_top: progress_btt(ctx, entries); break;
    case direction::top_to_bottom: progress_ttb(ctx, entries); break;
    }
}

}
