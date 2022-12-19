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

#ifndef WINDOWER_HOOKS_FFXIMAIN_HPP
#define WINDOWER_HOOKS_FFXIMAIN_HPP

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace windower
{

class ffximain
{
public:
    static void install();
    static void uninstall() noexcept;

    static std::u8string lookup_autotranslate(char32_t code_point);
    static void add_to_chat(
        std::u8string_view text, std::uint8_t type = 206,
        bool indented = false);

    static void const* menu(
        std::u8string_view name,
        std::u8string_view type = u8"menu    ") noexcept;

    template<typename T>
    static T const* menu(
        std::u8string_view name,
        std::u8string_view type = u8"menu    ") noexcept
    {
        return static_cast<T const*>(menu(name, type));
    }
};

struct shared_window_config
{
    std::byte _unknown_00_3F[0x2C];
    std::uint32_t window_type;
    bool window_effect;
    std::uint32_t multi_window;
    std::uint32_t timestamp;
};

struct window_config
{
    std::byte _unknown_00_3F[0x40];
    struct
    {
        std::uint16_t left;
        std::uint16_t top;
        std::uint16_t right;
        std::uint16_t bottom;
    } bounds;
    float _unknown_48_4B;
    std::uint16_t _unknown_4C_4D;
    std::uint16_t max_lines;
    std::uint16_t min_lines;
    std::uint16_t width;
    std::uint16_t resize_time;
    std::uint8_t timestamp_format;
    bool reactive_sizing;
};

}

#endif
