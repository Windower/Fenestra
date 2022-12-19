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

#include "ui/widget/color_picker.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/transform.hpp"
#include "utility.hpp"

#include <format>
#include <optional>

namespace windower::ui::widget
{

namespace
{

static constexpr auto h_track     = patch{{123, 99, 253, 229}};
static constexpr auto a_track     = patch{{123, 231, 253, 241}};
static constexpr auto a_grid      = patch{{123, 243, 253, 253}};
static constexpr auto hsv_thumb   = patch{{243, 83, 253, 93}};
static constexpr auto a_thumb     = patch{{245, 50, 253, 62}};
static constexpr auto a_thumb_hot = patch{{245, 66, 253, 78}};

void chroma_value_track(
    context& ctx, float hue, vector const& center, bool enabled) noexcept
{
    constexpr auto tx0 = .97037500f;
    constexpr auto tx1 = .97169924f;
    constexpr auto tx2 = .98286330f;
    constexpr auto tx3 = .88019140f;
    constexpr auto tx4 = .89034370f;
    constexpr auto tx5 = .87509376f;
    constexpr auto tx6 = .63380860f;
    constexpr auto tx7 = .62233204f;
    constexpr auto tx8 = .62641800f;

    constexpr auto ty0 = .02962500f;
    constexpr auto ty1 = .01713672f;
    constexpr auto ty2 = .02830078f;
    constexpr auto ty3 = .36619142f;
    constexpr auto ty4 = .37358204f;
    constexpr auto ty5 = .37766796f;
    constexpr auto ty6 = .11980859f;
    constexpr auto ty7 = .12490625f;
    constexpr auto ty8 = .10965625f;

    auto const origin       = ctx.origin();
    auto const zoom         = ctx.zoom_factor();
    auto const scale        = ctx.scale_factor();
    auto const [depth, rhw] = ctx.depth();

    int sector = 0;
    auto const theta =
        std::remquo(hue * .017453292f, .5235988f, &sector) - .2617994f;
    sector %= 12;

    auto transform = transform::scale(zoom, zoom) *
                     scale_correction(ctx.current_transform(), scale) *
                     transform::translation(center) *
                     transform::scale(scale.x, scale.y) *
                     transform::rotation(-theta);
    auto c = std::array<color, 3>{hsv(hue, 1, 1), {}, colors::white};
    std::rotate(c.begin(), c.begin() + sector / 4, c.end());

    auto c0 = color{gsl::at(c, 0)};
    auto c1 = color{gsl::at(c, 1)};
    auto c2 = color{gsl::at(c, 2)};

    switch (sector % 4)
    {
    case 0:
        transform = transform * transform::scale(-1, 1);
        std::swap(c0, c2);
        break;
    case 1: break;
    case 2:
        transform = transform * transform::scale(1, -1);
        std::swap(c0, c1);
        break;
    case 3:
        transform = transform * transform::scale(-1, -1);
        std::swap(c0, c1);
        std::swap(c0, c2);
        break;
    default: windower::fail_fast();
    }

    if (!enabled)
    {
        c0 = to_associated_alpha(fade(c0, 128));
        c1 = to_associated_alpha(fade(c1, 128));
        c2 = to_associated_alpha(fade(c2, 128));
    }

    auto const origin_x = std::round(scale.x * origin.x) - .5f;
    auto const origin_y = std::round(scale.y * origin.y) - .5f;

    auto const p0 = transform * vector{+36.416f, -36.416f};
    auto const p1 = transform * vector{+36.755f, -39.613f};
    auto const p2 = transform * vector{+39.613f, -36.755f};
    auto const p3 = transform * vector{+13.329f, +49.745f};
    auto const p4 = transform * vector{+15.928f, +51.637f};
    auto const p5 = transform * vector{+12.024f, +52.683f};
    auto const p6 = transform * vector{-49.745f, -13.329f};
    auto const p7 = transform * vector{-52.683f, -12.024f};
    auto const p8 = transform * vector{-51.637f, -15.928f};

    ctx.draw_triangle_list(
        {{p0.x + origin_x, p0.y + origin_y, depth, rhw, tx0, ty0, c0},
         {p1.x + origin_x, p1.y + origin_y, depth, rhw, tx1, ty1, c0},
         {p2.x + origin_x, p2.y + origin_y, depth, rhw, tx2, ty2, c0},
         {p3.x + origin_x, p3.y + origin_y, depth, rhw, tx3, ty3, c1},
         {p4.x + origin_x, p4.y + origin_y, depth, rhw, tx4, ty4, c1},
         {p5.x + origin_x, p5.y + origin_y, depth, rhw, tx5, ty5, c1},
         {p6.x + origin_x, p6.y + origin_y, depth, rhw, tx6, ty6, c2},
         {p7.x + origin_x, p7.y + origin_y, depth, rhw, tx7, ty7, c2},
         {p7.x + origin_x, p8.y + origin_y, depth, rhw, tx8, ty8, c2}},
        {0, 3, 6, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 2, 3,
         4, 3, 2, 3, 5, 6, 7, 6, 5, 6, 8, 0, 1, 0, 8},
        determinant(transform) < 0);
}

enum class color_picker_part
{
    none,
    hue,
    chroma_value,
    alpha,
};

struct color_picker_state
{
    std::optional<color_picker_part> active_part;
    color color  = colors::black;
    float hue    = 0;
    float chroma = 0;
    float value  = 0;
    float alpha  = 255;
};

}

color color_picker(context& ctx, id id, color value, bool alpha) noexcept
{
    auto const& bounds  = ctx.bounds();
    auto const position = bounds.position();

    ctx.use_id(id);

    ctx.push_clip(bounds);

    auto s = color_picker_state{};
    if (auto const* const ptr = ctx.active_state<color_picker_state>(id))
    {
        s = *ptr;
    }

    if (s.color != value)
    {
        s.color  = value;
        s.hue    = value.hue();
        s.chroma = value.chroma();
        s.value  = value.value();
        s.alpha  = value.a / 255.f;
    }

    auto const scale = ctx.scale_factor();

    auto const theta     = s.hue * .017453292f;
    auto const sin_theta = std::sinf(theta);
    auto const cos_theta = std::cosf(theta);

    auto const h_track_bounds = rectangle{
        position.x + 4, position.y + 4, position.x + 132, position.y + 132};
    auto const a_track_bounds = rectangle{
        position.x + 4, position.y + 140, position.x + 132, position.y + 148};

    auto const l = h_track_bounds.x0;
    auto const t = h_track_bounds.y0;
    auto const r = h_track_bounds.x1;
    auto const b = h_track_bounds.y1;

    auto const l_scaled = std::round(l * scale.x);
    auto const t_scaled = std::round(t * scale.y);
    auto const r_scaled = std::round(r * scale.x);
    auto const b_scaled = std::round(b * scale.y);

    auto const center = vector{l + r, t + b} / 2;
    auto const center_scaled =
        vector{l_scaled + r_scaled, t_scaled + b_scaled} / 2;

    auto const opaque_color = hcv(s.hue, s.chroma, s.value);

    auto const h_thumb_x = center.x + 63 * cos_theta;
    auto const h_thumb_y = center.y - 63 * sin_theta;
    auto const h_thumb_bounds =
        expand({h_thumb_x, h_thumb_y, h_thumb_x, h_thumb_y}, {4});

    auto u = 45.03332f * s.chroma - 90.06664f * s.value + 45.03332f;
    auto v = 78 * s.chroma - 26;
    auto const sv_thumb_x = u * sin_theta + v * cos_theta + center.x;
    auto const sv_thumb_y = u * cos_theta - v * sin_theta + center.y;
    auto const sv_thumb_bounds =
        expand({sv_thumb_x, sv_thumb_y, sv_thumb_x, sv_thumb_y}, {4});

    auto const thumb_color     = ctx.system_color(system_color::black);
    auto const thumb_highlight = ctx.system_color(system_color::white);
    auto const sv_thumb_color  = lerp(
        ctx.system_color(system_color::color_picker_highlight), thumb_color,
        std::clamp(s.value * s.value * 4, 0.f, 1.f));

    auto const a_thumb_x = position.x + s.alpha * 124 + 3;
    auto const a_thumb_y = position.y + 139;
    auto const a_thumb_bounds =
        rectangle{a_thumb_x, a_thumb_y, a_thumb_x + 6, a_thumb_y + 10};

    auto hot = color_picker_part::none;
    auto x   = 0.f;
    auto y   = 0.f;

    auto const hue = s.hue;

    auto const enabled = ctx.enabled();
    if (enabled)
    {
        auto const mouse = ctx.to_widget(ctx.mouse().position());
        if (mouse)
        {
            x = mouse->x - center.x;
            y = mouse->y - center.y;

            if (ctx.hit_test(id, expand(h_track_bounds, {4})))
            {
                auto const dist_squared = x * x + y * y;
                if (dist_squared <= 4489)
                {
                    if (dist_squared > 2916)
                    {
                        hot = color_picker_part::hue;
                    }
                    else
                    {
                        hot = color_picker_part::chroma_value;
                    }
                }
            }
            else if (alpha && ctx.hit_test(id, expand(a_track_bounds, {2})))
            {
                hot = color_picker_part::alpha;
            }
        }

        if (hot != color_picker_part::none &&
            ctx.mouse().was_pressed(mouse_button::left))
        {
            s.active_part = hot;
            ctx.activate(id);
            ctx.active_state(id, s);
        }

        auto const active_part =
            s.active_part.value_or(color_picker_part::none);
        if (active_part != color_picker_part::none)
        {
            switch (active_part)
            {
            case color_picker_part::hue:
                s.hue = std::atan2f(-y, x) * 57.295779f;
                if (s.hue < 0)
                {
                    s.hue += 360;
                }
                break;

            case color_picker_part::chroma_value:
                u = x * sin_theta + y * cos_theta;
                v = x * cos_theta - y * sin_theta;

                s.chroma = .012820513f * v + .33333334f;
                s.value  = .0064102565f * v - .01110289f * u + .6666667f;
                if (s.chroma < 0)
                {
                    s.value  = std::clamp(s.value - s.chroma * .5f, 0.f, 1.f);
                    s.chroma = 0;
                }
                else if (s.value > 1.f)
                {
                    s.chroma =
                        std::clamp(s.chroma - s.value * .5f + .5f, 0.f, 1.f);
                    s.value = 1;
                }
                else if (s.value < s.chroma)
                {
                    s.value  = std::clamp((s.chroma + s.value) * .5f, 0.f, 1.f);
                    s.chroma = s.value;
                }
                break;

            case color_picker_part::alpha:
                s.alpha =
                    std::clamp((mouse->x - position.x - 6) / 124, 0.f, 1.f);
                break;
            }

            auto const a = std::clamp(std::round(s.alpha * 255), 0.f, 255.f);

            auto const uint8_a = gsl::narrow_cast<std::uint8_t>(a);

            value   = hcv(s.hue, s.chroma, s.value, uint8_a);
            s.color = value;
            ctx.active_state(id, s);

            if (ctx.mouse().was_released(mouse_button::left))
            {
                s.active_part = std::nullopt;
                ctx.active_state(id, s);
                ctx.deactivate(id);
            }
        }
    }

    primitive::set_texture(ctx, u8":system");
    chroma_value_track(ctx, hue, center_scaled, enabled);
    primitive::rectangle(
        ctx, h_track_bounds, h_track,
        enabled ? colors::white : color{255, 255, 255, 128});

    auto const active_part = s.active_part.value_or(hot);

    if (active_part != color_picker_part::none)
    {
        ctx.set_cursor(system_cursor::hot);
    }

    switch (active_part)
    {
    case color_picker_part::none:
        primitive::rectangle(
            ctx, h_thumb_bounds, hsv_thumb,
            enabled ? thumb_color : fade(thumb_color, 128));
        primitive::rectangle(
            ctx, sv_thumb_bounds, hsv_thumb,
            enabled ? sv_thumb_color : fade(sv_thumb_color, 128));
        break;

    case color_picker_part::hue:
        primitive::rectangle(
            ctx, h_thumb_bounds, hsv_thumb,
            enabled ? thumb_highlight : fade(thumb_highlight, 128));
        primitive::rectangle(
            ctx, sv_thumb_bounds, hsv_thumb,
            enabled ? sv_thumb_color : fade(sv_thumb_color, 128));
        break;

    case color_picker_part::chroma_value:
        primitive::rectangle(
            ctx, h_thumb_bounds, hsv_thumb,
            enabled ? thumb_color : fade(thumb_color, 128));
        primitive::rectangle(
            ctx, sv_thumb_bounds, hsv_thumb,
            enabled ? thumb_highlight : fade(thumb_highlight, 128));
        break;

    case color_picker_part::alpha:
        primitive::rectangle(
            ctx, h_thumb_bounds, hsv_thumb,
            enabled ? thumb_color : fade(thumb_color, 128));
        primitive::rectangle(
            ctx, sv_thumb_bounds, hsv_thumb,
            enabled ? sv_thumb_color : fade(sv_thumb_color, 128));
        break;
    }

    if (alpha)
    {
        primitive::rectangle(
            ctx, a_track_bounds, a_grid,
            enabled ? colors::white : color{255, 255, 255, 128});
        primitive::rectangle(
            ctx, a_track_bounds, a_track,
            enabled ? opaque_color : fade(opaque_color, 128));

        switch (active_part)
        {
        case color_picker_part::none:
        case color_picker_part::hue:
        case color_picker_part::chroma_value:
            primitive::rectangle(
                ctx, a_thumb_bounds, a_thumb,
                enabled ? thumb_highlight : fade(thumb_highlight, 128));
            break;

        case color_picker_part::alpha:
            primitive::rectangle(
                ctx, a_thumb_bounds, a_thumb_hot,
                enabled ? thumb_highlight : fade(thumb_highlight, 128));
            break;
        }
    }

    ctx.pop_clip();

    return value;
}

}
