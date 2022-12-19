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

#ifndef WINDOWER_UI_TEXTURE_CACHE_HPP
#define WINDOWER_UI_TEXTURE_CACHE_HPP

#include "ui/bitmap.hpp"
#include "ui/texture.hpp"

#include <d3d8.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <array>
#include <compare>
#include <concepts>
#include <cstdint>
#include <string>
#include <vector>

namespace windower::ui
{

class context;

enum class texture_type : std::uint32_t
{
    file,
    memory,
    text,
};

template<typename T>
concept texture_loader = requires(T const& loader)
{
    // clang-format off
    { loader() } noexcept -> std::convertible_to<texture>;
    // clang-format on
};

class texture_cache
{
    class entry;

public:
    class descriptor
    {
    public:
        descriptor(texture_type type, std::u8string_view name) noexcept;
        descriptor(void const* ptr) noexcept;

        bool operator==(descriptor const&) const noexcept;
        std::weak_ordering operator<=>(descriptor const&) const noexcept;

        void set_integer(std::size_t index, std::intptr_t value) noexcept;
        void set_float(std::size_t index, float value) noexcept;

    private:
        texture_type m_type;
        std::u8string_view m_name;
        std::array<std::intptr_t, 6> m_integers = {};
        std::array<float, 3> m_floats           = {};

        friend class texture_cache;
    };

    static winrt::com_ptr<::IDirect3DTexture8>
    allocate(context&, dimension const&, ::D3DFORMAT);

    texture_cache() noexcept            = default;
    texture_cache(texture_cache const&) = delete;
    texture_cache(texture_cache&&)      = delete;

    ~texture_cache() noexcept;

    texture_cache& operator=(texture_cache const&) = delete;
    texture_cache& operator=(texture_cache&&) = delete;

    template<texture_loader L>
    texture const&
    get(descriptor const& descriptor, std::size_t time_to_live,
        L&& loader) noexcept
    {
        auto it = std::ranges::lower_bound(m_entries, descriptor);
        if (it == std::cend(m_entries) || *it > descriptor)
        {
            it = m_entries.emplace(it, descriptor);

            it->m_texture = loader();
            if (it->m_texture.token == no_texture)
            {
                winrt::com_ptr<::IDirect3DTexture8> temp;
                temp.copy_from(m_error_texture.token.m_value);
                temp.detach();
                it->m_texture = m_error_texture;
            }
        }
        it->m_time_to_live = time_to_live;
        return it->m_texture;
    }

    void initialize(context&) noexcept;
    void update() noexcept;
    void clear() noexcept;

private:
    class entry
    {
    public:
        entry(entry const&) noexcept;
        entry(entry&&) noexcept;
        entry(descriptor const&) noexcept;

        ~entry() noexcept = default;

        entry& operator=(entry const&) noexcept;
        entry& operator=(entry&&) noexcept;

        bool operator==(entry const&) const noexcept;
        bool operator==(descriptor const&) const noexcept;
        std::weak_ordering operator<=>(entry const&) const noexcept;
        std::weak_ordering operator<=>(descriptor const&) const noexcept;

    private:
        descriptor m_descriptor;
        texture m_texture;
        std::uint32_t m_time_to_live = 0;
        std::u8string m_name;

        friend class texture_cache;
    };

    texture m_error_texture;
    std::vector<entry> m_entries;
};

}

#endif
