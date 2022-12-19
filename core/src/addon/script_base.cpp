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

#include "addon/script_base.hpp"

#include "addon/errors/package_error.hpp"
#include "addon/lua.hpp"
#include "addon/lua_internal.hpp"
#include "addon/modules/channel.hpp"
#include "addon/modules/chat.hpp"
#include "addon/modules/class.hpp"
#include "addon/modules/command.hpp"
#include "addon/modules/event.hpp"
#include "addon/modules/hash.hpp"
#include "addon/modules/os.hpp"
#include "addon/modules/packet.hpp"
#include "addon/modules/pin.hpp"
#include "addon/modules/scanner.hpp"
#include "addon/modules/serializer.hpp"
#include "addon/modules/ui.hpp"
#include "addon/modules/unicode.hpp"
#include "addon/modules/windower.hpp"
#include "addon/package_manager.hpp"
#include "addon/scheduler.hpp"
#include "addon/unsafe.hpp"
#include "errors/windower_error.hpp"
#include "library.hpp"

#include <windows.h>

#include <lua.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace
{

std::byte script_base_key;
std::byte scheduler_data_key;
std::byte sleep_delay_key;
std::byte sleep_type_key;
std::byte binary_module_key;

enum class sleep_type
{
    time,
    frame
};

int load_preloaded_module(windower::lua::state s)
{
    using namespace windower;

    lua::stack_guard guard{s};
    lua::push(guard, u8"_PRELOAD");
    lua::raw_get(guard, lua::registry);
    if (lua::typeof(guard, -1) == lua::type::table)
    {
        lua::copy(guard, 1);
        lua::raw_get(guard, -2);
    }
    else
    {
        lua::push(guard, lua::nil);
    }
    lua::replace(guard, -2);
    return guard.release();
}

int load_internal_module(windower::lua::state s)
{
    using namespace windower;

    lua::stack_guard guard{s};
    lua::push(guard, lua::nil);
    return guard.release();
}

int load_external_module(windower::lua::state s)
{
    using namespace windower;

    auto name_buffer         = lua::get<std::u8string>(s, 1);
    auto name                = std::u8string_view{name_buffer};
    auto const delimiter_pos = name.find(u8':');

    std::u8string_view package_name;
    if (delimiter_pos != std::string::npos)
    {
        package_name = name.substr(0, delimiter_pos);
        name         = name.substr(delimiter_pos + 1);
    }
    else
    {
        package_name = name.substr(0, name.find(u8'.'));
    }

    std::filesystem::path file_name;
    while (!name.empty())
    {
        auto const pos = name.find(u8'.');
        file_name /= name.substr(0, pos);
        name.remove_prefix(
            pos != std::u8string_view::npos ? pos + 1 : name.size());
    }
    file_name += u8".lua";

    lua::stack_guard guard{s};

    auto const* base = script_base::get_script_base(s);
    if (!base)
    {
        throw windower_error{u8"INT:1"};
    }

    try
    {
        auto dependency = base->find_dependency(s, package_name);
        if (!dependency)
        {
            std::u8string error_message;
            error_message.append(u8"\n    '");
            error_message.append(package_name);
            error_message.append(u8"' package not found");
            lua::push(guard, error_message);
            return guard.release();
        }

        if (dependency->type() != package_type::library)
        {
            std::u8string error_message;
            error_message.append(u8"\n    '");
            error_message.append(package_name);
            error_message.append(u8"' is not a library package");
            lua::push(guard, error_message);
            return guard.release();
        }

        auto stream = dependency->resolve(file_name);
        std::u8string chunk_name;
        chunk_name.append(1, u8'@');
        chunk_name.append(package_name);
        chunk_name.append(1, u8':');
        chunk_name.append(file_name.u8string());
        lua::load(guard, stream, chunk_name);
    }
    catch (package_error const& e)
    {
        std::u8string error_message;
        error_message.append(u8"\n    [");
        error_message.append(e.error_code());
        error_message.append(u8"] ");
        error_message.append(e.message());
        lua::push(guard, error_message);
    }
    return guard.release();
}

int load_binary_module(windower::lua::state s)
{
    using namespace windower;

    auto name                = lua::get<std::u8string>(s, 1);
    auto const delimiter_pos = name.find(u8':');

    std::u8string package_name;
    if (delimiter_pos != std::string::npos)
    {
        package_name = name.substr(0, delimiter_pos);
        name         = name.substr(delimiter_pos + 1);
    }
    else
    {
        package_name = name.substr(0, name.find('.'));
    }

    auto path = name;
    std::replace(path.begin(), path.end(), u8'.', u8'\\');
    path.append(u8".dll");

    lua::stack_guard guard{s};

    auto const* base = script_base::get_script_base(s);
    if (!base)
    {
        throw windower_error{u8"INT:1"};
    }

    try
    {
        auto dependency = base->find_dependency(s, package_name);
        if (!dependency)
        {
            std::u8string error_message;
            error_message.append(u8"\n    '");
            error_message.append(package_name);
            error_message.append(u8"' package not found");
            lua::push(guard, error_message);
            return guard.release();
        }

        if (dependency->type() != package_type::library)
        {
            std::u8string error_message;
            error_message.append(u8"\n    '");
            error_message.append(package_name);
            error_message.append(u8"' is not a library package");
            lua::push(guard, error_message);
            return guard.release();
        }

        auto absolute_path = dependency->absolute_path(path);

        auto function_name = u8"luaopen_" + name;
        std::replace(function_name.begin(), function_name.end(), u8'.', u8'_');

        lua::push(guard, &binary_module_key);
        lua::raw_get(guard, lua::registry);
        lua::push(guard, package_name);
        lua::raw_get(guard, -2);

        auto library = lua::get<windower::library>(guard, -1);
        if (!library)
        {
            windower::library temp{absolute_path};
            if (!temp)
            {
                lua::pop(guard, 1);
                std::u8string error_message;
                error_message.append(u8"\n    unable to load library '");
                error_message.append(absolute_path.u8string());
                error_message.append(u8"'");
                lua::push(guard, error_message);
                return guard.release();
            }
            lua::pop(guard);
            lua::push(guard, package_name);
            library = lua::create<windower::library>(guard, std::move(temp));
            if (!library)
            {
                windower::fail_fast();
            }
            lua::raw_set(guard, -3);
        }
        lua::pop(guard, 2);

        auto function = library->get_function<::lua_CFunction>(function_name);
        if (!function)
        {
            std::u8string error_message;
            error_message.append(
                u8"\n    unable to locate exported function '");
            error_message.append(function_name);
            error_message.append(u8"' in module '");
            error_message.append(path);
            error_message.append(u8"'");
            lua::push(guard, error_message);
            return guard.release();
        }
        ::lua_pushcclosure(lua::unsafe::unwrap(guard), function, 0);
    }
    catch (package_error const& e)
    {
        std::u8string error;
        error.append(u8"\n    [");
        error.append(e.error_code());
        error.append(u8"] ");
        error.append(e.message());
        lua::push(guard, error);
    }
    return guard.release();
}

int yield_impl(windower::lua::state s)
{
    using namespace windower;

    {
        lua::stack_guard guard{s};
        lua::push(guard, &sleep_delay_key);
        lua::push(guard, lua::nil);
        lua::raw_set(guard, lua::registry);
    }

    return ::lua_yield(lua::unsafe::unwrap(s), lua::top(s));
}

int sleep_impl(windower::lua::state s)
{
    using namespace windower;

    {
        lua::stack_guard const guard{s};
        lua::push(guard, &scheduler_data_key);
        lua::raw_get(guard, lua::registry);
        auto test = lua::get<lua::state>(guard, -1);
        if (lua::unsafe::unwrap(s) == lua::unsafe::unwrap(test))
        {
            lua::push(guard, &sleep_type_key);
            lua::push(guard, std::to_underlying(sleep_type::time));
            lua::raw_set(guard, lua::registry);
            lua::push(guard, &sleep_delay_key);
            lua::push(guard, lua::get<double>(guard, 1));
            lua::raw_set(guard, lua::registry);
        }
        else
        {
            throw lua::error{
                "Attempt to sleep from an unschedulable coroutine.", s};
        }
    }

    auto const top = lua::top(s);
    auto state     = lua::unsafe::unwrap(s);
    if (top > 0)
    {
        ::lua_remove(state, 1);
    }
    return ::lua_yield(state, top - 1);
}

int sleep_frame_impl(windower::lua::state s)
{
    using namespace windower;

    {
        lua::stack_guard guard{s};
        lua::push(guard, &scheduler_data_key);
        lua::raw_get(guard, lua::registry);
        auto test = lua::get<lua::state>(guard, -1);
        if (lua::unsafe::unwrap(s) == lua::unsafe::unwrap(test))
        {
            lua::push(guard, &sleep_type_key);
            lua::push(guard, std::to_underlying(sleep_type::frame));
            lua::raw_set(guard, lua::registry);
            lua::push(guard, &sleep_delay_key);
            lua::push(guard, lua::get<double>(guard, 1));
            lua::raw_set(guard, lua::registry);
        }
        else
        {
            throw lua::error{
                "Attempt to sleep from an unschedulable coroutine.", s};
        }
    }

    auto const top = lua::top(s);
    auto state     = lua::unsafe::unwrap(s);
    if (top > 0)
    {
        ::lua_remove(state, 1);
    }
    return ::lua_yield(state, top - 1);
}

windower::task create_schedulable_coroutine_task(
    windower::lua::coroutine_handle s,
    std::chrono::duration<double> initial_delay)
{
    using namespace windower;

    lua::stack_guard guard{s};
    lua::unsafe::set_base(guard, 0);

    co_yield sleep_for(initial_delay);

    auto count = lua::top(guard) - 1;
    while (co_yield lua::schedulable_resume(guard, count))
    {
        count = lua::top(guard);
    }
}

int schedule_impl(windower::lua::state s)
{
    using namespace windower;

    lua::check_argument(s, 1, lua::type::function);
    lua::check_optional_argument(s, 2, lua::type::number);

    if (auto script_base = script_base::get_script_base(s))
    {
        auto const initial_delay =
            std::chrono::duration<double>{lua::get<double>(s, 2)};

        lua::stack_guard guard{s};
        if (lua::top(s) >= 2)
        {
            lua::remove(guard, 2);
        }
        lua::coroutine_handle coroutine{s};
        lua::stack_guard coroutine_guard{coroutine};
        lua::xmove(guard, coroutine_guard, lua::top(guard));
        coroutine_guard.release();

        script_base->schedule(create_schedulable_coroutine_task(
            std::move(coroutine), initial_delay));
    }

    return 0;
}

void initialize(
    windower::lua::interpreter const& interpreter, windower::script_base& base)
{
    using namespace windower;

    lua::load(interpreter, lua::lib::package);

    lua::preload(interpreter, lua::lib::math);
    lua::preload(interpreter, lua::lib::string);
    lua::preload(interpreter, lua::lib::table);
    lua::preload(interpreter, lua::lib::io);
    lua::preload(interpreter, lua::lib::os);
    lua::preload(interpreter, lua::lib::debug);
    lua::preload(interpreter, lua::lib::bit);
    lua::preload(interpreter, lua::lib::jit);
    lua::preload(interpreter, lua::lib::ffi);

    preload_os_module(interpreter);

    lua::preload(interpreter, u8"core.channel", load_channel_module);
    lua::preload(interpreter, u8"core.chat", load_chat_module);
    lua::preload(interpreter, u8"core.class", load_class_module);
    lua::preload(interpreter, u8"core.command", load_command_module);
    lua::preload(interpreter, u8"core.event", load_event_module);
    lua::preload(interpreter, u8"core.hash", load_hash_module);
    lua::preload(interpreter, u8"core.packet", load_packet_module);
    lua::preload(interpreter, u8"core.pin", load_pin_module);
    lua::preload(interpreter, u8"core.scanner", load_scanner_module);
    lua::preload(interpreter, u8"core.serializer", load_serializer_module);
    lua::preload(interpreter, u8"core.ui", load_ui_module);
    lua::preload(interpreter, u8"core.unicode", load_unicode_module);
    lua::preload(interpreter, u8"core.windower", load_windower_module);

    lua::stack_guard guard{interpreter};

    lua::push(guard, &script_base_key);
    lua::push(guard, &base);
    lua::raw_set(guard, lua::registry);

    lua::push(guard, u8"coroutine");
    lua::raw_get(guard, lua::globals);
    lua::push(guard, u8"sleep");
    lua::push(guard, sleep_impl);
    lua::raw_set(guard, -3);
    lua::push(guard, u8"sleep_frame");
    lua::push(guard, sleep_frame_impl);
    lua::raw_set(guard, -3);
    lua::push(guard, u8"schedule");
    lua::push(guard, schedule_impl);
    lua::raw_set(guard, -3);

    lua::push(guard, u8"package");
    lua::raw_get(guard, lua::globals);

    // Set up the registry table for loaded binary modules
    lua::push(guard, &binary_module_key);
    lua::create_table(guard);
    lua::raw_set(guard, lua::registry);

    // Remove some stuff we're not using.
    lua::push(guard, u8"module");
    lua::push(guard, lua::nil);
    lua::raw_set(guard, lua::globals);

    lua::push(guard, u8"preload");
    lua::push(guard, lua::nil);
    lua::raw_set(guard, -3);
    lua::push(guard, u8"path");
    lua::push(guard, lua::nil);
    lua::raw_set(guard, -3);
    lua::push(guard, u8"cpath");
    lua::push(guard, lua::nil);
    lua::raw_set(guard, -3);
    lua::push(guard, u8"seeall");
    lua::push(guard, lua::nil);
    lua::raw_set(guard, -3);

    // Replace loader list.
    lua::push(guard, u8"loaders");
    lua::create_table(guard, 4);
    lua::push(guard, load_preloaded_module);
    lua::raw_set(guard, -2, 1);
    lua::push(guard, load_internal_module);
    lua::raw_set(guard, -2, 2);
    lua::push(guard, load_external_module);
    lua::raw_set(guard, -2, 3);
    lua::push(guard, load_binary_module);
    lua::raw_set(guard, -2, 4);
    lua::raw_set(guard, -3);
}
}

windower::script_base* windower::script_base::get_script_base(lua::state s)
{
    lua::stack_guard guard{s};
    lua::push(guard, &script_base_key);
    lua::raw_get(guard, lua::registry);
    return static_cast<script_base*>(lua::get<void*>(guard, -1));
}

std::shared_ptr<windower::package const>
windower::script_base::find_dependency(lua::state, std::u8string_view) const
    noexcept(false)
{
    return nullptr;
}

windower::script_base::script_base() noexcept :
    m_root_handle{std::make_shared<lua::state>(m_interpreter)}
{
    initialize(m_interpreter, *this);
}

windower::wait_state windower::script_base::run_until_idle()
{
    return m_scheduler.run_until_idle();
}

void windower::script_base::reset()
{
    m_scheduler.reset();
    m_interpreter = lua::interpreter{};
    m_root_handle = std::make_shared<lua::state>(m_interpreter);
    initialize(m_interpreter, *this);
}

std::weak_ptr<windower::lua::state>
windower::script_base::root_handle() const noexcept
{
    return m_root_handle;
}

void windower::script_base::schedule(windower::task&& task)
{
    m_scheduler.schedule(std::move(task));
}

std::tuple<bool, windower::wait_state>
windower::lua::schedulable_resume(stack_guard& s, std::size_t args)
{
    {
        lua::stack_guard guard{s};
        lua::push(guard, &scheduler_data_key);
        lua::push(guard, s);
        lua::raw_set(guard, lua::registry);
    }

    bool yielded;
    wait_state delay;
    try
    {
        yielded = resume(s, args);

        lua::stack_guard guard{s};
        lua::push(guard, &sleep_delay_key);
        lua::raw_get(guard, lua::registry);
        lua::push(guard, &sleep_type_key);
        lua::raw_get(guard, lua::registry);
        switch (sleep_type{lua::get<std::int32_t>(guard, -1)})
        {
        default: fail_fast();
        case sleep_type::time:
            delay = sleep_for(
                std::chrono::duration<double>{lua::get<double>(guard, -2)});
            break;
        case sleep_type::frame:
            delay = sleep_frame(lua::get<int>(guard, -2) + 1);
            break;
        }

        lua::push(guard, &scheduler_data_key);
        lua::push(guard, lua::nil);
        lua::raw_set(guard, lua::registry);
    }
    catch (...)
    {
        lua::stack_guard guard{s};
        lua::push(guard, &scheduler_data_key);
        lua::push(guard, lua::nil);
        lua::raw_set(guard, lua::registry);
        throw;
    }

    return std::make_tuple(yielded, delay);
}

windower::lua::coroutine_handle::coroutine_handle(state s)
{
    lua::stack_guard guard{s};
    auto coroutine = lua::create_coroutine(guard);
    lua::push(guard, true);
    lua::raw_set(guard, lua::registry);
    lua::unsafe::unwrap(*this) = lua::unsafe::unwrap(coroutine);
}

windower::lua::coroutine_handle::~coroutine_handle()
{
    if (lua::unsafe::unwrap(*this))
    {
        lua::stack_guard guard{*this};
        lua::push(guard, guard);
        lua::push(guard, lua::nil);
    }
}
