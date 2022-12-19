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

#ifndef WINDOWER_ADDON_MODULES_EVENT_HPP
#define WINDOWER_ADDON_MODULES_EVENT_HPP

#include "addon/lua.hpp"
#include "core.hpp"

#include <variant>

namespace windower
{

class block_t
{
public:
    explicit constexpr block_t() = default;
};

constexpr block_t block = block_t{};

template<typename T>
class basic_result
{
public:
    constexpr basic_result() noexcept : m_data{std::in_place_index<1>} {}
    constexpr basic_result(block_t) noexcept : m_data{std::in_place_index<0>} {}
    template<typename Arg, typename... Args>
    basic_result(Arg&& arg, Args&&... args) noexcept :
        m_data{std::in_place_index<2>, arg, args...}
    {}

    bool blocked() const noexcept { return m_data.index() == 0; }
    bool unchanged() const noexcept { return m_data.index() == 1; }

protected:
    T const& wrapped_value() const noexcept { return *std::get_if<2>(&m_data); }

private:
    std::variant<std::monostate, std::monostate, T> m_data;
};

template<typename F>
void run_on_all_interpreters(F&& function)
{
    auto const& core = core::instance();

    if (auto s = core.script_environment.root_handle().lock())
    {
        function(*s);
    }
    if (core.addon_manager)
    {
        for (auto const& addon : core.addon_manager->loaded())
        {
            if (auto s = addon->root_handle().lock())
            {
                function(*s);
            }
        }
    }
}

int load_event_module(lua::state);

}

#endif
