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

#include "ui/primitives.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/patch.hpp"
#include "ui/rectangle.hpp"
#include "ui/texture_loaders.hpp"
#include "ui/vector.hpp"
#include "ui/vertex.hpp"

#include <gsl/gsl>

#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <utility>

namespace windower::ui::primitive
{

void set_texture(
    context& ctx, std::u8string_view name, std::size_t time_to_live) noexcept
{
    set_texture(ctx, get_texture(ctx, name, time_to_live));
}

void set_texture(
    context& ctx, ffxi_image const& image, std::size_t time_to_live) noexcept
{
    set_texture(ctx, get_texture(ctx, image, time_to_live));
}

void set_texture(context& ctx, texture_token texture) noexcept
{
    ctx.set_texture(texture);
}

texture_token get_texture(
    context& ctx, std::u8string_view name, std::size_t time_to_live) noexcept
{
    return load_texture(ctx, name, time_to_live).token;
}

texture_token get_texture(
    context& ctx, ffxi_image const& image, std::size_t time_to_live) noexcept
{
    return load_texture(ctx, image, time_to_live).token;
}

void rectangle(context& ctx, ui::rectangle const& bounds, color c) noexcept
{
    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    auto transform = ctx.current_transform();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x1 = bounds.x1;
    auto y1 = bounds.y1;

    if (x1 < x0)
    {
        std::swap(x0, x1);
        transform = transform * transform::scale(-1, 1);
    }

    if (y1 < y0)
    {
        std::swap(y0, y1);
        transform = transform * transform::scale(1, -1);
    }

    transform = scale_correction(transform, scale);

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const scale_x = scale.x * zoom;
    auto const scale_y = scale.y * zoom;

    x0 = std::round(scale_x * x0);
    y0 = std::round(scale_y * y0);
    x1 = std::round(scale_x * x1);
    y1 = std::round(scale_y * y1);

    auto p0 = transform * vector{x0, y0};
    auto p1 = transform * vector{x1, y0};
    auto p2 = transform * vector{x0, y1};
    auto p3 = transform * vector{x1, y1};

    if (determinant(transform) < 0)
    {
        using std::swap;

        swap(p0, p3);
        swap(p1, p2);
    }

    c = to_associated_alpha(c);

    set_texture(ctx, no_texture);
    ctx.draw_triangle_list(
        {{p0.x + origin_x, p0.y + origin_y, depth, rhw, 0, 0, c},
         {p1.x + origin_x, p1.y + origin_y, depth, rhw, 0, 0, c},
         {p2.x + origin_x, p2.y + origin_y, depth, rhw, 0, 0, c},
         {p3.x + origin_x, p3.y + origin_y, depth, rhw, 0, 0, c}},
        {0, 1, 2, 3, 2, 1});
}

void rectangle(
    context& ctx, ui::rectangle const& bounds, patch const& patch,
    color c) noexcept
{
    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    auto transform = ctx.current_transform();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x1 = bounds.x1;
    auto y1 = bounds.y1;

    if (x1 < x0)
    {
        std::swap(x0, x1);
        transform = transform * transform::scale(-1, 1);
    }

    if (y1 < y0)
    {
        std::swap(y0, y1);
        transform = transform * transform::scale(1, -1);
    }

    transform = scale_correction(transform, scale);

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const overdraw_l = patch.overdraw.left;
    auto const overdraw_t = patch.overdraw.top;
    auto const overdraw_r = patch.overdraw.right;
    auto const overdraw_b = patch.overdraw.bottom;

    auto const w0 = patch.bounds.width();
    auto const h0 = patch.bounds.height();

    auto const w = w0 - overdraw_l - overdraw_r;
    auto const h = h0 - overdraw_t - overdraw_b;

    auto const scale_x = scale.x * zoom;
    auto const scale_y = scale.y * zoom;

    x0 = std::round(scale_x * x0);
    y0 = std::round(scale_y * y0);
    x1 = std::round(scale_x * x1);
    y1 = std::round(scale_y * y1);

    if (x0 >= x1 || y0 >= y1)
    {
        return;
    }

    auto const x_ratio = (x1 - x0) / std::max(w, 1.f);
    auto const y_ratio = (y1 - y0) / std::max(h, 1.f);

    x0 -= x_ratio * overdraw_l;
    y0 -= y_ratio * overdraw_t;
    x1 += x_ratio * overdraw_r;
    y1 += y_ratio * overdraw_b;

    auto const tx0 = patch.bounds.x0 / patch.texture_size.width;
    auto const tx1 = patch.bounds.x1 / patch.texture_size.width;

    auto const ty0 = patch.bounds.y0 / patch.texture_size.height;
    auto const ty1 = patch.bounds.y1 / patch.texture_size.height;

    auto const p0 = transform * vector{x0, y0};
    auto const p1 = transform * vector{x1, y0};
    auto const p2 = transform * vector{x0, y1};
    auto const p3 = transform * vector{x1, y1};

    c = to_associated_alpha(c);

    ctx.draw_triangle_list(
        {{p0.x + origin_x, p0.y + origin_y, depth, rhw, tx0, ty0, c},
         {p1.x + origin_x, p1.y + origin_y, depth, rhw, tx1, ty0, c},
         {p2.x + origin_x, p2.y + origin_y, depth, rhw, tx0, ty1, c},
         {p3.x + origin_x, p3.y + origin_y, depth, rhw, tx1, ty1, c}},
        {0, 1, 2, 3, 2, 1}, determinant(transform) < 0);
}

void rectangle(
    context& ctx, ui::rectangle const& bounds, nine_patch const& patch,
    color c) noexcept
{
    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    auto transform = ctx.current_transform();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x3 = bounds.x1;
    auto y3 = bounds.y1;

    if (x3 < x0)
    {
        std::swap(x0, x3);
        transform = transform * transform::scale(-1, 1);
    }

    if (y3 < y0)
    {
        std::swap(y0, y3);
        transform = transform * transform::scale(1, -1);
    }

    transform = scale_correction(transform, scale);

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const overdraw_l = patch.overdraw.left;
    auto const overdraw_t = patch.overdraw.top;
    auto const overdraw_r = patch.overdraw.right;
    auto const overdraw_b = patch.overdraw.bottom;

    auto const w0 = patch.slice.left;
    auto const h0 = patch.slice.top;
    auto const w2 = patch.slice.right;
    auto const h2 = patch.slice.bottom;

    auto const scale_x = scale.x * zoom;
    auto const scale_y = scale.y * zoom;

    x0 = std::round(scale_x * x0);
    y0 = std::round(scale_y * y0);
    x3 = std::round(scale_x * x3);
    y3 = std::round(scale_y * y3);

    auto x1 = x0 + scale_x * (w0 - overdraw_l);
    auto y1 = y0 + scale_y * (h0 - overdraw_t);
    auto x2 = x3 - scale_x * (w2 - overdraw_r);
    auto y2 = y3 - scale_y * (h2 - overdraw_b);

    if (x1 > x2)
    {
        auto const a = w0 - overdraw_l;
        auto const b = w2 - overdraw_r;

        auto const x_ratio = (x3 - x0) / (a + b);

        x1 = x0 + x_ratio * a;
        x2 = x1;
        x0 -= x_ratio * overdraw_l;
        x3 += x_ratio * overdraw_r;
    }
    else
    {
        x0 -= scale_x * overdraw_l;
        x3 += scale_x * overdraw_r;
    }

    if (y1 > y2)
    {
        auto const a = h0 - overdraw_t;
        auto const b = h2 - overdraw_b;

        auto const y_ratio = (y3 - y0) / (a + b);

        y1 = y0 + y_ratio * a;
        y2 = y1;
        y0 -= y_ratio * overdraw_t;
        y3 += y_ratio * overdraw_b;
    }
    else
    {
        y0 -= scale_y * overdraw_t;
        y3 += scale_y * overdraw_b;
    }

    auto const tx0 = patch.bounds.x0 / patch.texture_size.width;
    auto const tx3 = patch.bounds.x1 / patch.texture_size.width;
    auto const tx1 = tx0 + patch.slice.left / patch.texture_size.width;
    auto const tx2 = tx3 - patch.slice.right / patch.texture_size.width;

    auto const ty0 = patch.bounds.y0 / patch.texture_size.height;
    auto const ty3 = patch.bounds.y1 / patch.texture_size.height;
    auto const ty1 = ty0 + patch.slice.top / patch.texture_size.height;
    auto const ty2 = ty3 - patch.slice.bottom / patch.texture_size.height;

    auto const p00 = transform * vector{x0, y0};
    auto const p01 = transform * vector{x1, y0};
    auto const p02 = transform * vector{x2, y0};
    auto const p03 = transform * vector{x3, y0};
    auto const p04 = transform * vector{x0, y1};
    auto const p05 = transform * vector{x1, y1};
    auto const p06 = transform * vector{x2, y1};
    auto const p07 = transform * vector{x3, y1};
    auto const p08 = transform * vector{x0, y2};
    auto const p09 = transform * vector{x1, y2};
    auto const p10 = transform * vector{x2, y2};
    auto const p11 = transform * vector{x3, y2};
    auto const p12 = transform * vector{x0, y3};
    auto const p13 = transform * vector{x1, y3};
    auto const p14 = transform * vector{x2, y3};
    auto const p15 = transform * vector{x3, y3};

    c = to_associated_alpha(c);

    ctx.draw_triangle_list(
        {{p00.x + origin_x, p00.y + origin_y, depth, rhw, tx0, ty0, c},
         {p01.x + origin_x, p01.y + origin_y, depth, rhw, tx1, ty0, c},
         {p02.x + origin_x, p02.y + origin_y, depth, rhw, tx2, ty0, c},
         {p03.x + origin_x, p03.y + origin_y, depth, rhw, tx3, ty0, c},
         {p04.x + origin_x, p04.y + origin_y, depth, rhw, tx0, ty1, c},
         {p05.x + origin_x, p05.y + origin_y, depth, rhw, tx1, ty1, c},
         {p06.x + origin_x, p06.y + origin_y, depth, rhw, tx2, ty1, c},
         {p07.x + origin_x, p07.y + origin_y, depth, rhw, tx3, ty1, c},
         {p08.x + origin_x, p08.y + origin_y, depth, rhw, tx0, ty2, c},
         {p09.x + origin_x, p09.y + origin_y, depth, rhw, tx1, ty2, c},
         {p10.x + origin_x, p10.y + origin_y, depth, rhw, tx2, ty2, c},
         {p11.x + origin_x, p11.y + origin_y, depth, rhw, tx3, ty2, c},
         {p12.x + origin_x, p12.y + origin_y, depth, rhw, tx0, ty3, c},
         {p13.x + origin_x, p13.y + origin_y, depth, rhw, tx1, ty3, c},
         {p14.x + origin_x, p14.y + origin_y, depth, rhw, tx2, ty3, c},
         {p15.x + origin_x, p15.y + origin_y, depth, rhw, tx3, ty3, c}},
        {0, 1, 4,  4,  1, 5,  1, 2,  5,  5,  2,  6,  2,  3,  6,  6,  3,  7,
         4, 5, 8,  8,  5, 9,  5, 6,  9,  9,  6,  10, 6,  7,  10, 10, 7,  11,
         8, 9, 12, 12, 9, 13, 9, 10, 13, 13, 10, 14, 10, 11, 14, 14, 11, 15},
        determinant(transform) < 0);
}

void rectangle(
    context& ctx, ui::rectangle const& bounds, h_patch const& patch,
    color c) noexcept
{
    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    auto transform = ctx.current_transform();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x3 = bounds.x1;
    auto y1 = bounds.y1;

    if (x3 < x0)
    {
        std::swap(x0, x3);
        transform = transform * transform::scale(-1, 1);
    }

    if (y1 < y0)
    {
        std::swap(y0, y1);
        transform = transform * transform::scale(1, -1);
    }

    transform = scale_correction(transform, scale);

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const overdraw_l = patch.overdraw.left;
    auto const overdraw_t = patch.overdraw.top;
    auto const overdraw_r = patch.overdraw.right;
    auto const overdraw_b = patch.overdraw.bottom;

    auto const w0 = patch.slice.left;
    auto const w2 = patch.slice.right;
    auto const h0 = patch.bounds.height();

    auto const h = h0 - overdraw_t - overdraw_b;

    auto const scale_x = scale.x * zoom;
    auto const scale_y = scale.y * zoom;

    x0 = std::round(scale_x * x0);
    y0 = std::round(scale_y * y0);
    x3 = std::round(scale_x * x3);
    y1 = std::round(scale_y * y1);

    auto x1 = x0 + scale_x * (w0 - overdraw_l);
    auto x2 = x3 - scale_x * (w2 - overdraw_r);

    if (x1 > x2)
    {
        auto const a = w0 - overdraw_l;
        auto const b = w2 - overdraw_r;

        auto const x_ratio = (x3 - x0) / (a + b);

        x1 = x0 + x_ratio * a;
        x2 = x1;
        x0 -= x_ratio * overdraw_l;
        x3 += x_ratio * overdraw_r;
    }
    else
    {
        x0 -= scale_x * overdraw_l;
        x3 += scale_x * overdraw_r;
    }

    auto const y_ratio = (y1 - y0) / std::max(h, 1.f);

    y0 -= y_ratio * overdraw_t;
    y1 += y_ratio * overdraw_b;

    auto const tx0 = patch.bounds.x0 / patch.texture_size.width;
    auto const tx3 = patch.bounds.x1 / patch.texture_size.width;
    auto const tx1 = tx0 + patch.slice.left / patch.texture_size.width;
    auto const tx2 = tx3 - patch.slice.right / patch.texture_size.width;

    auto const ty0 = patch.bounds.y0 / patch.texture_size.height;
    auto const ty1 = patch.bounds.y1 / patch.texture_size.height;

    auto const p0 = transform * vector{x0, y0};
    auto const p1 = transform * vector{x1, y0};
    auto const p2 = transform * vector{x2, y0};
    auto const p3 = transform * vector{x3, y0};
    auto const p4 = transform * vector{x0, y1};
    auto const p5 = transform * vector{x1, y1};
    auto const p6 = transform * vector{x2, y1};
    auto const p7 = transform * vector{x3, y1};

    c = to_associated_alpha(c);

    ctx.draw_triangle_list(
        {{p0.x + origin_x, p0.y + origin_y, depth, rhw, tx0, ty0, c},
         {p1.x + origin_x, p1.y + origin_y, depth, rhw, tx1, ty0, c},
         {p2.x + origin_x, p2.y + origin_y, depth, rhw, tx2, ty0, c},
         {p3.x + origin_x, p3.y + origin_y, depth, rhw, tx3, ty0, c},
         {p4.x + origin_x, p4.y + origin_y, depth, rhw, tx0, ty1, c},
         {p5.x + origin_x, p5.y + origin_y, depth, rhw, tx1, ty1, c},
         {p6.x + origin_x, p6.y + origin_y, depth, rhw, tx2, ty1, c},
         {p7.x + origin_x, p7.y + origin_y, depth, rhw, tx3, ty1, c}},
        {0, 1, 4, 4, 1, 5, 1, 2, 5, 5, 2, 6, 2, 3, 6, 6, 3, 7},
        determinant(transform) < 0);
}

void rectangle(
    context& ctx, ui::rectangle const& bounds, v_patch const& patch,
    color c) noexcept
{
    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    auto transform = ctx.current_transform();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x1 = bounds.x1;
    auto y3 = bounds.y1;

    if (x1 < x0)
    {
        std::swap(x0, x1);
        transform = transform * transform::scale(-1, 1);
    }

    if (y3 < y0)
    {
        std::swap(y0, y3);
        transform = transform * transform::scale(1, -1);
    }

    transform = scale_correction(transform, scale);

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const overdraw_l = patch.overdraw.left;
    auto const overdraw_t = patch.overdraw.top;
    auto const overdraw_r = patch.overdraw.right;
    auto const overdraw_b = patch.overdraw.bottom;

    auto const w0 = patch.bounds.width();
    auto const h0 = patch.slice.top;
    auto const h2 = patch.slice.bottom;

    auto const w = w0 - overdraw_l - overdraw_r;

    auto const scale_x = scale.x * zoom;
    auto const scale_y = scale.y * zoom;

    x0 = std::round(scale_x * x0);
    y0 = std::round(scale_y * y0);
    x1 = std::round(scale_x * x1);
    y3 = std::round(scale_y * y3);

    auto const x_ratio = (x1 - x0) / std::max(w, 1.f);

    x0 -= x_ratio * overdraw_l;
    x1 += x_ratio * overdraw_r;

    auto y1 = y0 + scale_y * (h0 - overdraw_t);
    auto y2 = y3 - scale_y * (h2 - overdraw_b);

    if (y1 > y2)
    {
        auto const a = h0 - overdraw_t;
        auto const b = h2 - overdraw_b;

        auto const y_ratio = (y3 - y0) / (a + b);

        y1 = y0 + y_ratio * a;
        y2 = y1;
        y0 -= y_ratio * overdraw_t;
        y3 += y_ratio * overdraw_b;
    }
    else
    {
        y0 -= scale_y * overdraw_t;
        y3 += scale_y * overdraw_b;
    }

    auto const tx0 = patch.bounds.x0 / patch.texture_size.width;
    auto const tx1 = patch.bounds.x1 / patch.texture_size.width;

    auto const ty0 = patch.bounds.y0 / patch.texture_size.height;
    auto const ty3 = patch.bounds.y1 / patch.texture_size.height;
    auto const ty1 = ty0 + patch.slice.top / patch.texture_size.height;
    auto const ty2 = ty3 - patch.slice.bottom / patch.texture_size.height;

    auto const p0 = transform * vector{x0, y0};
    auto const p1 = transform * vector{x1, y0};
    auto const p2 = transform * vector{x0, y1};
    auto const p3 = transform * vector{x1, y1};
    auto const p4 = transform * vector{x0, y2};
    auto const p5 = transform * vector{x1, y2};
    auto const p6 = transform * vector{x0, y3};
    auto const p7 = transform * vector{x1, y3};

    c = to_associated_alpha(c);

    ctx.draw_triangle_list(
        {{p0.x + origin_x, p0.y + origin_y, depth, rhw, tx0, ty0, c},
         {p1.x + origin_x, p1.y + origin_y, depth, rhw, tx1, ty0, c},
         {p2.x + origin_x, p2.y + origin_y, depth, rhw, tx0, ty1, c},
         {p3.x + origin_x, p3.y + origin_y, depth, rhw, tx1, ty1, c},
         {p4.x + origin_x, p4.y + origin_y, depth, rhw, tx0, ty2, c},
         {p5.x + origin_x, p5.y + origin_y, depth, rhw, tx1, ty2, c},
         {p6.x + origin_x, p6.y + origin_y, depth, rhw, tx0, ty3, c},
         {p7.x + origin_x, p7.y + origin_y, depth, rhw, tx1, ty3, c}},
        {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7},
        determinant(transform) < 0);
}

// void poly_line(
//     context& ctx, [[maybe_unused]] std::span<ui::vector> points,
//     [[maybe_unused]] color c) noexcept
//{
//     auto const scale  = ctx.scale_factor();
//     auto const origin = ctx.origin();
//
//     data_segment<vertex> vertices;
//     data_segment<std::uint16_t> indices;
//     std::iota(indices.data.begin(), indices.data.end(), vertices.offset);
//
//     auto offset    = std::size_t{0};
//     auto remaining = points.size();
//     while (remaining > 1)
//     {
//         auto const chunk_size = std::min(remaining, vertices.data.size());
//         auto const chunk      = points.subspan(offset, chunk_size);
//         offset += chunk.size() - 1;
//         remaining -= chunk.size() - 1;
//         std::transform(
//             chunk.begin(), chunk.end(), vertices.data.begin(), [=](auto& p) {
//                 auto const x = (p.x + origin.x) * scale.x;
//                 auto const y = (p.y + origin.y) * scale.y;
//                 return vertex{x, y, c};
//             });
//         // auto const vertex_span = std::span{vertices}.subspan(0,
//         chunk_size);
//         // auto const index_span = std::span{indices}.subspan(0, chunk_size);
//         // ctx.line_strip(vertex_span, index_span);
//     }
// }

void text(
    context& ctx, ui::rectangle const& bounds, std::u8string_view text,
    text_rasterization_options const& rasterization_options,
    text_layout_options const& layout_options) noexcept
{
    primitive::text(
        ctx, bounds.position(),
        layout_text(ctx, bounds.size(), text, layout_options),
        rasterization_options);
}

void text(
    context& ctx, vector const& position, text_layout const& layout,
    text_rasterization_options const& rasterization_options) noexcept
{
    auto const scale = ctx.scale_factor();

    auto& rasterizer = ctx.text_rasterizer();

    auto const count = rasterizer.chunk_count(ctx, layout);
    for (auto chunk = std::size_t{0}; chunk < count; ++chunk)
    {
        auto const texture =
            rasterizer.rasterize(ctx, layout, chunk, rasterization_options);

        auto const size = vector{texture.patch.bounds.size()} / scale;
        ctx.set_texture(texture.token);
        rectangle(ctx, {position, position + size}, texture.patch);
    }
}

text_layout layout_text(
    context& ctx, dimension const& size, std::u8string_view text,
    text_layout_options const& layout_options) noexcept
{
    return ctx.text_layout_engine().layout(ctx, text, size, layout_options);
}

}
