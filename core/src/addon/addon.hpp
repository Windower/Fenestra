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

#ifndef WINDOWER_ADDON_ADDON_HPP
#define WINDOWER_ADDON_ADDON_HPP

#include "lua.hpp"
#include "package_manager.hpp"
#include "script_base.hpp"

#include <memory>
#include <string>

namespace windower
{

class addon : public script_base
{
public:
    static std::shared_ptr<windower::package const> get_package(lua::state);

    addon(std::shared_ptr<windower::package const> const&);

    virtual ~addon() = default;

    std::shared_ptr<windower::package const>
        find_dependency(lua::state, std::u8string_view) const override;

    std::shared_ptr<windower::package const> package() const;

private:
    std::u8string m_package_name;
    mutable std::weak_ptr<windower::package const> m_package;
};

}

#endif
