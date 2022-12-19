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

#ifndef WINDOWER_ADDON_LUA_INTERNAL_HPP
#define WINDOWER_ADDON_LUA_INTERNAL_HPP

#include "addon/lua.hpp"

#include <cstddef>
#include <memory>

namespace windower::lua
{

class interpreter
{
public:
    interpreter() noexcept;
    interpreter(interpreter&&) noexcept;
    interpreter(interpreter const&) = delete;

    ~interpreter();

    interpreter& operator=(interpreter&&) noexcept;
    interpreter& operator=(interpreter const&) = delete;

    operator state() const noexcept;

private:
    state m_state = {};
};

enum class lib
{
    math,
    string,
    table,
    io,
    os,
    package,
    debug,
    bit,
    jit,
    ffi,
};

void load(interpreter const&, lib);
void load(interpreter&&, lib) = delete;

void preload(interpreter const&, lib);
void preload(interpreter&&, lib) = delete;
void preload(
    interpreter const&, std::u8string_view, std::function<int(state)> const&);
void preload(
    interpreter&&, std::u8string_view,
    std::function<int(state)> const&) = delete;

}

#endif
