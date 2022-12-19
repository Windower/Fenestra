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

#ifndef WINDOWER_UI_TEXT_LAYOUT_ENGINE_HPP
#define WINDOWER_UI_TEXT_LAYOUT_ENGINE_HPP

#include "ui/color.hpp"
#include "ui/com_base.hpp"
#include "ui/dimension.hpp"
#include "ui/inline_object.hpp"
#include "ui/rectangle.hpp"
#include "ui/text_layout_options.hpp"
#include "ui/vector.hpp"

#include <windows.h>

#include <dwrite.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace windower::ui
{

class context;
class text_layout_engine;
class text_state;

class text_position
{
public:
    constexpr text_position() noexcept = default;
    constexpr text_position(std::size_t position, bool is_trailing) noexcept :
        m_position{position}, m_is_trailing{is_trailing}
    {}

    constexpr std::size_t position() noexcept { return m_position; }
    constexpr std::size_t is_trailing() noexcept { return m_is_trailing; }

private:
    std::size_t m_position{0};
    bool m_is_trailing{false};
};

class text_layout
{
public:
    text_layout() noexcept                   = default;
    text_layout(text_layout const&) noexcept = default;
    text_layout(text_layout&&) noexcept      = default;

    ~text_layout() noexcept = default;

    text_layout& operator=(text_layout const&) noexcept = default;
    text_layout& operator=(text_layout&&) noexcept = default;

    dimension const& size() const noexcept;
    rectangle const& metric_bounds() const noexcept;

    text_position hit_test(vector) const noexcept;
    text_position next_position(text_position) const noexcept;
    text_position previous_position(text_position) const noexcept;

private:
    ::IDWriteTextLayout* m_ptr{nullptr};
    dimension m_size;
    rectangle m_metric_bounds;
    rectangle m_draw_bounds;
    vector m_offset;

    friend class text_layout_engine;
    friend class text_rasterizer;
};

class text_layout_engine
{
public:
    text_layout layout(
        context& ctx, std::u8string_view text, dimension const& size,
        text_layout_options const& options = {}) noexcept;

    text_layout layout(
        context& ctx, std::span<std::u8string_view> text, dimension const& size,
        text_layout_options const& options = {}) noexcept;

private:
    class descriptor
    {
    public:
        descriptor(
            dimension const&, text_layout_options const&,
            std::span<std::u8string_view>) noexcept;

        dimension const& size() const noexcept;
        text_layout_options const& options() const noexcept;
        std::span<std::u8string_view> segments() const noexcept;

    private:
        dimension const& m_size;
        text_layout_options const& m_options;
        std::span<std::u8string_view> m_segments;

        friend class text_layout_engine;
    };

    class cache_entry
    {
    public:
        dimension const& size() const noexcept;
        text_layout_options const& options() const noexcept;
        std::span<std::u8string const> segments() const noexcept;

    private:
        text_layout m_layout;
        text_layout_options m_options;
        std::vector<std::u8string> m_segments;
        winrt::com_ptr<::IDWriteTextLayout> m_layout_ptr;
        bool m_used{false};

        friend class text_layout_engine;
    };

    std::vector<cache_entry> m_cache;

    winrt::com_ptr<::IDWriteTextFormat> m_base_format;

    inline_bitmap::descriptor m_element_fire;
    inline_bitmap::descriptor m_element_ice;
    inline_bitmap::descriptor m_element_wind;
    inline_bitmap::descriptor m_element_earth;
    inline_bitmap::descriptor m_element_lightning;
    inline_bitmap::descriptor m_element_water;
    inline_bitmap::descriptor m_element_light;
    inline_bitmap::descriptor m_element_dark;
    inline_bitmap::descriptor m_auto_translate_begin;
    inline_bitmap::descriptor m_auto_translate_end;

    text_layout_engine() = default;

    void initialize(context& ctx) noexcept;
    void update() noexcept;

    friend class context;
};

class text_state : public com_base<::IUnknown>
{
public:
    // IUnknown
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void** ppvObject) noexcept final;

    static void set_fill_color(
        gsl::not_null<::IDWriteTextLayout*> layout, color color,
        std::size_t start, std::size_t length) noexcept;
    static void set_underline_color(
        gsl::not_null<::IDWriteTextLayout*> layout, color color,
        std::size_t start, std::size_t length) noexcept;
    static void set_strike_color(
        gsl::not_null<::IDWriteTextLayout*> layout, color color,
        std::size_t start, std::size_t length) noexcept;
    static void set_stroke_width(
        gsl::not_null<::IDWriteTextLayout*> layout, float width,
        std::size_t start, std::size_t length) noexcept;
    static void set_stroke_color(
        gsl::not_null<::IDWriteTextLayout*> layout, color color,
        std::size_t start, std::size_t length) noexcept;

    static color
    get_fill_color(::IUnknown* effect, color default_color) noexcept;
    static color
    get_underline_color(::IUnknown* effect, color default_color) noexcept;
    static color
    get_strike_color(::IUnknown* effect, color default_color) noexcept;
    static float
    get_stroke_width(::IUnknown* effect, float default_width) noexcept;
    static color
    get_stroke_color(::IUnknown* effect, color default_color) noexcept;

private:
    bool m_has_fill_color{false};
    bool m_has_underline_color{false};
    bool m_has_strike_color{false};

    color m_fill_color{colors::transparent};
    color m_underline_color{colors::transparent};
    color m_strike_color{colors::transparent};

    float m_stroke_width{0.f};
    color m_stroke_color{colors::transparent};

    text_state() noexcept = default;

    static winrt::com_ptr<text_state> convert(::IUnknown*) noexcept;
    static winrt::com_ptr<text_state>
        clone(gsl::not_null<::IDWriteTextLayout*>, std::size_t) noexcept;
};

}

#endif
