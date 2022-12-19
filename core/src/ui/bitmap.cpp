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

#include "ui/bitmap.hpp"

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/patch.hpp"
#include "ui/texture_loaders.hpp"
#include "ui/texture_token.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <d3d8.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <cmath>
#include <cstddef>
#include <string_view>
#include <utility>
#include <variant>

namespace windower::ui
{

namespace
{

using d3d_format        = std::tuple<::D3DFORMAT, guid>;
using conversion_format = std::tuple<guid, guid>;

static auto const supported_formats = std::array{
    // clang-format off
    d3d_format{::D3DFMT_R8G8B8, ::GUID_WICPixelFormat24bppBGR},
    d3d_format{::D3DFMT_A8R8G8B8, ::GUID_WICPixelFormat32bppPBGRA},
    d3d_format{::D3DFMT_X8R8G8B8, ::GUID_WICPixelFormat32bppBGR},
    d3d_format{::D3DFMT_R5G6B5, ::GUID_WICPixelFormat16bppBGR565},
    d3d_format{::D3DFMT_X1R5G5B5, ::GUID_WICPixelFormat16bppBGR555},
    d3d_format{::D3DFMT_A8, ::GUID_WICPixelFormat8bppAlpha},
    d3d_format{::D3DFMT_L8, ::GUID_WICPixelFormat8bppGray},
    // clang-format on
};

static auto const conversion_formats = std::array{
    // clang-format off
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppBGRA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppRGBA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppPRGBA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat1bppIndexed},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat2bppIndexed},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat4bppIndexed},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat8bppIndexed},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat16bppBGRA5551},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppRGBA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppBGRA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppPRGBA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppPBGRA},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat128bppRGBAFloat},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat128bppPRGBAFloat},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppRGBAFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppBGRAFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat128bppRGBAFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppRGBAHalf},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bppPRGBAHalf},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppRGBA1010102},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppRGBA1010102XR},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppR10G10B10A2},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bppR10G10B10A2HDR10},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat40bppCMYKAlpha},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat80bppCMYKAlpha},
    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat32bpp3ChannelsAlpha},

    conversion_format{::GUID_WICPixelFormat32bppPBGRA, ::GUID_WICPixelFormat64bpp3ChannelsAlpha},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat24bppBGR},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat24bppRGB},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat32bppRGB},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bppRGB},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bppBGR},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat64bppRGB},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat32bppBGR101010},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bppRGBFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bppBGRFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat96bppRGBFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat96bppRGBFloat},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat128bppRGBFloat},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat32bppCMYK},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat64bppRGBFixedPoint},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat64bppRGBHalf},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bppRGBHalf},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat32bppRGBE},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat64bppCMYK},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat24bpp3Channels},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat48bpp3Channels},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat8bppY},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat8bppCb},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat8bppCr},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat16bppCbCr},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat16bppYQuantizedDctCoefficients},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat16bppCbQuantizedDctCoefficients},
    conversion_format{::GUID_WICPixelFormat32bppBGR, ::GUID_WICPixelFormat16bppCrQuantizedDctCoefficients},

    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormatBlackWhite},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat2bppGray},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat4bppGray},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat16bppGray},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat32bppGrayFloat},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat16bppGrayFixedPoint},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat16bppGrayHalf},
    conversion_format{::GUID_WICPixelFormat8bppGray, ::GUID_WICPixelFormat32bppGrayFixedPoint},
    // clang-format on
};

winrt::com_ptr<::IWICStream>
open_file(context& ctx, std::filesystem::path const& path)
{
    winrt::com_ptr<::IWICStream> stream;
    if (SUCCEEDED(ctx.wic_factory()->CreateStream(stream.put())) &&
        SUCCEEDED(stream->InitializeFromFilename(path.c_str(), GENERIC_READ)))
    {
        return stream;
    }
    return nullptr;
}

winrt::com_ptr<::IWICStream>
open_resource(context& ctx, std::filesystem::path const& path)
{
    auto const library = static_cast<::HMODULE>(windower::windower_module());
    if (auto const hrsrc = ::FindResourceW(library, path.c_str(), RT_RCDATA))
    {
        if (auto const handle = ::LoadResource(library, hrsrc))
        {
            auto const data = ::LockResource(handle);
            auto const size = ::SizeofResource(library, hrsrc);
            if (data != nullptr && size != 0)
            {
                auto const ptr = static_cast<::WICInProcPointer>(data);
                winrt::com_ptr<::IWICStream> stream;
                if (SUCCEEDED(ctx.wic_factory()->CreateStream(stream.put())) &&
                    SUCCEEDED(stream->InitializeFromMemory(ptr, size)))
                {
                    return stream;
                }
            }
        }
    }
    return nullptr;
}

winrt::com_ptr<::IWICStream>
open(context& ctx, bool is_resource, std::filesystem::path const& path)
{
    return is_resource ? open_resource(ctx, path) : open_file(ctx, path);
}

std::tuple<winrt::com_ptr<::IWICStream>, float>
open(context& ctx, std::u8string_view name)
{
    if (!name.empty())
    {
        auto is_resource = false;
        if (name.starts_with(u8':'))
        {
            name.remove_prefix(1);
            is_resource = true;
        }

        auto path = std::filesystem::path{name}.lexically_normal();

        auto const extesion = path.extension();
        path.replace_extension();

        std::filesystem::path scaled_path;
        for (auto scale_index = std::clamp(
                 static_cast<int>(std::ceil(ctx.scale_factor_uniform() * 4)), 4,
                 16);
             scale_index >= 4; --scale_index)
        {
            scaled_path = path;
            scaled_path += u8'.';
            scaled_path += std::to_string(scale_index * 25);
            scaled_path += extesion;
            if (auto ptr = open(ctx, is_resource, scaled_path))
            {
                return {std::move(ptr), scale_index / 4.f};
            }
        }

        path += extesion;
        return {open(ctx, is_resource, path), 1.f};
    }
    return {nullptr, 1.f};
}

std::tuple<float, float> texture_coordinate(float value, float size)
{
    if (size <= 1)
    {
        return {};
    }

    float i_part      = 0.f;
    auto const temp   = std::fmodf(value * size, size);
    auto const f_part = std::modf(temp, &i_part);
    return {i_part, f_part};
}

winrt::com_ptr<::IWICBitmapSource> load(context& ctx, ::IWICStream* stream)
{
    winrt::com_ptr<::IWICBitmapDecoder> decoder;
    winrt::com_ptr<::IWICBitmapFrameDecode> frame;
    if (SUCCEEDED(ctx.wic_factory()->CreateDecoderFromStream(
            stream, nullptr, ::WICDecodeMetadataCacheOnDemand,
            decoder.put())) &&
        SUCCEEDED(decoder->GetFrame(0, frame.put())))
    {
        return frame;
    }
    return nullptr;
}

}

bitmap bitmap::load(context& ctx, std::u8string_view name) noexcept
{
    auto [stream, scale] = open(ctx, name);

    winrt::com_ptr<::IWICBitmapDecoder> decoder;
    winrt::com_ptr<::IWICBitmapFrameDecode> frame;
    if (SUCCEEDED(ctx.wic_factory()->CreateDecoderFromStream(
            stream.get(), nullptr, ::WICDecodeMetadataCacheOnDemand,
            decoder.put())) &&
        SUCCEEDED(decoder->GetFrame(0, frame.put())))
    {
        ::UINT uint_width;
        ::UINT uint_height;
        frame->GetSize(&uint_width, &uint_height);
        auto const width  = gsl::narrow_cast<float>(uint_width) / scale;
        auto const height = gsl::narrow_cast<float>(uint_height) / scale;
        return {std::move(frame), {{0, 0, width, height}, {}, {width, height}}};
    }
    return {};
}

bitmap::bitmap(
    winrt::com_ptr<::IWICBitmapSource> bitmap, ui::patch const& patch) noexcept
    :
    m_bitmap{bitmap},
    m_patch{patch}
{}

bitmap::operator bool() const noexcept { return m_bitmap != nullptr; }

guid bitmap::wic_format() const noexcept
{
    guid format;
    if (!m_bitmap || FAILED(m_bitmap->GetPixelFormat(format.put())))
    {
        format = ::GUID_WICPixelFormatUndefined;
    }
    return format;
}

::D3DFORMAT bitmap::d3d_format() const noexcept
{
    if (auto const it = std::ranges::find(
            supported_formats, wic_format(),
            [](auto const& f) { return std::get<1>(f); });
        it != supported_formats.end())
    {
        return std::get<0>(*it);
    }
    return ::D3DFMT_UNKNOWN;
}

bitmap bitmap::convert(
    context& ctx, ::WICPixelFormatGUID const& to_format) const noexcept
{
    winrt::com_ptr<::IWICFormatConverter> converter;
    if (m_bitmap && wic_format() != to_format &&
        SUCCEEDED(ctx.wic_factory()->CreateFormatConverter(converter.put())) &&
        SUCCEEDED(converter->Initialize(
            m_bitmap.get(), to_format, ::WICBitmapDitherTypeErrorDiffusion,
            nullptr, 0., ::WICBitmapPaletteTypeCustom)))
    {
        return {std::move(converter), m_patch};
    }
    return {};
}

bitmap bitmap::convert(context& ctx) const noexcept
{
    if (!m_bitmap)
    {
        return {};
    }

    winrt::com_ptr<::IWICFormatConverter> result;
    if (auto const it = std::ranges::find(
            conversion_formats, wic_format(),
            [](auto const& f) { return std::get<1>(f); });
        it != conversion_formats.end() &&
        SUCCEEDED(ctx.wic_factory()->CreateFormatConverter(result.put())) &&
        SUCCEEDED(result->Initialize(
            m_bitmap.get(), std::get<0>(*it).get(),
            ::WICBitmapDitherTypeErrorDiffusion, nullptr, 0.,
            ::WICBitmapPaletteTypeCustom)))
    {
        return {std::move(result), m_patch};
    }
    return {};
}

patch const& bitmap::patch() const noexcept { return m_patch; }

dimension const& bitmap::size() const noexcept { return m_patch.texture_size; }

dimension bitmap::raw_size() const noexcept
{
    auto uint_width  = ::UINT{};
    auto uint_height = ::UINT{};
    if (m_bitmap && SUCCEEDED(m_bitmap->GetSize(&uint_width, &uint_height)))
    {
        return {
            gsl::narrow_cast<float>(uint_width),
            gsl::narrow_cast<float>(uint_height)};
    }
    return {};
}

::IWICBitmapSource* bitmap::get() const noexcept { return m_bitmap.get(); }

color bitmap::sample(context& ctx, vector const& coordinate) const noexcept
{
    auto converted = convert(ctx, ::GUID_WICPixelFormat32bppPBGRA);

    ::UINT int_width;
    ::UINT int_height;
    if (FAILED(m_bitmap->GetSize(&int_width, &int_height)))
    {
        return colors::magenta;
    }
    auto const width  = gsl::narrow_cast<float>(int_width);
    auto const height = gsl::narrow_cast<float>(int_height);

    auto const [x, dx] = texture_coordinate(coordinate.x, width);
    auto const [y, dy] = texture_coordinate(coordinate.y, height);

    auto data               = std::array<std::uint32_t, 4>{};
    auto const buffer       = std::as_writable_bytes(std::span{data});
    void* const buffer_ptr  = buffer.data();
    auto const buffer_bytes = static_cast<::BYTE*>(buffer_ptr);
    ::WICRect rect{
        .X      = gsl::narrow_cast<::INT>(x),
        .Y      = gsl::narrow_cast<::INT>(y),
        .Width  = 2,
        .Height = 2,
    };
    if (FAILED(m_bitmap->CopyPixels(
            &rect, buffer.size() / 2, buffer.size(), buffer_bytes)))
    {
        return colors::magenta;
    }

    auto const c0 = color{gsl::at(data, 0)};
    auto const c1 = color{gsl::at(data, 1)};
    auto const c2 = color{gsl::at(data, 2)};
    auto const c3 = color{gsl::at(data, 3)};

    auto const c4 = lerp(c0, c1, dx, true);
    auto const c5 = lerp(c2, c3, dx, true);
    auto const c6 = lerp(c4, c5, dy, true);

    return to_straight_alpha(c6);
}

bool bitmap::copy_to(::IDirect3DTexture8* texture) const noexcept
{
    auto surface = ::D3DSURFACE_DESC{};
    if (!texture || FAILED(texture->GetLevelDesc(0, &surface)))
    {
        return false;
    }

    auto rect   = ::WICRect{};
    rect.Width  = gsl::narrow<::INT>(surface.Width);
    rect.Height = gsl::narrow<::INT>(surface.Height);
    auto data   = ::D3DLOCKED_RECT{};
    return SUCCEEDED(texture->LockRect(0, &data, nullptr, D3DLOCK_DISCARD)) &&
           SUCCEEDED(m_bitmap->CopyPixels(
               &rect, data.Pitch, surface.Size,
               static_cast<::BYTE*>(data.pBits))) &&
           SUCCEEDED(texture->UnlockRect(0));
}

}
