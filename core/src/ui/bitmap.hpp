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

#ifndef WINDOWER_UI_BITMAP_HPP
#define WINDOWER_UI_BITMAP_HPP

#include "guid.hpp"
#include "ui/color.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/patch.hpp"
#include "ui/vector.hpp"

#include <windows.h>

#include <d3d8.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <cstddef>
#include <optional>
#include <span>
#include <variant>

namespace windower::ui
{

class context;

class bitmap
{
public:
    static bitmap load(context& ctx, std::u8string_view name) noexcept;

    bitmap() noexcept = default;
    bitmap(winrt::com_ptr<::IWICBitmapSource>, ui::patch const&) noexcept;

    explicit operator bool() const noexcept;

    guid wic_format() const noexcept;
    ::D3DFORMAT d3d_format() const noexcept;
    bitmap convert(context& ctx, ::WICPixelFormatGUID const&) const noexcept;
    bitmap convert(context& ctx) const noexcept;

    ::IWICBitmapSource* get() const noexcept;
    ui::patch const& patch() const noexcept;
    ui::dimension const& size() const noexcept;
    ui::dimension raw_size() const noexcept;

    color sample(context&, vector const&) const noexcept;
    bool copy_to(::IDirect3DTexture8*) const noexcept;

private:
    winrt::com_ptr<::IWICBitmapSource> m_bitmap;
    ui::patch m_patch;
};

}

#endif
