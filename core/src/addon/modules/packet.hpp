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

#ifndef WINDOWER_ADDON_MODULES_PACKET_HPP
#define WINDOWER_ADDON_MODULES_PACKET_HPP

#include "addon/lua.hpp"
#include "addon/modules/event.hpp"

#include <cstdint>
#include <deque>
#include <span>

namespace windower
{

namespace detail
{

struct packet_result_data
{
    packet_result_data(std::uint16_t id, std::vector<std::byte> data) noexcept :
        id{id}, data{std::move(data)}
    {}

    std::uint16_t id;
    std::vector<std::byte> data;
};

}

class packet_result : public basic_result<detail::packet_result_data>
{
public:
    using basic_result::basic_result;

    std::uint16_t id() const noexcept;
    std::span<std::byte const> data() const noexcept;
};

packet_result trigger_packet(
    bool incoming, std::uint16_t id, std::uint16_t counter,
    std::uint32_t timestamp, std::span<std::byte const> data,
    std::u8string_view injected_by = {});

int load_packet_module(lua::state);

}

#endif
