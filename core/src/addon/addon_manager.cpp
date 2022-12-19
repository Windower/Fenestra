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

#include "addon/addon_manager.hpp"

#include "addon/addon.hpp"
#include "core.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

windower::addon_manager::~addon_manager() noexcept { unload_all(); }

windower::addon const*
windower::addon_manager::get(std::u8string_view name) const
{
    auto it = std::find_if(
        m_loaded_addons.begin(), m_loaded_addons.end(),
        [&name](auto const& addon) {
            return addon->package()->name() == name;
        });
    return it == m_loaded_addons.end() ? nullptr : it->get();
}

std::vector<std::unique_ptr<windower::addon>> const&
windower::addon_manager::loaded() const noexcept
{
    return m_loaded_addons;
}

void windower::addon_manager::load(std::vector<std::u8string> const& names)
{
    load(core::instance().package_manager->load_order(names));
}

void windower::addon_manager::unload(std::vector<std::u8string> const& names)
{
    unload(core::instance().package_manager->unload_order(names));
}

void windower::addon_manager::reload(std::vector<std::u8string> const& names)
{
    auto const& package_manager = core::instance().package_manager;

    std::vector<std::shared_ptr<package const>> to_unload;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        auto unload_order = package_manager->unload_order(names);
        std::copy_if(
            unload_order.begin(), unload_order.end(),
            std::back_inserter(to_unload), [=](auto p) {
                auto it = std::find_if(
                    m_loaded_addons.begin(), m_loaded_addons.end(),
                    [=](auto const& addon) {
                        return addon->package()->name() == p->name();
                    });
                return it != m_loaded_addons.end();
            });
    }
    unload(to_unload);

    load(package_manager->load_order(names));
}

void windower::addon_manager::unload_all() noexcept
{
    unload(core::instance().package_manager->unload_order());
}

void windower::addon_manager::reload_all()
{
    std::vector<std::u8string> names;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        std::transform(
            m_loaded_addons.begin(), m_loaded_addons.end(),
            std::back_inserter(names),
            [](auto const& a) { return a->package()->name(); });
    }
    reload(names);
}

void windower::addon_manager::run_until_idle()
{
    for (auto const& a : m_loaded_addons)
    {
        try
        {
            a->run_until_idle();
        }
        catch (...)
        {
            raise_error(a->package().get(), std::current_exception());
        }
    }
}

void windower::addon_manager::raise_error(
    gsl::not_null<package const*> package, std::exception_ptr exception)
{
    auto const& name = package->name();
    core::error(name, exception);
    unload({name});
}

void windower::addon_manager::load(
    std::vector<std::shared_ptr<package const>> const& packages)
{
    for (auto const& package : packages)
    {
        if (package->type() != package_type::library)
        {
            std::lock_guard<std::mutex> lock{m_mutex};
            auto it = std::find_if(
                m_loaded_addons.begin(), m_loaded_addons.end(),
                [=](auto const& addon) {
                    return addon->package()->name() == package->name();
                });
            if (it == m_loaded_addons.end())
            {
                auto ptr = std::make_unique<addon>(package);
                m_loaded_addons.emplace_back(std::move(ptr));
                core::output(u8"", package->name() + u8" loaded");
            }
        }
    }
}

void windower::addon_manager::unload(
    std::vector<std::shared_ptr<package const>> const& packages)
{
    for (auto const& package : packages)
    {
        if (package->type() != package_type::library)
        {
            std::lock_guard<std::mutex> lock{m_mutex};
            auto it = std::find_if(
                m_loaded_addons.begin(), m_loaded_addons.end(),
                [=](auto const& addon) {
                    return addon->package()->name() == package->name();
                });
            if (it != m_loaded_addons.end())
            {
                m_loaded_addons.erase(it);
                command_manager::instance().purge();
                core::output(u8"", package->name() + u8" unloaded");
            }
        }
    }
}
