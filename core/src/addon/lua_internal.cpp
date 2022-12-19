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

#include "addon/lua_internal.hpp"

#include "addon/error.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp"
#include "utility.hpp"

#include <lua.hpp>

#include <memory>
#include <sstream>

namespace
{

template<std::size_t N>
constexpr std::size_t size(char const (&)[N])
{
    return N - 1;
}

void cpcall(::lua_State* s, ::lua_CFunction function, void* data)
{
    if (::lua_cpcall(s, function, data) != 0)
    {
        throw windower::lua::error{windower::lua::unsafe::wrap(s)};
    }
}

int wrap(::lua_State* s, ::lua_CFunction loader_function)
{
    ::lua_pushvalue(s, 1);
    ::lua_rawget(s, LUA_GLOBALSINDEX);
    ::lua_pushcclosure(s, loader_function, 0);
    ::lua_pushvalue(s, 1);
    ::lua_call(s, 1, 0);
    ::lua_pushvalue(s, 1);
    ::lua_rawget(s, LUA_GLOBALSINDEX);
    ::lua_insert(s, -2);
    ::lua_pushvalue(s, 1);
    ::lua_insert(s, -2);
    ::lua_rawset(s, LUA_GLOBALSINDEX);
    return 1;
}
}

extern "C"
{
    static int panic_handler(::lua_State*) noexcept(false)
    {
        windower::fail_fast();
    }

    static int error_handler(::lua_State* s) noexcept(false)
    {
        windower::lua::unsafe::set_stack_trace(s);
        return 1;
    }

    static int initialize(::lua_State* s) noexcept(false)
    {
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::error_handler);
        ::lua_pushcclosure(s, ::error_handler, 0);
        ::lua_rawset(s, LUA_REGISTRYINDEX);

        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::cache);
        ::lua_createtable(s, 0, 0);
        ::lua_createtable(s, 0, 1);
        ::lua_pushlstring(s, "__mode", 6);
        ::lua_pushlstring(s, "v", 1);
        ::lua_rawset(s, -3);
        ::lua_setmetatable(s, -2);
        ::lua_rawset(s, LUA_REGISTRYINDEX);

        ::lua_pushcclosure(s, ::luaopen_base, 0);
        ::lua_pushlstring(s, "", 0);
        ::lua_call(s, 1, 0);

        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::to_string);
        ::lua_pushlstring(s, "tostring", 8);
        ::lua_rawget(s, LUA_GLOBALSINDEX);
        ::lua_rawset(s, LUA_REGISTRYINDEX);

        return 0;
    }

    static int luaopen_math_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_math);
    }

    static int luaopen_string_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_string);
    }

    static int luaopen_table_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_table);
    }

    static int luaopen_io_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_io);
    }

    static int luaopen_os_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_os);
    }

    static int luaopen_package_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_package);
    }

    static int luaopen_debug_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_debug);
    }

    static int luaopen_bit_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_bit);
    }

    static int luaopen_jit_wrapped(::lua_State* s)
    {
        return wrap(s, ::luaopen_jit);
    }

    static void
    push_lib_values(::lua_State* s, windower::lua::lib lib, bool wrapped)
    {
        switch (lib)
        {
        case windower::lua::lib::math:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_math_wrapped : ::luaopen_math, 0);
            ::lua_pushlstring(s, LUA_MATHLIBNAME, size(LUA_MATHLIBNAME));
            break;

        case windower::lua::lib::string:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_string_wrapped : ::luaopen_string, 0);
            ::lua_pushlstring(s, LUA_STRLIBNAME, size(LUA_STRLIBNAME));
            break;

        case windower::lua::lib::table:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_table_wrapped : ::luaopen_table, 0);
            ::lua_pushlstring(s, LUA_TABLIBNAME, size(LUA_TABLIBNAME));
            break;

        case windower::lua::lib::io:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_io_wrapped : ::luaopen_io, 0);
            ::lua_pushlstring(s, LUA_IOLIBNAME, size(LUA_IOLIBNAME));
            break;

        case windower::lua::lib::os:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_os_wrapped : ::luaopen_os, 0);
            ::lua_pushlstring(s, LUA_OSLIBNAME, size(LUA_OSLIBNAME));
            break;

        case windower::lua::lib::package:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_package_wrapped : ::luaopen_package, 0);
            ::lua_pushlstring(s, LUA_LOADLIBNAME, size(LUA_LOADLIBNAME));
            break;

        case windower::lua::lib::debug:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_debug_wrapped : ::luaopen_debug, 0);
            ::lua_pushlstring(s, LUA_DBLIBNAME, size(LUA_DBLIBNAME));
            break;

        case windower::lua::lib::bit:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_bit_wrapped : ::luaopen_bit, 0);
            ::lua_pushlstring(s, LUA_BITLIBNAME, size(LUA_BITLIBNAME));
            break;

        case windower::lua::lib::jit:
            ::lua_pushcclosure(
                s, wrapped ? ::luaopen_jit_wrapped : ::luaopen_jit, 0);
            ::lua_pushlstring(s, LUA_JITLIBNAME, size(LUA_JITLIBNAME));
            break;

        case windower::lua::lib::ffi:
            ::lua_pushcclosure(
                s, ::luaopen_ffi, 0); // doesn't need to be wrapped
            ::lua_pushlstring(s, LUA_FFILIBNAME, size(LUA_FFILIBNAME));
            break;
        }
    }

    static int load_lib(::lua_State* s) noexcept(false)
    {
        auto const lib =
            *static_cast<windower::lua::lib*>(::lua_touserdata(s, 1));
        ::push_lib_values(s, lib, false);
        ::lua_call(s, 1, 0);
        return 0;
    }

    static int preload_lib(::lua_State* s) noexcept(false)
    {
        auto const lib =
            *static_cast<windower::lua::lib*>(::lua_touserdata(s, 1));
        ::lua_pushlstring(s, "_PRELOAD", 8);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        if (::lua_type(s, -1) != LUA_TTABLE)
        {
            ::lua_pushlstring(
                s, "\"package\" lib must be loaded to use preload.", 44);
            ::lua_error(s);
        }
        ::push_lib_values(s, lib, true);
        ::lua_insert(s, -2);
        ::lua_rawset(s, -3);
        ::lua_settop(s, -2);
        return 0;
    }
}

windower::lua::interpreter::interpreter() noexcept
{
    auto ptr = ::luaL_newstate();
    ::lua_atpanic(ptr, ::panic_handler);
    cpcall(ptr, ::initialize, nullptr);
    unsafe::unwrap(m_state) = ptr;
    load(*this, lib::jit);
}

windower::lua::interpreter::interpreter(interpreter&& other) noexcept
{
    if (&other != this)
    {
        unsafe::unwrap(m_state)       = unsafe::unwrap(other.m_state);
        unsafe::unwrap(other.m_state) = nullptr;
    }
}

windower::lua::interpreter::~interpreter()
{
    if (auto ptr = unsafe::unwrap(m_state))
    {
        ::lua_close(ptr);
    }
    unsafe::unwrap(m_state) = nullptr;
}

windower::lua::interpreter&
windower::lua::interpreter::operator=(interpreter&& other) noexcept
{
    if (auto ptr = unsafe::unwrap(m_state))
    {
        ::lua_close(ptr);
    }
    unsafe::unwrap(m_state)       = nullptr;
    unsafe::unwrap(m_state)       = unsafe::unwrap(other.m_state);
    unsafe::unwrap(other.m_state) = nullptr;
    return *this;
}

windower::lua::interpreter::operator windower::lua::state() const noexcept
{
    return m_state;
}

void windower::lua::load(interpreter const& i, lib l)
{
    state s = i;
    cpcall(unsafe::unwrap(s), ::load_lib, &l);
}

void windower::lua::preload(interpreter const& i, lib l)
{
    state s = i;
    cpcall(unsafe::unwrap(s), ::preload_lib, &l);
}

void windower::lua::preload(
    interpreter const& i, std::u8string_view name,
    std::function<int(state)> const& function)
{
    stack_guard guard{i};
    push(guard, u8"_PRELOAD");
    raw_get(guard, registry);
    if (typeof(guard, -1) != type::table)
    {
        throw error{"\"package\" lib must be loaded to use preload.", guard};
    }
    push(guard, name);
    push(guard, function);
    raw_set(guard, -3);
}
