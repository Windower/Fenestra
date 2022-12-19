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

#include "addon/modules/command.hpp"

#include "addon/addon.hpp"
#include "addon/lua.hpp"
#include "addon/modules/command.lua.hpp"
#include "addon/modules/event.hpp"
#include "addon/script_base.hpp"
#include "addon/unsafe.hpp"
#include "command_manager.hpp"
#include "core.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{

std::byte call_command_handler_key;

int parse_args(windower::lua::state s)
{
    using namespace windower;

    auto const arg_string = lua::get<std::u8string_view>(s, 1);
    auto const arg_count  = lua::get<std::size_t>(s, 2);
    auto const results = command_manager::get_arguments(arg_string, arg_count);
    lua::stack_guard guard{s};
    for (auto const& arg : results)
    {
        lua::push(guard, arg);
    }
    return guard.release();
}

int register_handler(windower::lua::state s)
{
    using namespace windower;

    auto const handle = script_base::get_script_base(s)->root_handle();

    auto const command = lua::get<std::u8string>(s, 1);
    auto const raw     = lua::get<bool>(s, 2);
    auto const tag     = handle.lock();

    auto layer     = command_manager::layer::script;
    auto component = std::u8string_view{u8"__script"};
    if (auto package = addon::get_package(s))
    {
        layer     = command_manager::layer::addon;
        component = package->name();
    }

    command_manager::instance().register_command(
        layer, component, command,
        [handle, command](
            std::vector<std::u8string> const& args,
            windower::command_source source) {
            if (auto ptr = handle.lock())
            {
                try
                {
                    lua::stack_guard guard{*ptr};
                    lua::push(guard, &::call_command_handler_key);
                    lua::raw_get(guard, lua::registry);

                    lua::push(guard, command);
                    lua::push(guard, static_cast<std::int32_t>(source));
                    for (auto const& arg : args)
                    {
                        lua::push(guard, arg);
                    }

                    lua::call(guard, args.size() + 2);
                }
                catch (std::exception const&)
                {
                    core::instance().addon_manager->raise_error(
                        addon::get_package(*ptr).get(),
                        std::current_exception());
                }
            }
        },
        raw, tag);

    return 0;
}

int unregister_handler(windower::lua::state s)
{
    using namespace windower;

    auto const command = lua::get<std::u8string>(s, 1);

    auto layer     = command_manager::layer::script;
    auto component = std::u8string_view{u8"__script"};
    if (auto package = addon::get_package(s))
    {
        layer     = command_manager::layer::addon;
        component = package->name();
    }

    windower::command_manager::instance().unregister_command(
        layer, component, command);

    return 0;
}

}

extern "C"
{
    static void input(
        char8_t const* command_ptr, std::size_t command_length,
        std::int32_t source)
    {
        windower::command_manager::instance().handle_command(
            {command_ptr, command_length}, windower::command_source{source});
    }
}

bool windower::trigger_unknown_command(
    std::u8string_view command, windower::command_source source)
{
    bool handled = false;

    run_on_all_interpreters([&](lua::state s) {
        lua::stack_guard guard{s};
        lua::push(guard, &call_command_handler_key);
        lua::raw_get(guard, lua::registry);
        if (lua::typeof(guard, -1) != lua::type::function)
        {
            return;
        }
        lua::push(guard, lua::nil);
        lua::push(guard, static_cast<std::int32_t>(source));
        lua::push(guard, command);
        lua::push(guard, handled);
        lua::call(guard, 4, 1);
        handled |= lua::get<bool>(guard, -1);
    });

    return handled;
}

int windower::load_command_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_command_source, u8"core.command");

    lua::copy(guard, lua::registry);
    lua::push(guard, &::call_command_handler_key);
    lua::push(guard, ::register_handler);
    lua::push(guard, ::unregister_handler);
    lua::push(guard, ::parse_args);
    lua::push(guard, ::input);

    lua::call(guard, 6);

    return guard.release();
}
