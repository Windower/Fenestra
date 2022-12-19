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

#ifndef WINDOWER_ADDON_ADDON_MANAGER_HPP
#define WINDOWER_ADDON_ADDON_MANAGER_HPP

#include "addon/addon.hpp"
#include "addon/scheduler.hpp"

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace windower
{

class package;

class addon_manager
{
public:
    addon_manager()                     = default;
    addon_manager(addon_manager const&) = delete;
    addon_manager(addon_manager&&)      = delete;

    ~addon_manager() noexcept;

    addon_manager& operator=(addon_manager const&) = delete;
    addon_manager& operator=(addon_manager&&)      = delete;

    addon const* get(std::u8string_view) const;
    std::vector<std::unique_ptr<addon>> const& loaded() const noexcept;

    void load(std::vector<std::u8string> const&);
    void unload(std::vector<std::u8string> const&);
    void reload(std::vector<std::u8string> const&);
    void unload_all() noexcept;
    void reload_all();

    void run_until_idle();

    void raise_error(gsl::not_null<package const*>, std::exception_ptr);

private:
    std::mutex m_mutex;
    std::vector<std::unique_ptr<addon>> m_loaded_addons;

    void load(std::vector<std::shared_ptr<package const>> const&);
    void unload(std::vector<std::shared_ptr<package const>> const&);
};

}

#endif
