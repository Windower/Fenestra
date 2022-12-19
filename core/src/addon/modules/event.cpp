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

#include "addon/modules/event.hpp"

#include "addon/lua.hpp"
#include "addon/modules/event.lua.hpp"
#include "addon/unsafe.hpp"
#include "unicode.hpp"

#include <exception>

extern "C"
{
    using namespace windower;

    static int save_stack(lua::state s)
    {
        lua::stack_guard guard{s};
        lua::unsafe::set_stack_trace(lua::unsafe::unwrap(guard));
        lua::copy(guard, 1);
        return guard.release();
    }

    static int error_addon(lua::state s)
    {
        auto const name    = lua::get<std::u8string_view>(s, 1);
        auto const message = lua::get<std::u8string_view>(s, 2);

        lua::stack_guard guard{s};
        if (auto addon_manager = core::instance().addon_manager.get())
        {
            if (auto addon = addon_manager->get(name))
            {
                auto handle = addon->root_handle();
                if (auto ptr = handle.lock())
                {
                    addon_manager->raise_error(
                        addon->get_package(*ptr).get(),
                        std::make_exception_ptr(
                            lua::error{windower::to_string(message), *ptr}));
                }
            }
        }
        return guard.release();
    }
}

int windower::load_event_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_event_source, u8"core.event");

    lua::push(guard, save_stack);
    lua::push(guard, error_addon);

    lua::call(guard, 2);

    return guard.release();
}
