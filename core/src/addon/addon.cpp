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

#include "addon/addon.hpp"

#include "addon/errors/package_error.hpp"
#include "addon/lua.hpp"
#include "addon/lua_internal.hpp"
#include "addon/package_manager.hpp"
#include "core.hpp"
#include "errors/windower_error.hpp"

#include <algorithm>
#include <fstream>
#include <queue>
#include <string>
#include <utility>

namespace
{
std::byte addon_key;
std::byte implicit_packages_key;

int load_internal_module(windower::lua::state s)
{
    using namespace windower;

    lua::stack_guard guard{s};

    auto name_buffer = lua::get<std::u8string>(s, 1);
    auto name        = std::u8string_view{name_buffer};

    if (name.find(u8':') != std::string::npos)
    {
        lua::push(guard, lua::nil);
        return guard.release();
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

    auto package = addon::get_package(s);
    if (!package)
    {
        throw windower_error{u8"INT:2"};
    }

    try
    {
        auto stream = package->resolve(file_name);
        lua::load(
            guard, stream,
            u8'@' + package->name() + u8":" + file_name.u8string());
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
}

std::shared_ptr<windower::package const>
windower::addon::get_package(lua::state s)
{
    lua::stack_guard guard{s};
    lua::push(guard, &addon_key);
    lua::raw_get(guard, lua::registry);
    if (auto const ptr = static_cast<addon const*>(lua::get<void*>(guard, -1)))
    {
        return ptr->package();
    }
    return nullptr;
}

windower::addon::addon(
    std::shared_ptr<windower::package const> const& package) :
    m_package_name{package->name()},
    m_package{package}
{
    lua::stack_guard guard{m_interpreter};

    lua::push(guard, &addon_key);
    lua::push(guard, this);
    lua::raw_set(guard, lua::registry);

    lua::push(guard, u8"package");
    lua::raw_get(guard, lua::globals);

    lua::push(guard, u8"name");
    lua::push(guard, package->name());
    lua::raw_set(guard, -3);

    lua::push(guard, u8"loaders");
    lua::raw_get(guard, -2);

    lua::push(guard, load_internal_module);
    lua::raw_set(guard, -2, 2);

    auto name = package->name();
    std::replace(name.begin(), name.end(), u8'.', u8'\\');
    name.append(u8".lua");
    auto stream = package->resolve(name);

    lua::load(guard, stream, u8'@' + package->name() + u8':' + name);
    lua::call(guard, 0, 0);
}

std::shared_ptr<windower::package const> windower::addon::find_dependency(
    windower::lua::state s, std::u8string_view package_name) const
{
    if (auto pkg = addon::get_package(s))
    {
        auto const& dependencies = pkg->dependencies();

        auto const& core = core::instance();

        std::queue<std::shared_ptr<windower::package const>>
            indirect_dependencies;
        for (auto const& d : dependencies)
        {
            if (auto p = core.package_manager->get_package(d.name()))
            {
                if (d.name() == package_name)
                {
                    return p;
                }
                if (p->type() == package_type::library)
                {
                    for (auto const& temp : p->dependencies())
                    {
                        if (auto indirect_dependency =
                                core.package_manager->get_package(temp.name()))
                        {
                            indirect_dependencies.push(indirect_dependency);
                        }
                    }
                }
            }
        }

        if (core.settings.developer_mode)
        {
            lua::stack_guard guard{s};
            lua::push(guard, &implicit_packages_key);
            lua::raw_get(guard, lua::registry);
            if (lua::typeof(guard, -1) == lua::type::table)
            {
                lua::push(guard, lua::nil);
                while (lua::next(guard, -2))
                {
                    if (auto p = core.package_manager->get_package(
                            lua::get<std::u8string>(guard, -1));
                        p && p->type() == package_type::library)
                    {
                        indirect_dependencies.push(p);
                    }
                    lua::pop(guard);
                }
            }
        }

        while (!indirect_dependencies.empty())
        {
            auto p = indirect_dependencies.front();
            indirect_dependencies.pop();
            if (p->name() == package_name)
            {
                return p;
            }
            if (p->type() == package_type::library)
            {
                for (auto const& temp : p->dependencies())
                {
                    if (auto indirect_dependency =
                            core.package_manager->get_package(temp.name()))
                    {
                        indirect_dependencies.push(indirect_dependency);
                    }
                }
            }
        }

        if (core.settings.developer_mode)
        {
            if (auto p = core.package_manager->get_package(package_name))
            {
                std::u8string warning_message;
                warning_message.append(u8"!!! WARNING !!!\n");
                warning_message.append(u8"The package \"");
                warning_message.append(package_name);
                warning_message.append(
                    u8"\" is being loaded, but is not referenced in the "
                    u8"manifest dependency list.\nPlease contact the "
                    u8"developer, and tell them to add a reference to the "
                    u8"package manifest.");
                core::error(pkg->name(), warning_message);

                lua::stack_guard guard{s};
                lua::push(guard, &implicit_packages_key);
                lua::raw_get(guard, lua::registry);
                if (lua::typeof(guard, -1) != lua::type::table)
                {
                    lua::pop(guard);
                    lua::create_table(guard);
                    lua::push(guard, &implicit_packages_key);
                    lua::copy(guard, -2);
                    lua::raw_set(guard, lua::registry);
                }
                lua::push(guard, package_name);
                lua::copy(guard, -1);
                lua::raw_set(guard, -3);
                return p;
            }
            else
            {
                throw package_error{u8"PKG:P1", package_name};
            }
        }

        throw package_error{u8"PKG:P2", package_name};
    }

    throw windower_error{u8"INT:2"};
}

std::shared_ptr<windower::package const> windower::addon::package() const
{
    auto ptr = m_package.lock();
    if (!ptr)
    {
        ptr = core::instance().package_manager->get_package(m_package_name);
        if (!ptr)
        {
            throw windower_error{u8"INT:2"};
        }
        m_package = ptr;
    }
    return ptr;
}
