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

#ifndef WINDOWER_UI_WIDGET_WINDOW_HPP
#define WINDOWER_UI_WIDGET_WINDOW_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "utility.hpp"

#include <gsl/gsl>

#include <cstdint>

namespace windower::ui::widget
{

enum class window_style : std::uint8_t
{
    standard   = 0,
    tooltip    = 1,
    chromeless = 2,
};

enum class window_flags : std::uint8_t
{
    none           = 0,
    hidden         = 1 << 0,
    movable        = 1 << 1,
    resizable      = 1 << 2,
    closeable      = 1 << 3,
    layout_enabled = 1 << 4,
    click_through  = 1 << 5,
};

constexpr window_flags operator&(window_flags lhs, window_flags rhs)
{
    return gsl::narrow_cast<window_flags>(
        to_underlying(lhs) & to_underlying(rhs));
}

constexpr window_flags operator|(window_flags lhs, window_flags rhs)
{
    return gsl::narrow_cast<window_flags>(
        to_underlying(lhs) | to_underlying(rhs));
}

constexpr window_flags& operator&=(window_flags& lhs, window_flags rhs)
{
    return lhs = lhs & rhs;
}

constexpr window_flags& operator|=(window_flags& lhs, window_flags rhs)
{

    return lhs = lhs | rhs;
}

class window_state
{
public:
    constexpr void layer(ui::layer layer) { m_layer = layer; }
    constexpr void depth(float depth) { m_depth = depth; }
    constexpr void title(std::u8string_view title)
    {
        m_title_data = title.data();
        m_title_size = title.size();
    }
    constexpr void bounds(rectangle const& bounds) { m_bounds = bounds; }
    constexpr void min_size(dimension const& size) { m_min_size = size; }
    constexpr void max_size(dimension const& size) { m_max_size = size; }
    constexpr void zoom_factor(float zoom) { m_zoom_factor = zoom; }
    constexpr void color(ui::color color) { m_color = color; }
    constexpr void style(window_style style) { m_style = style; }
    constexpr void flags(window_flags flags) { m_flags = flags; }

    constexpr ui::layer layer() const { return m_layer; }
    constexpr float depth() const { return m_depth; }
    constexpr std::u8string_view title() const
    {
        return {m_title_data, m_title_size};
    }
    constexpr rectangle const& bounds() const { return m_bounds; }
    constexpr dimension const& min_size() const { return m_min_size; }
    constexpr dimension const& max_size() const { return m_max_size; }
    constexpr float zoom_factor() const { return m_zoom_factor; }
    constexpr ui::color color() const { return m_color; }
    constexpr window_style style() const { return m_style; }
    constexpr window_flags flags() const { return m_flags; }

    constexpr rectangle& bounds() { return m_bounds; }
    constexpr dimension& min_size() { return m_min_size; }
    constexpr dimension& max_size() { return m_max_size; }

private:
    float m_depth = 0.f;
    char8_t const* m_title_data;
    std::size_t m_title_size;
    rectangle m_bounds;
    dimension m_min_size;
    dimension m_max_size = {dimension::unbounded, dimension::unbounded};
    float m_zoom_factor  = 1.f;
    ui::color m_color    = colors::white;
    ui::layer m_layer    = layer::screen;
    window_style m_style = window_style::standard;
    window_flags m_flags = window_flags::none;
};

static_assert(std::is_standard_layout_v<window_state>);

bool begin_window(context& ctx, window_state& state) noexcept;
void end_window(context& ctx) noexcept;

}

#endif
