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

#ifndef WINDOWER_SETTINGS_CHANNEL_HPP
#define WINDOWER_SETTINGS_CHANNEL_HPP

#include "utility.hpp"

#include <gsl/gsl>
#include <pugixml.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace windower
{

class settings_channel
{
public:
    settings_channel() noexcept;
    settings_channel(settings_channel const&) = delete;
    settings_channel(settings_channel&&)      = default;

    ~settings_channel();

    settings_channel& operator=(settings_channel const&) = delete;
    settings_channel& operator=(settings_channel&&) = default;

    auto get(u8zstring_view key, std::u8string_view def) const -> std::u8string;
    auto get(u8zstring_view key, char8_t const* def) const -> std::u8string;
    auto get(u8zstring_view key, bool def) const -> bool;
    auto get(u8zstring_view key, float def) const -> float;
    auto get(u8zstring_view key, double def) const -> double;
    auto get(u8zstring_view key, char def) const -> char;
    auto get(u8zstring_view key, signed char def) const -> signed char;
    auto get(u8zstring_view key, short def) const -> short;
    auto get(u8zstring_view key, int def) const -> int;
    auto get(u8zstring_view key, long def) const -> long int;
    auto get(u8zstring_view key, long long def) const -> long long int;
    auto get(u8zstring_view key, unsigned char def) const -> unsigned char;
    auto get(u8zstring_view key, unsigned short def) const -> unsigned short;
    auto get(u8zstring_view key, unsigned def) const -> unsigned;
    auto get(u8zstring_view key, unsigned long def) const -> unsigned long;
    auto get(u8zstring_view key, unsigned long long def) const
        -> unsigned long long;

    template<std::size_t N>
    auto get(u8zstring_view key, char8_t const (&def)[N]) const -> std::u8string
    {
        return get(key, {def, N});
    }

    template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    auto get(u8zstring_view key, T def) const
    {
        auto def_value    = gsl::narrow_cast<std::underlying_type_t<T>>(def);
        auto result_value = get(key, def_value);
        return gsl::narrow_cast<T>(result_value);
    }

    template<
        typename T, typename = std::enable_if_t<
                        std::is_same_v<T, decltype(get(std::declval<T>()))>>>
    auto get(u8zstring_view key) const
    {
        return get(key, T{});
    }

private:
    pugi::xml_document m_document;
    pugi::xml_node m_root;
};

}

#endif
