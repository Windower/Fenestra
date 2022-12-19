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

#ifndef WINDOWER_ADDON_SCRIPT_ENVIRONMENT_HPP
#define WINDOWER_ADDON_SCRIPT_ENVIRONMENT_HPP

#include "package_manager.hpp"
#include "script_base.hpp"

#include <string_view>

namespace windower
{

class script_environment : public script_base
{
public:
    script_environment() noexcept;

    virtual ~script_environment() = default;

    void run_until_idle();

    void execute(std::u8string_view) const;
    void evaluate(std::u8string_view) const;

    void reset();

    void initialize() const;

    std::shared_ptr<package const>
        find_dependency(lua::state, std::u8string_view) const override;
};

}

#endif
