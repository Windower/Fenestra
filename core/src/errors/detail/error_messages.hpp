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

#ifndef WINDOWER_ERRORS_DETAIL_ERROR_MESSAGES_HPP
#define WINDOWER_ERRORS_DETAIL_ERROR_MESSAGES_HPP

#include <array>
#include <compare>
#include <utility>

namespace windower::detail
{

using error_message = std::tuple<char8_t const*, char8_t const*>;

constexpr auto error_messages = std::array{
    // clang-format off
    error_message{u8"SIG:1", u8"signature exceeds the maximum size of 64 bytes"},
    error_message{u8"SIG:2", u8"unexpected symbol"},
    error_message{u8"SIG:3", u8"unexpected end of string"},
    error_message{u8"SIG:4", u8"too many offset markers"},
    error_message{u8"SIG:5", u8"wildcard expected"},
    error_message{u8"SIG:6", u8"hexadecimal digit expected"},

    error_message{u8"CMD:A1", u8"alias name is empty"},
    error_message{u8"CMD:A2", u8"error while processing expanded alias"},
    error_message{u8"CMD:N1", u8"invalid name"},
    error_message{u8"CMD:R1", u8"command name is empty"},
    error_message{u8"CMD:L1", u8"command is ambiguous"},
    error_message{u8"CMD:L2", u8"unknown command"},
    error_message{u8"CMD:P1", u8"\"/\" expected"},
    error_message{u8"CMD:P2", u8"command or component name expected"},
    error_message{u8"CMD:P3", u8"command name expected"},
    error_message{u8"CMD:P4", u8"invalid command or component name"},
    error_message{u8"CMD:P5", u8"invalid command name"},
    error_message{u8"CMD:X1", u8"error while processing command"},

    error_message{u8"BIND:C1", u8"command is missing"},
    error_message{u8"BIND:L1", u8"unknown key"},
    error_message{u8"BIND:L2", u8"key is ambiguous"},
    error_message{u8"BIND:L3", u8"unknown device"},
    error_message{u8"BIND:L4", u8"device does not have key registered"},
    error_message{u8"BIND:L5", u8"unknown flag"},
    error_message{u8"BIND:R1", u8"device has already been registered"},
    error_message{u8"BIND:R2", u8"too many devices"},
    error_message{u8"BIND:U1", u8"no device with that name has been registered"},
    error_message{u8"BIND:P10", u8"device or key name expected"},
    error_message{u8"BIND:P13", u8"device or key name expected"},
    error_message{u8"BIND:P16", u8"key name expected"},
    error_message{u8"BIND:P21", u8"illegal device or key name"},
    error_message{u8"BIND:P26", u8"flag name expected"},
    error_message{u8"BIND:P26", u8"flag name expected"},
    error_message{u8"BIND:S10", u8"unexpected token after bind string"},
    error_message{u8"BIND:S12", u8"\"+\" or predicate expected"},
    error_message{u8"BIND:S14", u8"illegal key name"},
    error_message{u8"BIND:S19", u8"illegal device or key name"},
    error_message{u8"BIND:S20", u8"illegal device or key name"},
    error_message{u8"BIND:S21", u8"illegal device or key name"},
    error_message{u8"BIND:S23", u8"flag name or \"]\" expected"},
    error_message{u8"BIND:S24", u8"illegal flag name"},
    error_message{u8"BIND:S27", u8"illegal device, key or flag name"},

    error_message{u8"XML", u8"error parsing xml"},

    error_message{u8"PKG:M1", u8"manifest missing <package> element"},
    error_message{u8"PKG:M2", u8"unknown <type> element value"},
    error_message{u8"PKG:F1", u8"invalid package"},
    error_message{u8"PKG:F2", u8"file not found"},
    error_message{u8"PKG:S1", u8"source already exists"},
    error_message{u8"PKG:S2", u8"source not found"},
    error_message{u8"PKG:P1", u8"package not installed"},
    error_message{u8"PKG:P2", u8"missing dependency"},
    error_message{u8"PKG:P3", u8"dependency cycle detected"},

    error_message{u8"INT:1", u8"[INTERNAL ERROR] script_base not set"},
    error_message{u8"INT:2", u8"[INTERNAL ERROR] addon package not set"},
    // clang-format on
};

}

#endif
