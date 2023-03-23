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

#include "ui/text_rasterizer.hpp"

#include "ui/bitmap.hpp"
#include "ui/context.hpp"
#include "ui/dwrite_iids.hpp"
#include "ui/patch.hpp"
#include "ui/text_layout_engine.hpp"
#include "utility.hpp"

#include <d2d1.h>
#include <dwrite_2.h>

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <cmath>

namespace windower::ui
{
namespace
{

class rasterizer_base : public com_base<::IDWriteTextRenderer>
{
public:
    // IUnknown
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void** ppvObject) noexcept final
    {
        if (!ppvObject)
        {
            return E_POINTER;
        }

        if (::IsEqualGUID(riid, ::IID_IDWriteTextRenderer))
        {
            ::IDWriteTextRenderer* const result = this;
            *ppvObject                          = result;
            AddRef();
            return S_OK;
        }
        else if (::IsEqualGUID(riid, ::IID_IDWritePixelSnapping))
        {
            ::IDWritePixelSnapping* const result = this;
            *ppvObject                           = result;
            AddRef();
            return S_OK;
        }

        return com_base::QueryInterface(riid, ppvObject);
    }

    // IDWritePixelSnapping
    ::HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
        void* clientDrawingContext, ::BOOL* isDisabled) noexcept final
    {
        if (!clientDrawingContext || !isDisabled)
        {
            return E_POINTER;
        }

        *isDisabled = false;
        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE GetCurrentTransform(
        void* clientDrawingContext, ::DWRITE_MATRIX* transform) noexcept final
    {
        if (!clientDrawingContext || !transform)
        {
            return E_POINTER;
        }

        ::D2D1_MATRIX_3X2_F d2d_transform;
        static_cast<text_rasterizer_context*>(clientDrawingContext)
            ->render_target->GetTransform(&d2d_transform);

        transform->m11 = d2d_transform.m11;
        transform->m12 = d2d_transform.m12;
        transform->m21 = d2d_transform.m21;
        transform->m22 = d2d_transform.m22;
        transform->dx  = d2d_transform.dx;
        transform->dy  = d2d_transform.dy;

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
        void* clientDrawingContext, FLOAT* pixelsPerDip) noexcept final
    {
        if (!clientDrawingContext || !pixelsPerDip)
        {
            return E_POINTER;
        }

        ::FLOAT dpiX, dpiY;
        static_cast<text_rasterizer_context*>(clientDrawingContext)
            ->render_target->GetDpi(&dpiX, &dpiY);
        *pixelsPerDip = std::max(dpiX, dpiY) / 96.f;

        return S_OK;
    }
};

class fill_rasterizer : public rasterizer_base
{
public:
    static void create(
        gsl::not_null<::ID2D1Factory*> m_dwrite_factory,
        gsl::not_null<::IDWriteTextRenderer**> ppvObject) noexcept
    {
#pragma warning(push)
#pragma warning(disable : 28193)
        *ppvObject = new (std::nothrow) fill_rasterizer{m_dwrite_factory};
#pragma warning(pop)
    }

    // IDWriteTextRenderer
    ::HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_MEASURING_MODE measuringMode,
        ::DWRITE_GLYPH_RUN const* glyphRun,
        ::DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !glyphRun || !glyphRunDescription)
        {
            return E_POINTER;
        }

        auto const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        auto const render_target = context->render_target;
        auto const brush         = context->brush;

        auto const fill_color = text_state::get_fill_color(
            clientDrawingEffect, context->options->fill_color);
        auto const stroke_width = text_state::get_stroke_width(
            clientDrawingEffect, context->options->stroke_width);
        auto const stroke_color = text_state::get_stroke_color(
            clientDrawingEffect, context->options->stroke_color);

        auto const has_stroke = stroke_width > 0.f && stroke_color.a > 0;

        winrt::com_ptr<::IDWriteColorGlyphRunEnumerator> enumerator;
        if (auto factory = context->dwrite_factory2;
            factory &&
            SUCCEEDED(factory->TranslateColorGlyphRun(
                baselineOriginX, baselineOriginY, glyphRun, glyphRunDescription,
                measuringMode, nullptr, 0, enumerator.put())))
        {
            ::BOOL more;
            enumerator->MoveNext(&more);
            while (more)
            {
                ::DWRITE_COLOR_GLYPH_RUN const* run;
                if (SUCCEEDED(enumerator->GetCurrentRun(&run)))
                {
                    if (run->paletteIndex == 0xFFFF)
                    {
                        brush->SetColor(fill_color);
                    }
                    else
                    {
                        brush->SetColor(run->runColor);
                    }
                    draw(
                        render_target, brush, false, &run->glyphRun,
                        run->baselineOriginX, run->baselineOriginY,
                        measuringMode);
                }
                enumerator->MoveNext(&more);
            }
        }
        else
        {
            brush->SetColor(fill_color);
            draw(
                render_target, brush, has_stroke, glyphRun, baselineOriginX,
                baselineOriginY, measuringMode);
        }

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawUnderline(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_UNDERLINE const* underline,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !underline)
        {
            return E_POINTER;
        }

        auto const* const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        auto const render_target = context->render_target;
        auto const brush         = context->brush;

        auto const fill_color = text_state::get_underline_color(
            clientDrawingEffect, context->options->fill_color);

        ::FLOAT dpiX, dpiY;
        static_cast<text_rasterizer_context*>(clientDrawingContext)
            ->render_target->GetDpi(&dpiX, &dpiY);

        auto rect   = ::D2D1_RECT_F{};
        rect.left   = baselineOriginX;
        rect.top    = baselineOriginY + underline->offset;
        rect.right  = rect.left + underline->width;
        rect.bottom = rect.top + underline->thickness;

        rect.left   = std::round(rect.left * dpiX / 96.f);
        rect.top    = std::round(rect.top * dpiY / 96.f);
        rect.right  = std::round(rect.right * dpiX / 96.f);
        rect.bottom = std::round(rect.bottom * dpiY / 96.f);

        rect.bottom = std::max(rect.bottom, rect.top + 1.f);

        rect.left *= 96.f / dpiX;
        rect.top *= 96.f / dpiY;
        rect.right *= 96.f / dpiX;
        rect.bottom *= 96.f / dpiY;

        brush->SetColor(fill_color);
        render_target->FillRectangle(rect, brush);

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_STRIKETHROUGH const* strikethrough,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !strikethrough)
        {
            return E_POINTER;
        }

        auto const* const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        auto const render_target = context->render_target;
        auto const brush         = context->brush;

        auto const fill_color = text_state::get_strike_color(
            clientDrawingEffect, context->options->fill_color);

        auto rect   = ::D2D1_RECT_F{};
        rect.left   = baselineOriginX;
        rect.top    = baselineOriginY + strikethrough->offset;
        rect.right  = rect.left + strikethrough->width;
        rect.bottom = rect.top + strikethrough->thickness;

        ::FLOAT dpiX, dpiY;
        static_cast<text_rasterizer_context*>(clientDrawingContext)
            ->render_target->GetDpi(&dpiX, &dpiY);
        rect.left   = std::round(rect.left * dpiX / 96.f);
        rect.top    = std::round(rect.top * dpiY / 96.f);
        rect.right  = std::round(rect.right * dpiX / 96.f);
        rect.bottom = std::round(rect.bottom * dpiY / 96.f);

        rect.bottom = std::max(rect.bottom, rect.top + 1.f);

        rect.left *= 96.f / dpiX;
        rect.top *= 96.f / dpiY;
        rect.right *= 96.f / dpiX;
        rect.bottom *= 96.f / dpiY;

        brush->SetColor(fill_color);
        render_target->FillRectangle(rect, brush);

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawInlineObject(
        void* clientDrawingContext, ::FLOAT originX, ::FLOAT originY,
        ::IDWriteInlineObject* inlineObject, ::BOOL isSideways,
        ::BOOL isRightToLeft, ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!inlineObject)
        {
            return E_POINTER;
        }

        return inlineObject->Draw(
            clientDrawingContext, this, originX, originY, isSideways,
            isRightToLeft, clientDrawingEffect);
    }

private:
    ::ID2D1Factory* m_d2d_factory;

    fill_rasterizer(gsl::not_null<::ID2D1Factory*> d2d_factory) noexcept :
        m_d2d_factory{d2d_factory}
    {}

    ::HRESULT draw(
        gsl::not_null<::ID2D1RenderTarget*> render_target,
        gsl::not_null<::ID2D1SolidColorBrush*> brush, bool has_stroke,
        gsl::not_null<::DWRITE_GLYPH_RUN const*> glyph_run, ::FLOAT baseline_x,
        ::FLOAT baseline_y, ::DWRITE_MEASURING_MODE measuring_mode) noexcept
    {
        if (has_stroke)
        {
            winrt::com_ptr<::ID2D1PathGeometry> path;
            m_d2d_factory->CreatePathGeometry(path.put());

            winrt::com_ptr<::ID2D1GeometrySink> sink;
            path->Open(sink.put());

            glyph_run->fontFace->GetGlyphRunOutline(
                glyph_run->fontEmSize, glyph_run->glyphIndices,
                glyph_run->glyphAdvances, glyph_run->glyphOffsets,
                glyph_run->glyphCount, glyph_run->isSideways,
                glyph_run->bidiLevel % 2 == 1, sink.get());
            sink->Close();

            winrt::com_ptr<::ID2D1TransformedGeometry> transformed_path;
            m_d2d_factory->CreateTransformedGeometry(
                path.get(),
                D2D1::Matrix3x2F::Translation(baseline_x, baseline_y),
                transformed_path.put());

            render_target->FillGeometry(transformed_path.get(), brush);
        }
        else
        {
            render_target->DrawGlyphRun(
                {baseline_x, baseline_y}, glyph_run, brush, measuring_mode);
        }

        return S_OK;
    }
};

class stroke_rasterizer : public rasterizer_base
{
public:
    static void create(
        gsl::not_null<::ID2D1Factory*> m_dwrite_factory,
        gsl::not_null<::IDWriteTextRenderer**> ppvObject) noexcept
    {
#pragma warning(push)
#pragma warning(disable : 28193)
        *ppvObject = new (std::nothrow) stroke_rasterizer{m_dwrite_factory};
#pragma warning(pop)
    }

    // IDWriteTextRenderer
    ::HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_MEASURING_MODE measuringMode,
        ::DWRITE_GLYPH_RUN const* glyphRun,
        ::DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !glyphRun || !glyphRunDescription)
        {
            return E_POINTER;
        }

        auto const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        winrt::com_ptr<::IDWriteColorGlyphRunEnumerator> enumerator;
        if (auto factory = context->dwrite_factory2;
            !factory ||
            FAILED(factory->TranslateColorGlyphRun(
                baselineOriginX, baselineOriginY, glyphRun, glyphRunDescription,
                measuringMode, nullptr, 0, enumerator.put())))
        {
            auto const render_target = context->render_target;
            auto const brush         = context->brush;

            auto const stroke_width = text_state::get_stroke_width(
                clientDrawingEffect, context->options->stroke_width);
            auto const stroke_color = text_state::get_stroke_color(
                clientDrawingEffect, context->options->stroke_color);

            if (stroke_width > 0.f && stroke_color.a > 0)
            {
                winrt::com_ptr<::ID2D1GeometrySink> sink;

                winrt::com_ptr<::ID2D1PathGeometry> outline_path;
                winrt::com_ptr<::ID2D1PathGeometry> widened_path;

                m_d2d_factory->CreatePathGeometry(outline_path.put());
                m_d2d_factory->CreatePathGeometry(widened_path.put());

                outline_path->Open(sink.put());
                glyphRun->fontFace->GetGlyphRunOutline(
                    glyphRun->fontEmSize, glyphRun->glyphIndices,
                    glyphRun->glyphAdvances, glyphRun->glyphOffsets,
                    glyphRun->glyphCount, glyphRun->isSideways,
                    glyphRun->bidiLevel % 2 == 1, sink.get());
                sink->Close();
                sink = nullptr;

                widened_path->Open(sink.put());
                outline_path->Widen(
                    2.f * stroke_width, m_stroke_style.get(), {}, sink.get());
                sink->Close();
                sink = nullptr;

                winrt::com_ptr<::ID2D1TransformedGeometry> transformed_path;
                m_d2d_factory->CreateTransformedGeometry(
                    widened_path.get(),
                    D2D1::Matrix3x2F::Translation(
                        baselineOriginX, baselineOriginY),
                    transformed_path.put());

                brush->SetColor(stroke_color);
                render_target->FillGeometry(transformed_path.get(), brush);
            }
        }

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawUnderline(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_UNDERLINE const* underline,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !underline)
        {
            return E_POINTER;
        }

        auto const* const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        auto const render_target = context->render_target;
        auto const brush         = context->brush;

        auto const stroke_width = text_state::get_stroke_width(
            clientDrawingEffect, context->options->stroke_width);
        auto const stroke_color = text_state::get_stroke_color(
            clientDrawingEffect, context->options->stroke_color);

        if (stroke_width > 0.f && stroke_color.a > 0)
        {
            auto rect   = ::D2D1_RECT_F{};
            rect.left   = baselineOriginX;
            rect.top    = baselineOriginY + underline->offset;
            rect.right  = rect.left + underline->width;
            rect.bottom = rect.top + underline->thickness;

            ::FLOAT dpiX, dpiY;
            static_cast<text_rasterizer_context*>(clientDrawingContext)
                ->render_target->GetDpi(&dpiX, &dpiY);
            rect.left   = std::round(rect.left * dpiX / 96.f);
            rect.top    = std::round(rect.top * dpiY / 96.f);
            rect.right  = std::round(rect.right * dpiX / 96.f);
            rect.bottom = std::round(rect.bottom * dpiY / 96.f);

            rect.bottom = std::max(rect.bottom, rect.top + 1.f);

            rect.left *= 96.f / dpiX;
            rect.top *= 96.f / dpiY;
            rect.right *= 96.f / dpiX;
            rect.bottom *= 96.f / dpiY;

            rect.left -= stroke_width;
            rect.top -= stroke_width;
            rect.right += stroke_width;
            rect.bottom += stroke_width;

            auto stroke    = ::D2D1_ROUNDED_RECT{};
            stroke.rect    = rect;
            stroke.radiusX = stroke_width;
            stroke.radiusY = stroke_width;

            brush->SetColor(stroke_color);
            render_target->FillRoundedRectangle(stroke, brush);
        }

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        void* clientDrawingContext, ::FLOAT baselineOriginX,
        ::FLOAT baselineOriginY, ::DWRITE_STRIKETHROUGH const* strikethrough,
        ::IUnknown* clientDrawingEffect) noexcept final
    {
        if (!clientDrawingContext || !strikethrough)
        {
            return E_POINTER;
        }

        auto const* const context =
            static_cast<text_rasterizer_context*>(clientDrawingContext);

        auto const render_target = context->render_target;
        auto const brush         = context->brush;

        auto const stroke_width = text_state::get_stroke_width(
            clientDrawingEffect, context->options->stroke_width);
        auto const stroke_color = text_state::get_stroke_color(
            clientDrawingEffect, context->options->stroke_color);

        if (stroke_width > 0.f && stroke_color.a > 0)
        {
            auto rect   = ::D2D1_RECT_F{};
            rect.left   = baselineOriginX;
            rect.top    = baselineOriginY + strikethrough->offset;
            rect.right  = rect.left + strikethrough->width;
            rect.bottom = rect.top + strikethrough->thickness;

            ::FLOAT dpiX, dpiY;
            static_cast<text_rasterizer_context*>(clientDrawingContext)
                ->render_target->GetDpi(&dpiX, &dpiY);
            rect.left   = std::round(rect.left * dpiX / 96.f);
            rect.top    = std::round(rect.top * dpiY / 96.f);
            rect.right  = std::round(rect.right * dpiX / 96.f);
            rect.bottom = std::round(rect.bottom * dpiY / 96.f);

            rect.bottom = std::max(rect.bottom, rect.top + 1.f);

            rect.left *= 96.f / dpiX;
            rect.top *= 96.f / dpiY;
            rect.right *= 96.f / dpiX;
            rect.bottom *= 96.f / dpiY;

            rect.left -= stroke_width;
            rect.top -= stroke_width;
            rect.right += stroke_width;
            rect.bottom += stroke_width;

            auto stroke    = ::D2D1_ROUNDED_RECT{};
            stroke.rect    = rect;
            stroke.radiusX = stroke_width;
            stroke.radiusY = stroke_width;

            brush->SetColor(stroke_color);
            render_target->FillRoundedRectangle(stroke, brush);
        }

        return S_OK;
    }

    ::HRESULT STDMETHODCALLTYPE DrawInlineObject(
        [[maybe_unused]] void* clientDrawingContext,
        [[maybe_unused]] ::FLOAT originX, [[maybe_unused]] ::FLOAT originY,
        [[maybe_unused]] ::IDWriteInlineObject* inlineObject,
        [[maybe_unused]] ::BOOL isSideways,
        [[maybe_unused]] ::BOOL isRightToLeft,
        [[maybe_unused]] ::IUnknown* clientDrawingEffect) noexcept final
    {
        return S_OK;
    }

private:
    ::ID2D1Factory* m_d2d_factory;

    winrt::com_ptr<::ID2D1StrokeStyle> m_stroke_style;

    stroke_rasterizer(gsl::not_null<::ID2D1Factory*> d2d_factory) noexcept :
        m_d2d_factory{d2d_factory}
    {
        d2d_factory->CreateStrokeStyle(
            {D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT,
             D2D1_LINE_JOIN_ROUND, 0.f, D2D1_DASH_STYLE_SOLID, 0.f},
            nullptr, 0, m_stroke_style.put());
    }
};

}

void text_rasterizer::initialize(context& ctx) noexcept
{
    ::D2D1CreateFactory(
        ::D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2d_factory.put());
    fill_rasterizer::create(m_d2d_factory.get(), m_fill_rasterizer.put());
    stroke_rasterizer::create(m_d2d_factory.get(), m_stroke_rasterizer.put());

    ::D3DCAPS8 caps;
    ctx.d3d_device()->GetDeviceCaps(&caps);
    m_pow2_textures =
        (caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0 ||
        (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0;
    m_chunk_width  = caps.MaxTextureWidth - 2.f;
    m_chunk_height = caps.MaxTextureHeight - 2.f;
}

std::size_t text_rasterizer::chunk_count(
    context const& ctx, text_layout const& layout,
    text_rasterization_options const& options) const noexcept
{
    auto const scale = ctx.scale_factor();

    auto draw_l = layout.m_draw_bounds.x0 + layout.m_offset.x;
    auto draw_t = layout.m_draw_bounds.y0 + layout.m_offset.y;
    auto draw_r = layout.m_draw_bounds.x1 + layout.m_offset.x;
    auto draw_b = layout.m_draw_bounds.y1 + layout.m_offset.y;

    if (has_flag(options.flags, text_rasterization_flags::clip_to_bounds))
    {
        draw_l = std::max(draw_l, 0.f);
        draw_t = std::max(draw_t, 0.f);
        draw_r = std::min(draw_r, layout.m_size.width);
        draw_b = std::min(draw_b, layout.m_size.height);
    }

    draw_l = std::floor(draw_l * scale.x);
    draw_t = std::floor(draw_t * scale.y);
    draw_r = std::ceil(draw_r * scale.x);
    draw_b = std::ceil(draw_b * scale.y);

    auto const w = draw_r - draw_l;
    auto const h = draw_b - draw_t;

    auto const chunk_w = std::ceil(w / m_chunk_width);
    auto const chunk_h = std::ceil(h / m_chunk_height);

    return gsl::narrow_cast<std::size_t>(chunk_w * chunk_h);
}

texture text_rasterizer::rasterize(
    context& ctx, text_layout const& layout, std::size_t chunk,
    text_rasterization_options const& options) noexcept
{
    auto const scale = ctx.scale_factor();

    auto desc = texture_cache::descriptor{texture_type::text, u8""};
    desc.set_integer(0, std::bit_cast<std::intptr_t>(layout.m_ptr));
    desc.set_integer(1, gsl::narrow_cast<std::int32_t>(options.fill_color));
    desc.set_integer(2, gsl::narrow_cast<std::int32_t>(options.stroke_color));
    desc.set_integer(4, gsl::narrow_cast<std::int32_t>(options.flags));
    desc.set_integer(5, chunk);
    desc.set_float(0, scale.x);
    desc.set_float(1, scale.y);
    desc.set_float(2, options.stroke_width);

    return ctx.texture_cache().get(desc, 1, [&]() noexcept -> texture {
        auto draw_l = layout.m_draw_bounds.x0 + layout.m_offset.x;
        auto draw_t = layout.m_draw_bounds.y0 + layout.m_offset.y;
        auto draw_r = layout.m_draw_bounds.x1 + layout.m_offset.x;
        auto draw_b = layout.m_draw_bounds.y1 + layout.m_offset.y;

        if (has_flag(options.flags, text_rasterization_flags::clip_to_bounds))
        {
            draw_l = std::max(draw_l, 0.f);
            draw_t = std::max(draw_t, 0.f);
            draw_r = std::min(draw_r, layout.m_size.width);
            draw_b = std::min(draw_b, layout.m_size.height);
        }

        draw_l = std::floor(draw_l * scale.x);
        draw_t = std::floor(draw_t * scale.y);
        draw_r = std::ceil(draw_r * scale.x);
        draw_b = std::ceil(draw_b * scale.y);

        auto w = draw_r - draw_l;
        auto h = draw_b - draw_t;

        // HACK: add room for cursor
        w += 1;

        auto const chunk_w =
            gsl::narrow_cast<std::size_t>(std::ceil(w / m_chunk_width));

        auto const chunk_x = chunk % chunk_w * m_chunk_width;
        auto const chunk_y = chunk / chunk_w * m_chunk_height;

        w = std::min(w - chunk_x, m_chunk_width);
        h = std::min(h - chunk_y, m_chunk_height);

        auto texture_w = gsl::narrow_cast<std::uint32_t>(w) + 2;
        auto texture_h = gsl::narrow_cast<std::uint32_t>(h) + 2;
        if (m_pow2_textures)
        {
            texture_w = std::bit_ceil(texture_w);
            texture_h = std::bit_ceil(texture_h);
        }

        auto const offset_x = chunk_x + draw_l;
        auto const offset_y = chunk_y + draw_t;

        auto const draw_offset_x = (1 - offset_x) / scale.x + layout.m_offset.x;
        auto const draw_offset_y = (1 - offset_y) / scale.y + layout.m_offset.y;

        winrt::com_ptr<::IWICBitmap> bitmap_ptr;
        ctx.wic_factory()->CreateBitmap(
            texture_w, texture_h, ::GUID_WICPixelFormat32bppPBGRA,
            ::WICBitmapCacheOnDemand, bitmap_ptr.put());

        ::D2D1_RENDER_TARGET_PROPERTIES const properties = {
            ::D2D1_RENDER_TARGET_TYPE_DEFAULT,
            {::DXGI_FORMAT_B8G8R8A8_UNORM, ::D2D1_ALPHA_MODE_PREMULTIPLIED},
            96.f * scale.x,
            96.f * scale.y,
            ::D2D1_RENDER_TARGET_USAGE_NONE,
            ::D2D1_FEATURE_LEVEL_DEFAULT};
        winrt::com_ptr<::ID2D1RenderTarget> render_target;
        m_d2d_factory->CreateWicBitmapRenderTarget(
            bitmap_ptr.get(), &properties, render_target.put());
        winrt::com_ptr<::IDWriteRenderingParams> parameters;
        ctx.dwrite_factory()->CreateCustomRenderingParams(
            2.2f, .5f, 0.f, ::DWRITE_PIXEL_GEOMETRY_FLAT,
            ::DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC, parameters.put());
        render_target->SetTextRenderingParams(parameters.get());
        render_target->SetTextAntialiasMode(
            ::D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

        render_target->BeginDraw();
        render_target->Clear();

        winrt::com_ptr<::ID2D1SolidColorBrush> brush;
        render_target->CreateSolidColorBrush({}, brush.put());

        text_rasterizer_context rasterizer_context{
            .options         = &options,
            .render_target   = render_target.get(),
            .brush           = brush.get(),
            .dwrite_factory2 = m_dwrite_factory2.get()};
        layout.m_ptr->Draw(
            &rasterizer_context, m_stroke_rasterizer.get(), draw_offset_x,
            draw_offset_y);
        layout.m_ptr->Draw(
            &rasterizer_context, m_fill_rasterizer.get(), draw_offset_x,
            draw_offset_y);

        if (has_flag(options.flags, text_rasterization_flags::show_cursor))
        {
            auto const cursor_position = gsl::narrow_cast<::UINT32>(-1);

            ::DWRITE_HIT_TEST_METRICS primary_cursor{};

            auto x = ::FLOAT{};
            auto y = ::FLOAT{};
            layout.m_ptr->HitTestTextPosition(
                cursor_position, false, &x, &y, &primary_cursor);

            primary_cursor.left  = x;
            primary_cursor.width = 1.f;

            ::D2D1_RECT_F rect{};
            // HACK: This gets the height of the cursor entirely wrong
            rect.left  = draw_offset_x + std::round(primary_cursor.left);
            rect.top   = draw_offset_y + std::round(primary_cursor.top) + 1.f;
            rect.right = draw_offset_x +
                         std::round(primary_cursor.left + primary_cursor.width);
            rect.bottom =
                draw_offset_y +
                std::round(primary_cursor.top + primary_cursor.height) + 1.f;

            render_target->FillRectangle(rect, brush.get());
        }

        render_target->EndDraw();

        auto const patch = ui::patch{
            {1, 1, w + 1, h + 1},
            {-offset_x, -offset_y, offset_x, offset_y},
            {gsl::narrow_cast<float>(texture_w),
             gsl::narrow_cast<float>(texture_h)}};
        auto const bitmap = ui::bitmap{bitmap_ptr, patch};

        if (auto texture_ptr = texture_cache::allocate(
                ctx, patch.texture_size, D3DFMT_A8R8G8B8);
            texture_ptr && bitmap.copy_to(texture_ptr.get()))
        {
            return {texture_token{texture_ptr.detach()}, patch};
        }

        return {};
    });
}

}
