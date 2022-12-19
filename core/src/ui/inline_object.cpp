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

#include "ui/inline_object.hpp"

#include "ui/com_base.hpp"
#include "ui/dwrite_iids.hpp"
#include "ui/text_rasterizer.hpp"

#include <windows.h>

#include <dwrite.h>
#include <guiddef.h>
#include <objbase.h>
#include <winerror.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <optional>

namespace windower::ui
{

::HRESULT STDMETHODCALLTYPE
inline_object_base::QueryInterface(REFIID riid, void** ppvObject) noexcept
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if (::IsEqualGUID(riid, ::IID_IDWriteInlineObject))
    {
        ::IDWriteInlineObject* const result = this;
        *ppvObject                          = result;
        AddRef();
        return S_OK;
    }

    return com_base::QueryInterface(riid, ppvObject);
}

inline_bitmap::descriptor::descriptor(bitmap& bitmap) noexcept :
    descriptor{bitmap, bitmap.patch().bounds}
{}

inline_bitmap::descriptor::descriptor(
    bitmap& bitmap, rectangle const& bounds) noexcept :
    descriptor{bitmap, bounds, bounds.height()}
{}

inline_bitmap::descriptor::descriptor(
    bitmap& bitmap, rectangle const& bounds, float baseline) noexcept :
    descriptor{
        bitmap, bounds, baseline,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_NEUTRAL,
        ::DWRITE_BREAK_CONDITION::DWRITE_BREAK_CONDITION_NEUTRAL}
{}

inline_bitmap::descriptor::descriptor(
    bitmap& bitmap, rectangle const& bounds, float baseline,
    ::DWRITE_BREAK_CONDITION break_before,
    ::DWRITE_BREAK_CONDITION break_after) noexcept
{
    m_bitmap.copy_from(bitmap.get());
    m_bounds       = bounds;
    m_baseline     = baseline;
    m_break_before = break_before;
    m_break_after  = break_after;
}

void inline_bitmap::insert(
    gsl::not_null<::IDWriteTextLayout*> layout, std::size_t start,
    std::size_t length, descriptor const& descriptor) noexcept
{
    if (length == 0)
    {
        return;
    }

    ::FLOAT font_size;
    layout->GetFontSize(start, &font_size);

    auto const width =
        font_size * descriptor.m_bounds.width() / descriptor.m_bounds.height();
    auto const height = font_size;

    winrt::com_ptr<inline_bitmap> object;
    object.attach(new (std::nothrow)
                      inline_bitmap({width, height}, descriptor));
    layout->SetInlineObject(object.get(), {start, length});
}

inline_bitmap::inline_bitmap(dimension size, descriptor descriptor) noexcept :
    m_size{size}, m_descriptor{descriptor}
{}

::HRESULT STDMETHODCALLTYPE inline_bitmap::Draw(
    void* clientDrawingContext, ::IDWriteTextRenderer* renderer,
    ::FLOAT originX, ::FLOAT originY, [[maybe_unused]] ::BOOL isSideways,
    [[maybe_unused]] ::BOOL isRightToLeft,
    [[maybe_unused]] ::IUnknown* clientDrawingEffect) noexcept
{
    if (!clientDrawingContext || !renderer)
    {
        return E_POINTER;
    }

    auto const* const context =
        static_cast<text_rasterizer_context*>(clientDrawingContext);

    auto const render_target = context->render_target;

    auto x0 = originX;
    auto y0 = originY;
    auto x1 = x0 + m_size.width;
    auto y1 = y0 + m_size.height;

    ::FLOAT dpi_x;
    ::FLOAT dpi_y;
    render_target->GetDpi(&dpi_x, &dpi_y);
    x0 = std::round(x0 * dpi_x / 96.f) * 96.f / dpi_x;
    y0 = std::round(y0 * dpi_y / 96.f) * 96.f / dpi_y;
    x1 = std::round(x1 * dpi_x / 96.f) * 96.f / dpi_x;
    y1 = std::round(y1 * dpi_y / 96.f) * 96.f / dpi_y;

    if (m_descriptor.m_bitmap)
    {
        winrt::com_ptr<::ID2D1Bitmap> d2d_bitmap;
        render_target->CreateBitmapFromWicBitmap(
            m_descriptor.m_bitmap.get(), d2d_bitmap.put());
        render_target->DrawBitmap(
            d2d_bitmap.get(), {x0, y0, x1, y1}, 1.f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            {m_descriptor.m_bounds.x0, m_descriptor.m_bounds.y0,
             m_descriptor.m_bounds.x1, m_descriptor.m_bounds.y1});
    }
    else
    {
        winrt::com_ptr<::ID2D1SolidColorBrush> brush;
        render_target->CreateSolidColorBrush({1.f, 0.f, 1.f, 1.f}, brush.put());
        render_target->FillRectangle({x0, y0, x1, x1}, brush.get());
    }

    return S_OK;
}

::HRESULT STDMETHODCALLTYPE
inline_bitmap::GetMetrics(::DWRITE_INLINE_OBJECT_METRICS* metrics) noexcept
{
    if (!metrics)
    {
        return E_POINTER;
    }

    metrics->width    = m_size.width;
    metrics->height   = m_size.height;
    metrics->baseline = m_descriptor.m_baseline /
                        m_descriptor.m_bounds.height() * m_size.height;
    metrics->supportsSideways = false;

    return S_OK;
}

::HRESULT STDMETHODCALLTYPE
inline_bitmap::GetOverhangMetrics(::DWRITE_OVERHANG_METRICS* overhangs) noexcept
{
    if (!overhangs)
    {
        return E_POINTER;
    }

    *overhangs = {};

    return S_OK;
}

::HRESULT STDMETHODCALLTYPE inline_bitmap::GetBreakConditions(
    ::DWRITE_BREAK_CONDITION* breakConditionBefore,
    ::DWRITE_BREAK_CONDITION* breakConditionAfter) noexcept
{
    if (!breakConditionBefore || !breakConditionAfter)
    {
        return E_POINTER;
    }

    *breakConditionBefore = m_descriptor.m_break_before;
    *breakConditionAfter  = m_descriptor.m_break_after;

    return S_OK;
}

}
