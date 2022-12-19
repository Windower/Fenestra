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

#ifndef WINDOWER_UTILITY_HPP
#define WINDOWER_UTILITY_HPP

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <compare>
#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <numeric>
#include <span>
#include <string>
#include <type_traits>

#if defined(_MSC_VER)
#    include <intrin.h>
#    define WINDOWER_DEBUG_BREAK __debugbreak()
#    define WINDOWER_RETURN_ADDRESS _ReturnAddress()
#elif defined(__GNUC__)
#    define WINDOWER_DEBUG_BREAK __builtin_trap()
#    define WINDOWER_RETURN_ADDRESS                                            \
        __builtin_extract_return_addr(__builtin_return_address(0))
#else
#    include <windows.h>
#    define WINDOWER_DEBUG_BREAK ::DebugBreak()
#    error "WINDOWER_RETURN_ADDRESS macro is not defined for this compiler"
#endif

#if defined(__clang__)
// clang-format off
#    define WINDOWER_SUPPRESS(id) [[gsl::suppress(#id)]]
// clang-format on
#elif defined(_MSC_VER)
#    define WINDOWER_SUPPRESS(id) [[gsl::suppress(id)]]
#endif

namespace windower
{

void* module_for(void const*) noexcept;
void* windower_module() noexcept;
bool is_windower_module(void const*) noexcept;
bool is_game_module(void const*) noexcept;

std::filesystem::path windower_path();
std::filesystem::path settings_path();
std::filesystem::path user_path();
std::filesystem::path temp_path();
std::filesystem::path client_path();

template<typename C, typename T = std::char_traits<C>>
class basic_zstring_view : public std::basic_string_view<C, T>
{
    using base_type = std::basic_string_view<C, T>;

public:
    using base_type::const_pointer;
    using base_type::const_reference;
    using base_type::pointer;
    using base_type::reference;
    using base_type::traits_type;
    using base_type::value_type;

    using base_type::const_iterator;
    using base_type::const_reverse_iterator;
    using base_type::iterator;
    using base_type::reverse_iterator;

    using base_type::difference_type;
    using base_type::size_type;

    using base_type::npos;

    constexpr basic_zstring_view() noexcept                          = default;
    constexpr basic_zstring_view(basic_zstring_view const&) noexcept = default;
    constexpr basic_zstring_view(C const* str) noexcept : base_type{str} {}
    constexpr basic_zstring_view(C const* str, std::size_t size) noexcept :
        base_type{str, size}
    {}
    constexpr basic_zstring_view(std::basic_string<C, T> const& str) noexcept :
        base_type{str.data(), str.size()}
    {}

    basic_zstring_view& operator=(const basic_zstring_view&) noexcept = default;

    constexpr operator std::basic_string_view<C, T>() const
    {
        return base_type{data(), size()};
    }

    constexpr C const* c_str() const { return data(); }

    constexpr base_type remove_suffix_view(std::size_t count) const
    {
        auto copy = base_type{*this};
        copy.remove_suffix(count);
        return copy;
    }

    constexpr int swap(basic_zstring_view v) { return base_type::swap(v); }

    constexpr int compare(basic_zstring_view v)
    {
        return base_type::compare(v);
    }

    constexpr int
    compare(std::size_t pos, std::size_t count, basic_zstring_view v)
    {
        return base_type::compare(pos, count, v);
    }

    constexpr int compare(
        std::size_t pos1, std::size_t count1, basic_zstring_view v,
        std::size_t pos2, std::size_t count2)
    {
        return base_type::compare(pos1, count1, v, pos2, count2);
    }

    using base_type::begin;
    using base_type::cbegin;
    using base_type::cend;
    using base_type::crbegin;
    using base_type::crend;
    using base_type::end;
    using base_type::rbegin;
    using base_type::rend;

    using base_type::operator[];
    using base_type::at;
    using base_type::back;
    using base_type::data;
    using base_type::front;

    using base_type::empty;
    using base_type::length;
    using base_type::max_size;
    using base_type::size;

    using base_type::remove_prefix;

    using base_type::compare;
    using base_type::copy;
    using base_type::ends_with;
    using base_type::find;
    using base_type::find_first_not_of;
    using base_type::find_first_of;
    using base_type::find_last_not_of;
    using base_type::find_last_of;
    using base_type::rfind;
    using base_type::starts_with;
    using base_type::substr;
};

template<typename C, typename T>
bool operator==(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template<typename C, typename T>
bool operator==(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return std::basic_string_view<C, T>{lhs} == rhs;
}

template<typename C, typename T>
bool operator==(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return lhs == std::basic_string_view<C, T>{rhs};
}

template<typename C, typename T>
bool operator!=(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename C, typename T>
bool operator!=(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename C, typename T>
bool operator!=(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename C, typename T>
bool operator<(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return std::basic_string_view<C, T>{lhs} <
           std::basic_string_view<C, T>{rhs};
}

template<typename C, typename T>
bool operator<(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return std::basic_string_view<C, T>{lhs} < rhs;
}

template<typename C, typename T>
bool operator<(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return lhs < std::basic_string_view<C, T>{rhs};
}

template<typename C, typename T>
bool operator<=(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename C, typename T>
bool operator<=(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename C, typename T>
bool operator<=(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename C, typename T>
bool operator>(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return rhs < lhs;
}

template<typename C, typename T>
bool operator>(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return rhs < lhs;
}

template<typename C, typename T>
bool operator>(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return rhs < lhs;
}

template<typename C, typename T>
bool operator>=(
    basic_zstring_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(lhs < rhs);
}

template<typename C, typename T>
bool operator>=(
    basic_zstring_view<C, T> lhs, std::basic_string_view<C, T> rhs) noexcept
{
    return !(lhs < rhs);
}

template<typename C, typename T>
bool operator>=(
    std::basic_string_view<C, T> lhs, basic_zstring_view<C, T> rhs) noexcept
{
    return !(lhs < rhs);
}

using zstring_view    = basic_zstring_view<char>;
using wzstring_view   = basic_zstring_view<wchar_t>;
using u8zstring_view  = basic_zstring_view<char8_t>;
using u16zstring_view = basic_zstring_view<char16_t>;
using u32zstring_view = basic_zstring_view<char32_t>;

void set_thread_name(u8zstring_view name);

std::string_view to_string_view(std::u8string_view value) noexcept;
zstring_view to_zstring_view(u8zstring_view value) noexcept;

std::u8string to_u8string(std::string_view value);
std::u8string to_u8string(signed char value, int base = 10);
std::u8string to_u8string(signed short int value, int base = 10);
std::u8string to_u8string(signed int value, int base = 10);
std::u8string to_u8string(signed long int value, int base = 10);
std::u8string to_u8string(signed long long int value, int base = 10);
std::u8string to_u8string(unsigned char value, int base = 10);
std::u8string to_u8string(unsigned short int value, int base = 10);
std::u8string to_u8string(unsigned int value, int base = 10);
std::u8string to_u8string(unsigned long int value, int base = 10);
std::u8string to_u8string(unsigned long long int value, int base = 10);
std::u8string to_u8string(float value);
std::u8string to_u8string(double value);

std::string to_string(std::u8string_view value);

std::size_t
parse(std::u8string_view s, std::int8_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::int16_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::int32_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::int64_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::uint8_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::uint16_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::uint32_t& value, int base = 10) noexcept;
std::size_t
parse(std::u8string_view s, std::uint64_t& value, int base = 10) noexcept;

std::size_t
parse(std::string_view s, std::int8_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::int16_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::int32_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::int64_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::uint8_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::uint16_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::uint32_t& value, int base = 10) noexcept;
std::size_t
parse(std::string_view s, std::uint64_t& value, int base = 10) noexcept;

consteval bool char_is_ascii() noexcept
{
    return std::ranges::equal(
        // Native
        "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007"
        "\u0008\u0009\u000A\u000B\u000C\u000D\u000E\u000F"
        "\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017"
        "\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F"
        "\u0020\u0021\u0022\u0023\u0024\u0025\u0026\u0027"
        "\u0028\u0029\u002A\u002B\u002C\u002D\u002E\u002F"
        "\u0030\u0031\u0032\u0033\u0034\u0035\u0036\u0037"
        "\u0038\u0039\u003A\u003B\u003C\u003D\u003E\u003F"
        "\u0040\u0041\u0042\u0043\u0044\u0045\u0046\u0047"
        "\u0048\u0049\u004A\u004B\u004C\u004D\u004E\u004F"
        "\u0050\u0051\u0052\u0053\u0054\u0055\u0056\u0057"
        "\u0058\u0059\u005A\u005B\u005C\u005D\u005E\u005F"
        "\u0060\u0061\u0062\u0063\u0064\u0065\u0066\u0067"
        "\u0068\u0069\u006A\u006B\u006C\u006D\u006E\u006F"
        "\u0070\u0071\u0072\u0073\u0074\u0075\u0076\u0077"
        "\u0078\u0079\u007A\u007B\u007C\u007D\u007E\u007F",
        // UTF-8 (ASCII Compatible from U+0000 to U+007F)
        u8"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007"
        u8"\u0008\u0009\u000A\u000B\u000C\u000D\u000E\u000F"
        u8"\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017"
        u8"\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F"
        u8"\u0020\u0021\u0022\u0023\u0024\u0025\u0026\u0027"
        u8"\u0028\u0029\u002A\u002B\u002C\u002D\u002E\u002F"
        u8"\u0030\u0031\u0032\u0033\u0034\u0035\u0036\u0037"
        u8"\u0038\u0039\u003A\u003B\u003C\u003D\u003E\u003F"
        u8"\u0040\u0041\u0042\u0043\u0044\u0045\u0046\u0047"
        u8"\u0048\u0049\u004A\u004B\u004C\u004D\u004E\u004F"
        u8"\u0050\u0051\u0052\u0053\u0054\u0055\u0056\u0057"
        u8"\u0058\u0059\u005A\u005B\u005C\u005D\u005E\u005F"
        u8"\u0060\u0061\u0062\u0063\u0064\u0065\u0066\u0067"
        u8"\u0068\u0069\u006A\u006B\u006C\u006D\u006E\u006F"
        u8"\u0070\u0071\u0072\u0073\u0074\u0075\u0076\u0077"
        u8"\u0078\u0079\u007A\u007B\u007C\u007D\u007E\u007F");
}

template<typename T>
constexpr std::underlying_type_t<T> to_underlying(T value)
{
    return gsl::narrow_cast<std::underlying_type_t<T>>(value);
}

template<typename T>
constexpr bool has_flag(T value, T flag) noexcept
{
    return (to_underlying(value) & to_underlying(flag)) != 0;
}

template<typename T>
T map_value(
    T const& value, T const& in_min, T const& in_max, T const& out_min,
    T const& out_max) noexcept
{
    if (in_min == in_max)
    {
        return std::midpoint(out_min, out_max);
    }
    auto const in_range  = in_max - in_min;
    auto const out_range = out_max - out_min;
    return (value - in_min) * out_range / in_range + out_min;
}

std::ostream& hex_dump(std::ostream& stream, std::span<std::byte const> buffer);

[[noreturn]] void throw_system_error(std::uint32_t error);
[[noreturn]] void throw_system_error();

[[noreturn]] void fail_fast() noexcept;

template<typename C, typename T = std::char_traits<C>>
class basic_format_guard
{
public:
    basic_format_guard(basic_format_guard const&) = delete;
    basic_format_guard(basic_format_guard&&)      = delete;
    basic_format_guard(std::basic_ios<C, T>& stream) :
        m_stream{stream}, m_format{nullptr}
    {
        m_format.copyfmt(m_stream);
    }

    ~basic_format_guard() { m_stream.copyfmt(m_format); }

    basic_format_guard& operator=(basic_format_guard const&) = delete;
    basic_format_guard& operator=(basic_format_guard&&) = delete;

private:
    std::ios& m_stream;
    std::ios m_format;
};

using format_guard = basic_format_guard<char>;

template<typename T, std::size_t N>
constexpr std::size_t array_size(T const (&)[N]) noexcept
{
    return N;
}

#ifdef _MSC_VER
template<typename T>
constexpr T change_endian(T value)
{
    if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(unsigned short))
    {
        return std::bit_cast<T>(
            _byteswap_ushort(std::bit_cast<unsigned short>(value)));
    }
    else if constexpr (
        std::is_integral_v<T> && sizeof(T) == sizeof(unsigned long))
    {
        return std::bit_cast<T>(
            _byteswap_ulong(std::bit_cast<unsigned long>(value)));
    }
    else if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(__int64))
    {
        return std::bit_cast<T>(
            _byteswap_uint64(std::bit_cast<__int64>(value)));
    }
}
#endif

template<typename... T>
class overloaded : public T...
{
public:
    using T::operator()...;
};

template<typename... T>
overloaded(T...) -> overloaded<T...>;

}

#endif
