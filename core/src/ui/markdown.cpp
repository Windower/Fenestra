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

#include "ui/markdown.hpp"

#include <dwrite.h>

#include <gsl/gsl>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace windower::ui
{
namespace
{

constexpr std::array<std::pair<wchar_t const*, ::DWRITE_FONT_STRETCH>, 10>
    stretch_values{{
        {L"condensed", ::DWRITE_FONT_STRETCH_CONDENSED},
        {L"expanded", ::DWRITE_FONT_STRETCH_EXPANDED},
        {L"extra_condensed", ::DWRITE_FONT_STRETCH_EXTRA_CONDENSED},
        {L"extra_expanded", ::DWRITE_FONT_STRETCH_EXTRA_EXPANDED},
        {L"medium", ::DWRITE_FONT_STRETCH_MEDIUM},
        {L"normal", ::DWRITE_FONT_STRETCH_NORMAL},
        {L"semi_condensed", ::DWRITE_FONT_STRETCH_SEMI_CONDENSED},
        {L"semi_expanded", ::DWRITE_FONT_STRETCH_SEMI_EXPANDED},
        {L"ultra_condensed", ::DWRITE_FONT_STRETCH_ULTRA_CONDENSED},
        {L"ultra_expanded", ::DWRITE_FONT_STRETCH_ULTRA_EXPANDED},
    }};

constexpr std::array<std::pair<wchar_t const*, ::DWRITE_FONT_WEIGHT>, 17>
    weight_values{{
        {L"black", ::DWRITE_FONT_WEIGHT_BLACK},
        {L"bold", ::DWRITE_FONT_WEIGHT_BOLD},
        {L"demi_bold", ::DWRITE_FONT_WEIGHT_DEMI_BOLD},
        {L"extra_black", ::DWRITE_FONT_WEIGHT_EXTRA_BLACK},
        {L"extra_bold", ::DWRITE_FONT_WEIGHT_EXTRA_BOLD},
        {L"extra_light", ::DWRITE_FONT_WEIGHT_EXTRA_LIGHT},
        {L"heavy", ::DWRITE_FONT_WEIGHT_HEAVY},
        {L"light", ::DWRITE_FONT_WEIGHT_LIGHT},
        {L"medium", ::DWRITE_FONT_WEIGHT_MEDIUM},
        {L"normal", ::DWRITE_FONT_WEIGHT_NORMAL},
        {L"regular", ::DWRITE_FONT_WEIGHT_REGULAR},
        {L"semi_bold", ::DWRITE_FONT_WEIGHT_SEMI_BOLD},
        {L"semi_light", ::DWRITE_FONT_WEIGHT_SEMI_LIGHT},
        {L"thin", ::DWRITE_FONT_WEIGHT_THIN},
        {L"ultra_black", ::DWRITE_FONT_WEIGHT_ULTRA_BLACK},
        {L"ultra_bold", ::DWRITE_FONT_WEIGHT_ULTRA_BOLD},
        {L"ultra_light", ::DWRITE_FONT_WEIGHT_LIGHT},
    }};

constexpr std::array<std::pair<wchar_t const*, ::DWRITE_FONT_STYLE>, 3>
    style_values{{
        {L"italic", ::DWRITE_FONT_STYLE_ITALIC},
        {L"normal", ::DWRITE_FONT_STYLE_NORMAL},
        {L"oblique", ::DWRITE_FONT_STYLE_OBLIQUE},
    }};

constexpr std::array<std::pair<wchar_t const*, size_unit>, 6> unit_values{{
    {L"", size_unit::point},
    {L"%", size_unit::relative},
    {L"em", size_unit::relative},
    {L"pc", size_unit::pica},
    {L"pt", size_unit::point},
    {L"px", size_unit::pixel},
}};

constexpr std::array<std::pair<wchar_t const*, color>, 147> color_values{{
    {L"aliceblue", colors::aliceblue},
    {L"antiquewhite", colors::antiquewhite},
    {L"aqua", colors::aqua},
    {L"aquamarine", colors::aquamarine},
    {L"azure", colors::azure},
    {L"beige", colors::beige},
    {L"bisque", colors::bisque},
    {L"black", colors::black},
    {L"blanchedalmond", colors::blanchedalmond},
    {L"blue", colors::blue},
    {L"blueviolet", colors::blueviolet},
    {L"brown", colors::brown},
    {L"burlywood", colors::burlywood},
    {L"cadetblue", colors::cadetblue},
    {L"chartreuse", colors::chartreuse},
    {L"chocolate", colors::chocolate},
    {L"coral", colors::coral},
    {L"cornflowerblue", colors::cornflowerblue},
    {L"cornsilk", colors::cornsilk},
    {L"crimson", colors::crimson},
    {L"cyan", colors::cyan},
    {L"darkblue", colors::darkblue},
    {L"darkcyan", colors::darkcyan},
    {L"darkgoldenrod", colors::darkgoldenrod},
    {L"darkgray", colors::darkgray},
    {L"darkgreen", colors::darkgreen},
    {L"darkgrey", colors::darkgrey},
    {L"darkkhaki", colors::darkkhaki},
    {L"darkmagenta", colors::darkmagenta},
    {L"darkolivegreen", colors::darkolivegreen},
    {L"darkorange", colors::darkorange},
    {L"darkorchid", colors::darkorchid},
    {L"darkred", colors::darkred},
    {L"darksalmon", colors::darksalmon},
    {L"darkseagreen", colors::darkseagreen},
    {L"darkslateblue", colors::darkslateblue},
    {L"darkslategray", colors::darkslategray},
    {L"darkslategrey", colors::darkslategrey},
    {L"darkturquoise", colors::darkturquoise},
    {L"darkviolet", colors::darkviolet},
    {L"deeppink", colors::deeppink},
    {L"deepskyblue", colors::deepskyblue},
    {L"dimgray", colors::dimgray},
    {L"dimgrey", colors::dimgrey},
    {L"dodgerblue", colors::dodgerblue},
    {L"firebrick", colors::firebrick},
    {L"floralwhite", colors::floralwhite},
    {L"forestgreen", colors::forestgreen},
    {L"fuchsia", colors::fuchsia},
    {L"gainsboro", colors::gainsboro},
    {L"ghostwhite", colors::ghostwhite},
    {L"gold", colors::gold},
    {L"goldenrod", colors::goldenrod},
    {L"gray", colors::gray},
    {L"green", colors::green},
    {L"greenyellow", colors::greenyellow},
    {L"grey", colors::grey},
    {L"honeydew", colors::honeydew},
    {L"hotpink", colors::hotpink},
    {L"indianred", colors::indianred},
    {L"indigo", colors::indigo},
    {L"ivory", colors::ivory},
    {L"khaki", colors::khaki},
    {L"lavender", colors::lavender},
    {L"lavenderblush", colors::lavenderblush},
    {L"lawngreen", colors::lawngreen},
    {L"lemonchiffon", colors::lemonchiffon},
    {L"lightblue", colors::lightblue},
    {L"lightcoral", colors::lightcoral},
    {L"lightcyan", colors::lightcyan},
    {L"lightgoldenrodyellow", colors::lightgoldenrodyellow},
    {L"lightgray", colors::lightgray},
    {L"lightgreen", colors::lightgreen},
    {L"lightgrey", colors::lightgrey},
    {L"lightpink", colors::lightpink},
    {L"lightsalmon", colors::lightsalmon},
    {L"lightseagreen", colors::lightseagreen},
    {L"lightskyblue", colors::lightskyblue},
    {L"lightslategray", colors::lightslategray},
    {L"lightslategrey", colors::lightslategrey},
    {L"lightsteelblue", colors::lightsteelblue},
    {L"lightyellow", colors::lightyellow},
    {L"lime", colors::lime},
    {L"limegreen", colors::limegreen},
    {L"linen", colors::linen},
    {L"magenta", colors::magenta},
    {L"maroon", colors::maroon},
    {L"mediumaquamarine", colors::mediumaquamarine},
    {L"mediumblue", colors::mediumblue},
    {L"mediumorchid", colors::mediumorchid},
    {L"mediumpurple", colors::mediumpurple},
    {L"mediumseagreen", colors::mediumseagreen},
    {L"mediumslateblue", colors::mediumslateblue},
    {L"mediumspringgreen", colors::mediumspringgreen},
    {L"mediumturquoise", colors::mediumturquoise},
    {L"mediumvioletred", colors::mediumvioletred},
    {L"midnightblue", colors::midnightblue},
    {L"mintcream", colors::mintcream},
    {L"mistyrose", colors::mistyrose},
    {L"moccasin", colors::moccasin},
    {L"navajowhite", colors::navajowhite},
    {L"navy", colors::navy},
    {L"oldlace", colors::oldlace},
    {L"olive", colors::olive},
    {L"olivedrab", colors::olivedrab},
    {L"orange", colors::orange},
    {L"orangered", colors::orangered},
    {L"orchid", colors::orchid},
    {L"palegoldenrod", colors::palegoldenrod},
    {L"palegreen", colors::palegreen},
    {L"paleturquoise", colors::paleturquoise},
    {L"palevioletred", colors::palevioletred},
    {L"papayawhip", colors::papayawhip},
    {L"peachpuff", colors::peachpuff},
    {L"peru", colors::peru},
    {L"pink", colors::pink},
    {L"plum", colors::plum},
    {L"powderblue", colors::powderblue},
    {L"purple", colors::purple},
    {L"red", colors::red},
    {L"rosybrown", colors::rosybrown},
    {L"royalblue", colors::royalblue},
    {L"saddlebrown", colors::saddlebrown},
    {L"salmon", colors::salmon},
    {L"sandybrown", colors::sandybrown},
    {L"seagreen", colors::seagreen},
    {L"seashell", colors::seashell},
    {L"sienna", colors::sienna},
    {L"silver", colors::silver},
    {L"skyblue", colors::skyblue},
    {L"slateblue", colors::slateblue},
    {L"slategray", colors::slategray},
    {L"slategrey", colors::slategrey},
    {L"snow", colors::snow},
    {L"springgreen", colors::springgreen},
    {L"steelblue", colors::steelblue},
    {L"tan", colors::tan},
    {L"teal", colors::teal},
    {L"thistle", colors::thistle},
    {L"tomato", colors::tomato},
    {L"turquoise", colors::turquoise},
    {L"violet", colors::violet},
    {L"wheat", colors::wheat},
    {L"white", colors::white},
    {L"whitesmoke", colors::whitesmoke},
    {L"yellow", colors::yellow},
    {L"yellowgreen", colors::yellowgreen},
}};

constexpr std::array<std::pair<wchar_t const*, font_size>, 9> size_values{{
    {L"large", {size_unit::absolute, 1.15f}},
    {L"larger", {size_unit::relative, 1.15f}},
    {L"medium", {size_unit::absolute, 1.f}},
    {L"small", {size_unit::absolute, 1.f / 1.15f}},
    {L"smaller", {size_unit::relative, 1.f / 1.15f}},
    {L"x-large", {size_unit::absolute, 1.15f * 1.15f}},
    {L"x-small", {size_unit::absolute, 1.f / 1.15f / 1.15f}},
    {L"xx-large", {size_unit::absolute, 1.15f * 1.15f * 1.15f}},
    {L"xx-small", {size_unit::absolute, 1.f / 1.15f / 1.15f / 1.15f}},
}};

template<typename T>
struct comparator
{
    bool
    operator()(std::pair<wchar_t const*, T> lhs, std::wstring_view rhs) noexcept
    {
        return std::wcscmp(lhs.first, rhs.data()) < 0;
    }

    bool
    operator()(std::wstring_view lhs, std::pair<wchar_t const*, T> rhs) noexcept
    {
        return std::wcscmp(lhs.data(), rhs.first) < 0;
    }
};

template<typename T, std::size_t N>
std::optional<T> lookup(
    std::array<std::pair<wchar_t const*, T>, N> table,
    std::wstring_view key) noexcept
{
    auto const result =
        std::equal_range(table.begin(), table.end(), key, comparator<T>{});

    if (result.first == result.second)
    {
        return {};
    }

    return result.first->second;
}

template<typename T>
void append(std::vector<T>& base, std::vector<T> const& extra) noexcept
{
    base.insert(base.end(), extra.begin(), extra.end());
}

std::optional<font_size> parse_size(std::wstring_view value) noexcept
{
    auto parsed = lookup(size_values, value);
    if (parsed)
    {
        return parsed.value();
    }

    auto size_value = 0.0f;
    std::wstring unit_name;
    auto divisor = 0;
    for (auto const& c : value)
    {
        if (!unit_name.empty())
        {
            unit_name.push_back(c);
            continue;
        }

        if (c == u'.')
        {
            divisor = 1;
            continue;
        }

        auto const digit = c - u'0';
        if (digit < 0 || digit > 10)
        {
            unit_name.push_back(c);
            continue;
        }

        size_value *= 10;
        size_value += digit;
        if (divisor > 0)
        {
            divisor *= 10;
        }
    }

    if (divisor > 0)
    {
        size_value /= divisor;
    }

    if (unit_name == L"%")
    {
        size_value /= 100;
    }

    auto unit = lookup(unit_values, unit_name);
    if (!unit)
    {
        return {};
    }

    return font_size{unit.value(), size_value};
}

constexpr std::uint8_t extract_nibble(wchar_t c) noexcept
{
    auto first = gsl::narrow_cast<std::uint8_t>(c - '0');

    if (first < 10)
    {
        return first;
    }

    first -= u'A' - u'0';
    first += 10;

    if (first < 0x10)
    {
        return first;
    }

    first -= u'a' - u'A';
    return first;
}

std::optional<std::uint8_t>
extract_byte(std::wstring_view value, std::size_t index) noexcept
{
    auto const first  = extract_nibble(value.at(index));
    auto const second = extract_nibble(value.at(index + 1));

    if (first >= 0x10 || second >= 0x10)
    {
        return {};
    }

    return gsl::narrow_cast<std::uint8_t>(first << 4 | second);
}

std::optional<color> parse_color(std::wstring_view value) noexcept
{
    if (value.at(0) == u'#')
    {
        if (value.size() == 7)
        {
            auto r = extract_byte(value, 1);
            auto g = extract_byte(value, 3);
            auto b = extract_byte(value, 5);
            if (!r || !g || !b)
            {
                return {};
            }

            return color{r.value(), g.value(), b.value()};
        }

        if (value.size() == 9)
        {
            auto r = extract_byte(value, 1);
            auto g = extract_byte(value, 3);
            auto b = extract_byte(value, 5);
            auto a = extract_byte(value, 7);
            if (!r || !g || !b || !a)
            {
                return {};
            }

            return color{r.value(), g.value(), b.value(), a.value()};
        }

        return {};
    }

    auto parsed = lookup(color_values, value);
    if (!parsed)
    {
        return {};
    }

    return parsed.value();
}

std::optional<stroke> parse_stroke(std::wstring_view value) noexcept
{
    auto const space = value.find(' ');

    auto const size = parse_size(
        value.substr(0, space == std::string::npos ? value.size() : space));
    auto const color =
        parse_color(value.substr(space == std::string::npos ? 0 : space + 1));
    if (!size && !color)
    {
        return {};
    }

    return stroke{size, color};
}

std::optional<option>
parse_option(std::wstring_view key, std::wstring_view value) noexcept
{
    if (key.empty())
    {
        return {};
    }

    if (key == L"typeface")
    {
        return typeface{std::wstring{value}};
    }
    if (key == L"size")
    {
        auto parsed = parse_size(value);
        if (!parsed)
        {
            return {};
        }

        return parsed.value();
    }
    if (key == L"stretch")
    {
        auto parsed = lookup(stretch_values, value);
        if (!parsed)
        {
            return {};
        }

        return stretch{parsed.value()};
    }
    if (key == L"weight")
    {
        auto parsed = lookup(weight_values, value);
        if (!parsed)
        {
            return {};
        }

        return weight{parsed.value()};
    }
    if (key == L"style")
    {
        auto parsed = lookup(style_values, value);
        if (!parsed)
        {
            return {};
        }

        return style{parsed.value()};
    }
    if (key == L"color")
    {
        auto parsed = parse_color(value);
        if (!parsed)
        {
            return {};
        }

        return parsed.value();
    }
    if (key == L"underline")
    {
        return underline{value == L"true"};
    }
    if (key == L"strikethrough")
    {
        return strikethrough{value == L"true"};
    }
    if (key == L"stroke")
    {
        auto parsed = parse_stroke(value);
        if (!parsed)
        {
            return {};
        }

        return parsed.value();
    }

    return {};
}

template<typename T, std::size_t N>
bool contains(
    std::array<std::pair<wchar_t const*, T>, N> table,
    wchar_t const* key) noexcept
{
    return std::binary_search(table.begin(), table.end(), key, comparator<T>{});
}

template<typename T, std::size_t N>
void find_duplicates(
    std::array<std::pair<wchar_t const*, T>, N> test,
    std::vector<std::wstring_view>& control,
    std::vector<std::wstring_view>& duplicates) noexcept
{
    for (auto const& pair : test)
    {
        if (std::binary_search(control.begin(), control.end(), pair.first))
        {
            duplicates.push_back(pair.first);
        }
    }

    for (auto const& pair : test)
    {
        control.push_back(pair.first);
    }

    std::sort(control.begin(), control.end());
}

std::vector<std::wstring_view> find_duplicates() noexcept
{
    std::vector<std::wstring_view> duplicates;
    std::vector<std::wstring_view> all;

    find_duplicates(stretch_values, all, duplicates);
    find_duplicates(weight_values, all, duplicates);
    find_duplicates(style_values, all, duplicates);
    find_duplicates(color_values, all, duplicates);
    find_duplicates(size_values, all, duplicates);

    std::sort(duplicates.begin(), duplicates.end());

    return duplicates;
}

std::optional<option> parse_option(std::wstring_view value) noexcept
{
    static auto duplicates = find_duplicates();

    if (std::binary_search(duplicates.begin(), duplicates.end(), value))
    {
        return {};
    }

    {
        auto parsed = lookup(style_values, value);
        if (parsed)
        {
            return style{parsed.value()};
        }
    }

    {
        auto parsed = lookup(weight_values, value);
        if (parsed)
        {
            return weight{parsed.value()};
        }
    }

    {
        auto parsed = parse_size(value);
        if (parsed)
        {
            return parsed.value();
        }
    }

    {
        auto parsed = parse_color(value);
        if (parsed)
        {
            return parsed.value();
        }
    }

    {
        auto parsed = lookup(stretch_values, value);
        if (parsed)
        {
            return stretch{parsed.value()};
        }
    }

    if (value == L"underline")
    {
        return underline{true};
    }

    if (value == L"strikethrough")
    {
        return strikethrough{true};
    }

    return typeface{value.data()};
}

void add_option(
    std::vector<option>& result, std::wstring_view key,
    std::wstring_view value) noexcept
{
    std::optional<option> parsed;
    if (!key.empty())
    {
        parsed = parse_option(key, value);
    }
    else
    {
        parsed = parse_option(value);
    }

    if (parsed)
    {
        result.push_back(parsed.value());
    }
}

std::vector<option>
parse_format(std::wstring_view text, std::size_t& index) noexcept
{
    std::vector<option> result;

    auto quote  = false;
    auto escape = false;
    std::wstring key, value;
    for (; index < text.size(); ++index)
    {
        auto c = text.at(index);

        if (escape)
        {
            value.push_back(c);
            escape = false;
            continue;
        }

        if (c == u'\\')
        {
            escape = true;
            continue;
        }

        if (c == u'"')
        {
            quote = !quote;
            continue;
        }

        if ((c == u' ' || c == u'}') && !quote)
        {
            if (value.empty())
            {
                continue;
            }

            add_option(result, key, value);

            if (c == u'}')
            {
                return result;
            }

            key.clear();
            value.clear();

            continue;
        }

        if (c == u':')
        {
            key = value;
            value.clear();
            continue;
        }

        value.push_back(c);
    }

    return {};
}

std::wstring_view escape_chars{L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"};

std::vector<fragment>
parse_inner(std::wstring_view text, std::size_t& index, int level) noexcept
{
    std::vector<fragment> result;

    auto escape = false;
    for (; index < text.size(); ++index)
    {
        auto c = text.at(index);

        if (escape)
        {
            if (escape_chars.find(c) != std::wstring::npos)
            {
                result.push_back({index - 1, 1, format{}});
            }
            escape = false;
            continue;
        }

        if (c == '\\')
        {
            escape = true;
            continue;
        }

        if (c == u'[')
        {
            auto const fragment_start = index++;
            auto const inner          = parse_inner(text, index, ++level);
            if (index >= text.size() - 1)
            {
                append(result, inner);
                break;
            }
            auto format_start = index++;
            if (text.at(index) != u'{')
            {
                continue;
            }
            ++index;

            auto format_fragments = parse_format(text, index);
            if (index < text.size())
            {
                auto const fragment_end = index + 1;
                for (auto& option : format_fragments)
                {
                    result.push_back(
                        {fragment_start, fragment_end - fragment_start,
                         std::move(option)});
                }
                result.push_back({fragment_start, 1, format{}});
                append(result, inner);
                result.push_back(
                    {format_start, fragment_end - format_start, format{}});
            }
            else
            {
                append(result, inner);
                index = format_start + 1;
            }

            continue;
        }

        if (c == u']' && level > 0)
        {
            break;
        }
    }

    return result;
}

}

std::vector<fragment> parse_markdown(std::wstring_view text) noexcept
{
    std::size_t index = 0;
    return parse_inner(text, index, 0);
}

}
