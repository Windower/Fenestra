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

#include "addon/lua.hpp"

#include "addon/error.hpp"
#include "addon/unsafe.hpp"
#include "utility.hpp"

#include <lua.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <new>
#include <span>
#include <string>
#include <tuple>
#include <vector>

namespace
{

std::byte std_function_metatable_key;

void throw_argument_error(windower::lua::state s, std::size_t index)
{
    using namespace windower;
    using std::to_string;

    ::lua_Debug info;
    if (::lua_getstack(lua::unsafe::unwrap(s), 0, &info) &&
        ::lua_getinfo(lua::unsafe::unwrap(s), "n", &info))
    {
        throw lua::error{
            "bad argument #" + to_string(index) + " to '" + info.name +
            "' (value expected)"};
    }
    throw lua::error{"bad argument #" + to_string(index) + " (value expected)"};
}

void throw_argument_type_error(
    windower::lua::state s, std::size_t index, windower::lua::type expected,
    windower::lua::type actual)
{
    using namespace windower;
    using std::to_string;

    ::lua_Debug info;
    if (::lua_getstack(lua::unsafe::unwrap(s), 0, &info) &&
        ::lua_getinfo(lua::unsafe::unwrap(s), "n", &info))
    {
        auto message = std::u8string{};
        message.append(u8"bad argument #")
            .append(to_u8string(index))
            .append(u8" to '")
            .append(to_u8string(info.name))
            .append(u8"' (")
            .append(to_u8string_view(expected))
            .append(u8" expected, got ")
            .append(to_u8string_view(actual))
            .append(u8")");
        throw lua::error{windower::to_string(message)};
    }
    else
    {
        auto message = std::u8string{};
        message.append(u8"bad argument #")
            .append(to_u8string(index))
            .append(u8" (")
            .append(to_u8string_view(expected))
            .append(u8" expected, got ")
            .append(to_u8string_view(actual))
            .append(u8")");
        throw lua::error{windower::to_string(message)};
    }
}

}

extern "C"
{
    static int destroy_std_function(::lua_State* s)
    {
        if (::lua_getmetatable(s, 1))
        {
            ::lua_pushlightuserdata(s, &std_function_metatable_key);
            ::lua_rawget(s, LUA_REGISTRYINDEX);
            auto const result = ::lua_rawequal(s, -2, -1);
            ::lua_settop(s, -3);
            if (result)
            {
                if (auto ptr = ::lua_touserdata(s, 1))
                {
                    ::lua_pushnil(s);
                    ::lua_setmetatable(s, 1);
                    static_cast<std::function<int(windower::lua::state)>*>(ptr)
                        ->~function();
                    return 0;
                }
            }
        }
        ::lua_pushlstring(s, "[INTERNAL ERROR] invalid state", 30);
        return ::lua_error(s);
    }

    static int call_std_function(::lua_State* s)
    {
        try
        {
            if (::lua_getmetatable(s, windower::lua::upvalue(0)))
            {
                ::lua_pushlightuserdata(s, &std_function_metatable_key);
                ::lua_rawget(s, LUA_REGISTRYINDEX);
                auto const result = ::lua_rawequal(s, -2, -1);
                ::lua_settop(s, -3);
                if (result)
                {
                    if (auto ptr =
                            ::lua_touserdata(s, windower::lua::upvalue(0)))
                    {
                        return (*static_cast<
                                std::function<int(windower::lua::state)>*>(
                            ptr))(windower::lua::unsafe::wrap(s));
                    }
                }
            }
            ::lua_pushlstring(s, "[INTERNAL ERROR] invalid state", 30);
        }
        catch (windower::lua::error const& e)
        {
            if (e.has_stack_trace())
            {
                windower::lua::unsafe::set_stack_trace(s, e.stack_trace());
            }
            else
            {
                windower::lua::unsafe::set_stack_trace(s);
            }
            ::lua_pushstring(s, e.what());
        }
        return ::lua_error(s);
    }

    static char const*
    reader_impl(::lua_State*, void* context, std::size_t* size)
    {
        auto const buffer =
            static_cast<windower::lua::reader*>(context)->read();
        *size = buffer.size();
        GSL_SUPPRESS(type.1)
        {
            return reinterpret_cast<char const*>(buffer.data());
        }
    }

    static int
    writer_impl(::lua_State*, void const* data, std::size_t size, void* context)
    {
        static_cast<windower::lua::writer*>(context)->write(
            {static_cast<std::byte const*>(data), size});
        return 0;
    }
}

windower::lua::stack_guard::stack_guard(state s) :
    m_state{s}, m_base{gsl::narrow_cast<std::size_t>(
                    ::lua_gettop(unsafe::unwrap(s)))}
{}

windower::lua::stack_guard::stack_guard(stack_guard& s) :
    m_state{s.m_state}, m_base{gsl::narrow_cast<std::size_t>(
                            ::lua_gettop(unsafe::unwrap(s)))}
{}

windower::lua::stack_guard::~stack_guard()
{
    if (auto unwrapped = unsafe::unwrap(m_state);
        unwrapped && ::lua_gettop(unwrapped) > gsl::narrow_cast<int>(m_base))
    {
        ::lua_settop(unwrapped, m_base);
    }
}

windower::lua::stack_guard::operator windower::lua::state() const noexcept
{
    return m_state;
}

std::size_t windower::lua::stack_guard::base() const noexcept { return m_base; }

std::size_t windower::lua::stack_guard::size() const noexcept
{
    return ::lua_gettop(unsafe::unwrap(m_state)) - m_base;
}

std::size_t windower::lua::stack_guard::release() noexcept
{
    auto const count        = size();
    unsafe::unwrap(m_state) = nullptr;
    m_base                  = 0;
    return count;
}

int windower::lua::absolute(state s, int index) noexcept
{
    return unsafe::absolute(unsafe::unwrap(s), index);
}

std::u8string_view windower::lua::to_u8string_view(type value)
{
    switch (value)
    {
    case type::none: return u8"no value";
    case type::nil: return u8"nil";
    case type::boolean: return u8"boolean";
    case type::lightuserdata: return u8"userdata";
    case type::number: return u8"number";
    case type::string: return u8"string";
    case type::table: return u8"table";
    case type::function: return u8"function";
    case type::userdata: return u8"userdata";
    case type::coroutine: return u8"thread";
    case type::cdata: return u8"cdata";
    }

    throw error{"[INTERNAL ERROR] unknown lua type"};
}

std::u8string windower::lua::to_u8string(type value)
{
    return std::u8string{to_u8string_view(value)};
}

void windower::lua::reserve(state s, std::size_t size)
{
    if (!::lua_checkstack(unsafe::unwrap(s), size))
    {
        throw error{"stack overflow", s};
    }
}

std::size_t windower::lua::top(state s)
{
    return ::lua_gettop(unsafe::unwrap(s));
}

windower::lua::type windower::lua::typeof(state s, int index)
{
    return static_cast<type>(::lua_type(unsafe::unwrap(s), index));
}

void windower::lua::check_argument(state s, std::size_t index)
{
    auto const argument_type = typeof(s, index);
    if (argument_type == type::none)
    {
        throw_argument_error(s, index);
    }
}

void windower::lua::check_argument(state s, std::size_t index, type expected)
{
    auto const argument_type = typeof(s, index);
    if (argument_type != expected)
    {
        throw_argument_type_error(s, index, expected, argument_type);
    }
}

bool windower::lua::check_optional_argument(
    state s, std::size_t index, type expected)
{
    auto const argument_type = typeof(s, index);
    if (argument_type == type::nil || argument_type == type::none)
    {
        return false;
    }

    if (argument_type != expected)
    {
        throw_argument_type_error(s, index, expected, argument_type);
    }
    return true;
}

std::size_t windower::lua::size(state s, int index)
{
    return ::lua_objlen(unsafe::unwrap(s), index);
}

bool windower::lua::equal(state s, int index1, int index2)
{
    return ::lua_equal(unsafe::unwrap(s), index1, index2) != 0;
}

bool windower::lua::raw_equal(state s, int index1, int index2)
{
    return ::lua_rawequal(unsafe::unwrap(s), index1, index2) != 0;
}

bool windower::lua::less(state s, int index1, int index2)
{
    return ::lua_lessthan(unsafe::unwrap(s), index1, index2) != 0;
}

windower::lua::coroutine_status windower::lua::status(state s)
{
    return static_cast<coroutine_status>(::lua_status(unsafe::unwrap(s)));
}

void windower::lua::save(state s, writer& w)
{
    ::lua_dump(unsafe::unwrap(s), ::writer_impl, &w);
}

void windower::lua::save(state s, std::ostream& stream)
{
    class stream_writer sealed : public writer
    {
    public:
        stream_writer(std::ostream& stream) noexcept : m_stream{stream} {}

        void write(std::span<std::byte const> buffer) noexcept override
        {
            if (m_stream)
            {
                m_stream.write(
                    reinterpret_cast<char const*>(buffer.data()),
                    buffer.size());
            }
        }

        virtual ~stream_writer() = default;

    private:
        std::ostream& m_stream;
    };

    stream_writer w{stream};
    save(s, w);
}

std::size_t windower::lua::memory_usage(state s)
{
    return gsl::narrow_cast<std::size_t>(
        ::lua_gc(unsafe::unwrap(s), LUA_GCCOUNT, 0) * 1024 +
        ::lua_gc(unsafe::unwrap(s), LUA_GCCOUNTB, 0));
}

void windower::lua::gc_start(state s)
{
    ::lua_gc(unsafe::unwrap(s), LUA_GCRESTART, 0);
}

void windower::lua::gc_stop(state s)
{
    ::lua_gc(unsafe::unwrap(s), LUA_GCSTOP, 0);
}

void windower::lua::gc_configure(state s, float pause, float step_multiplier)
{
    ::lua_gc(
        unsafe::unwrap(s), LUA_GCSETPAUSE,
        gsl::narrow_cast<int>(std::round(pause * 100)));
    ::lua_gc(
        unsafe::unwrap(s), LUA_GCSETSTEPMUL,
        gsl::narrow_cast<int>(std::round(step_multiplier * 100)));
}

void windower::lua::gc_collect(state s)
{
    ::lua_gc(unsafe::unwrap(s), LUA_GCCOLLECT, 0);
}

void windower::lua::gc_increment(state s, std::size_t step)
{
    ::lua_gc(unsafe::unwrap(s), LUA_GCSTEP, step);
}

void windower::lua::top(stack_guard const& s, int top)
{
    auto absolute = unsafe::absolute(unsafe::unwrap(s), top);
    if (absolute > 0)
    {
        if (gsl::narrow_cast<std::size_t>(absolute) < s.base())
        {
            absolute = s.base();
        }
        ::lua_settop(unsafe::unwrap(s), absolute);
    }
}

void windower::lua::pop(stack_guard const& s, std::size_t count)
{
    auto new_top = ::lua_gettop(unsafe::unwrap(s)) - count;
    if (new_top < s.base())
    {
        new_top = s.base();
    }
    ::lua_settop(unsafe::unwrap(s), new_top);
}

void windower::lua::copy(stack_guard const& s, int index)
{
    ::lua_pushvalue(unsafe::unwrap(s), index);
}

void windower::lua::push(stack_guard const& s, nil_t)
{
    ::lua_pushnil(unsafe::unwrap(s));
}

void windower::lua::push(stack_guard const& s, bool value)
{
    ::lua_pushboolean(unsafe::unwrap(s), value);
}

void windower::lua::push(stack_guard const& s, void* value)
{
    ::lua_pushlightuserdata(unsafe::unwrap(s), value);
}

void windower::lua::push(stack_guard const& s, double value)
{
    ::lua_pushnumber(unsafe::unwrap(s), value);
}

void windower::lua::push(stack_guard const& s, std::int32_t value)
{
    ::lua_pushinteger(unsafe::unwrap(s), value);
}

void windower::lua::push(stack_guard const& s, char8_t const* value)
{
    auto const view = to_string_view(value);
    ::lua_pushlstring(unsafe::unwrap(s), view.data(), view.size());
}

void windower::lua::push(stack_guard const& s, std::u8string_view value)
{
    auto const view = to_string_view(value);
    ::lua_pushlstring(unsafe::unwrap(s), view.data(), view.size());
}

void windower::lua::push(stack_guard const& s, std::span<std::byte const> value)
{
    GSL_SUPPRESS(type.1)
    {
        auto const ptr  = reinterpret_cast<char const*>(value.data());
        auto const size = value.size();
        ::lua_pushlstring(unsafe::unwrap(s), ptr, size);
    }
}

void windower::lua::push(
    stack_guard const& s, std::function<int(state)> const& value,
    std::size_t upvalues)
{
    if (s.size() < upvalues)
    {
        throw error{"too few stack arguments"};
    }

    auto ptr =
        windower::lua::create_userdata(s, sizeof(std::function<int(state)>));
    push(s, &std_function_metatable_key);
    raw_get(s, registry);
    if (typeof(s, -1) != type::table)
    {
        pop(s);
        create_table(s, 0, 1);
        push(s, u8"__gc");
        ::lua_pushcclosure(unsafe::unwrap(s), ::destroy_std_function, 0);
        raw_set(s, -3);
        push(s, u8"__metatable");
        push(s, nil);
        raw_set(s, -3);
        push(s, &std_function_metatable_key);
        copy(s, -2);
        raw_set(s, registry);
    }
    set_metatable(s, -2);
    try
    {
        [[gsl::suppress(r .11)]] new (ptr) std::function<int(state)>{value};
    }
    catch (...)
    {
        push(s, nil);
        set_metatable(s, -2);
        throw;
    }

    if (upvalues)
    {
        insert(s, -gsl::narrow<int>(upvalues + 1));
    }
    ::lua_pushcclosure(unsafe::unwrap(s), ::call_std_function, upvalues + 1);
}

bool windower::lua::push(stack_guard const& s, state value)
{
    auto const result = ::lua_pushthread(unsafe::unwrap(value)) != 0;
    if (unsafe::unwrap(s) != unsafe::unwrap(value))
    {
        ::lua_xmove(unsafe::unwrap(value), unsafe::unwrap(s), 1);
    }
    return result;
}

bool windower::lua::push(stack_guard const& s, stack_guard const& value)
{
    return push(s, state{value});
}

void windower::lua::create_table(
    stack_guard const& s, std::size_t array_size, std::size_t hash_size)
{
    ::lua_createtable(unsafe::unwrap(s), array_size, hash_size);
}

void* windower::lua::create_userdata(stack_guard const& s, std::size_t size)
{
    return ::lua_newuserdata(unsafe::unwrap(s), size);
}

windower::lua::state windower::lua::create_coroutine(stack_guard const& s)
{
    return unsafe::wrap(::lua_newthread(unsafe::unwrap(s)));
}

void windower::lua::get(stack_guard const& s, int index)
{
    ::lua_gettable(unsafe::unwrap(s), index);
}

void windower::lua::set(stack_guard const& s, int index)
{
    ::lua_settable(unsafe::unwrap(s), index);
}

void windower::lua::get(stack_guard const& s, int index, u8zstring_view key)
{
    auto const view = to_zstring_view(key);
    ::lua_getfield(unsafe::unwrap(s), index, view.data());
}

void windower::lua::set(stack_guard const& s, int index, u8zstring_view key)
{
    auto const view = to_zstring_view(key);
    ::lua_setfield(unsafe::unwrap(s), index, view.data());
}

void windower::lua::raw_get(stack_guard const& s, int index)
{
    ::lua_rawget(unsafe::unwrap(s), index);
}

void windower::lua::raw_set(stack_guard const& s, int index)
{
    ::lua_rawset(unsafe::unwrap(s), index);
}

void windower::lua::raw_get(stack_guard const& s, int index, int key)
{
    ::lua_rawgeti(unsafe::unwrap(s), index, key);
}

void windower::lua::raw_set(stack_guard const& s, int index, int key)
{
    ::lua_rawseti(unsafe::unwrap(s), index, key);
}

bool windower::lua::get_metatable(stack_guard const& s, int index)
{
    return ::lua_getmetatable(unsafe::unwrap(s), index) != 0;
}

void windower::lua::set_metatable(stack_guard const& s, int index)
{
    ::lua_setmetatable(unsafe::unwrap(s), index);
}

void windower::lua::get_environment(stack_guard const& s, int index)
{
    ::lua_getfenv(unsafe::unwrap(s), index);
}

void windower::lua::set_environment(stack_guard const& s, int index)
{
    ::lua_setfenv(unsafe::unwrap(s), index);
}

void windower::lua::insert(stack_guard const& s, int index)
{
    ::lua_insert(unsafe::unwrap(s), index);
}

void windower::lua::remove(stack_guard const& s, int index)
{
    ::lua_remove(unsafe::unwrap(s), index);
}

void windower::lua::replace(stack_guard const& s, int index)
{
    ::lua_replace(unsafe::unwrap(s), index);
}

void windower::lua::concat(stack_guard const& s, std::size_t count)
{
    ::lua_concat(unsafe::unwrap(s), count);
}

void windower::lua::xmove(
    stack_guard const& src, stack_guard const& dst, std::size_t count)
{
    ::lua_xmove(unsafe::unwrap(src), unsafe::unwrap(dst), count);
}

void windower::lua::call(
    stack_guard const& s, std::size_t args, std::size_t results)
{
    if (s.size() < args)
    {
        throw error{"too few stack arguments"};
    }

    auto unwrapped = unsafe::unwrap(s);

    auto const top = ::lua_gettop(unwrapped) - args;
    ::lua_pushlightuserdata(
        unwrapped, windower::lua::unsafe::key::error_handler);
    ::lua_rawget(unwrapped, LUA_REGISTRYINDEX);
    ::lua_insert(unwrapped, top);
    auto const result = ::lua_pcall(unwrapped, args, results, top);
    ::lua_remove(unwrapped, top);
    switch (result)
    {
    case LUA_OK: return;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    case LUA_ERRERR: throw windower::lua::error{s};
    default: throw error{"[INTERNAL ERROR] invalid state", s};
    }
}

bool windower::lua::resume(stack_guard const& s, std::size_t args)
{
    if (s.size() < args)
    {
        throw error{"too few stack arguments"};
    }

    switch (::lua_resume(unsafe::unwrap(s), args))
    {
    case LUA_OK: return false;
    case LUA_YIELD: return true;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    case LUA_ERRERR:
        push(s, unsafe::key::error_handler);
        raw_get(s, registry);
        insert(s, -2);
        call(s, 1);
        throw error{s};
    default: throw error{"[INTERNAL ERROR] invalid state", s};
    }
}

bool windower::lua::next(stack_guard const& s, int index)
{
    return ::lua_next(unsafe::unwrap(s), index) != 0;
}

void windower::lua::load(stack_guard const& s, reader& r, u8zstring_view name)
{
    auto const view = to_zstring_view(name);
    if (::lua_load(unsafe::unwrap(s), ::reader_impl, &r, view.c_str()))
    {
        throw lua::error{s};
    }
}

void windower::lua::load(
    stack_guard const& s, std::span<std::byte const> buffer,
    u8zstring_view name)
{
    class buffer_reader final : public reader
    {
    public:
        buffer_reader(std::span<std::byte const> buffer) noexcept :
            m_buffer{buffer}
        {}

        std::span<std::byte const> read() noexcept override
        {
            auto result = m_buffer;
            if (!m_done)
            {
                m_buffer = {};
                m_done   = true;
            }
            return result;
        }

        ~buffer_reader() = default;

    private:
        std::span<std::byte const> m_buffer;
        bool m_done = false;
    };

    buffer_reader r{buffer};
    load(s, r, name);
}

void windower::lua::load(
    stack_guard const& s, std::u8string_view source, u8zstring_view name)
{
    load(s, std::as_bytes(std::span<char8_t const>{source}), name);
}

void windower::lua::load(stack_guard const& s, std::u8string_view source)
{
    load(s, source, std::u8string{source}.c_str());
}

void windower::lua::load(
    stack_guard const& s, std::istream& stream, u8zstring_view name)
{
    class stream_reader final : public reader
    {
    public:
        stream_reader(std::istream& stream) noexcept : m_stream{stream} {}

        std::span<std::byte const> read() noexcept override
        {
            if (!m_stream)
            {
                return {};
            }

            GSL_SUPPRESS(type.1)
            {
                auto ptr        = reinterpret_cast<char*>(m_buffer.data());
                auto const size = m_buffer.size();
                m_stream.read(ptr, size);
            }
            return std::span{m_buffer}.subspan(
                0, gsl::narrow_cast<std::size_t>(m_stream.gcount()));
        }

        ~stream_reader() = default;

    private:
        std::array<std::byte, 0x2000> m_buffer = {};
        std::istream& m_stream;
    };

    stream_reader r{stream};
    load(s, r, name);
}

bool windower::lua::detail::get_bool(state s, int index)
{
    return ::lua_toboolean(unsafe::unwrap(s), index) != 0;
}

double windower::lua::detail::get_number(state s, int index)
{
    return ::lua_tonumber(unsafe::unwrap(s), index);
}

std::ptrdiff_t windower::lua::detail::get_integer(state s, int index)
{
    return ::lua_tointeger(unsafe::unwrap(s), index);
}

std::u8string_view windower::lua::detail::get_string_view(state s, int index)
{
    if (typeof(s, index) == type::string)
    {
        std::size_t size;
        if (auto ptr = ::lua_tolstring(unsafe::unwrap(s), index, &size))
        {
            // HACK: I don't think this is legal?
            GSL_SUPPRESS(type.1)
            {
                return {reinterpret_cast<char8_t const*>(ptr), size};
            }
        }
    }
    return {};
}

std::span<std::byte const>
windower::lua::detail::get_data_string_span(state s, int index)
{
    if (typeof(s, index) == type::string)
    {
        std::size_t size;
        if (auto ptr = ::lua_tolstring(unsafe::unwrap(s), index, &size))
        {
            return std::as_bytes(std::span{ptr, size});
        }
    }
    return {};
}

void* windower::lua::detail::get_userdata(state s, int index)
{
    return ::lua_touserdata(unsafe::unwrap(s), index);
}

void const* windower::lua::detail::get_pointer(state s, int index)
{
    return ::lua_topointer(unsafe::unwrap(s), index);
}

windower::lua::state windower::lua::detail::get_coroutine(state s, int index)
{
    return unsafe::wrap(::lua_tothread(unsafe::unwrap(s), index));
}
