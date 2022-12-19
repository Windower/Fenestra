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

#include "ui/ffxi_image.hpp"

#include <d3d8.h>

#include <gsl/gsl>

namespace windower::ui
{

namespace
{

constexpr void decode_ffxi_rgba(
    std::span<std::byte> dst, std::size_t dst_offset,
    std::span<std::byte const> src, std::size_t src_offset) noexcept
{
    auto const b_byte = gsl::at(std::as_const(src), src_offset + 0);
    auto const g_byte = gsl::at(std::as_const(src), src_offset + 1);
    auto const r_byte = gsl::at(std::as_const(src), src_offset + 2);
    auto const a_byte = gsl::at(std::as_const(src), src_offset + 3);

    auto a = std::to_integer<int>(a_byte) / 128.f;
    auto r = std::to_integer<int>(r_byte) * a;
    auto g = std::to_integer<int>(g_byte) * a;
    auto b = std::to_integer<int>(b_byte) * a;

    a *= 255;

    b = std::clamp(b, 0.f, 255.f);
    g = std::clamp(g, 0.f, 255.f);
    r = std::clamp(r, 0.f, 255.f);
    a = std::clamp(a, 0.f, 255.f);

    gsl::at(dst, dst_offset + 0) = gsl::narrow_cast<std::byte>(b);
    gsl::at(dst, dst_offset + 1) = gsl::narrow_cast<std::byte>(g);
    gsl::at(dst, dst_offset + 2) = gsl::narrow_cast<std::byte>(r);
    gsl::at(dst, dst_offset + 3) = gsl::narrow_cast<std::byte>(a);
}

}

::D3DFORMAT ffxi_image::d3d_format() const noexcept
{
    switch (m_type)
    {
    default: return ::D3DFMT_UNKNOWN;
    case 0x91: [[fallthrough]];
    case 0xB1: return ::D3DFMT_A8R8G8B8;
    case 0xA1:
        auto const header = raw_data(0, 12);
        auto const type =
            std::to_integer<std::uint32_t>(gsl::at(header, 0)) << 0x00 |
            std::to_integer<std::uint32_t>(gsl::at(header, 1)) << 0x08 |
            std::to_integer<std::uint32_t>(gsl::at(header, 2)) << 0x10 |
            std::to_integer<std::uint32_t>(gsl::at(header, 3)) << 0x18;

        switch (type)
        {
        default: return ::D3DFMT_UNKNOWN;
        case 0x44585431: return ::D3DFMT_DXT1;
        case 0x44585432: return ::D3DFMT_DXT2;
        case 0x44585433: return ::D3DFMT_DXT3;
        case 0x44585434: return ::D3DFMT_DXT4;
        case 0x44585435: return ::D3DFMT_DXT5;
        }
    }
}

ui::patch ffxi_image::patch() const noexcept
{
    auto const s = size();
    return {{0, 0, s.width, s.height}, {}, s};
}

ui::dimension windower::ui::ffxi_image::size() const noexcept
{
    auto const width  = gsl::narrow<float>(m_bitmap_header.biWidth);
    auto const height = gsl::narrow<float>(m_bitmap_header.biHeight);
    return {width, height};
}

ui::dimension ffxi_image::raw_size() const noexcept { return size(); }

bool ffxi_image::copy_to(::IDirect3DTexture8* texture) const noexcept
{
    auto surface = ::D3DSURFACE_DESC{};
    auto data    = ::D3DLOCKED_RECT{};
    if (!texture || FAILED(texture->GetLevelDesc(0, &surface)) ||
        FAILED(texture->LockRect(0, &data, nullptr, D3DLOCK_DISCARD)))
    {
        return false;
    }

    auto const buffer =
        std::span{static_cast<std::byte*>(data.pBits), surface.Size};

    switch (m_type)
    {
    case 0x91: [[fallthrough]];
    case 0xB1:
        switch (m_bitmap_header.biBitCount)
        {
        case 0x08: copy_indexed_to(buffer, data.Pitch); break;
        case 0x20: copy_rgba_to(buffer, data.Pitch); break;
        default: return false;
        }
        break;
    case 0xA1: copy_dxt_to(buffer, data.Pitch); break;
    default: return false;
    }

    return SUCCEEDED(texture->UnlockRect(0));
}

void ffxi_image::copy_indexed_to(
    std::span<std::byte> buffer, std::int32_t stride) const noexcept
{
    auto const offset = m_type == 0xB1 ? 4 : 0;

    auto const width  = m_bitmap_header.biWidth;
    auto const height = m_bitmap_header.biHeight;

    constexpr auto palette_count = 0x100;
    auto const pixel_count = gsl::narrow_cast<std::size_t>(width * height);

    auto const data    = raw_data(offset, pixel_count + palette_count * 4);
    auto const palette = data.subspan(0, palette_count * 4);
    auto const pixels  = data.subspan(palette_count * 4);

    for (auto y = 0; y < height; ++y)
    {
        for (auto x = 0; x < width; ++x)
        {
            auto const pixel = x + width * (height - y - 1);

            auto const dst_offset = y * stride + x * 4;
            auto const src_offset =
                std::to_integer<std::size_t>(gsl::at(pixels, pixel)) * 4;

            decode_ffxi_rgba(buffer, dst_offset, palette, src_offset);
        }
    }
}

void ffxi_image::copy_rgba_to(
    std::span<std::byte> buffer, std::int32_t stride) const noexcept
{
    auto const offset = m_type == 0xB1 ? 4 : 0;

    auto const width  = m_bitmap_header.biWidth;
    auto const height = m_bitmap_header.biHeight;

    auto const pixel_count = gsl::narrow_cast<std::size_t>(width * height);

    auto const data = raw_data(offset, pixel_count * 4);

    for (auto y = 0; y < height; ++y)
    {
        for (auto x = 0; x < width; ++x)
        {
            auto const pixel = x + width * (height - y - 1);

            auto const dst_offset = y * stride + x * 4;
            auto const src_offset = pixel * 4;

            decode_ffxi_rgba(buffer, dst_offset, data, src_offset);
        }
    }
}

void ffxi_image::copy_dxt_to(
    std::span<std::byte> buffer,
    [[maybe_unused]] std::int32_t stride) const noexcept
{
    auto const data = raw_data(12, buffer.size());
    std::copy(data.begin(), data.end(), buffer.begin());
}

std::span<std::byte const>
ffxi_image::raw_data(std::size_t offset, std::size_t size) const noexcept
{
    return std::span<std::byte const>{
        static_cast<std::byte const*>(m_data), offset + size}
        .subspan(offset);
}

}
