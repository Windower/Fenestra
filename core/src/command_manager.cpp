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

#include "command_manager.hpp"

#include "addon/modules/command.hpp"
#include "errors/command_error.hpp"
#include "errors/syntax_error.hpp"
#include "errors/windower_error.hpp"
#include "hooks/ffximain.hpp"
#include "unicode.hpp"

#include <cstddef>
#include <iterator>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace
{

std::pair<
    std::optional<std::pair<std::size_t, std::size_t>>,
    std::pair<std::size_t, std::size_t>>
parse_command(std::u8string_view command_string)
{
    // command             → '/' command_name
    // command_name        → name opt_q_name | q_name
    //
    // opt_q_name          → q_name | ε
    // q_name              → ':' name
    // opt_name            → name | ε
    // name                → name_char opt_name
    //
    // name_char           → 'a' | 'b' | 'c' | 'd' | 'e'
    //                     | 'f' | 'g' | 'h' | 'i' | 'j'
    //                     | 'k' | 'l' | 'm' | 'n' | 'o'
    //                     | 'p' | 'q' | 'r' | 's' | 't'
    //                     | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'
    //                     | 'A' | 'B' | 'C' | 'D' | 'E'
    //                     | 'F' | 'G' | 'H' | 'I' | 'J'
    //                     | 'K' | 'L' | 'M' | 'N' | 'O'
    //                     | 'P' | 'Q' | 'R' | 'S' | 'T'
    //                     | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z'
    //                     | '0' | '1' | '2' | '3' | '4'
    //                     | '5' | '6' | '7' | '8' | '9'
    //                     | '?' | '_' | '-'

    using namespace windower;

    constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 5>, 5>
        state_table{{
            //  /       :      nm       *        $
            {{{1, 2}, {0, 4}, {0, 4}, {0, 4}, {-1, 4}}},
            {{{0, 7}, {2, 1}, {3, 0}, {0, 7}, {-1, 5}}},
            {{{0, 8}, {0, 6}, {4, 0}, {0, 8}, {-1, 6}}},
            {{{0, 7}, {2, 1}, {3, 0}, {0, 7}, {-1, 3}}},
            {{{0, 8}, {0, 8}, {4, 0}, {0, 8}, {-1, 3}}},
        }};

    constexpr auto next = [](std::u8string_view string,
                             std::size_t& offset) -> std::int8_t {
        if (offset == string.size())
        {
            return 4;
        }
        switch (auto const c = windower::next_code_point(string, offset))
        {
        // clang-format off
        case U'/': return 0;
        case U':': return 1;
        case U'a': case U'b': case U'c': case U'd': case U'e':
        case U'f': case U'g': case U'h': case U'i': case U'j':
        case U'k': case U'l': case U'm': case U'n': case U'o':
        case U'p': case U'q': case U'r': case U's': case U't':
        case U'u': case U'v': case U'w': case U'x': case U'y': case U'z':
        case U'A': case U'B': case U'C': case U'D': case U'E':
        case U'F': case U'G': case U'H': case U'I': case U'J':
        case U'K': case U'L': case U'M': case U'N': case U'O':
        case U'P': case U'Q': case U'R': case U'S': case U'T':
        case U'U': case U'V': case U'W': case U'X': case U'Y': case U'Z':
        case U'0': case U'1': case U'2': case U'3': case U'4':
        case U'5': case U'6': case U'7': case U'8': case U'9':
        case U'?': case U'_': case U'-': return 2;
        // clang-format on
        default: return windower::is_whitespace(c) ? 4 : 3;
        }
    };

    auto state = 0;
    auto it    = std::size_t{};
    auto mark  = it;

    std::pair<
        std::optional<std::pair<std::size_t, std::size_t>>,
        std::pair<std::size_t, std::size_t>>
        result;

    while (state >= 0)
    {
        auto next_it          = it;
        auto const char_class = next(command_string, next_it);
        auto const [next_state, action] =
            gsl::at(gsl::at(state_table, state), char_class);
        state = next_state;
        switch (action)
        {
        case 0: break;
        case 1: result.first = {mark, it}; [[fallthrough]];
        case 2: mark = next_it; break;
        case 3: result.second = {mark, it}; break;
        case 4: throw syntax_error{u8"CMD:P1", command_string, it};
        case 5: throw syntax_error{u8"CMD:P2", command_string, it};
        case 6: throw syntax_error{u8"CMD:P3", command_string, it};
        case 7: throw syntax_error{u8"CMD:P4", command_string, it, mark, it};
        case 8: throw syntax_error{u8"CMD:P5", command_string, it, mark, it};
        default: fail_fast();
        }
        it = next_it;
    }

    return result;
}

constexpr std::u8string_view name_chars =
    u8"abcdefghijklmnopqrstuvwxyz"
    u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    u8"0123456789?_-";

std::u8string unescape(std::u8string_view string)
{
    // escape                   → '\' escape_sequence
    // escape_sequence          → '\' | '{' | '}' | *
    //                          | 'u' opt_code_point_sequence
    // opt_code_point_sequence  → code_point_sequence | ε
    // code_point_sequence      → hex hex hex hex | '{' hex opt_hex5 '}'
    // opt_hex5                 → hex opt_hex4 | ε
    // opt_hex4                 → hex opt_hex3 | ε
    // opt_hex3                 → hex opt_hex2 | ε
    // opt_hex2                 → hex opt_hex1 | ε
    // opt_hex1                 → hex | ε
    // hex                      → '0' | '1' | '2' | '3' | '4'
    //                          | '5' | '6' | '7' | '8' | '9'
    //                          | 'A' | 'B' | 'C' | 'D' | 'E' | 'F'
    //                          | 'a' | 'b' | 'c' | 'd' | 'e' | 'f'

    using namespace windower;

    constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 7>, 13>
        state_table{{
            // clang-format off
            //  \       u       hex      {       }       *        $
            {{{1, 1}, {0, 0}, { 0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{0, 0}, {2, 2}, { 0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 3, 3}, {4, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 5, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 6, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 7, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 8, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 0, 4}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 9, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, {10, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, {11, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, {12, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            {{{1, 1}, {0, 0}, { 0, 0}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            // clang-format on
        }};

    constexpr auto next = [](std::u8string_view::iterator it,
                             std::u8string_view::iterator end)
        -> std::pair<std::int8_t, std::u8string_view::iterator> {
        if (it == end)
        {
            return {6, end};
        }
        switch (*it++)
        {
            // clang-format off
        case u8'\\': return {0, it};
        case u8'u': return {1, it};
        case u8'0': case u8'1': case u8'2': case u8'3': case u8'4':
        case u8'5': case u8'6': case u8'7': case u8'8': case u8'9':
        case u8'A': case u8'B': case u8'C':
        case u8'D': case u8'E': case u8'F':
        case u8'a': case u8'b': case u8'c':
        case u8'd': case u8'e': case u8'f': return {2, it};
        case u8'{': return {3, it};
        case u8'}': return {4, it};
        // clang-format on
        default: return {5, it};
        }
    };

    std::u8string result;

    auto state     = 0;
    auto it        = string.begin();
    auto mark      = it;
    char32_t value = 0;

    while (state >= 0)
    {
        auto const [char_class, next_it] = next(it, string.end());
        auto const [next_state, action] =
            gsl::at(gsl::at(state_table, state), char_class);
        state = next_state;
        switch (action)
        {
        case 0: break;
        case 1:
            result.append(mark, it);
            mark = next_it;
            break;
        case 2: value = 0; break;
        case 3: value = value << 4 | ((*it | 0x1B0) * 0x0E422D48U) >> 28; break;
        case 4:
            value = value << 4 | ((*it | 0x1B0) * 0x0E422D48U) >> 28;
            [[fallthrough]];
        case 5:
            if (value <= U'\U0010FFFF')
            {
                append(result, value);
                mark = next_it;
            }
            break;
        default: fail_fast();
        }
        it = next_it;
    }

    return result;
}

void parse_arguments(
    std::u8string_view argument_string, std::size_t count,
    std::vector<std::u8string>& output)
{
    // argument_string         → opt_whitespace opt_argument_list
    //
    // opt_argument_list       → argument_list | ε
    // argument_list           → argument opt_whitespace opt_argument_list
    //
    // argument                → unquoted_arg
    //                         | "'" opt_s_quoted_argument "'"
    //                         | '"' opt_d_quoted_argument '"'
    //                         | auto_translate_character
    //
    // opt_unquoted_argument   → unquoted_argument | ε
    // unquoted_argument       → character opt_unquoted_argument
    //
    // opt_s_quoted_argument   → s_quote_argument | ε
    // s_quoted_argument       → s_quote_character opt_s_quote_argument
    // s_quoted_character      → '"' | W | character
    //
    // opt_d_quoted_argument   → d_quote_argument | ε
    // d_quoted_argument       → d_quote_character opt_d_quote_argument
    // d_quoted_character      → '"' | W | character
    //
    // opt_whitespace          → whitespace | ε
    // whitespace              → whitespace_character opt_whitespace
    //
    // character               → '\' any | auto_translate_character | *
    // any                     → '\' | "'" | '"'
    //                         | auto_translate_character
    //                         | whitespace_character
    //                         | *

    using namespace windower;

    constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 7>, 6>
        state_table{{
            //  \       '       "      at      ws       *        $
            {{{3, 0}, {2, 1}, {1, 1}, {0, 3}, {0, 1}, {0, 0}, {-1, 1}}},
            {{{4, 0}, {1, 0}, {0, 2}, {1, 0}, {1, 0}, {1, 0}, {-1, 2}}},
            {{{5, 0}, {0, 2}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {-1, 2}}},
            {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
            {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {-1, 1}}},
            {{{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {-1, 1}}},
        }};

    constexpr auto next = [](std::u8string_view string,
                             std::size_t& offset) -> std::int8_t {
        if (offset == string.size())
        {
            return 6;
        }
        switch (auto const c = windower::next_code_point(string, offset))
        {
        case u8'\\': return 0;
        case u8'\'': return 1;
        case u8'"': return 2;
        default:
            if (c >= U'\U000F0000' && c <= U'\U000FFFFD' ||
                c >= U'\U00100000' && c <= U'\U0010FFFD')
            {
                return 3;
            }
            if (windower::is_whitespace(c))
            {
                return 4;
            }
            return 5;
        }
    };

    constexpr auto append = [](std::vector<std::u8string>& output,
                               std::u8string_view string,
                               std::size_t begin_offset, std::size_t end_offset,
                               std::size_t& count, bool allow_empty = false) {
        if (begin_offset == end_offset)
        {
            if (allow_empty)
            {
                output.emplace_back();
                --count;
            }
        }
        else
        {
            auto const size = end_offset - begin_offset;
            output.push_back(::unescape(string.substr(begin_offset, size)));
            --count;
        }
        return count != 0;
    };

    auto state = 0;
    auto it    = std::size_t{};
    auto mark  = it;

    while (count != 0 && state >= 0)
    {
        auto next_it          = it;
        auto const char_class = next(argument_string, next_it);
        auto const [next_state, action] =
            gsl::at(gsl::at(state_table, state), char_class);
        state = next_state;
        switch (action)
        {
        case 0: break;
        case 1:
            mark =
                append(output, argument_string, mark, it, count) ? next_it : it;
            break;
        case 2:
            append(output, argument_string, mark, it, count, true);
            mark = next_it;
            break;
        case 3:
            if (append(output, argument_string, mark, it, count))
            {
                append(output, argument_string, it, next_it, count);
                mark = next_it;
                break;
            }
            mark = it;
            break;
        default: fail_fast();
        }
        it = next_it;
    }

    if (mark != argument_string.size())
    {
        output.emplace_back(argument_string.substr(mark));
    }
}

std::u8string_view substring(
    std::u8string_view string,
    std::pair<std::size_t, std::size_t> range) noexcept
{
    return string.substr(range.first, range.second - range.first);
}

std::optional<std::u8string_view> substring(
    std::u8string_view string,
    std::optional<std::pair<std::size_t, std::size_t>> range) noexcept
{
    if (!range)
    {
        return std::nullopt;
    }
    return substring(string, *range);
}

}

void windower::command_manager::initialize() noexcept { instance(); }

windower::command_manager& windower::command_manager::instance() noexcept
{
    static command_manager instance;
    return instance;
}

bool windower::command_manager::is_valid_name(std::u8string_view name) noexcept
{
    return name.find_first_not_of(name_chars) != std::u8string_view::npos;
}

void windower::command_manager::check_name(std::u8string_view name)
{
    if (auto const offset = name.find_first_not_of(name_chars);
        offset != std::u8string_view::npos)
    {
        throw syntax_error{u8"CMD:N1", name, offset};
    }
}

std::u8string windower::command_manager::escape(std::u8string_view string)
{
    std::u8string result;
    result.reserve(string.size());
    auto mark = std::size_t{};
    for (auto it = std::size_t{}, next_it = it; it != string.size();)
    {
        auto const c = next_code_point(string, next_it);
        if (c == U'\\' || c == U'\'' || c == U'"' || is_whitespace(c))
        {
            result.append(string.substr(mark, it - mark));
            result.append(1, u8'\\');
            mark = it;
        }
        it = next_it;
    }
    return result;
}

std::u8string windower::command_manager::unescape(std::u8string_view string)
{
    return ::unescape(string);
}

std::vector<std::u8string> windower::command_manager::get_arguments(
    std::u8string_view string, std::size_t count)
{
    std::vector<std::u8string> results;
    ::parse_arguments(string, count, results);
    return results;
}

void windower::command_manager::register_command(
    layer layer, std::u8string_view component, std::u8string_view command,
    std::function<void(std::vector<std::u8string>, command_source)> handler,
    bool raw, std::shared_ptr<void> const& tag)
{
    check_name(component);
    check_name(command);
    if (command.empty())
    {
        throw syntax_error{u8"CMD:R1"};
    }

    descriptor d;
    d.component = component;
    d.command   = command;
    d.tag       = tag ? tag : m_default_tag;
    d.handler   = std::move(handler);
    d.raw       = raw;
    d.layer     = layer;

    auto& commands = m_commands.at(gsl::narrow_cast<int>(layer));
    if (auto it = std::lower_bound(commands.begin(), commands.end(), d);
        it == commands.end() || d < *it)
    {
        commands.insert(it, std::move(d));
    }
    else if (it != commands.end())
    {
        *it = std::move(d);
    }
}

void windower::command_manager::unregister_command(
    layer layer, std::u8string_view component, std::u8string_view command)
{
    name_view const name{component, command};
    auto& commands = m_commands.at(gsl::narrow_cast<int>(layer));
    commands.erase(
        std::remove(commands.begin(), commands.end(), name), commands.end());
}

void windower::command_manager::register_alias(
    std::u8string_view alias, std::u8string_view command)
{
    check_name(alias);
    if (alias.empty())
    {
        throw syntax_error{u8"CMD:A1"};
    }
    validate_command(command);

    if (auto it = std::lower_bound(
            m_aliases.begin(), m_aliases.end(), alias,
            [](auto const& a, auto const& b) { return a.first < b; });
        it == m_aliases.end() || it->first != alias)
    {
        m_aliases.emplace(it, alias, command);
    }
    else if (it != m_aliases.end())
    {
        it->second = command;
    }
}

void windower::command_manager::unregister_alias(std::u8string_view alias)
{
    m_aliases.erase(
        std::remove_if(
            m_aliases.begin(), m_aliases.end(),
            [&](auto const& a) { return a.first == alias; }),
        m_aliases.end());
}

void windower::command_manager::validate_command(
    std::u8string_view const command_string) const
{
    parse_command(command_string);
}

void windower::command_manager::handle_command(
    std::u8string_view command_string, command_source const source)
{
    std::u8string expanded;
    auto [component, command] = parse_command(command_string);
    if (!component)
    {
        if (auto alias = resolve_alias(substring(command_string, command)))
        {
            expanded.append(*alias);
            expanded.append(command_string.substr(command.second));
            command_string    = expanded;
            auto const result = parse_command(command_string);
            component         = result.first;
            command           = result.second;
        }
    }

    while (auto descriptor = find(command_string, component, command))
    {
        auto lock = descriptor->tag.lock();
        if (!lock)
        {
            unregister_command(
                descriptor->layer, descriptor->component, descriptor->command);
            continue;
        }
        std::vector<std::u8string> arguments;
        if (descriptor->raw)
        {
            std::u8string processed_command;
            processed_command.append(1, u8'/');
            processed_command.append(substring(command_string, command));
            processed_command.append(command_string.substr(command.second));
            arguments.emplace_back(processed_command);
        }
        else
        {
            auto const arg_string = command_string.substr(command.second);
            ::parse_arguments(arg_string, unlimited, arguments);
        }
        try
        {
            descriptor->handler(arguments, source);
            return;
        }
        catch (command_error const&)
        {
            throw;
        }
        catch (std::exception const&)
        {
            throw command_error{u8"CMD:X1", command_string};
        }
    }

    if (component || !trigger_unknown_command(command_string, source))
    {
        throw syntax_error{
            u8"CMD:L2", command_string, command.first, command.first,
            command.second};
    }
}

void windower::command_manager::purge() noexcept
{
    for (auto& commands : m_commands)
    {
        commands.erase(
            std::remove_if(
                commands.begin(), commands.end(),
                [](descriptor& d) noexcept { return d.tag.expired(); }),
            commands.end());
    }
}

std::optional<std::u8string_view>
windower::command_manager::resolve_alias(std::u8string_view name) const
{
    auto it = std::lower_bound(
        m_aliases.begin(), m_aliases.end(), name,
        [](auto const& a, auto const& b) { return a.first < b; });
    if (it != m_aliases.end() && it->first == name)
    {
        return it->second;
    }
    return std::nullopt;
}

windower::command_manager::descriptor const* windower::command_manager::find(
    std::u8string_view command_string, std::optional<u8range> component,
    u8range command) const
{
    name_view const name{
        substring(command_string, component),
        substring(command_string, command)};
    for (auto& commands : m_commands)
    {
        auto const bounds =
            std::equal_range(commands.begin(), commands.end(), name);
        auto const count = std::distance(bounds.first, bounds.second);
        if (count == 1)
        {
            return &*bounds.first;
        }
        if (count > 1 && !component)
        {
            std::vector<std::u8string> options;
            std::transform(
                bounds.first, bounds.second, std::back_inserter(options),
                [](auto const& d) {
                    return u8'/' + d.component + u8':' + d.command;
                });
            throw syntax_error{u8"CMD:L1",    command_string, command.first,
                               command.first, command.second, options};
        }
    }
    if (component)
    {
        throw syntax_error{
            u8"CMD:L2", command_string, component->first, component->first,
            command.second};
    }
    return nullptr;
}

bool windower::command_manager::descriptor::operator==(
    descriptor const& other) const noexcept
{
    return command == other.command && component == other.component;
}

std::strong_ordering windower::command_manager::descriptor::operator<=>(
    descriptor const& other) const noexcept
{
    auto const result = command.compare(other.command);
    return result != 0 ? result <=> 0
                       : component.compare(other.component) <=> 0;
}

windower::command_manager::name_view::name_view(
    descriptor const& descriptor) noexcept :
    component{descriptor.component},
    command{descriptor.command}
{}

windower::command_manager::name_view::name_view(
    std::u8string_view command) noexcept :
    component{std::nullopt},
    command{std::move(command)}
{}

windower::command_manager::name_view::name_view(
    std::optional<std::u8string_view> component,
    std::u8string_view command) noexcept :
    component{std::move(component)},
    command{std::move(command)}
{}

bool windower::command_manager::name_view::operator==(
    name_view const& other) const noexcept
{
    return command == other.command &&
           (!component || !other.component || *component == *other.component);
}

std::weak_ordering windower::command_manager::name_view::operator<=>(
    name_view const& other) const noexcept
{
    auto const result = command.compare(other.command);
    return result != 0 ? result <=> 0
         : !component || !other.component
             ? std::weak_ordering::equivalent
             : component->compare(*other.component) <=> 0;
}
