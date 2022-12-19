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

#ifndef WINDOWER_SCANNER_HPP
#define WINDOWER_SCANNER_HPP

#include "errors/syntax_error.hpp"
#include "library.hpp"
#include "utility.hpp"

#include <gsl/gsl>

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <limits>
#include <new>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace windower
{

class signature
{
    static_assert(
        std::numeric_limits<std::underlying_type_t<std::byte>>::digits == 8,
        "this type requires an architecture where std::byte is 8 bits.");

    static constexpr std::size_t alignment =
        std::hardware_constructive_interference_size;

public:
    static constexpr std::size_t max_size = alignment;

    template<std::size_t N>
    constexpr explicit signature(char8_t const (&string)[N]) :
        signature{std::u8string_view{string}}
    {}

    constexpr explicit signature(std::u8string_view string)
    {
        // signature    → byte body
        //              | wildcard signature
        //              | offset_mark leading_tail
        //
        // body         → byte body | wildcard body | offset_mark tail
        // tail         → byte tail | wildcard tail
        // leading_tail → byte tail | wildcard marked_tail
        //
        // byte         → nibble nibble
        // wildcard     → '?' '?'
        // offset_mark  → '*' | '&'
        //
        // nibble       → '0' | '1' | '2' | '3' | '4'
        //              | '5' | '6' | '7' | '8' | '9'
        //              | 'A' | 'B' | 'C' | 'D' | 'E' | 'F'
        //              | 'a' | 'b' | 'c' | 'd' | 'e' | 'f'

        constexpr std::array<
            std::array<std::pair<std::int8_t, std::int8_t>, 6>, 10>
            state_table{{
                // clang-format off
                //  hex       ?        *        &        .        $
                {{{3,  1}, {2,  0}, {1,  0}, {1,  5}, {0,  8}, {-1, 9}}},
                {{{5,  1}, {4,  0}, {0, 10}, {0, 10}, {0,  8}, {-1, 9}}},
                {{{0, 11}, {0,  0}, {0, 11}, {0, 11}, {0, 11}, {-1, 9}}},
                {{{6,  2}, {0, 12}, {0, 12}, {0, 12}, {0, 12}, {-1, 9}}},
                {{{0, 11}, {1,  3}, {0, 11}, {0, 11}, {0, 11}, {-1, 9}}},
                {{{7,  2}, {0, 12}, {0, 12}, {0, 12}, {0, 12}, {-1, 9}}},
                {{{3,  1}, {8,  0}, {7,  7}, {7,  6}, {0,  8}, {-1, 7}}},
                {{{5,  1}, {9,  0}, {0, 10}, {0, 10}, {0,  8}, {-1, 0}}},
                {{{0, 11}, {6,  4}, {0, 11}, {0, 11}, {0, 11}, {-1, 9}}},
                {{{0, 11}, {7,  4}, {0, 11}, {0, 11}, {0, 11}, {-1, 9}}},
                // clang-format on
            }};

        constexpr auto next = [](std::u8string_view string,
                                 std::size_t& offset) -> std::uint8_t {
            if (offset == string.size())
            {
                return 5;
            }
            switch (gsl::at(string, offset++))
            {
            // clang-format off
            case u8'0': case u8'1': case u8'2': case u8'3': case u8'4':
            case u8'5': case u8'6': case u8'7': case u8'8': case u8'9':
            case u8'A': case u8'B': case u8'C': case u8'D': case u8'E': case u8'F':
            case u8'a': case u8'b': case u8'c': case u8'd': case u8'e': case u8'f':
                return 0;
            case u8'?': return 1;
            case u8'*': return 2;
            case u8'&': return 3;
            // clang-format on
            default: return 4;
            }
        };

        constexpr auto value = [](char8_t c) -> std::byte {
            return gsl::narrow_cast<std::byte>(
                ((c | 0x1B0) * 0x0E422D48U) >> 28);
        };

        auto state       = 0;
        auto high_nibble = std::byte{};
        auto mask_count  = 0;
        auto offset      = std::size_t{};

        while (state >= 0)
        {
            auto next_offset      = offset;
            auto const char_class = next(string, next_offset);
            auto const [next_state, action] =
                gsl::at(gsl::at(state_table, state), char_class);
            switch (action)
            {
            case 0: break;
            case 1: high_nibble = value(gsl::at(string, offset)); break;
            case 2:
                m_size += mask_count;
                if (m_size >= max_size)
                {
                    throw windower::syntax_error{u8"SIG:1", string, offset};
                }
                mask_count = 0;
                gsl::at(m_data, m_size) =
                    high_nibble << 4 | value(gsl::at(string, offset));
                gsl::at(m_mask, m_size) = std::byte{0xFF};
                ++m_size;
                break;
            case 3: --m_offset; break;
            case 4: ++mask_count; break;
            case 5: m_dereference = false; break;
            case 6: m_dereference = false; [[fallthrough]];
            case 7: m_offset = m_size; break;
            case 8: throw windower::syntax_error{u8"SIG:2", string, offset};
            case 9: throw windower::syntax_error{u8"SIG:3", string, offset};
            case 10: throw windower::syntax_error{u8"SIG:4", string, offset};
            case 11: throw windower::syntax_error{u8"SIG:5", string, offset};
            case 12: throw windower::syntax_error{u8"SIG:6", string, offset};
            default: fail_fast();
            }
            state  = next_state;
            offset = next_offset;
        }
    }

    constexpr std::span<std::byte const> data() const noexcept
    {
        return {m_data.data(), m_size};
    }

    constexpr std::span<std::byte const> mask() const noexcept
    {
        return {m_mask.data(), m_size};
    }

    constexpr std::size_t size() const noexcept { return m_size; }

    constexpr std::ptrdiff_t offset() const noexcept { return m_offset; }

    constexpr bool dereference() const noexcept { return m_dereference; }

private:
    alignas(alignment) std::array<std::byte, max_size> m_data = {};
    alignas(alignment) std::array<std::byte, max_size> m_mask = {};

    std::size_t m_size      = 0;
    std::ptrdiff_t m_offset = 0;
    bool m_dereference      = true;
};

inline namespace signature_literals
{

consteval signature operator"" _sig(char8_t const* string, std::size_t size)
{
    return signature{std::u8string_view{string, size}};
}

}

template<typename T>
concept address_compatible = requires
{
    std::bit_cast<T>(std::declval<void*>());
    std::bit_cast<void*>(std::declval<T>());
};

class address
{
public:
    constexpr address() noexcept {}

    template<address_compatible T>
    explicit(std::is_arithmetic_v<T>) constexpr address(T ptr) noexcept :
        m_ptr{std::bit_cast<void*>(ptr)}
    {}

    address operator*() const noexcept
    {
        return *this ? address{*static_cast<void**>(m_ptr)} : address{};
    }

    address& operator+=(std::size_t rhs) noexcept
    {
        return *this ? *this = *this + rhs : *this;
    }

    address& operator-=(std::size_t rhs) noexcept
    {
        return *this ? *this = *this - rhs : *this;
    }

    explicit constexpr operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    template<address_compatible T>
    explicit(std::is_arithmetic_v<T>) constexpr operator T() const noexcept
    {
        return std::bit_cast<T>(m_ptr);
    }

    constexpr bool operator==(address const&) const noexcept = default;
    constexpr std::strong_ordering
    operator<=>(address const&) const noexcept = default;

private:
    void* m_ptr = nullptr;

    friend constexpr address operator+(address lhs, std::size_t rhs) noexcept
    {
        return lhs ? address{std::next(static_cast<std::byte*>(lhs.m_ptr), rhs)}
                   : address{};
    }

    friend constexpr address operator-(address lhs, std::size_t rhs) noexcept
    {
        return lhs ? address{std::prev(static_cast<std::byte*>(lhs.m_ptr), rhs)}
                   : address{};
    }

    friend constexpr std::ptrdiff_t operator-(address lhs, address rhs) noexcept
    {
        return static_cast<std::byte*>(lhs.m_ptr) -
               static_cast<std::byte*>(rhs.m_ptr);
    }
};

void scan(
    library const& library, signature const& sig,
    std::span<address> results) noexcept;

inline void scan(
    std::u8string_view library_name, signature const& sig,
    std::span<address> results) noexcept
{
    return scan(library{library_name}, sig, results);
}

inline auto scan(library const& library, signature const& sig) noexcept
{
    address result;
    scan(library, sig, {&result, 1});
    return result;
}

inline auto scan(std::u8string_view library_name, signature const& sig) noexcept
{
    return scan(library{library_name}, sig);
}

template<std::size_t N>
std::array<address, N>
scan(library const& library, signature const& sig) noexcept
{
    std::array<address, N> results;
    scan(library, sig, results);
    return results;
}

template<std::size_t N>
std::array<address, N>
scan(std::u8string_view library_name, signature const& sig) noexcept
{
    std::array<address, N> results;
    scan(library{library_name}, sig, results);
    return results;
}

}

#endif
