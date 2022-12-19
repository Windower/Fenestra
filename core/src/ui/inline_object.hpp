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

#ifndef WINDOWER_UI_INLINE_OBJECT_HPP
#define WINDOWER_UI_INLINE_OBJECT_HPP

#include "ui/bitmap.hpp"
#include "ui/com_base.hpp"
#include "ui/rectangle.hpp"

#include <windows.h>

#include <dwrite.h>
#include <guiddef.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <optional>

namespace windower::ui
{

class inline_object_base : public com_base<::IDWriteInlineObject>
{
public:
    // IUnknown
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void** ppvObject) noexcept final;

private:
};

class inline_bitmap : public inline_object_base
{
public:
    class descriptor
    {
    public:
        descriptor() noexcept = default;

        descriptor(bitmap& bitmap) noexcept;

        descriptor(bitmap& bitmap, rectangle const& bounds) noexcept;

        descriptor(
            bitmap& bitmap, rectangle const& bounds, float baseline) noexcept;

        descriptor(
            bitmap& bitmap, rectangle const& bounds, float baseline,
            ::DWRITE_BREAK_CONDITION break_before,
            ::DWRITE_BREAK_CONDITION m_break_after) noexcept;

    private:
        winrt::com_ptr<::IWICBitmapSource> m_bitmap;
        rectangle m_bounds;
        float m_baseline = 0.f;
        ::DWRITE_BREAK_CONDITION m_break_before =
            ::DWRITE_BREAK_CONDITION_NEUTRAL;
        ::DWRITE_BREAK_CONDITION m_break_after =
            ::DWRITE_BREAK_CONDITION_NEUTRAL;

        friend class inline_bitmap;
    };

    static void insert(
        gsl::not_null<::IDWriteTextLayout*> layout, std::size_t start,
        std::size_t length, descriptor const& descriptor) noexcept;

    // IDWriteInlineObject
    ::HRESULT STDMETHODCALLTYPE Draw(
        void* clientDrawingContext, ::IDWriteTextRenderer* renderer,
        ::FLOAT originX, ::FLOAT originY, ::BOOL isSideways,
        ::BOOL isRightToLeft, ::IUnknown* clientDrawingEffect) noexcept final;

    ::HRESULT STDMETHODCALLTYPE
    GetMetrics(::DWRITE_INLINE_OBJECT_METRICS* metrics) noexcept final;

    ::HRESULT STDMETHODCALLTYPE
    GetOverhangMetrics(::DWRITE_OVERHANG_METRICS* overhangs) noexcept final;

    ::HRESULT STDMETHODCALLTYPE GetBreakConditions(
        ::DWRITE_BREAK_CONDITION* breakConditionBefore,
        ::DWRITE_BREAK_CONDITION* breakConditionAfter) noexcept final;

private:
    dimension m_size;
    descriptor m_descriptor;

    inline_bitmap(dimension size, descriptor descriptor) noexcept;
};

}

#endif
