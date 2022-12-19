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

#include "addon/modules/channel.hpp"

#include "addon/addon.hpp"
#include "addon/lua.hpp"
#include "addon/modules/channel.lua.hpp"
#include "addon/unsafe.hpp"
#include "core.hpp"

#include <lua.hpp>

#include <bit>
#include <memory>
#include <new>

namespace
{

std::byte remote_pcall_key;

}

extern "C"
{
    static int get_remote_handle(windower::lua::state s)
    {
        using namespace windower;

        auto const name = lua::get<std::u8string_view>(s, 1);

        lua::stack_guard guard{s};
        if (auto addon_manager = core::instance().addon_manager.get())
        {
            if (auto addon = addon_manager->get(name))
            {
                lua::create<std::weak_ptr<lua::state>>(
                    guard, addon->root_handle());
                return guard.release();
            }
        }
        lua::push(guard, lua::nil);
        return guard.release();
    }

    static std::int32_t
    remote_pcall_native(void* handle, void*& data_ptr, std::int32_t& data_size)
    {
        using namespace windower;

        using handle_type = std::weak_ptr<lua::state>;

        auto state_ptr =
            handle ? static_cast<handle_type const*>(handle)->lock()
                   : core::instance().script_environment.root_handle().lock();
        if (!state_ptr)
        {
            return 1;
        }
        auto s = *state_ptr;

        lua::stack_guard guard{s};
        lua::push(guard, &remote_pcall_key);
        lua::raw_get(guard, lua::registry);
        lua::push(guard, data_ptr);
        lua::push(guard, data_size);
        try
        {
            lua::call(guard, 2, 2);
        }
        catch (lua::error const&)
        {
            return 2;
        }
        auto const ptr_value = lua::get<std::intptr_t>(guard, -2);
        data_ptr             = std::bit_cast<void*>(ptr_value);
        data_size            = lua::get<std::size_t>(guard, -1);
        return 0;
    }
}

int windower::load_channel_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_channel_source, u8"core.channel");

    lua::copy(guard, lua::registry);
    lua::push(guard, &remote_pcall_key);
    lua::push(guard, get_remote_handle);
    lua::push(guard, remote_pcall_native);

    lua::call(guard, 4);

    return guard.release();
}
