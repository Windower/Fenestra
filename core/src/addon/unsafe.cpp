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

#include "addon/unsafe.hpp"

#include "addon/error.hpp"
#include "addon/lua.hpp"

#include <windows.h>

#include <shlwapi.h>

#include <gsl/gsl>
#include <lua.hpp>

#include <array>
#include <bit>
#include <climits>
#include <cstdint>
#include <cstring>
#include <utility>

namespace
{

extern "C"
{
    static int check_stack_trace(::lua_State* s)
    {
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::stack_trace);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        auto const type = ::lua_type(s, -1);
        ::lua_settop(s, -2);
        return type == LUA_TTABLE;
    }

#if defined(LUAJIT_VERSION_NUM)
//#    if LUAJIT_VERSION_NUM > 20100
//#        error                                                                 \
//            "Please confirm ::estimate_record_count still works on this version of LuaJIT."
//#    endif
    static std::size_t estimate_record_count(::lua_State* s)
    {
        // This depends on undocumented behavior of LuaJIT!
        // When ::lua_getstack fails because an invalid level is passed,
        // the private field ::lua_Debug::i_ci contains the total number of
        // activation records on the stack.
        ::lua_Debug record;
        if (!::lua_getstack(s, -10, &record))
        {
            return record.i_ci;
        }
        return 0;
    }
#else
    static std::size_t estimate_record_count(::lua_State*) { return 0; }
#endif

    static void const* get_target(::lua_State* s, int index) noexcept(false)
    {
        return ::lua_tocfunction(s, index);
    }

    static void push_source_string(::lua_State* s, int index) noexcept(false)
    {
        static constexpr auto ptr_nibbles =
            (sizeof(std::intptr_t) * CHAR_BIT + 3) / 4;

        auto ptr         = ::get_target(s, index);
        auto module      = ::HMODULE{};
        auto const flags = ::DWORD{
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT};
        auto count = 0;
        auto utf8buffer =
            std::array<char, (MAX_PATH - 4) * 4 + 3 + ptr_nibbles>{};
        if (::GetModuleHandleExW(flags, std::bit_cast<::LPCWSTR>(ptr), &module))
        {
            auto utf16buffer      = std::array<::WCHAR, MAX_PATH>{};
            auto const path_count = ::GetModuleFileNameW(
                module, utf16buffer.data(), utf16buffer.size());
            if (path_count != 0 &&
                (path_count < MAX_PATH ||
                 gsl::at(utf16buffer, MAX_PATH - 1) == L'\0'))
            {
                auto const* filename = ::PathFindFileNameW(utf16buffer.data());
                auto const filename_count = ::WideCharToMultiByte(
                    CP_UTF8, 0, filename,
                    utf16buffer.data() + path_count - filename,
                    utf8buffer.data(), (MAX_PATH - 4) * 4, nullptr, nullptr);
                if (filename_count > 0)
                {
                    gsl::at(utf8buffer, filename_count) = u8'!';
                    count                               = filename_count + 1;
                }
            }
        }
        gsl::at(utf8buffer, count)     = u8'0';
        gsl::at(utf8buffer, count + 1) = u8'x';
        for (auto i = 0; i != ptr_nibbles; ++i)
        {
            auto const nibble = std::bit_cast<std::intptr_t>(ptr) >>
                                    ((ptr_nibbles - 1 - i) * 4) &
                                0xF;
            gsl::at(utf8buffer, count + 2 + i) =
                char(nibble < 0xA ? u8'0' + nibble : nibble - 0xA + u8'a');
        }
        ::lua_pushlstring(s, utf8buffer.data(), count + 2 + ptr_nibbles);
    }

    static void
    push_variable(::lua_State* s, int index, char const* name) noexcept(false)
    {
        index = windower::lua::unsafe::absolute(s, index);
        ::lua_createtable(s, 0, 2);
        if (name)
        {
            ::lua_pushlstring(s, "name", 4);
            ::lua_pushstring(s, name);
            ::lua_rawset(s, -3);
        }
        ::lua_pushlstring(s, "type", 4);
        auto type = ::lua_typename(s, ::lua_type(s, index));
        ::lua_pushstring(
            s, std::strcmp(type, "thread") == 0 ? "coroutine" : type);
        ::lua_rawset(s, -3);
        ::lua_pushlstring(s, "value", 5);
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::to_string);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        ::lua_pushvalue(s, index);
        ::lua_call(s, 1, 1);
        ::lua_rawset(s, -3);
    }

    static int save_stack_trace1(::lua_State* s) noexcept(false)
    {
        ::lua_createtable(s, estimate_record_count(s), 0);

        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::stack_trace);
        ::lua_pushvalue(s, -2);
        ::lua_rawset(s, LUA_REGISTRYINDEX);

        auto record       = ::lua_Debug{};
        auto record_count = 0;
        for (auto level = 2; ::lua_getstack(s, level, &record); ++level)
        {
            ::lua_getinfo(s, "nSluf", &record);
            ::lua_createtable(s, 0, 3);
            if (record.name)
            {
                ::lua_pushlstring(s, "name", 4);
                ::lua_pushstring(s, record.name);
                ::lua_rawset(s, -3);
            }

            if (record.namewhat && record.namewhat[0] != u8'\0')
            {
                ::lua_pushlstring(s, "type", 4);
                ::lua_pushstring(s, record.namewhat);
                ::lua_rawset(s, -3);
            }

            ::lua_pushlstring(s, "source", 6);
            ::lua_createtable(s, 0, 0);
            if (::lua_iscfunction(s, -4))
            {
                ::lua_pushlstring(s, "type", 4);
                ::lua_pushlstring(s, "native", 6);
                ::lua_rawset(s, -3);
                ::lua_pushlstring(s, "value", 5);
                ::push_source_string(s, -5);
                ::lua_rawset(s, -3);
            }
            else if (record.source)
            {
                ::lua_pushlstring(s, "type", 4);
                auto src = record.source;
                if (src[0] == u8'@')
                {
                    ::lua_pushlstring(s, "file", 4);
                    ++src;
                }
                else
                {
                    ::lua_pushlstring(s, "string", 6);
                }
                ::lua_rawset(s, -3);
                ::lua_pushlstring(s, "value", 5);
                ::lua_pushstring(s, src);
                ::lua_rawset(s, -3);
            }
            if (record.currentline != 0 && record.currentline != -1)
            {
                ::lua_pushlstring(s, "line", 4);
                ::lua_pushnumber(s, std::size_t(record.currentline));
                ::lua_rawset(s, -3);
            }
            ::lua_rawset(s, -3);

            ::lua_pushlstring(s, "locals", 6);
            ::lua_createtable(s, 0, 0);
            auto locals_count = 0;
            auto local        = 1;
            while (auto name = ::lua_getlocal(s, &record, local))
            {
                ::push_variable(s, -1, name);
                ::lua_rawseti(s, -3, ++locals_count);
                ::lua_settop(s, -2);
                ++local;
            }
            local = -1;
            while (auto name = ::lua_getlocal(s, &record, local))
            {
                ::push_variable(s, -1, name);
                ::lua_rawseti(s, -3, ++locals_count);
                ::lua_settop(s, -2);
                --local;
            }
            ::lua_rawset(s, -3);

            ::lua_pushlstring(s, "upvalues", 8);
            ::lua_createtable(s, 0, 0);
            auto const is_wrapped =
                ::lua_iscfunction(s, -4) &&
                ::get_target(s, -4) != ::lua_tocfunction(s, -4);
            auto upvalue = is_wrapped ? 2 : 1;
            while (auto name = ::lua_getupvalue(s, -4, upvalue))
            {
                ::push_variable(s, -1, name[0] != u8'\0' ? name : nullptr);
                ::lua_rawseti(s, -3, is_wrapped ? upvalue - 1 : upvalue);
                ::lua_settop(s, -2);
                ++upvalue;
            }
            ::lua_rawset(s, -3);

            ::lua_rawseti(s, -3, ++record_count);
            ::lua_settop(s, -2);
        }
        return 0;
    }

    static void push_variables(
        ::lua_State* s, std::vector<windower::lua::variable_descriptor> const&
                            variables) noexcept(false)
    {
        ::lua_createtable(s, variables.size(), 0);
        for (std::size_t i = 0; i < variables.size(); ++i)
        {
            auto& variable = gsl::at(variables, i);
            ::lua_createtable(s, 0, 2);
            if (!variable.name.empty())
            {
                ::lua_pushlstring(s, "name", 4);
                auto const name = windower::to_string_view(variable.name);
                ::lua_pushlstring(s, name.data(), name.size());
                ::lua_rawset(s, -3);
            }
            ::lua_pushlstring(s, "type", 4);
            auto const type = windower::to_string_view(variable.type);
            ::lua_pushlstring(s, type.data(), type.size());
            ::lua_rawset(s, -3);
            ::lua_pushlstring(s, "value", 5);
            auto const value = windower::to_string_view(variable.value);
            ::lua_pushlstring(s, value.data(), value.size());
            ::lua_rawset(s, -3);
            ::lua_rawseti(s, -2, i + 1);
        }
    }

    static int save_stack_trace2(::lua_State* s) noexcept(false)
    {
        auto& trace =
            *static_cast<std::vector<windower::lua::stack_frame> const*>(
                ::lua_touserdata(s, 1));
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::stack_trace);
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::cache);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        ::lua_pushlightuserdata(
            s, const_cast<std::vector<windower::lua::stack_frame>*>(&trace));
        ::lua_rawget(s, -2);
        ::lua_pushlightuserdata(
            s, const_cast<std::vector<windower::lua::stack_frame>*>(&trace));
        ::lua_pushnil(s);
        ::lua_rawset(s, -4);
        ::lua_replace(s, -2);
        if (::lua_type(s, -1) != LUA_TTABLE)
        {
            ::lua_settop(s, -2);
            ::lua_createtable(s, trace.size(), 0);
            for (std::size_t i = 0; i < trace.size(); ++i)
            {
                auto& frame = gsl::at(trace, i);
                ::lua_createtable(s, 0, 3);
                if (!frame.name.empty())
                {
                    ::lua_pushlstring(s, "name", 4);
                    auto const name = windower::to_string_view(frame.name);
                    ::lua_pushlstring(s, name.data(), name.size());
                    ::lua_rawset(s, -3);
                }

                if (!frame.type.empty())
                {
                    ::lua_pushlstring(s, "type", 4);
                    auto const type = windower::to_string_view(frame.type);
                    ::lua_pushlstring(s, type.data(), type.size());
                    ::lua_rawset(s, -3);
                }

                ::lua_pushlstring(s, "source", 6);
                ::lua_createtable(s, 0, 0);
                if (!frame.source.type.empty())
                {
                    ::lua_pushlstring(s, "type", 4);
                    auto const source_type =
                        windower::to_string_view(frame.source.type);
                    ::lua_pushlstring(
                        s, source_type.data(), source_type.size());
                    ::lua_rawset(s, -3);
                }
                if (!frame.source.type.empty())
                {
                    ::lua_pushlstring(s, "value", 5);
                    auto const source_value =
                        windower::to_string_view(frame.source.value);
                    ::lua_pushlstring(
                        s, source_value.data(), source_value.size());
                    ::lua_rawset(s, -3);
                }
                if (frame.source.line)
                {
                    ::lua_pushlstring(s, "line", 4);
                    ::lua_pushnumber(s, frame.source.line);
                    ::lua_rawset(s, -3);
                }
                ::lua_rawset(s, -3);

                ::lua_pushlstring(s, "locals", 6);
                ::push_variables(s, frame.locals);
                ::lua_rawset(s, -3);

                ::lua_pushlstring(s, "upvalues", 8);
                ::push_variables(s, frame.upvalues);
                ::lua_rawset(s, -3);

                ::lua_rawseti(s, -2, i + 1);
            }
        }
        ::lua_rawset(s, LUA_REGISTRYINDEX);
        return 0;
    }

    static void get_variables(
        ::lua_State* s, std::vector<windower::lua::variable_descriptor>& list)
    {
        for (auto i = 1;; ++i)
        {
            ::lua_rawgeti(s, -1, i);
            if (::lua_type(s, -1) != LUA_TTABLE)
            {
                ::lua_settop(s, -2);
                break;
            }

            ::lua_pushlstring(s, "name", 4);
            ::lua_rawget(s, -2);
            ::lua_pushlstring(s, "type", 4);
            ::lua_rawget(s, -3);
            ::lua_pushlstring(s, "value", 5);
            ::lua_rawget(s, -4);

            list.emplace_back();
            auto& variable = list.back();
            variable.name  = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -3);
            variable.type = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -2);
            variable.value = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -1);

            ::lua_settop(s, -5);
        }
    }

    static int get_stack_trace(::lua_State* s) noexcept(false)
    {
        auto& trace = *static_cast<std::vector<windower::lua::stack_frame>*>(
            ::lua_touserdata(s, 1));
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::stack_trace);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::stack_trace);
        ::lua_pushnil(s);
        ::lua_rawset(s, LUA_REGISTRYINDEX);
        ::lua_pushlightuserdata(s, windower::lua::unsafe::key::cache);
        ::lua_rawget(s, LUA_REGISTRYINDEX);
        ::lua_pushlightuserdata(s, &trace);
        ::lua_pushvalue(s, -3);
        ::lua_rawset(s, -3);
        ::lua_settop(s, -2);

        for (auto i = 1;; ++i)
        {
            ::lua_rawgeti(s, -1, i);
            if (::lua_type(s, -1) != LUA_TTABLE)
            {
                ::lua_settop(s, -2);
                break;
            }

            ::lua_pushlstring(s, "name", 4);
            ::lua_rawget(s, -2);
            ::lua_pushlstring(s, "type", 4);
            ::lua_rawget(s, -3);
            ::lua_pushlstring(s, "source", 6);
            ::lua_rawget(s, -4);
            ::lua_pushlstring(s, "type", 4);
            ::lua_rawget(s, -2);
            ::lua_pushlstring(s, "value", 5);
            ::lua_rawget(s, -3);
            ::lua_pushlstring(s, "line", 4);
            ::lua_rawget(s, -4);

            trace.emplace_back();
            auto& frame = trace.back();
            frame.name  = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -6);
            frame.type = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -5);
            frame.source.type = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -3);
            frame.source.value = windower::lua::get<std::u8string>(
                windower::lua::unsafe::wrap(s), -2);
            frame.source.line = windower::lua::get<std::size_t>(
                windower::lua::unsafe::wrap(s), -1);

            ::lua_settop(s, -7);

            ::lua_pushlstring(s, "locals", 6);
            ::lua_rawget(s, -2);
            ::get_variables(s, frame.locals);
            ::lua_settop(s, -2);

            ::lua_pushlstring(s, "upvalues", 8);
            ::lua_rawget(s, -2);
            ::get_variables(s, frame.upvalues);
            ::lua_settop(s, -3);
        }

        return 0;
    }
}

char error_handler_key = 0;
char cache_key         = 0;
char to_string_key     = 0;
char stack_trace_key   = 0;

// Evil sneaky trickery here!
// This is, in fact, legal c++, as bad as it looks.

template<typename T, typename T::type M>
class steal_private_member
{
public:
    friend typename T::type get(T) noexcept { return M; }
};

class state_tag
{
public:
    using type = void* windower::lua::state::*;
    friend type get(state_tag) noexcept;
};

class stack_guard_tag
{
public:
    using type = std::size_t windower::lua::stack_guard::*;
    friend type get(stack_guard_tag) noexcept;
};

template class steal_private_member<state_tag, &windower::lua::state::m_ptr>;
template class steal_private_member<
    stack_guard_tag, &windower::lua::stack_guard::m_base>;

}

::lua_State*& windower::lua::unsafe::unwrap(state& s) noexcept
{
    return *std::bit_cast<::lua_State**>(&(s.*get(state_tag{})));
}

::lua_State* const& windower::lua::unsafe::unwrap(state const& s) noexcept
{
    return *std::bit_cast<::lua_State* const*>(&(s.*get(state_tag{})));
}

::lua_State* windower::lua::unsafe::unwrap(stack_guard const& s) noexcept
{
    state temp = s;
    return unwrap(temp);
}

windower::lua::state windower::lua::unsafe::wrap(::lua_State* s) noexcept
{
    windower::lua::state wrapped;
    unwrap(wrapped) = s;
    return wrapped;
}

int windower::lua::unsafe::absolute(lua_State* s, int index) noexcept
{
    return index > 0 || index <= LUA_REGISTRYINDEX
               ? index
               : ::lua_gettop(s) + index + 1;
}

void windower::lua::unsafe::set_base(stack_guard& s, std::size_t base) noexcept
{
    *&(s.*get(stack_guard_tag{})) = base;
}

void windower::lua::unsafe::set_stack_trace(lua_State* s)
{
    if (!check_stack_trace(s))
    {
        ::lua_cpcall(s, ::save_stack_trace1, nullptr);
    }
}

void windower::lua::unsafe::set_stack_trace(
    lua_State* s, std::vector<stack_frame> const& trace)
{
    if (!check_stack_trace(s))
    {
        ::lua_cpcall(
            s, ::save_stack_trace2,
            const_cast<std::vector<stack_frame>*>(&trace));
    }
}

std::unique_ptr<std::vector<windower::lua::stack_frame> const>
windower::lua::unsafe::get_stack_trace(lua_State* s)
{
    if (check_stack_trace(s))
    {
        auto ptr = std::make_unique<std::vector<windower::lua::stack_frame>>();
        ::lua_cpcall(s, ::get_stack_trace, ptr.get());
        return std::move(ptr);
    }
    return {};
}

void* const windower::lua::unsafe::key::error_handler = &::error_handler_key;
void* const windower::lua::unsafe::key::cache         = &::cache_key;
void* const windower::lua::unsafe::key::to_string     = &::to_string_key;
void* const windower::lua::unsafe::key::stack_trace   = &::stack_trace_key;
