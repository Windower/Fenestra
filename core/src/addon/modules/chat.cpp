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

#include "addon/modules/chat.hpp"

#include "addon/lua.hpp"
#include "addon/modules/chat.lua.hpp"
#include "addon/modules/event.hpp"
#include "core.hpp"
#include "hooks/ffximain.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace
{

std::byte trigger_key;

}

extern "C"
{
    static void add_text_native(
        char8_t const* text_ptr, size_t text_size, uint8_t type, bool indented)
    {
        std::u8string text{text_ptr, text_size};

        windower::core::instance().run_on_next_frame(
            [text = std::move(text), type, indented]() {
                windower::ffximain::add_to_chat(text, type, indented);
            });
    }
}

std::u8string_view windower::text_added_result::text() const noexcept
{
    return wrapped_value().text;
}

std::uint8_t windower::text_added_result::type() const noexcept
{
    return wrapped_value().type;
}

bool windower::text_added_result::indented() const noexcept
{
    return wrapped_value().indented;
}

windower::text_added_result windower::trigger_text_added(
    std::u8string_view text, std::uint8_t type, bool indented)
{
    bool blocked = false;
    bool unchanged = true;
    std::u8string result_text;
    std::uint8_t result_type;
    bool result_indented;

    run_on_all_interpreters([&](lua::state s) {
        lua::stack_guard guard{s};
        lua::push(guard, &trigger_key);
        lua::raw_get(guard, lua::registry);
        if (lua::typeof(guard, -1) != lua::type::function)
        {
            return;
        }
        lua::push(guard, text);
        lua::push(guard, type);
        lua::push(guard, indented);
        if (unchanged)
        {
            lua::copy(guard, -3);
            lua::copy(guard, -3);
            lua::copy(guard, -3);
        }
        else
        {
            lua::push(guard, result_text);
            lua::push(guard, result_type);
            lua::push(guard, result_indented);
        }
        lua::push(guard, blocked);
        lua::call(guard, 7, 3);
        if (blocked)
        {
            return;
        }
        switch (lua::typeof(guard, -3))
        {
        case lua::type::nil: break;
        case lua::type::boolean: blocked = true; break;
        default:
            unchanged = false;
            result_text = lua::get<std::u8string_view>(guard, -3);
            result_type = lua::get<std::uint8_t>(guard, -2);
            result_indented = lua::get<bool>(guard, -1);
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

    return {std::move(result_text), result_type, result_indented};
}

int windower::load_chat_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_chat_source, u8"core.chat");

    lua::copy(guard, lua::registry);
    lua::push(guard, &trigger_key);
    lua::push(guard, &add_text_native);

    lua::call(guard, 3);

    return guard.release();
}
