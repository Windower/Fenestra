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

#include "addon/modules/packet.hpp"

#include "addon/lua.hpp"
#include "addon/modules/packet.lua.hpp"
#include "hooks/ffximain.hpp"

#include <cstdint>
#include <span>
#include <utility>

namespace
{

std::byte trigger_key;

}

extern "C"
{
    static void inject_incoming_native(
        std::uint16_t id, std::byte const* data_ptr, std::size_t data_size,
        char8_t const* injected_by_ptr, std::size_t injected_by_size)
    {
        std::vector<std::byte> data{data_ptr, data_ptr + data_size};
        std::u8string injected_by{injected_by_ptr, injected_by_size};

        windower::core::instance().run_on_next_frame(
            [id, data = std::move(data),
             injected_by = std::move(injected_by)]() mutable {
                windower::core::instance().incoming_packet_queue->queue(
                    id, std::move(data), std::move(injected_by));
            });
    }

    static void inject_outgoing_native(
        std::uint16_t id, std::byte const* data_ptr, std::size_t data_size,
        char8_t const* injected_by_ptr, std::size_t injected_by_size)
    {
        std::vector<std::byte> data{data_ptr, data_ptr + data_size};
        std::u8string injected_by{injected_by_ptr, injected_by_size};

        windower::core::instance().run_on_next_frame(
            [id, data = std::move(data),
             injected_by = std::move(injected_by)]() mutable {
                windower::core::instance().outgoing_packet_queue->queue(
                    id, std::move(data), std::move(injected_by));
            });
    }
}

std::uint16_t windower::packet_result::id() const noexcept
{
    return basic_result::wrapped_value().id;
}

std::span<std::byte const> windower::packet_result::data() const noexcept
{
    return basic_result::wrapped_value().data;
}

windower::packet_result windower::trigger_packet(
    bool incoming, std::uint16_t id, std::uint16_t counter,
    std::uint32_t timestamp, std::span<std::byte const> data,
    std::u8string_view injected_by)
{
    bool blocked   = false;
    bool unchanged = true;
    std::uint16_t result_id;
    std::vector<std::byte> result_data;

    run_on_all_interpreters([&](lua::state s) {
        lua::stack_guard guard{s};
        lua::push(guard, &trigger_key);
        lua::raw_get(guard, lua::registry);
        if (lua::typeof(guard, -1) != lua::type::function)
        {
            return;
        }
        lua::push(guard, incoming);
        lua::push(guard, id);
        lua::push(guard, data);
        if (unchanged)
        {
            lua::copy(guard, -2);
            lua::copy(guard, -2);
        }
        else
        {
            lua::push(guard, result_id);
            lua::push(guard, result_data);
        }
        lua::push(guard, counter);
        lua::push(guard, double(timestamp));
        lua::push(guard, blocked);
        lua::push(guard, injected_by);
        lua::call(guard, 9, 2);
        if (blocked)
        {
            return;
        }
        switch (lua::typeof(guard, -2))
        {
        case lua::type::nil: break;
        case lua::type::boolean: blocked = true; break;
        default:
            unchanged   = false;
            result_id   = lua::get<std::uint16_t>(guard, -2);
            result_data = lua::get<std::vector<std::byte>>(guard, -1);
            break;
        }
    });

    if (blocked)
    {
        return block;
    }

    if (unchanged)
    {
        return {};
    }

    return {result_id, std::move(result_data)};
}

int windower::load_packet_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_packet_source, u8"core.packet");

    lua::copy(guard, lua::registry);
    lua::push(guard, &trigger_key);
    lua::push(guard, &inject_incoming_native);
    lua::push(guard, &inject_outgoing_native);

    lua::call(guard, 4);

    return guard.release();
}
