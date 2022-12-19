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

#include "addon/script_environment.hpp"

#include "addon/error.hpp"
#include "addon/lua.hpp"
#include "addon/lua_internal.hpp"
#include "addon/package_manager.hpp"
#include "addon/scheduler.hpp"
#include "core.hpp"
#include "utility.hpp"

#include <filesystem>
#include <fstream>
#include <utility>

namespace
{

int load_script_module(windower::lua::state s)
{
    using namespace windower;

    auto name = lua::get<std::u8string>(s, 1);
    std::replace(name.begin(), name.end(), u8'.', u8'\\');
    name.append(u8".lua");
    auto path = user_path() / u8"scripts" / name;
    std::ifstream stream{path, std::ios::binary};
    lua::stack_guard guard{s};
    if (stream.is_open())
    {
        lua::load(guard, stream, u8'@' + path.u8string());
    }
    else
    {
        lua::push(guard, u8"\n    no file '" + path.u8string() + u8'\'');
    }
    return guard.release();
}

}

void windower::script_environment::reset()
{
    script_base::reset();
    initialize();
}

std::shared_ptr<windower::package const>
windower::script_environment::find_dependency(
    lua::state, std::u8string_view package_name) const
{
    return core::instance().package_manager->get_package(package_name);
}

windower::script_environment::script_environment() noexcept
{
    initialize();
    m_scheduler.error_handler([=](std::exception_ptr exception, void const*) {
        core::error(u8"<script>", exception);
        return true;
    });
}

void windower::script_environment::initialize() const
{
    lua::stack_guard guard{m_interpreter};

    lua::push(guard, u8"package");
    lua::raw_get(guard, lua::globals);
    lua::push(guard, u8"loaders");
    lua::raw_get(guard, -2);

    lua::push(guard, load_script_module);
    lua::raw_set(guard, -2, 2);
}

void windower::script_environment::run_until_idle()
{
    script_base::run_until_idle();
}

void windower::script_environment::execute(std::u8string_view name) const
{
    auto path = user_path() / u8"scripts" / name;
    path += u8".lua";
    std::ifstream stream{path, std::ios::binary};
    if (stream.is_open())
    {
        lua::stack_guard guard{m_interpreter};
        lua::load(guard, stream, u8'@' + path.u8string());
        lua::call(guard, 0);
    }
    else
    {
        throw lua::error{"no file '" + path.string() + '\''};
    }
}

void windower::script_environment::evaluate(std::u8string_view string) const
{
    lua::stack_guard guard{m_interpreter};
    lua::load(guard, string);
    lua::call(guard, 0);
}
