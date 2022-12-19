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

#include "ui/text_layout_engine.hpp"

#include "ui/bitmap.hpp"
#include "ui/color.hpp"
#include "ui/com_base.hpp"
#include "ui/context.hpp"
#include "ui/inline_object.hpp"
#include "ui/markdown.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <gsl/gsl>

#include <bit>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <numeric>
#include <span>
#include <vector>

namespace windower::ui
{
namespace
{

class cache_order_t
{
public:
    template<typename Left, typename Right>
    constexpr bool operator()(Left&& lhs, Right&& rhs) const noexcept
    {
        if (auto const result = std::strong_order(lhs.size(), rhs.size());
            result != 0)
        {
            return result < 0;
        }
        if (auto const result = std::strong_order(lhs.options(), rhs.options());
            result != 0)
        {
            return result < 0;
        }
        return std::lexicographical_compare(
            lhs.segments().begin(), lhs.segments().end(),
            rhs.segments().begin(), rhs.segments().end());
    }
};

constexpr cache_order_t cache_order;

constexpr auto iid_text_state = ::GUID{
    0xd2800570, 0xa265, 0x478d, 0x8f, 0xa6, 0xe0, 0x7e, 0x18, 0xdb, 0x39, 0x37};

float get_size_value(
    font_size size, gsl::not_null<::IDWriteTextLayout*> layout,
    std::size_t start) noexcept
{
    switch (size.unit)
    {
    case size_unit::absolute: return 12.f * size.value;
    case size_unit::relative: {
        float current;
        layout->GetFontSize(start, &current, nullptr);
        return current * size.value;
    }
    case size_unit::pixel: return size.value;
    case size_unit::point: return size.value * (4.f / 3.f);
    case size_unit::pica: return size.value * 8.f;
    }
    fail_fast();
}

class trimming_string : public inline_object_base
{
public:
    static winrt::com_ptr<::IDWriteInlineObject> create(
        context& ctx, gsl::not_null<::IDWriteTextFormat*> format,
        std::u8string_view string) noexcept
    {
        auto const wstring = to_wstring(string);

        winrt::com_ptr<trimming_string> result;
        result.attach(new (std::nothrow) trimming_string);

        ctx.dwrite_factory()->CreateTextLayout(
            wstring.c_str(), wstring.size(), format, 0.f, 0.f,
            result->m_layout.put());
        result->m_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        result->m_layout->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        result->m_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        ::DWRITE_TEXT_METRICS layout_metrics{};
        result->m_layout->GetMetrics(&layout_metrics);
        result->m_layout->SetMaxWidth(layout_metrics.width);
        ::DWRITE_LINE_METRICS line_metrics;
        ::UINT32 line_count;
        result->m_layout->GetLineMetrics(&line_metrics, 1, &line_count);
        result->m_layout->SetMaxHeight(line_metrics.height);
        result->m_width    = layout_metrics.width;
        result->m_height   = line_metrics.height;
        result->m_baseline = line_metrics.baseline;
        result->m_size     = wstring.size();

        return result;
    }

    // IDWriteInlineObject
    ::HRESULT STDMETHODCALLTYPE Draw(
        void* clientDrawingContext, ::IDWriteTextRenderer* renderer,
        ::FLOAT originX, ::FLOAT originY, [[maybe_unused]] ::BOOL isSideways,
        [[maybe_unused]] ::BOOL isRightToLeft,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        m_layout->SetDrawingEffect(clientDrawingEffect, {0, m_size});
        return m_layout->Draw(clientDrawingContext, renderer, originX, originY);
    }

    ::HRESULT STDMETHODCALLTYPE
    GetMetrics(::DWRITE_INLINE_OBJECT_METRICS* metrics) noexcept final
    {
        if (!metrics)
        {
            return E_POINTER;
        }

        ::DWRITE_TEXT_METRICS layout_metrics{};
        m_layout->GetMetrics(&layout_metrics);
        metrics->width            = m_width;
        metrics->height           = m_height;
        metrics->baseline         = m_baseline;
        metrics->supportsSideways = false;
        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE
    GetOverhangMetrics(::DWRITE_OVERHANG_METRICS* overhangs) noexcept final
    {
        return m_layout->GetOverhangMetrics(overhangs);
    }

    ::HRESULT STDMETHODCALLTYPE GetBreakConditions(
        ::DWRITE_BREAK_CONDITION* breakConditionBefore,
        ::DWRITE_BREAK_CONDITION* breakConditionAfter) noexcept final
    {
        if (!breakConditionBefore || !breakConditionAfter)
        {
            return E_POINTER;
        }

        *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
        *breakConditionAfter  = DWRITE_BREAK_CONDITION_NEUTRAL;
        return S_OK;
    }

private:
    winrt::com_ptr<::IDWriteTextLayout> m_layout;
    ::FLOAT m_width    = 0.f;
    ::FLOAT m_height   = 0.f;
    ::FLOAT m_baseline = 0.f;
    std::size_t m_size = 0;

    trimming_string() noexcept = default;
};

}

dimension const& text_layout::size() const noexcept { return m_size; }

rectangle const& text_layout::metric_bounds() const noexcept
{
    return m_metric_bounds;
}

text_position text_layout::hit_test(vector point) const noexcept
{
    ::BOOL trailing;
    ::BOOL inside;
    ::DWRITE_HIT_TEST_METRICS metrics;
    m_ptr->HitTestPoint(point.x, point.y, &trailing, &inside, &metrics);
    return {metrics.textPosition, trailing == TRUE};
}

text_position text_layout::next_position(text_position position) const noexcept
{
    if (!position.is_trailing())
    {
        return {position.position(), true};
    }

    ::UINT count;
    m_ptr->GetClusterMetrics(nullptr, 0, &count);
    std::vector<::DWRITE_CLUSTER_METRICS> clusters{count};
    m_ptr->GetClusterMetrics(clusters.data(), count, &count);
    std::size_t next_position = 0;
    for (auto const& cluster : clusters)
    {
        next_position += cluster.length;
        if (next_position > position.position())
        {
            return {next_position, true};
        }
    }

    return position;
}

text_position
text_layout::previous_position(text_position position) const noexcept
{
    if (position.is_trailing())
    {
        return {position.position(), false};
    }

    ::UINT count;
    m_ptr->GetClusterMetrics(nullptr, 0, &count);
    std::vector<::DWRITE_CLUSTER_METRICS> clusters{count};
    m_ptr->GetClusterMetrics(clusters.data(), count, &count);
    std::size_t previous_position = 0;
    for (auto const& cluster : clusters)
    {
        auto const temp = previous_position + cluster.length;
        if (temp >= position.position())
        {
            return {previous_position, false};
        }
        previous_position = temp;
    }

    return position;
}

void text_layout_engine::initialize(context& ctx) noexcept
{
    ctx.dwrite_factory()->CreateTextFormat(
        L"Segoe UI", nullptr, ::DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
        ::DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
        ::DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_MEDIUM, 12, L"",
        m_base_format.put());

    auto icons          = bitmap::load(ctx, u8":icons");
    m_element_fire      = {icons, {1, 1, 17, 17}, 14};
    m_element_ice       = {icons, {18, 1, 34, 17}, 14};
    m_element_wind      = {icons, {35, 1, 51, 17}, 14};
    m_element_earth     = {icons, {1, 18, 17, 34}, 14};
    m_element_lightning = {icons, {18, 18, 34, 34}, 14};
    m_element_water     = {icons, {35, 18, 51, 34}, 14};
    m_element_light     = {icons, {1, 35, 17, 51}, 14};
    m_element_dark      = {icons, {18, 35, 34, 51}, 14};

    m_auto_translate_begin = {
        icons,
        {52, 1, 56, 17},
        14,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_NEUTRAL,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_MAY_NOT_BREAK};
    m_auto_translate_end = {
        icons,
        {57, 1, 61, 17},
        14,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_MAY_NOT_BREAK,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_NEUTRAL};
}

void text_layout_engine::update() noexcept
{
    std::erase_if(m_cache, [](auto const& entry) { return !entry.m_used; });
    for (auto& entry : m_cache)
    {
        entry.m_used = false;
    }
}

text_layout text_layout_engine::layout(
    context& ctx, std::u8string_view text, dimension const& size,
    text_layout_options const& options) noexcept
{
    return layout(ctx, std::span{&text, 1}, size, options);
}

text_layout text_layout_engine::layout(
    context& ctx, std::span<std::u8string_view> segments, dimension const& size,
    text_layout_options const& options) noexcept
{
    auto const descriptor =
        text_layout_engine::descriptor{size, options, segments};
    auto it = std::lower_bound(
        m_cache.begin(), m_cache.end(), descriptor, cache_order);
    if (it == m_cache.end() || cache_order(descriptor, *it))
    {
        auto wtext = std::wstring{};
        for (auto segment : segments)
        {
            wtext += to_wstring(segment);
        }

        auto fragments = parse_markdown(wtext);

        for (auto const& fragment : fragments)
        {
            std::visit(
                overloaded{
                    [&](format const&) noexcept {
                        auto const begin = wtext.begin() + fragment.start;
                        auto const end   = begin + fragment.length;
                        std::fill(begin, end, L'\0');
                    },
                    [&](auto const&) noexcept {},
                },
                fragment.data);
        }

        auto const calculate_width  = size.width == dimension::unbounded;
        auto const calculate_height = size.height == dimension::unbounded;

        auto const padding_l = options.padding.left;
        auto const padding_t = options.padding.top;
        auto const padding_r = options.padding.right;
        auto const padding_b = options.padding.bottom;

        auto const padding_h = padding_l + padding_r;
        auto const padding_v = padding_t + padding_b;

        constexpr auto min = 0.f;
        constexpr auto max = std::numeric_limits<float>::max();
        winrt::com_ptr<::IDWriteTextLayout> layout;
        ctx.dwrite_factory()->CreateTextLayout(
            wtext.data(), wtext.size(), m_base_format.get(),
            std::clamp(size.width - padding_h, min, max),
            std::clamp(size.height - padding_v, min, max), layout.put());

        layout->SetWordWrapping(
            gsl::narrow_cast<DWRITE_WORD_WRAPPING>(options.word_wrapping));

        if (options.trimming_granularity != text_trimming_granularity::none)
        {
            auto const trimming = ::DWRITE_TRIMMING{
                .granularity = gsl::narrow_cast<::DWRITE_TRIMMING_GRANULARITY>(
                    options.trimming_granularity),
                .delimiter      = options.trimming_delimiter,
                .delimiterCount = options.trimming_delimiter_count,
            };
            if (options.trimming_string.empty())
            {
                layout->SetTrimming(&trimming, nullptr);
            }
            else
            {
                auto const inline_object = trimming_string::create(
                    ctx, m_base_format.get(), options.trimming_string);
                layout->SetTrimming(&trimming, inline_object.get());
            }
        }

        auto max_stroke_width = 0.f;
        auto has_underline    = false;

        if (options.underline)
        {
            layout->SetUnderline(true, {0, wtext.size()});
            has_underline = true;
        }

        for (auto const& fragment : fragments)
        {
            std::visit(
                overloaded{
                    [&](format const&) noexcept {},
                    [&](typeface const& typeface) noexcept {
                        layout->SetFontFamilyName(
                            typeface.value.c_str(),
                            {fragment.start, fragment.length});
                    },
                    [&](font_size const& size) noexcept {
                        layout->SetFontSize(
                            get_size_value(size, layout.get(), fragment.start),
                            {fragment.start, fragment.length});
                    },
                    [&](stretch const& stretch) noexcept {
                        layout->SetFontStretch(
                            stretch.value, {fragment.start, fragment.length});
                    },
                    [&](weight const& weight) noexcept {
                        layout->SetFontWeight(
                            weight.value, {fragment.start, fragment.length});
                    },
                    [&](style const& style) noexcept {
                        layout->SetFontStyle(
                            style.value, {fragment.start, fragment.length});
                    },
                    [&](strikethrough const& strikethrough) noexcept {
                        layout->SetStrikethrough(
                            strikethrough.value,
                            {fragment.start, fragment.length});
                    },
                    [&](underline const& underline) noexcept {
                        has_underline |= underline.value;
                        layout->SetUnderline(
                            underline.value, {fragment.start, fragment.length});
                    },
                    [&](color const& color) noexcept {
                        text_state::set_fill_color(
                            layout.get(), color, fragment.start,
                            fragment.length);
                    },
                    [&](stroke const& stroke) noexcept {
                        if (stroke.color)
                        {
                            text_state::set_stroke_color(
                                layout.get(), *stroke.color, fragment.start,
                                fragment.length);
                            max_stroke_width = std::max(max_stroke_width, 1.f);
                        }

                        if (stroke.width)
                        {
                            auto const size = get_size_value(
                                *stroke.width, layout.get(), fragment.start);
                            text_state::set_stroke_width(
                                layout.get(), size, fragment.start,
                                fragment.length);
                            max_stroke_width = std::max(max_stroke_width, size);
                        }
                    },
                },
                fragment.data);
        }

        ::DWRITE_TEXT_METRICS metrics{};
        ::DWRITE_OVERHANG_METRICS overhang{};

        if (calculate_width || calculate_height)
        {
            layout->GetMetrics(&metrics);
            if (calculate_width)
            {
                layout->SetMaxWidth(metrics.widthIncludingTrailingWhitespace);
            }
            if (calculate_height)
            {
                layout->SetMaxHeight(metrics.height);
            }
        }

        layout->SetTextAlignment(
            gsl::narrow_cast<::DWRITE_TEXT_ALIGNMENT>(options.alignment));
        layout->SetParagraphAlignment(
            gsl::narrow_cast<::DWRITE_PARAGRAPH_ALIGNMENT>(
                options.vertical_alignment));

        layout->GetMetrics(&metrics);
        layout->GetOverhangMetrics(&overhang);

        auto draw_t = 0.f;
        auto draw_r = metrics.layoutWidth;
        auto draw_b = metrics.layoutHeight;
        auto draw_l = 0.f;

        draw_t -= 1 + max_stroke_width + overhang.top;
        draw_r += 1 + max_stroke_width + overhang.right;
        draw_b += 1 + max_stroke_width + overhang.bottom;
        draw_l -= 1 + max_stroke_width + overhang.left;

        if (has_underline)
        {
            draw_b = std::max(draw_b, metrics.top + metrics.height);
        }

        auto const metric_x = metrics.left;
        auto const metric_y = metrics.top;
        auto const metric_w = metrics.widthIncludingTrailingWhitespace;
        auto const metric_h = metrics.height;

        it                           = m_cache.emplace(it);
        it->m_layout.m_ptr           = layout.get();
        it->m_layout.m_size          = size;
        it->m_layout.m_metric_bounds = {
            metric_x, metric_y, metric_x + metric_w, metric_y + metric_h};
        it->m_layout.m_draw_bounds = {draw_l, draw_t, draw_r, draw_b};
        it->m_layout.m_offset      = {padding_l, padding_t};
        it->m_options              = options;
        it->m_segments             = {segments.begin(), segments.end()};
        it->m_layout_ptr           = std::move(layout);
    }
    it->m_used = true;
    return it->m_layout;
}

::HRESULT STDMETHODCALLTYPE
text_state::QueryInterface(REFIID riid, void** ppvObject) noexcept
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    if (::IsEqualGUID(riid, iid_text_state))
    {
        text_state* const result = this;
        *ppvObject               = result;
        AddRef();
        return S_OK;
    }

    return com_base::QueryInterface(riid, ppvObject);
}

void text_state::set_fill_color(
    gsl::not_null<::IDWriteTextLayout*> layout, color color, std::size_t start,
    std::size_t length) noexcept
{
    winrt::com_ptr<text_state> state = clone(layout, start);
    state->m_has_fill_color          = true;
    state->m_fill_color              = color;
    layout->SetDrawingEffect(state.get(), {start, length});
}

void text_state::set_underline_color(
    gsl::not_null<::IDWriteTextLayout*> layout, color color, std::size_t start,
    std::size_t length) noexcept
{
    winrt::com_ptr<text_state> state = clone(layout, start);
    state->m_has_underline_color     = true;
    state->m_underline_color         = color;
    layout->SetDrawingEffect(state.get(), {start, length});
}

void text_state::set_strike_color(
    gsl::not_null<::IDWriteTextLayout*> layout, color color, std::size_t start,
    std::size_t length) noexcept
{
    winrt::com_ptr<text_state> state = clone(layout, start);
    state->m_has_strike_color        = true;
    state->m_strike_color            = color;
    layout->SetDrawingEffect(state.get(), {start, length});
}

void text_state::set_stroke_width(
    gsl::not_null<::IDWriteTextLayout*> layout, float width, std::size_t start,
    std::size_t length) noexcept
{
    winrt::com_ptr<text_state> state = clone(layout, start);
    state->m_stroke_width            = width;
    layout->SetDrawingEffect(state.get(), {start, length});
}

void text_state::set_stroke_color(
    gsl::not_null<::IDWriteTextLayout*> layout, color color, std::size_t start,
    std::size_t length) noexcept
{
    winrt::com_ptr<text_state> state = clone(layout, start);
    state->m_stroke_color            = color;
    if (state->m_stroke_width <= 0.f)
    {
        state->m_stroke_width = 1.f;
    }
    layout->SetDrawingEffect(state.get(), {start, length});
}

color text_state::get_fill_color(
    ::IUnknown* effect, color default_color) noexcept
{
    if (auto state = convert(effect); state && state->m_has_fill_color)
    {
        return state->m_fill_color;
    }
    return default_color;
}

color text_state::get_underline_color(
    ::IUnknown* effect, color default_color) noexcept
{
    if (auto state = convert(effect))
    {
        if (state->m_has_underline_color)
        {
            return state->m_underline_color;
        }

        if (state->m_has_fill_color)
        {
            return state->m_fill_color;
        }
    }
    return default_color;
}

color text_state::get_strike_color(
    ::IUnknown* effect, color default_color) noexcept
{
    if (auto state = convert(effect))
    {
        if (state->m_has_strike_color)
        {
            return state->m_strike_color;
        }

        if (state->m_has_fill_color)
        {
            return state->m_fill_color;
        }
    }
    return default_color;
}

float text_state::get_stroke_width(
    ::IUnknown* effect, float default_width) noexcept
{
    if (auto state = convert(effect); state)
    {
        return state->m_stroke_width;
    }
    return default_width;
}

color text_state::get_stroke_color(
    ::IUnknown* effect, color default_color) noexcept
{
    if (auto state = convert(effect); state)
    {
        return state->m_stroke_color;
    }
    return default_color;
}

winrt::com_ptr<text_state> text_state::convert(::IUnknown* unknown) noexcept
{
    winrt::com_ptr<text_state> state;
    if (unknown)
    {
        winrt::com_ptr<::IUnknown> temp;
        temp.attach(unknown);
        temp.as(iid_text_state, state.put_void());
        temp.detach();
    }
    return state;
}

winrt::com_ptr<text_state> text_state::clone(
    gsl::not_null<::IDWriteTextLayout*> layout, std::size_t position) noexcept
{
    winrt::com_ptr<text_state> new_state;
    new_state.attach(new (std::nothrow) text_state);

    winrt::com_ptr<::IUnknown> temp;
    layout->GetDrawingEffect(position, temp.put());
    winrt::com_ptr<text_state> old_state;
    if (temp && SUCCEEDED(temp.as(iid_text_state, old_state.put_void())))
    {
        new_state->m_has_fill_color      = old_state->m_has_fill_color;
        new_state->m_has_underline_color = old_state->m_has_underline_color;
        new_state->m_has_strike_color    = old_state->m_has_strike_color;

        new_state->m_fill_color      = old_state->m_fill_color;
        new_state->m_underline_color = old_state->m_underline_color;
        new_state->m_strike_color    = old_state->m_strike_color;

        new_state->m_stroke_width = old_state->m_stroke_width;
        new_state->m_stroke_color = old_state->m_stroke_color;
    }

    return new_state;
}

text_layout_engine::descriptor::descriptor(
    dimension const& size, text_layout_options const& options,
    std::span<std::u8string_view> segments) noexcept :
    m_size{size},
    m_options{options}, m_segments{segments}
{}

dimension const& text_layout_engine::descriptor::size() const noexcept
{
    return m_size;
}

text_layout_options const&
text_layout_engine::descriptor::options() const noexcept
{
    return m_options;
}

std::span<std::u8string_view>
text_layout_engine::descriptor::segments() const noexcept
{
    return m_segments;
}

dimension const& text_layout_engine::cache_entry::size() const noexcept
{
    return m_layout.m_size;
}

text_layout_options const&
text_layout_engine::cache_entry::options() const noexcept
{
    return m_options;
}

std::span<std::u8string const>
text_layout_engine::cache_entry::segments() const noexcept
{
    return m_segments;
}

}
