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

#include "ui/texture_cache.hpp"

#include "ui/context.hpp"
#include "ui/texture.hpp"

#include <d3d8.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <algorithm>
#include <span>
#include <utility>

namespace windower::ui
{

texture_cache::descriptor::descriptor(
    texture_type type, std::u8string_view name) noexcept :
    m_type{type},
    m_name{name}
{}

texture_cache::descriptor::descriptor(void const* ptr) noexcept :
    m_type{texture_type::memory}, m_name{}
{
    set_integer(0, std::bit_cast<std::intptr_t>(ptr));
}

bool texture_cache::descriptor::operator==(
    descriptor const& other) const noexcept
{
    return m_type == other.m_type && m_name == other.m_name &&
           m_integers == other.m_integers && m_floats == other.m_floats;
}

std::weak_ordering
texture_cache::descriptor::operator<=>(descriptor const& other) const noexcept
{
    class float_weak_order_t
    {
    public:
        std::weak_ordering operator()(float lhs, float rhs) noexcept
        {
            if (lhs == rhs)
            {
                return std::weak_ordering::equivalent;
            }
            if (lhs < rhs)
            {
                return std::weak_ordering::less;
            }
            return std::weak_ordering::greater;
        }
    };

    static constexpr float_weak_order_t float_weak_order{};

    if (auto const result = m_type <=> other.m_type; std::is_neq(result))
    {
        return result;
    }

    if (auto const result = std::lexicographical_compare_three_way(
            m_name.begin(), m_name.end(), other.m_name.begin(),
            other.m_name.end());
        std::is_neq(result))
    {
        return result;
    }

    if (auto const result = std::lexicographical_compare_three_way(
            m_integers.begin(), m_integers.end(), other.m_integers.begin(),
            other.m_integers.end());
        std::is_neq(result))
    {
        return result;
    }

    return std::lexicographical_compare_three_way(
        m_floats.begin(), m_floats.end(), other.m_floats.begin(),
        other.m_floats.end(), float_weak_order);
}

void texture_cache::descriptor::set_integer(
    std::size_t index, std::intptr_t value) noexcept
{
    gsl::at(m_integers, index) = value;
}

void texture_cache::descriptor::set_float(
    std::size_t index, float value) noexcept
{
    gsl::at(m_floats, index) = value;
}

texture_cache::entry::entry(entry const& other) noexcept :
    m_descriptor{other.m_descriptor}, m_texture{other.m_texture},
    m_time_to_live{other.m_time_to_live}, m_name{other.m_name}
{
    m_descriptor.m_name = m_name;
}

texture_cache::entry::entry(entry&& other) noexcept :
    m_descriptor{other.m_descriptor}, m_texture{other.m_texture},
    m_time_to_live{other.m_time_to_live}, m_name{std::move(other.m_name)}
{
    m_descriptor.m_name = m_name;
}

texture_cache::entry::entry(descriptor const& desc) noexcept :
    m_descriptor{desc}, m_name{desc.m_name}
{
    m_descriptor.m_name = m_name;
}

texture_cache::entry&
texture_cache::entry::operator=(entry const& other) noexcept
{
    m_descriptor        = other.m_descriptor;
    m_texture           = other.m_texture;
    m_time_to_live      = other.m_time_to_live;
    m_name              = other.m_name;
    m_descriptor.m_name = m_name;
    return *this;
}

texture_cache::entry& texture_cache::entry::operator=(entry&& other) noexcept
{
    m_descriptor        = other.m_descriptor;
    m_texture           = other.m_texture;
    m_time_to_live      = other.m_time_to_live;
    m_name              = std::move(other.m_name);
    m_descriptor.m_name = m_name;
    return *this;
}

bool texture_cache::entry::operator==(entry const& other) const noexcept
{
    return m_descriptor == other.m_descriptor;
}

bool texture_cache::entry::operator==(descriptor const& other) const noexcept
{
    return m_descriptor == other;
}

std::weak_ordering
texture_cache::entry::operator<=>(entry const& other) const noexcept
{
    return m_descriptor <=> other.m_descriptor;
}

std::weak_ordering
texture_cache::entry::operator<=>(descriptor const& other) const noexcept
{
    return m_descriptor <=> other;
}

winrt::com_ptr<::IDirect3DTexture8> texture_cache::allocate(
    context& ctx, dimension const& dimension, ::D3DFORMAT format)
{
    winrt::com_ptr<::IDirect3DTexture8> texture;
    if (SUCCEEDED(ctx.d3d_device()->CreateTexture(
            gsl::narrow<::UINT>(std::ceil(dimension.width)),
            gsl::narrow<::UINT>(std::ceil(dimension.height)), 1, 0, format,
            ::D3DPOOL_MANAGED, texture.put())))
    {
        return texture;
    }
    return nullptr;
}

texture_cache::~texture_cache() noexcept
{
    clear();
    winrt::com_ptr<::IDirect3DTexture8> deleter;
    deleter.attach(m_error_texture.token.m_value);
    m_error_texture = {};
}

void texture_cache::initialize(context& ctx) noexcept
{
    auto data = ::D3DLOCKED_RECT{};
    if (auto texture = allocate(ctx, {1.f, 1.f}, D3DFMT_X8R8G8B8);
        texture &&
        SUCCEEDED(texture->LockRect(0, &data, nullptr, D3DLOCK_DISCARD)))
    {
        auto const buffer  = std::span{static_cast<std::byte*>(data.pBits), 4};
        gsl::at(buffer, 0) = std::byte{0xFF};
        gsl::at(buffer, 1) = std::byte{0x00};
        gsl::at(buffer, 2) = std::byte{0xFF};
        gsl::at(buffer, 3) = std::byte{0x00};
        if (SUCCEEDED(texture->UnlockRect(0)))
        {
            m_error_texture.token.m_value = texture.detach();
            m_error_texture.patch = {{0.f, 0.f, 1.f, 1.f}, {}, {1.f, 1.f}};
        }
    }
}

void texture_cache::update() noexcept
{
    auto end = std::stable_partition(
        m_entries.begin(), m_entries.end(),
        [](auto const& e) { return e.m_time_to_live > 0; });
    winrt::com_ptr<::IDirect3DTexture8> deleter;
    for (auto it = end; it != m_entries.end(); ++it)
    {
        deleter.attach(it->m_texture.token.m_value);
    }
    m_entries.erase(end, m_entries.cend());
    for (auto& entry : m_entries)
    {
        --entry.m_time_to_live;
    }
}

void texture_cache::clear() noexcept
{
    winrt::com_ptr<::IDirect3DTexture8> deleter;
    for (auto const& entry : m_entries)
    {
        deleter.attach(entry.m_texture.token.m_value);
    }
    m_entries.clear();
}

}
