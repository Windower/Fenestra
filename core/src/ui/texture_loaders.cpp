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

#include "ui/texture_loaders.hpp"

#include "ui/bitmap.hpp"
#include "ui/context.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/texture.hpp"
#include "utility.hpp"

#include <d3d8.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

namespace windower::ui
{

texture const& load_texture(
    context& ctx, std::u8string_view name, std::size_t time_to_live) noexcept
{
    auto desc = texture_cache::descriptor{texture_type::file, name};
    desc.set_float(0, ctx.scale_factor_uniform());
    return ctx.texture_cache().get(desc, time_to_live, [&]() noexcept {
        auto const bitmap = bitmap::load(ctx, name);
        if (auto const converted = bitmap.convert(ctx))
        {
            if (auto const format = converted.d3d_format();
                format != D3DFMT_UNKNOWN)
            {
                auto const size = converted.raw_size();
                if (auto tex = texture_cache::allocate(ctx, size, format);
                    tex && converted.copy_to(tex.get()))
                {
                    return texture{tex.detach(), converted.patch()};
                }
            }
        }
        return texture{};
    });
}

texture const& load_texture(
    context& ctx, ffxi_image const& image, std::size_t time_to_live) noexcept
{
    auto const desc = texture_cache::descriptor{&image};
    return ctx.texture_cache().get(desc, time_to_live, [&]() noexcept {
        auto const format = image.d3d_format();
        if (format != ::D3DFMT_UNKNOWN)
        {
            auto const size = image.raw_size();
            if (auto tex = texture_cache::allocate(ctx, size, format);
                tex && image.copy_to(tex.get()))
            {
                return texture{tex.detach(), image.patch()};
            }
        }
        return texture{};
    });
}

}
