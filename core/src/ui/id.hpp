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

#ifndef WINDOWER_UI_ID_HPP
#define WINDOWER_UI_ID_HPP

#include <gsl/gsl>

#include <bit>
#include <cstdint>

namespace windower::ui
{

class part_id
{
public:
    constexpr explicit part_id(std::uint8_t value) : m_value{value} {}

    constexpr std::uint8_t value() const { return m_value; }

    constexpr bool operator==(part_id const&) const = default;

private:
    std::uint8_t m_value;
};

constexpr part_id operator|(part_id lhs, part_id rhs)
{
    return part_id{gsl::narrow_cast<std::uint8_t>(lhs.value() + rhs.value())};
}

namespace part_ids
{

constexpr part_id none{0};

constexpr part_id scroll_panel_h{1};
constexpr part_id scroll_panel_v{6};

constexpr part_id scroll_bar_h_down{1};
constexpr part_id scroll_bar_h_up{2};
constexpr part_id scroll_bar_h_down_track{3};
constexpr part_id scroll_bar_h_up_track{4};
constexpr part_id scroll_bar_h_thumb{5};
constexpr part_id scroll_bar_v_down{6};
constexpr part_id scroll_bar_v_up{7};
constexpr part_id scroll_bar_v_down_track{8};
constexpr part_id scroll_bar_v_up_track{9};
constexpr part_id scroll_bar_v_thumb{10};

constexpr part_id slider_down_track{1};
constexpr part_id slider_up_track{2};
constexpr part_id slider_thumb{3};

}

class id
{
public:
    constexpr id() noexcept = default;

    constexpr explicit id(std::uint32_t id) noexcept : id{1, id} {}

    constexpr explicit id(std::uint32_t scope, std::uint32_t id) noexcept
    {
        std::uint64_t const high = scope;
        std::uint64_t const low  = id;

        m_value = high << 32 | low;
    }

    constexpr explicit id(std::uint64_t value) noexcept : m_value{value} {}

    constexpr explicit operator std::uint64_t() const noexcept
    {
        return m_value;
    }

    constexpr bool operator==(id const& rhs) const noexcept
    {
        return m_value == rhs.m_value;
    }

    constexpr bool operator!=(id const& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr id part(std::uint8_t part) const noexcept
    {
        Expects(part != 0);
        return id{scope() + part + 1, value()};
    }

    constexpr std::uint32_t scope() const noexcept
    {
        return gsl::narrow_cast<std::uint32_t>(m_value >> 32);
    }

    constexpr std::uint32_t value() const noexcept
    {
        return gsl::narrow_cast<std::uint32_t>(m_value);
    }

private:
    std::uint64_t m_value = 0;
};

constexpr id no_id{};

constexpr id make_id(void const* scope, std::uint32_t value) noexcept
{
    auto scope_value{std::bit_cast<std::uintptr_t>(scope)};
    return id{gsl::narrow_cast<std::uint32_t>(scope_value), value};
}

}

#endif
