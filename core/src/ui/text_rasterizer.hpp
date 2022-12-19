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

#ifndef WINDOWER_UI_TEXT_RASTERIZER_HPP
#define WINDOWER_UI_TEXT_RASTERIZER_HPP

#include "ui/color.hpp"
#include "ui/dimension.hpp"
#include "ui/patch.hpp"
#include "ui/rectangle.hpp"
#include "ui/text_rasterization_options.hpp"
#include "ui/texture.hpp"

#include <windows.h>

#include <d2d1.h>
#include <dwrite_2.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <cstddef>

namespace windower::ui
{

class context;
class text_layout;

class text_rasterizer
{
public:
    std::size_t chunk_count(
        context const& ctx, text_layout const& layout,
        text_rasterization_options const& options = {}) const noexcept;
    texture rasterize(
        context& ctx, text_layout const& layout, std::size_t chunk,
        text_rasterization_options const& options = {}) noexcept;

private:
    winrt::com_ptr<::ID2D1Factory> m_d2d_factory;
    winrt::com_ptr<::IDWriteFactory> m_dwrite_factory;
    winrt::com_ptr<::IDWriteFactory2> m_dwrite_factory2;

    winrt::com_ptr<::IDWriteTextRenderer> m_fill_rasterizer;
    winrt::com_ptr<::IDWriteTextRenderer> m_stroke_rasterizer;

    bool m_pow2_textures = false;
    float m_chunk_width  = 0;
    float m_chunk_height = 0;

    void initialize(context&) noexcept;

    friend class context;
};

struct text_rasterizer_context
{
    gsl::not_null<text_rasterization_options const*> options;

    gsl::not_null<::ID2D1RenderTarget*> render_target;
    gsl::not_null<::ID2D1SolidColorBrush*> brush;

    ::IDWriteFactory2* dwrite_factory2;
};

}

#endif
