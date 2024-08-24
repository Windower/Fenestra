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

#include "binding_manager.hpp"

#include "command_manager.hpp"
#include "errors/syntax_error.hpp"
#include "errors/windower_error.hpp"
#include "hooks/ffximain.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <gsl/gsl>

#include <cstddef>
#include <cstdint>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifndef VK_NUMPADENTER
#    define VK_NUMPADENTER 0xFF
#endif

namespace
{

// =========================================================================
// ==                  Binding Descriptor Formal Grammar                  ==
// =========================================================================
//
// binding           → opt_whitespace chord predicate
//
// chord             → key opt_whitespace chord_tail
// chord_tail        → '+' opt_whitespace key opt_whitespace chord_tail | ε
// key               → q_key_name
//                   | dev_or_key_name opt_q_key_name
//                   | special_key_name
// opt_q_key_name    → q_key_name | ε
// q_key_name        → ':' u_key_name
// u_key_name        → ':' | special_key_name | dev_or_key_name
// special_key_name  → '+' | '[' | ']' | '~'
// dev_or_key_name   → name_char_mu opt_name | 'n' dev_or_key_name_n
// dev_or_key_name_n → 'u' dev_or_key_name_u | name_char_mn opt_name | ε
// dev_or_key_name_u → 'm' dev_or_key_name_m | name_char_nu opt_name | ε
// dev_or_key_name_m → '+' | name
//
// predicate         → '[' opt_whitespace flag_list ']' opt_whitespace | ε
// flag_list         → flag flag_list_tail | ε
// flag_list_tail    → whitespace flag flag_list_tail | ε
// flag              → name | '~' name
//
// name              → name_char opt_name
// opt_name          → name_char opt_name | ε
//
// name_char         → 'm' | 'n' | 'u' | *
// name_char_mn      → 'm' | 'n' | *
// name_char_mu      → 'm' | 'u' | *
// name_char_nu      → 'n' | 'u' | *
//
// whitespace        → W opt_whitespace
// opt_whitespace    → W opt_whitespace | ε
//
// -------------------------------------------------------------------------
//
// W = {U+0009, U+000A, U+000B, U+000C, U+000D,
//      U+0020, U+0085, U+00A0, U+1680, U+2000,
//      U+2001, U+2002, U+2003, U+2004, U+2005,
//      U+2006, U+2007, U+2008, U+2009, U+200A,
//      U+2028, U+2029, U+202F, U+205F, U+3000}
//
// * = The complement of {'+', ':', '[', ']', 'm', 'n', 'u', '~'} ⋃ W.

constexpr std::int8_t start_state = 10;

constexpr std::array<std::array<std::int8_t, 11>, 34> parse_table{{
    // clang-format off
    // '+' ':' '[' ']' 'm' 'n' 'u' '~'  W   *   $  //
    {   0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  },
    {  -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1  },
    {  -1, -2,  0, -1, -1, -1, -1, -1, -1, -1, -1  },
    {  -1, -2, -1,  0, -2, -2, -2, -2, -1, -2, -2  },
    {  -1, -1, -1, -1,  0, -1, -1, -2, -1, -1, -1  },
    {  -1, -1, -1, -1, -1,  0, -1, -2, -1, -1, -1  },
    {  -1, -1, -1, -1, -1, -1,  0, -2, -1, -1, -1  },
    {  -1, -2, -1, -2, -1, -1, -1,  0, -1, -1, -1  },
    {  -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1  },
    {  -1, -1, -1, -1, -1, -1, -1, -2, -1,  0, -1  },
    {   1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1  },
    {   2,  2,  2,  2,  2,  2,  2,  2, -2,  2, -1  },
    {   3, -4,  0, -4, -4, -4, -4, -4, -2, -4,  0  },
    {   6,  4,  6,  6,  5,  5,  5,  6, -1,  5, -1  },
    {   0,  4,  0, -2, -2, -2, -2, -2,  0, -2,  0  },
    {  -1,  7, -1, -2, -2, -2, -2, -2, -1, -2, -1  },
    {   6,  8,  6,  6,  9,  9,  9,  6, -1,  9, -1  },
    {  10, -2, 11, 12, -2, -2, -2, 13, -1, -2, -1  },
    {  -1, -1, -1, -2, 14, 15, 14, -2, -1, 14, -1  },
    {   0,  0,  0, -2, 17, 17, 16, -2,  0, 17,  0  },
    {   0,  0,  0, -2, 18, 19, 19, -2,  0, 19,  0  },
    {  10, -1, -1, -2, 20, 20, 20, -2, -1, 20, -1  },
    {  -2, -2, 21, -2, -2, -2, -2, -2, -2, -2,  0  },
    {  -2, -2, -2,  0, 22, 22, 22, 22, -2, 22, -2  },
    {  -2, -2, -2,  0, -2, -2, -2, -2, 23, -2, -2  },
    {  -2, -2, -2, -1, 20, 20, 20, 24, -1, 20, -2  },
    {  -1, -1, -1, -1, 25, 25, 25, -2, -1, 25, -1  },
    {   0,  0,  0,  0, 25, 25, 25, -2,  0, 25,  0  },
    {  -1, -1, -1, -1, 26, 27, 28, -2, -1, 29, -1  },
    {  -1, -1, -1, -2, 26, 27, -1, -2, -1, 29, -1  },
    {  -1, -1, -1, -2, 26, -1, 28, -2, -1, 29, -1  },
    {  -1, -1, -1, -2, -1, 27, 28, -2, -1, 29, -1  },
    {  -2, -2, -2, -2, -1, -1, -1, -1, 30, -1, -2  },
    {   0,  0,  0,  0,  0,  0,  0,  0, 30,  0,  0  },
    // clang-format on
}};

constexpr std::array<std::tuple<std::uint8_t, std::array<std::int8_t, 7>>, 31>
    push_map{{
        {0, {}},
        {3, {22, 11, 33}},
        {5, {12, 33, -2, 13, -1}},
        {7, {12, 33, -2, 13, -1, 33, 0}},
        {1, {15}},
        {2, {14, 18}},
        {1, {17}},
        {4, {16, -1, 1, -1}},
        {1, {1}},
        {1, {18}},
        {1, {0}},
        {1, {2}},
        {1, {3}},
        {1, {7}},
        {2, {27, 30}},
        {2, {19, 5}},
        {2, {20, 6}},
        {2, {27, 29}},
        {2, {21, 4}},
        {2, {27, 31}},
        {1, {26}},
        {5, {33, 3, 23, 33, 2}},
        {4, {24, -3, 25, -1}},
        {5, {24, -3, 25, -1, 32}},
        {3, {26, -1, 7}},
        {2, {27, 28}},
        {1, {4}},
        {1, {5}},
        {1, {6}},
        {1, {9}},
        {2, {33, 8}},
    }};

using iterator = std::u8string_view::const_iterator;

struct parsed_key
{
    std::size_t device_offset;
    std::size_t device_size;
    std::size_t key_offset;
    std::size_t key_size;
};

struct parsed_flag
{
    bool negated;
    std::size_t name_offset;
    std::size_t name_size;
};

struct parse_result
{
    std::vector<parsed_key> keys;
    std::vector<parsed_flag> flags;
    std::size_t end = 0;
};

void mark(parse_result&, std::vector<std::size_t>& marks, std::size_t it)
{
    marks.push_back(it);
}

void complete_key(
    parse_result& result, std::vector<std::size_t>& marks, std::size_t it)
{
    switch (marks.size())
    {
    case 1:
        result.keys.emplace_back(
            gsl::at(marks, 0), gsl::at(marks, 0), gsl::at(marks, 0), it);
        break;
    case 3:
        result.keys.emplace_back(
            gsl::at(marks, 0), gsl::at(marks, 1), gsl::at(marks, 2), it);
        break;
    default:
        throw std::runtime_error{
            "[INTERNAL PARSE ERROR] Unexpected mark count: " +
            std::to_string(marks.size())};
    }
    marks.clear();
}

void complete_flag(
    parse_result& result, std::vector<std::size_t>& marks, std::size_t it)
{
    switch (marks.size())
    {
    case 1: result.flags.emplace_back(true, gsl::at(marks, 0), it); break;
    case 2: result.flags.emplace_back(false, gsl::at(marks, 1), it); break;
    default:
        throw std::runtime_error{
            "[INTERNAL PARSE ERROR] Unexpected mark count: " +
            std::to_string(marks.size())};
    }
    marks.clear();
}

constexpr std::array<
    void (*)(parse_result&, std::vector<std::size_t>&, std::size_t), 31>
    action_table = {mark, complete_key, complete_flag};

std::uint8_t next_token(std::u8string_view string, std::size_t& offset) noexcept
{
    if (string.empty())
    {
        switch (auto const input = windower::next_code_point(string, offset))
        {
        case U'+': return 0;
        case U':': return 1;
        case U'[': return 2;
        case U']': return 3;
        case U'M':
        case U'm':
        case U'\u1D39':
        case U'\u1D50':
        case U'\u2098':
        case U'\u2133':
        case U'\u216F':
        case U'\u217F':
        case U'\u24C2':
        case U'\u24DC':
        case U'\uFF2D':
        case U'\uFF4D':
        case U'\U0001D40C':
        case U'\U0001D426':
        case U'\U0001D440':
        case U'\U0001D45A':
        case U'\U0001D474':
        case U'\U0001D48E':
        case U'\U0001D4C2':
        case U'\U0001D4DC':
        case U'\U0001D4F6':
        case U'\U0001D510':
        case U'\U0001D52A':
        case U'\U0001D544':
        case U'\U0001D55E':
        case U'\U0001D578':
        case U'\U0001D592':
        case U'\U0001D5AC':
        case U'\U0001D5C6':
        case U'\U0001D5E0':
        case U'\U0001D5FA':
        case U'\U0001D614':
        case U'\U0001D62E':
        case U'\U0001D648':
        case U'\U0001D662':
        case U'\U0001D67C':
        case U'\U0001D696':
        case U'\U0001F13C': return 4;
        case U'N':
        case U'n':
        case U'\u1D3A':
        case U'\u207F':
        case U'\u2099':
        case U'\u2115':
        case U'\u24C3':
        case U'\u24DD':
        case U'\uFF2E':
        case U'\uFF4E':
        case U'\U0001D40D':
        case U'\U0001D427':
        case U'\U0001D441':
        case U'\U0001D45B':
        case U'\U0001D475':
        case U'\U0001D48F':
        case U'\U0001D4A9':
        case U'\U0001D4C3':
        case U'\U0001D4DD':
        case U'\U0001D4F7':
        case U'\U0001D511':
        case U'\U0001D52B':
        case U'\U0001D55F':
        case U'\U0001D579':
        case U'\U0001D593':
        case U'\U0001D5AD':
        case U'\U0001D5C7':
        case U'\U0001D5E1':
        case U'\U0001D5FB':
        case U'\U0001D615':
        case U'\U0001D62F':
        case U'\U0001D649':
        case U'\U0001D663':
        case U'\U0001D67D':
        case U'\U0001D697':
        case U'\U0001F13D': return 5;
        case U'U':
        case U'u':
        case U'\u1D41':
        case U'\u1D58':
        case U'\u1D64':
        case U'\u24CA':
        case U'\u24E4':
        case U'\uFF35':
        case U'\uFF55':
        case U'\U0001D414':
        case U'\U0001D42E':
        case U'\U0001D448':
        case U'\U0001D462':
        case U'\U0001D47C':
        case U'\U0001D496':
        case U'\U0001D4B0':
        case U'\U0001D4CA':
        case U'\U0001D4E4':
        case U'\U0001D4FE':
        case U'\U0001D518':
        case U'\U0001D532':
        case U'\U0001D54C':
        case U'\U0001D566':
        case U'\U0001D580':
        case U'\U0001D59A':
        case U'\U0001D5B4':
        case U'\U0001D5CE':
        case U'\U0001D5E8':
        case U'\U0001D602':
        case U'\U0001D61C':
        case U'\U0001D636':
        case U'\U0001D650':
        case U'\U0001D66A':
        case U'\U0001D684':
        case U'\U0001D69E':
        case U'\U0001F144': return 6;
        case U'~': return 7;
        default: return windower::is_whitespace(input) ? 8 : 9;
        }
    }
    return 10;
}

[[noreturn]] void throw_parse_error(
    std::int8_t state, std::int8_t error_type, std::u8string_view source,
    std::size_t error_mark, std::size_t error_begin, std::size_t error_end)
{
    using namespace windower;

    std::u8string error_code;
    error_code.append(u8"BIND:");
    switch (error_type)
    {
    case 0: error_code.append(1, u8'P'); break;
    case 1: error_code.append(1, u8'S'); break;
    default:
        error_code.append(1, u8'X');
        error_code.append(to_u8string(error_type));
        error_code.append(1, u8'-');
        break;
    }
    error_code.append(to_u8string(state));
    throw syntax_error{error_code, source, error_mark, error_begin, error_end};
}

parse_result parse_bind_string(std::u8string_view source, bool match_all)
{
    parse_result result;
    std::vector<std::size_t> marks;
    marks.reserve(3);
    std::vector<std::int8_t> state;
    state.push_back(start_state);
    auto next  = std::size_t{};
    auto index = next;
    auto input = next_token(source, next);
    while (!state.empty())
    {
        auto current_state = state.back();
        state.pop_back();
        if (current_state >= 0)
        {
            auto transition =
                gsl::at(gsl::at(parse_table, current_state), input);
            if (transition < 0)
            {
                if (transition < -2)
                {
                    if (!match_all)
                    {
                        result.end = index;
                        return result;
                    }
                    transition += 2;
                }
                auto const error_mark  = index;
                auto const error_begin = marks.empty() ? index : marks.back();
                auto const error_end   = next;
                throw_parse_error(
                    current_state, -transition - 1, source, error_mark,
                    error_begin, error_end);
            }
            if (input == current_state)
            {
                index = next;
                input = next_token(source, next);
            }
            auto const push_count = std::get<0>(gsl::at(push_map, transition));
            for (std::size_t i = 0; i < push_count; ++i)
            {
                state.push_back(
                    gsl::at(std::get<1>(gsl::at(push_map, transition)), i));
            }
        }
        else
        {
            gsl::at(action_table, -current_state - 1)(result, marks, index);
        }
    }

    if (match_all && index != source.size())
    {
        auto const error_begin = index;
        auto const error_end   = source.size();
        auto const error_mark  = error_begin;
        throw_parse_error(
            start_state, 1, source, error_mark, error_begin, error_end);
    }

    result.end = index;
    return result;
}

constexpr std::uint16_t locale_id(std::uint8_t lang, std::uint8_t sub_lang)
{
    return lang << 8 | sub_lang;
}

std::uint16_t locale_id(std::remove_pointer_t<::HKL> const* hkl) noexcept
{
    auto const value    = std::bit_cast<std::uintptr_t>(hkl);
    auto const lang     = gsl::narrow_cast<std::uint8_t>(value & 0xFF);
    auto const sub_lang = gsl::narrow_cast<std::uint8_t>(value >> 8 & 0xFF);
    return locale_id(lang, sub_lang);
}

constexpr bool locale_matches(std::uint16_t value, std::uint16_t target)
{
    return (value & 0xFF) == 0 ? value == (target & 0xFF00) : value == target;
}

using virtual_key_map_element = std::pair<std::u8string_view, std::uint8_t>;
template<std::size_t N>
using virtual_key_map = std::array<virtual_key_map_element, N>;
template<std::size_t N>
using locale_virtual_key_map = std::array<
    std::pair<std::uint16_t, std::span<virtual_key_map_element const>>, N>;

using name_map_element = std::pair<std::uint8_t, std::u8string_view>;
template<std::size_t N>
using name_map = std::array<name_map_element, N>;
template<std::size_t N>
using locale_name_map =
    std::array<std::pair<std::uint16_t, std::span<name_map_element const>>, N>;

constexpr virtual_key_map<257> virtual_key_map_default{{
    {u8"'", 0xDE},
    {u8",", 0xBC},
    {u8"-", 0xBD},
    {u8".", 0xBE},
    {u8"/", 0xBF},
    {u8"0", 0x30},
    {u8"1", 0x31},
    {u8"2", 0x32},
    {u8"3", 0x33},
    {u8"4", 0x34},
    {u8"5", 0x35},
    {u8"6", 0x36},
    {u8"7", 0x37},
    {u8"8", 0x38},
    {u8"9", 0x39},
    {u8";", 0xBA},
    {u8"=", 0xBB},
    {u8"[", 0xDB},
    {u8"\\", 0xDC},
    {u8"]", 0xDD},
    {u8"`", 0xC0},
    {u8"a", 0x41},
    {u8"alt", 0x12},
    {u8"app", 0x5D},
    {u8"app1", 0xB6},
    {u8"app2", 0xB6},
    {u8"application", 0x5D},
    {u8"application1", 0xB6},
    {u8"application2", 0xB6},
    {u8"applications", 0x5D},
    {u8"apps", 0x5D},
    {u8"b", 0x42},
    {u8"back", 0xA6},
    {u8"back-space", 0x08},
    {u8"backspace", 0x08},
    {u8"bookmarks", 0xAB},
    {u8"break", 0x13},
    {u8"browser-back", 0xA6},
    {u8"browser-bookmarks", 0xAB},
    {u8"browser-favorites", 0xAB},
    {u8"browser-forward", 0xA7},
    {u8"browser-home", 0xAC},
    {u8"browser-refresh", 0xA8},
    {u8"browser-search", 0xAA},
    {u8"browser-stop", 0xA9},
    {u8"browserback", 0xA6},
    {u8"browserbookmarks", 0xAB},
    {u8"browserfavorites", 0xAB},
    {u8"browserforward", 0xA7},
    {u8"browserhome", 0xAC},
    {u8"browserrefresh", 0xA8},
    {u8"browsersearch", 0xAA},
    {u8"browserstop", 0xA9},
    {u8"c", 0x43},
    {u8"caps", 0x14},
    {u8"caps-lock", 0x14},
    {u8"capslock", 0x14},
    {u8"control", 0x11},
    {u8"ctrl", 0x11},
    {u8"d", 0x44},
    {u8"del", 0x2E},
    {u8"delete", 0x2E},
    {u8"down", 0x28},
    {u8"e", 0x45},
    {u8"end", 0x23},
    {u8"enter", 0x0D},
    {u8"esc", 0x1B},
    {u8"escape", 0x1B},
    {u8"f", 0x46},
    {u8"f1", 0x70},
    {u8"f10", 0x79},
    {u8"f11", 0x7A},
    {u8"f12", 0x7B},
    {u8"f13", 0x7C},
    {u8"f14", 0x7D},
    {u8"f15", 0x7E},
    {u8"f16", 0x7F},
    {u8"f17", 0x80},
    {u8"f18", 0x81},
    {u8"f19", 0x82},
    {u8"f2", 0x71},
    {u8"f20", 0x83},
    {u8"f21", 0x84},
    {u8"f22", 0x85},
    {u8"f23", 0x86},
    {u8"f24", 0x87},
    {u8"f3", 0x72},
    {u8"f4", 0x73},
    {u8"f5", 0x74},
    {u8"f6", 0x75},
    {u8"f7", 0x76},
    {u8"f8", 0x77},
    {u8"f9", 0x78},
    {u8"favorites", 0xAB},
    {u8"forward", 0xA7},
    {u8"g", 0x47},
    {u8"h", 0x48},
    {u8"home", 0x24},
    {u8"i", 0x49},
    {u8"ins", 0x2D},
    {u8"insert", 0x2D},
    {u8"j", 0x4A},
    {u8"k", 0x4B},
    {u8"l", 0x4C},
    {u8"l-click", 0x01},
    {u8"l-mouse", 0x01},
    {u8"launch-app1", 0xB6},
    {u8"launch-app2", 0xB6},
    {u8"launch-application1", 0xB6},
    {u8"launch-application2", 0xB6},
    {u8"launch-mail", 0xB4},
    {u8"launch-media-player", 0xB4},
    {u8"launch1", 0xB6},
    {u8"launch2", 0xB6},
    {u8"launchapp1", 0xB6},
    {u8"launchapp2", 0xB6},
    {u8"launchapplication1", 0xB6},
    {u8"launchapplication2", 0xB6},
    {u8"launchmail", 0xB4},
    {u8"launchmediaplayer", 0xB4},
    {u8"lclick", 0x01},
    {u8"left", 0x25},
    {u8"left-mouse", 0x01},
    {u8"leftmouse", 0x01},
    {u8"lmouse", 0x01},
    {u8"m", 0x4D},
    {u8"m-click", 0x04},
    {u8"m-mouse", 0x04},
    {u8"mail", 0xB4},
    {u8"mclick", 0x04},
    {u8"media-player", 0xB4},
    {u8"mediaplayer", 0xB4},
    {u8"menu", 0x5D},
    {u8"middle-mouse", 0x04},
    {u8"middlemouse", 0x04},
    {u8"mmouse", 0x04},
    {u8"mute", 0xAD},
    {u8"n", 0x4E},
    {u8"next-track", 0xB0},
    {u8"nexttrack", 0xB0},
    {u8"num*", 0x6A},
    {u8"num+", 0x6B},
    {u8"num-", 0x6D},
    {u8"num-enter", 0xFF},
    {u8"num-lock", 0x90},
    {u8"num.", 0x6E},
    {u8"num/", 0x6F},
    {u8"num0", 0x60},
    {u8"num1", 0x61},
    {u8"num2", 0x62},
    {u8"num3", 0x63},
    {u8"num4", 0x64},
    {u8"num5", 0x65},
    {u8"num6", 0x66},
    {u8"num7", 0x67},
    {u8"num8", 0x68},
    {u8"num9", 0x69},
    {u8"numenter", 0xFF},
    {u8"numlock", 0x90},
    {u8"num\u00d7", 0x6A},
    {u8"num\u00f7", 0x6F},
    {u8"o", 0x4F},
    {u8"p", 0x50},
    {u8"page-dn", 0x22},
    {u8"page-down", 0x22},
    {u8"page-up", 0x21},
    {u8"pagedn", 0x22},
    {u8"pagedown", 0x22},
    {u8"pageup", 0x21},
    {u8"pause", 0x13},
    {u8"pg-dn", 0x22},
    {u8"pgdn", 0x22},
    {u8"pgup", 0x21},
    {u8"play", 0xB3},
    {u8"play-pause", 0xB3},
    {u8"play/pause", 0xB3},
    {u8"playpause", 0xB3},
    {u8"pr-sc", 0x2C},
    {u8"prev-track", 0xB1},
    {u8"previous-track", 0xB1},
    {u8"previoustrack", 0xB1},
    {u8"prevtrack", 0xB1},
    {u8"print-screen", 0x2C},
    {u8"print-scrn", 0x2C},
    {u8"printscreen", 0x2C},
    {u8"printscrn", 0x2C},
    {u8"prnt-scrn", 0x2C},
    {u8"prntscrn", 0x2C},
    {u8"prsc", 0x2C},
    {u8"prt-sc", 0x2C},
    {u8"prt-scn", 0x2C},
    {u8"prt-scr", 0x2C},
    {u8"prtsc", 0x2C},
    {u8"prtscn", 0x2C},
    {u8"prtscr", 0x2C},
    {u8"q", 0x51},
    {u8"r", 0x52},
    {u8"r-click", 0x02},
    {u8"r-mouse", 0x02},
    {u8"rclick", 0x02},
    {u8"refresh", 0xA8},
    {u8"return", 0x0D},
    {u8"right", 0x27},
    {u8"right-mouse", 0x02},
    {u8"rightmouse", 0x02},
    {u8"rmouse", 0x02},
    {u8"s", 0x53},
    {u8"scroll-lock", 0x91},
    {u8"scrolllock", 0x91},
    {u8"search", 0xAA},
    {u8"shift", 0x10},
    {u8"skip-track", 0xB0},
    {u8"skiptrack", 0xB0},
    {u8"sleep", 0x5F},
    {u8"space", 0x20},
    {u8"stop", 0xB2},
    {u8"sys-req", 0x2C},
    {u8"sys-rq", 0x2C},
    {u8"sysreq", 0x2C},
    {u8"sysrq", 0x2C},
    {u8"t", 0x54},
    {u8"tab", 0x09},
    {u8"u", 0x55},
    {u8"up", 0x26},
    {u8"v", 0x56},
    {u8"vol-dn", 0xAE},
    {u8"vol-down", 0xAE},
    {u8"vol-mute", 0xAD},
    {u8"vol-up", 0xAF},
    {u8"voldn", 0xAE},
    {u8"voldown", 0xAE},
    {u8"volmute", 0xAD},
    {u8"volume-down", 0xAE},
    {u8"volume-mute", 0xAD},
    {u8"volume-up", 0xAF},
    {u8"volumedown", 0xAE},
    {u8"volumemute", 0xAD},
    {u8"volumeup", 0xAF},
    {u8"volup", 0xAF},
    {u8"w", 0x57},
    {u8"win", 0x5B},
    {u8"windows", 0x5B},
    {u8"x", 0x58},
    {u8"x1-click", 0x05},
    {u8"x1-mouse", 0x05},
    {u8"x1click", 0x05},
    {u8"x1mouse", 0x05},
    {u8"x2-click", 0x06},
    {u8"x2-mouse", 0x06},
    {u8"x2click", 0x06},
    {u8"x2mouse", 0x06},
    {u8"y", 0x59},
    {u8"z", 0x5A},
    {u8"\u2190", 0x25},
    {u8"\u2191", 0x26},
    {u8"\u2192", 0x27},
    {u8"\u2193", 0x27},
}};

constexpr virtual_key_map<26> virtual_key_map_ja{{
    {u8";", 0xBB},
    {u8"=", 0x00},
    {u8"@", 0xC0},
    {u8"\\", 0xE2},
    {u8"^", 0xDE},
    {u8"`", 0x00},
    {u8"ax", 0xE1},
    {u8"\u00A5", 0xDC},
    {u8"\u3072\u3089\u304C\u306A", 0xF2},
    {u8"\u30A2\u30D7", 0x5D},
    {u8"\u30A2\u30D7\u30EA\u30B1\u30FC\u30B7\u30E7\u30F3", 0x5D},
    {u8"\u30A6\u30A3\u30F3\u30C9\u30A6\u30BA", 0x5B},
    {u8"\u30AB\u30BF\u30AB\u30CA", 0xF2},
    {u8"\u30AB\u30CA", 0x15},
    {u8"\u30B9\u30DA\u30FC\u30B9", 0x20},
    {u8"\u30B9\u30EA\u30FC\u30D7", 0x5F},
    {u8"\u30E1\u30CB\u30E5\u30FC", 0x5D},
    {u8"\u30ED\u30FC\u30DE\u5B57", 0xF2},
    {u8"\u5168\u89D2", 0xF3},
    {u8"\u534A\u89D2", 0xF3},
    {u8"\u534A\u89D2/\u5168\u89D2", 0xF3},
    {u8"\u534A\u89D2\uFF0F\u5168\u89D2", 0xF3},
    {u8"\u5909\u63DB", 0x1C},
    {u8"\u6F22\u5B57", 0x19},
    {u8"\u7121\u5909\u63DB", 0x1D},
    {u8"\u82F1\u6570", 0xF0},
}};

constexpr virtual_key_map<3> virtual_key_map_en_gb{{
    {u8"#", 0xDE},
    {u8"'", 0xC0},
    {u8"`", 0xDF},
}};

constexpr virtual_key_map<19> virtual_key_map_de{{
    {u8"#", 0xBF},      {u8"'", 0x00},      {u8"+", 0xBB},
    {u8"-", 0xBD},      {u8"/", 0x00},      {u8";", 0x00},
    {u8"<", 0xE2},      {u8"=", 0x00},      {u8"[", 0x00},
    {u8"\\", 0x00},     {u8"]", 0x00},      {u8"^", 0xDC},
    {u8"`", 0x00},      {u8"num,", 0x6E},   {u8"ss", 0xDB},
    {u8"\u00B4", 0xDD}, {u8"\u00E4", 0xDE}, {u8"\u00F6", 0xC0},
    {u8"\u00FC", 0xBA},
}};

constexpr virtual_key_map<37> virtual_key_map_fr{{
    {u8"!", 0xDF},      {u8"\"", 0x33},     {u8"$", 0xBA},
    {u8"&", 0x31},      {u8"'", 0x34},      {u8"(", 0x35},
    {u8")", 0xDB},      {u8"*", 0xDC},      {u8"-", 0x36},
    {u8".", 0x00},      {u8"/", 0x00},      {u8"0", 0x00},
    {u8"1", 0x00},      {u8"2", 0x00},      {u8"3", 0x00},
    {u8"4", 0x00},      {u8"5", 0x00},      {u8"6", 0x00},
    {u8"7", 0x00},      {u8"8", 0x00},      {u8"9", 0x00},
    {u8":", 0xBF},      {u8";", 0xBE},      {u8"<", 0xE2},
    {u8"=", 0x00},      {u8"[", 0x00},      {u8"\\", 0x00},
    {u8"]", 0x00},      {u8"^", 0xDD},      {u8"_", 0x38},
    {u8"`", 0x00},      {u8"\u00B2", 0xDE}, {u8"\u00E0", 0x30},
    {u8"\u00E7", 0x39}, {u8"\u00E8", 0x37}, {u8"\u00E9", 0x32},
    {u8"\u00F9", 0xC0},
}};

constexpr virtual_key_map<15> virtual_key_map_es{{
    {u8"'", 0xDB},
    {u8"+", 0xBB},
    {u8"/", 0x00},
    {u8";", 0x00},
    {u8"<", 0xE2},
    {u8"=", 0x00},
    {u8"[", 0x00},
    {u8"\\", 0x00},
    {u8"]", 0x00},
    {u8"`", 0xBA},
    {u8"\u00A1", 0xDD},
    {u8"\u00B4", 0xDE},
    {u8"\u00BA", 0xDC},
    {u8"\u00E7", 0xBF},
    {u8"\u00F1", 0xC0},
}};

constexpr locale_virtual_key_map<6> virtual_key_maps{{
    {locale_id(LANG_NEUTRAL, SUBLANG_NEUTRAL), virtual_key_map_default},
    {locale_id(LANG_GERMAN, SUBLANG_NEUTRAL), virtual_key_map_de},
    {locale_id(LANG_ENGLISH, SUBLANG_ENGLISH_UK), virtual_key_map_en_gb},
    {locale_id(LANG_SPANISH, SUBLANG_NEUTRAL), virtual_key_map_es},
    {locale_id(LANG_FRENCH, SUBLANG_NEUTRAL), virtual_key_map_fr},
    {locale_id(LANG_JAPANESE, SUBLANG_NEUTRAL), virtual_key_map_ja},
}};

constexpr name_map<136> name_map_default{{
    {0x01, u8"L-Click"},
    {0x02, u8"R-Click"},
    {0x04, u8"M-Click"},
    {0x05, u8"X1-Click"},
    {0x06, u8"X2-Click"},
    {0x08, u8"Backspace"},
    {0x09, u8"Tab"},
    {0x0D, u8"Enter"},
    {0x10, u8"Shift"},
    {0x11, u8"Ctrl"},
    {0x12, u8"Alt"},
    {0x13, u8"Pause"},
    {0x14, u8"Caps-Lock"},
    {0x1B, u8"Esc"},
    {0x20, u8"Space"},
    {0x21, u8"Page-Up"},
    {0x22, u8"Page-Down"},
    {0x23, u8"End"},
    {0x24, u8"Home"},
    {0x25, u8"Left"},
    {0x26, u8"Up"},
    {0x27, u8"Right"},
    {0x28, u8"Down"},
    {0x2C, u8"Print-Screen"},
    {0x2D, u8"Insert"},
    {0x2E, u8"Delete"},
    {0x30, u8"0"},
    {0x31, u8"1"},
    {0x32, u8"2"},
    {0x33, u8"3"},
    {0x34, u8"4"},
    {0x35, u8"5"},
    {0x36, u8"6"},
    {0x37, u8"7"},
    {0x38, u8"8"},
    {0x39, u8"9"},
    {0x41, u8"A"},
    {0x42, u8"B"},
    {0x43, u8"C"},
    {0x44, u8"D"},
    {0x45, u8"E"},
    {0x46, u8"F"},
    {0x47, u8"G"},
    {0x48, u8"H"},
    {0x49, u8"I"},
    {0x4A, u8"J"},
    {0x4B, u8"K"},
    {0x4C, u8"L"},
    {0x4D, u8"M"},
    {0x4E, u8"N"},
    {0x4F, u8"O"},
    {0x50, u8"P"},
    {0x51, u8"Q"},
    {0x52, u8"R"},
    {0x53, u8"S"},
    {0x54, u8"T"},
    {0x55, u8"U"},
    {0x56, u8"V"},
    {0x57, u8"W"},
    {0x58, u8"X"},
    {0x59, u8"Y"},
    {0x5A, u8"Z"},
    {0x5B, u8"Win"},
    {0x5D, u8"Menu"},
    {0x5F, u8"Sleep"},
    {0x60, u8"Num0"},
    {0x61, u8"Num1"},
    {0x62, u8"Num2"},
    {0x63, u8"Num3"},
    {0x64, u8"Num4"},
    {0x65, u8"Num5"},
    {0x66, u8"Num6"},
    {0x67, u8"Num7"},
    {0x68, u8"Num8"},
    {0x69, u8"Num9"},
    {0x6A, u8"Num*"},
    {0x6B, u8"Num+"},
    {0x6D, u8"Num-"},
    {0x6E, u8"Num."},
    {0x6F, u8"Num/"},
    {0x70, u8"F1"},
    {0x71, u8"F2"},
    {0x72, u8"F3"},
    {0x73, u8"F4"},
    {0x74, u8"F5"},
    {0x75, u8"F6"},
    {0x76, u8"F7"},
    {0x77, u8"F8"},
    {0x78, u8"F9"},
    {0x79, u8"F10"},
    {0x7A, u8"F11"},
    {0x7B, u8"F12"},
    {0x7C, u8"F13"},
    {0x7D, u8"F14"},
    {0x7E, u8"F15"},
    {0x7F, u8"F16"},
    {0x80, u8"F17"},
    {0x81, u8"F18"},
    {0x82, u8"F19"},
    {0x83, u8"F20"},
    {0x84, u8"F21"},
    {0x85, u8"F22"},
    {0x86, u8"F23"},
    {0x87, u8"F24"},
    {0x90, u8"Num-Lock"},
    {0x91, u8"Scroll-Lock"},
    {0xA6, u8"Browser-Back"},
    {0xA7, u8"Browser-Forward"},
    {0xA8, u8"Browser-Refresh"},
    {0xA9, u8"Browser-Stop"},
    {0xAA, u8"Browser-Search"},
    {0xAB, u8"Browser-Bookmarks"},
    {0xAC, u8"Browser-Home"},
    {0xAD, u8"Volume-Mute"},
    {0xAE, u8"Volume-Down"},
    {0xAF, u8"Volume-Up"},
    {0xB0, u8"Next-Track"},
    {0xB1, u8"Previous-Track"},
    {0xB2, u8"Stop"},
    {0xB3, u8"Play/Pause"},
    {0xB4, u8"Launch-Mail"},
    {0xB4, u8"Launch-Media-Player"},
    {0xB6, u8"Launch-App1"},
    {0xB6, u8"Launch-App2"},
    {0xBA, u8";"},
    {0xBB, u8"="},
    {0xBC, u8","},
    {0xBD, u8"-"},
    {0xBE, u8"."},
    {0xBF, u8"/"},
    {0xC0, u8"`"},
    {0xDB, u8"["},
    {0xDC, u8"\\"},
    {0xDD, u8"]"},
    {0xDE, u8"'"},
    {0xFF, u8"Num-Enter"},
}};

constexpr name_map<24> name_map_ja{{
    {0x15, u8"\u30AB\u30CA"},
    {0x19, u8"\u6F22\u5B57"},
    {0x1C, u8"\u5909\u63DB"},
    {0x1D, u8"\u7121\u5909\u63DB"},
    {0x20, u8"\u30B9\u30DA\u30FC\u30B9"},
    {0x5B, u8"Windows"},
    {0x5D, u8"\u30E1\u30CB\u30E5\u30FC"},
    {0x5F, u8"\u30B9\u30EA\u30FC\u30D7"},
    {0xA0, u8"\u5DE6Shift"},
    {0xA1, u8"\u53F3Shift"},
    {0xA2, u8"\u5DE6Ctrl"},
    {0xA3, u8"\u53F3Ctrl"},
    {0xA4, u8"\u5DE6Alt"},
    {0xA5, u8"\u53F3Alt"},
    {0xBA, u8":"},
    {0xBB, u8";"},
    {0xC0, u8"@"},
    {0xDC, u8"\u00A5"},
    {0xDE, u8"^"},
    {0xE1, u8"AX"},
    {0xE2, u8"\\"},
    {0xF0, u8"\u82F1\u6570"},
    {0xF2, u8"\u3072\u3089\u304C\u306A"},
    {0xF3, u8"\u534A\u89D2/\u5168\u89D2"},
}};

constexpr name_map<3> name_map_en_gb{{
    {0xC0, u8"'"},
    {0xDE, u8"#"},
    {0xDF, u8"`"},
}};

constexpr name_map<11> name_map_de{{
    {0x6E, u8"Num,"},
    {0xBA, u8"\u00DC"},
    {0xBB, u8"+"},
    {0xBD, u8"-"},
    {0xBF, u8"#"},
    {0xC0, u8"\u00D6"},
    {0xDB, u8"\u00DF"},
    {0xDC, u8"^"},
    {0xDD, u8"\u00B4"},
    {0xDE, u8"\u00C4"},
    {0xE2, u8"<"},
}};

constexpr name_map<20> name_map_fr{{
    {0x30, u8"\u00C0"}, {0x31, u8"&"},      {0x32, u8"\u00C9"},
    {0x33, u8"\""},     {0x34, u8"'"},      {0x35, u8"("},
    {0x36, u8"-"},      {0x37, u8"\u00C8"}, {0x38, u8"_"},
    {0x39, u8"\u00C7"}, {0xBA, u8"$"},      {0xBE, u8";"},
    {0xBF, u8":"},      {0xC0, u8"\u00D9"}, {0xDB, u8")"},
    {0xDC, u8"*"},      {0xDD, u8"^"},      {0xDE, u8"\u00B2"},
    {0xDF, u8"!"},      {0xE2, u8"<"},
}};

constexpr name_map<9> name_map_es{{
    {0xBA, u8"`"},
    {0xBB, u8"+"},
    {0xBF, u8"\u00C7"},
    {0xC0, u8"\u00D1"},
    {0xDB, u8"'"},
    {0xDC, u8"\u00BA"},
    {0xDD, u8"\u00A1"},
    {0xDE, u8"\u00B4"},
    {0xE2, u8"<"},
}};

constexpr locale_name_map<6> name_maps{{
    {locale_id(LANG_NEUTRAL, SUBLANG_NEUTRAL), name_map_default},
    {locale_id(LANG_JAPANESE, SUBLANG_NEUTRAL), name_map_ja},
    {locale_id(LANG_ENGLISH, SUBLANG_ENGLISH_UK), name_map_en_gb},
    {locale_id(LANG_GERMAN, SUBLANG_NEUTRAL), name_map_de},
    {locale_id(LANG_FRENCH, SUBLANG_NEUTRAL), name_map_fr},
    {locale_id(LANG_SPANISH, SUBLANG_NEUTRAL), name_map_es},
}};

template<typename TKey, typename TValue>
std::optional<TValue>
lookup(TKey const& key, std::span<std::pair<TKey, TValue> const> const& span)
{
    auto const it = std::lower_bound(
        span.begin(), span.end(), key,
        [](auto const& e, auto v) { return e.first < v; });
    if (it == span.end() || it->first != key)
    {
        return {};
    }

    return std::make_optional(it->second);
}

std::uint8_t get_virtual_key(
    std::u8string_view key_name, std::remove_pointer_t<::HKL> const* hkl)
{
    auto const lang_id = locale_id(hkl);
    for (auto const& pair : virtual_key_maps)
    {
        if (locale_matches(pair.first, lang_id))
        {
            auto const key = lookup(key_name, pair.second);
            if (key.has_value())
            {
                return key.value();
            }
            break;
        }
    }

    auto const key = lookup(key_name, virtual_key_maps.front().second);
    if (key.has_value())
    {
        return key.value();
    }

    return 0;
}

std::uint8_t get_virtual_key(std::uint16_t scan_code, ::HKL hkl)
{
    switch (scan_code)
    {
    case 0x1D: return VK_CONTROL;
    case 0x2A: return VK_SHIFT;
    case 0x38: return VK_MENU;
    case 0x47: return VK_NUMPAD7;
    case 0x48: return VK_NUMPAD8;
    case 0x49: return VK_NUMPAD9;
    case 0x4B: return VK_NUMPAD4;
    case 0x4C: return VK_NUMPAD5;
    case 0x4D: return VK_NUMPAD6;
    case 0x4F: return VK_NUMPAD1;
    case 0x50: return VK_NUMPAD2;
    case 0x51: return VK_NUMPAD3;
    case 0x52: return VK_NUMPAD0;
    case 0x9C: return VK_NUMPADENTER;
    case 0xB5: return VK_DIVIDE;
    case 0xC7: return VK_HOME;
    case 0xC9: return VK_PRIOR;
    case 0xCF: return VK_END;
    case 0xD1: return VK_NEXT;
    case 0xD2: return VK_INSERT;
    case 0xD3: return VK_DELETE;
    case 0xDB: return VK_LWIN;
    case 0xFB: return VK_LBUTTON;
    case 0xFC: return VK_RBUTTON;
    case 0xFD: return VK_MBUTTON;
    case 0xFE: return VK_XBUTTON1;
    case 0xFF: return VK_XBUTTON2;
    default:
        return gsl::narrow_cast<std::uint8_t>(
            ::MapVirtualKeyExW(scan_code, MAPVK_VSC_TO_VK_EX, hkl));
    }
}

std::uint8_t get_system_key(std::u8string_view key_name)
{
    auto const hkl         = ::GetKeyboardLayout(0);
    auto const virtual_key = get_virtual_key(key_name, hkl);
    switch (virtual_key)
    {
    case VK_LBUTTON: return 0xFB;
    case VK_RBUTTON: return 0xFC;
    case VK_MBUTTON: return 0xFD;
    case VK_XBUTTON1: return 0xFE;
    case VK_XBUTTON2: return 0xFF;
    case VK_PRIOR: return 0xC9;
    case VK_NEXT: return 0xD1;
    case VK_END: return 0xCF;
    case VK_HOME: return 0xC7;
    case VK_INSERT: return 0xD2;
    case VK_DELETE: return 0xD3;
    case VK_LWIN: return 0xDB;
    case VK_NUMPAD0: return 0x52;
    case VK_NUMPAD1: return 0x4F;
    case VK_NUMPAD2: return 0x50;
    case VK_NUMPAD3: return 0x51;
    case VK_NUMPAD4: return 0x4B;
    case VK_NUMPAD5: return 0x4C;
    case VK_NUMPAD6: return 0x4D;
    case VK_NUMPAD7: return 0x47;
    case VK_NUMPAD8: return 0x48;
    case VK_NUMPAD9: return 0x49;
    case VK_DIVIDE: return 0xB5;
    case VK_NUMPADENTER: return 0x9C;
    default:
        return gsl::narrow_cast<std::uint8_t>(
            ::MapVirtualKeyExW(virtual_key, MAPVK_VK_TO_VSC_EX, hkl));
    case 0: return 0;
    }
}

std::u8string get_system_key_name(std::uint8_t scan_code)
{
    auto const hkl         = ::GetKeyboardLayout(0);
    auto const virtual_key = get_virtual_key(scan_code, hkl);
    if (virtual_key == 0)
    {
        return u8"[scan-code:" + windower::to_u8string(scan_code) + u8"]";
    }

    auto const lang_id = locale_id(hkl);
    for (auto const& pair : name_maps)
    {
        if (locale_matches(pair.first, lang_id))
        {
            auto const key = lookup(virtual_key, pair.second);
            if (key.has_value())
            {
                return std::u8string{key.value()};
            }
            break;
        }
    }

    auto const key = lookup(virtual_key, name_maps.front().second);
    if (key.has_value())
    {
        return std::u8string{key.value()};
    }

    return u8"[virtual-key:" + windower::to_u8string(scan_code) + u8"]";
}

}

bool windower::binding_manager::binding::operator==(
    binding_manager::binding const& other) const
{
    return device_mask == other.device_mask &&
           predicate_mask == other.predicate_mask &&
           predicate == other.predicate && keys == other.keys;
}

bool windower::binding_manager::binding::operator<(
    binding_manager::binding const& other) const noexcept
{
    auto lhs_key_count       = keys.size();
    auto rhs_key_count       = other.keys.size();
    auto lhs_predicate_count = predicate_mask.count();
    auto rhs_predicate_count = other.predicate_mask.count();
    return std::tie(lhs_key_count, lhs_predicate_count) <
           std::tie(rhs_key_count, rhs_predicate_count);
}

windower::binding_manager::flag windower::binding_manager::get_flag(
    std::u8string_view bind_string, std::size_t flag_offset,
    std::size_t flag_size)
{
    auto flag_name = nfkc_fold_case(bind_string.substr(flag_offset, flag_size));
    if (flag_name == u8"up")
    {
        return flag::up;
    }
    if (flag_name == u8"chat")
    {
        return flag::chat;
    }

    throw syntax_error{
        u8"BIND:L5", bind_string, flag_offset, flag_offset,
        flag_offset + flag_size};
}

std::u8string_view
windower::binding_manager::get_flag_name(std::size_t id) noexcept
{
    switch (id)
    {
    case 0: return u8"up";
    case 1: return u8"chat";
    default: return u8"<unknown flag>";
    }
}

std::vector<windower::key_handle> windower::binding_manager::register_device(
    std::u8string_view device_name, std::vector<std::u8string> const& keys)
{
    auto normalized_device_name = nfkc_fold_case(device_name);
    using id_type               = decltype(device_descriptor::id);

    auto const it = m_descriptors.lower_bound(normalized_device_name);
    if (it != m_descriptors.end() && it->first == normalized_device_name)
    {
        throw windower_error{u8"BIND:R1"};
    }

    id_type device_id = 0;
    for (std::size_t i = 0; i < m_devices.size(); ++i)
    {
        if (!std::get<0>(gsl::at(m_devices, i)))
        {
            device_id = gsl::narrow<id_type>(i + 1);

            std::get<0>(gsl::at(m_devices, i)) = true;
            std::get<1>(gsl::at(m_devices, i)).resize(keys.size());
            break;
        }
    }

    if (device_id == 0)
    {
        throw windower_error{u8"BIND:R2"};
    }

    std::vector<key_handle> results;
    device_descriptor descriptor;
    descriptor.id   = device_id;
    descriptor.name = device_name;
    for (std::size_t i = 0; i < keys.size(); ++i)
    {
        auto const key_id                    = gsl::narrow<id_type>(i);
        auto const normalized_key_name       = nfkc_fold_case(gsl::at(keys, i));
        descriptor.keys[normalized_key_name] = {key_id, gsl::at(keys, i)};
        auto key                             = key_handle{};
        key.m_device_id                      = device_id;
        key.m_key_id                         = key_id;
        results.emplace_back(key);
    }
    m_descriptors.emplace_hint(
        it, normalized_device_name, std::move(descriptor));

    return results;
}

void windower::binding_manager::unregister_device(
    std::u8string_view device_name)
{
    auto normalized_device_name = nfkc_fold_case(device_name);

    auto const it = m_descriptors.lower_bound(normalized_device_name);
    if (it == m_descriptors.end() || it->first != normalized_device_name)
    {
        throw windower_error{u8"BIND:U1"};
    }

    auto const device_id = it->second.id;
    auto end             = std::remove_if(
                    m_bindings.begin(), m_bindings.end(),
                    [device_id](auto const& b) { return gsl::at(b.device_mask, device_id); });
    m_bindings.erase(end, m_bindings.end());
    m_descriptors.erase(it);
    std::get<1>(gsl::at(m_devices, device_id)).clear();
    std::get<1>(gsl::at(m_devices, device_id)).shrink_to_fit();
    std::get<0>(gsl::at(m_devices, device_id)) = false;
}

bool windower::binding_manager::key_state(key_handle key) const noexcept
{
    if (key.m_device_id == 0)
    {
        switch (key.m_key_id)
        {
        default: break;
        case 0x36: key.m_key_id = 0x2A; break;
        case 0x9D: key.m_key_id = 0x1D; break;
        case 0xB8: key.m_key_id = 0x38; break;
        case 0xDC: key.m_key_id = 0xDB; break;
        }
        return gsl::at(m_system_device, key.m_key_id);
    }

    return gsl::at(
        std::get<1>(gsl::at(m_devices, key.m_device_id - 1)), key.m_key_id);
}

bool windower::binding_manager::key_state(key_handle key, bool state)
{
    auto const state_changed = key_state(key) != state;

    auto flags = m_flags;
    flags.set(0, !state);

    if (key.m_device_id == 0)
    {
        switch (key.m_key_id)
        {
        default: break;
        case 0x36: key.m_key_id = 0x2A; break;
        case 0x9D: key.m_key_id = 0x1D; break;
        case 0xB8: key.m_key_id = 0x38; break;
        case 0xDC: key.m_key_id = 0xDB; break;
        }
        gsl::at(m_system_device, key.m_key_id) = state;
        if (!state)
        {
            gsl::at(m_system_device_client, key.m_key_id) = 0;
        }
    }
    else
    {
        gsl::at(
            std::get<1>(gsl::at(m_devices, key.m_device_id - 1)),
            key.m_key_id) = state;
    }

    binding const* best_match = nullptr;
    for (auto const& b : m_bindings)
    {
        if (b.device_mask.test(key.m_device_id) &&
            std::find(b.keys.begin(), b.keys.end(), key) != b.keys.end())
        {
            if ((flags & b.predicate_mask) == b.predicate &&
                std::all_of(
                    b.keys.begin(), b.keys.end(),
                    [=](auto const& k) noexcept { return key_state(k); }))
            {
                if (!best_match || *best_match < b)
                {
                    best_match = &b;
                }
            }
        }
    }

    if (best_match)
    {
        if (!state_changed || best_match->handler())
        {
            return true;
        }

        auto end =
            std::remove(m_bindings.begin(), m_bindings.end(), *best_match);
        m_bindings.erase(end, m_bindings.end());
    }

    if (key.m_device_id == 0 && state)
    {
        gsl::at(m_system_device_client, key.m_key_id) = 0x80;
    }
    return false;
}

bool windower::binding_manager::key_state(std::uint8_t key) const noexcept
{
    switch (key)
    {
    default: break;
    case 0x36: key = 0x2A; break;
    case 0x9D: key = 0x1D; break;
    case 0xB8: key = 0x38; break;
    case 0xDC: key = 0xDB; break;
    }
    return gsl::at(m_system_device, key);
}

bool windower::binding_manager::key_state(std::uint8_t key, bool state)
{
    auto handle        = key_handle{};
    handle.m_device_id = 0;
    handle.m_key_id    = key;
    return key_state(handle, state);
}

void windower::binding_manager::lost_focus()
{
    auto handle        = key_handle{};
    handle.m_device_id = 0;
    for (auto i = std::uint8_t{0}; i < 0xFB; ++i)
    {
        handle.m_key_id = i;
        key_state(handle, false);
    }
    std::fill(
        m_system_device_client.begin(), m_system_device_client.end(),
        std::uint8_t{0});
}

void windower::binding_manager::bind(std::u8string_view bind_string)
{
    auto [binding, end] = create_binding(bind_string, false);
    if (end == bind_string.size())
    {
        throw syntax_error{u8"BIND:C1", bind_string, end};
    }
    auto const command_string = std::u8string{bind_string.substr(end)};
    command_manager::instance().validate_command(command_string);
    binding.handler = [command_string = command_string]() {
        command_manager::instance().handle_command(
            command_string, command_source::binding);
        return true;
    };
    binding.handler_display = command_string;
    insert_binding(std::move(binding));
}

void windower::binding_manager::bind(
    std::u8string_view bind_string, std::u8string_view command_string)
{
    command_manager::instance().validate_command(command_string);
    bind(
        bind_string,
        [command_string = command_string]() {
            command_manager::instance().handle_command(
                command_string, command_source::binding);
            return true;
        },
        command_string);
}

void windower::binding_manager::bind(
    std::u8string_view bind_string, std::function<bool()> const& handler,
    std::u8string_view handler_display)
{
    auto tuple                         = create_binding(bind_string, true);
    std::get<0>(tuple).handler         = handler;
    std::get<0>(tuple).handler_display = handler_display;
    insert_binding(std::move(std::get<0>(tuple)));
}

void windower::binding_manager::unbind(std::u8string_view bind_string)
{
    auto tuple = create_binding(bind_string, true);
    auto end =
        std::remove(m_bindings.begin(), m_bindings.end(), std::get<0>(tuple));
    m_bindings.erase(end, m_bindings.end());
}

std::vector<std::pair<std::u8string, std::u8string>>
windower::binding_manager::get_binds() const
{
    std::vector<std::pair<std::u8string, std::u8string>> results;
    for (auto const& b : m_bindings)
    {
        std::u8string bind_string;
        for (auto const& k : b.keys)
        {
            bind_string.append(!bind_string.empty(), u8'+');
            bind_string.append(get_name(k));
        }

        if (b.predicate_mask.to_ullong() != 1 || b.predicate.test(0))
        {
            bind_string.append(1, u'[');
            if (b.predicate.test(0))
            {
                bind_string.append(get_flag_name(0));
            }
            for (std::size_t i = 1; i < b.predicate_mask.size(); ++i)
            {
                if (b.predicate_mask.test(i))
                {
                    bind_string.append(bind_string.back() != u8'[', u8' ');
                    bind_string.append(!b.predicate.test(i), u8'~');
                    bind_string.append(get_flag_name(i));
                }
            }
            bind_string.append(1, u']');
        }
        results.emplace_back(std::move(bind_string), b.handler_display);
    }
    return results;
}

std::array<std::uint8_t, 256> const&
windower::binding_manager::client_state() const noexcept
{
    return m_system_device_client;
}

std::optional<::LRESULT>
windower::binding_manager::process_message(::MSG const& msg) noexcept
{
    auto const key = msg.lParam >> 16 | msg.lParam >> 17 & 0x80;
    switch (msg.message)
    {
    default: break;

    case WM_KILLFOCUS: lost_focus(); break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (key_state(gsl::narrow_cast<std::uint8_t>(key), true))
        {
            return TRUE;
        }
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (key_state(gsl::narrow_cast<std::uint8_t>(key), false))
        {
            return TRUE;
        }
        break;
    }
    return std::nullopt;
}

std::tuple<windower::binding_manager::binding, std::size_t>
windower::binding_manager::create_binding(
    std::u8string_view bind_string, bool match_all) const
{
    auto result = parse_bind_string(bind_string, match_all);
    binding b;
    for (auto const& [device_offset, device_size, key_offset, key_size] :
         result.keys)
    {
        auto key = key_handle{};
        if (device_offset == key_offset)
        {
            key = find(bind_string, key_offset, key_size);
        }
        else
        {
            key = find(
                bind_string, device_offset, device_size, key_offset, key_size);
        }
        b.device_mask.set(key.m_device_id);
        b.keys.push_back(key);
    }
    for (auto const& [negated, name_offset, name_size] : result.flags)
    {
        auto const id    = get_flag(bind_string, name_offset, name_size);
        auto const index = std::to_underlying(id);
        b.predicate_mask.set(index);
        b.predicate.set(index, negated);
    }
    b.predicate_mask.set(0);
    return {b, result.end};
}

void windower::binding_manager::insert_binding(binding&& binding)
{
    auto it  = std::lower_bound(m_bindings.begin(), m_bindings.end(), binding);
    it       = m_bindings.emplace(it, binding);
    auto end = std::upper_bound(it + 1, m_bindings.end(), *it);
    m_bindings.erase(std::remove(it + 1, end, binding), end);
}

std::u8string windower::binding_manager::get_name(key_handle handle) const
{
    if (handle.m_device_id == 0)
    {
        auto key_name = get_system_key_name(handle.m_key_id);

        auto normalized_key_name = nfkc_fold_case(key_name);
        auto is_ambiguous        = false;
        for (auto& pair : m_descriptors)
        {
            auto& descriptor = pair.second;
            auto match_it    = std::find_if(
                   descriptor.keys.begin(), descriptor.keys.end(),
                   [&normalized_key_name](auto& k) noexcept {
                    return k.first == normalized_key_name;
                   });
            if (match_it != descriptor.keys.end())
            {
                is_ambiguous = true;
                break;
            }
        }

        return is_ambiguous ? u8":" + key_name : key_name;
    }

    auto device_it = std::find_if(
        m_descriptors.begin(), m_descriptors.end(),
        [handle](auto const& p) { return p.second.id == handle.m_device_id; });
    if (device_it == m_descriptors.end())
    {
        std::u8string name;
        name.assign(u8"[device:");
        name.assign(to_u8string(handle.m_device_id));
        name.assign(u8"]:[key:");
        name.assign(to_u8string(handle.m_key_id));
        name.assign(1, u8']');
        return name;
    }
    else
    {
        auto const& keys = device_it->second.keys;
        auto key_it =
            std::find_if(keys.begin(), keys.end(), [handle](auto const& p) {
                return std::get<0>(p.second) == handle.m_key_id;
            });

        if (key_it != keys.end())
        {
            auto normalized_key_name =
                nfkc_fold_case(std::get<1>(key_it->second));
            auto is_ambiguous = false;
            if (get_system_key(normalized_key_name))
            {
                is_ambiguous = true;
            }
            else
            {
                for (auto& pair : m_descriptors)
                {
                    if (pair.second.id != handle.m_device_id)
                    {
                        auto& descriptor = pair.second;
                        auto match_it    = std::find_if(
                               descriptor.keys.begin(), descriptor.keys.end(),
                               [&normalized_key_name](auto& k) noexcept {
                                return k.first == normalized_key_name;
                               });
                        if (match_it != descriptor.keys.end())
                        {
                            is_ambiguous = true;
                            break;
                        }
                    }
                }
            }

            return is_ambiguous ? device_it->second.name + u8":" +
                                      std::get<1>(key_it->second)
                                : std::get<1>(key_it->second);
        }

        std::u8string name;
        name.assign(device_it->second.name);
        name.assign(u8":[key:");
        name.assign(to_u8string(handle.m_key_id));
        name.assign(1, u8']');
        return name;
    }
}

windower::key_handle windower::binding_manager::find(
    std::u8string_view bind_string, std::size_t key_name_offset,
    std::size_t key_name_size) const
{
    auto key_name =
        nfkc_fold_case(bind_string.substr(key_name_offset, key_name_size));

    std::vector<key_handle> matching_keys;
    matching_keys.reserve(1);

    if (auto const id = get_system_key(key_name))
    {
        auto key        = key_handle{};
        key.m_device_id = 0;
        key.m_key_id    = id;
        matching_keys.push_back(key);
    }

    for (auto& pair : m_descriptors)
    {
        auto& descriptor = pair.second;
        auto key_it      = std::find_if(
                 descriptor.keys.begin(), descriptor.keys.end(),
                 [&key_name](auto& k) noexcept { return k.first == key_name; });
        if (key_it != descriptor.keys.end())
        {
            auto key        = key_handle{};
            key.m_device_id = descriptor.id;
            key.m_key_id    = std::get<0>(key_it->second);
            matching_keys.push_back(key);
        }
    }

    if (matching_keys.size() == 1)
    {
        return gsl::at(matching_keys, 0);
    }

    if (matching_keys.empty())
    {
        throw syntax_error{
            u8"BIND:L1", bind_string, key_name_offset, key_name_offset,
            key_name_offset + key_name_size};
    }

    std::vector<std::u8string> options;
    std::transform(
        matching_keys.begin(), matching_keys.end(), std::back_inserter(options),
        [this](auto const& k) { return get_name(k); });
    throw syntax_error{
        u8"BIND:L2",
        bind_string,
        key_name_offset,
        key_name_offset,
        key_name_offset + key_name_size,
        std::move(options)};
}

windower::key_handle windower::binding_manager::find(
    std::u8string_view bind_string, std::size_t device_name_offset,
    std::size_t device_name_size, std::size_t key_name_offset,
    std::size_t key_name_size) const
{
    auto device_name = nfkc_fold_case(
        bind_string.substr(device_name_offset, device_name_size));
    auto key_name =
        nfkc_fold_case(bind_string.substr(key_name_offset, key_name_size));

    if (device_name.empty())
    {
        auto key        = key_handle{};
        key.m_device_id = 0;
        key.m_key_id    = get_system_key(key_name);
        return key;
    }

    auto device_it = std::find_if(
        m_descriptors.begin(), m_descriptors.end(),
        [&device_name](auto& d) noexcept { return d.first == device_name; });
    if (device_it == m_descriptors.end())
    {
        throw syntax_error{
            u8"BIND:L3", bind_string, device_name_offset, device_name_offset,
            device_name_offset + device_name_size};
    }
    auto& descriptor = device_it->second;

    auto key_it = std::find_if(
        descriptor.keys.begin(), descriptor.keys.end(),
        [&key_name](auto& k) noexcept { return k.first == key_name; });
    if (key_it == descriptor.keys.end())
    {
        throw syntax_error{
            u8"BIND:L4", bind_string, key_name_offset, key_name_offset,
            key_name_offset + device_name_size};
    }

    auto key        = key_handle{};
    key.m_device_id = descriptor.id;
    key.m_key_id    = std::get<0>(key_it->second);
    return key;
}
