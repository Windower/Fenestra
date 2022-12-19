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

#ifndef WINDOWER_ADDON_LUA_HPP
#define WINDOWER_ADDON_LUA_HPP

#include "utility.hpp"

#include <gsl/gsl>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace windower::lua
{

class state
{
    void* m_ptr;
};

class stack_guard
{
public:
    explicit stack_guard(state);
    explicit stack_guard(stack_guard&);
    stack_guard(stack_guard&&) = delete;

    ~stack_guard();

    stack_guard& operator=(stack_guard const&) = delete;
    stack_guard& operator=(stack_guard&&) = delete;

    void* operator new(std::size_t)   = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete(void*)       = delete;
    void operator delete[](void*)     = delete;

    operator state() const noexcept;

    std::size_t base() const noexcept;
    std::size_t size() const noexcept;
    std::size_t release() noexcept;

private:
    state m_state;
    std::size_t m_base;
};

class reader
{
public:
    virtual std::span<std::byte const> read() noexcept = 0;

protected:
    ~reader() = default;
};

class writer
{
public:
    virtual void write(std::span<std::byte const>) noexcept = 0;

protected:
    ~writer() = default;
};

enum class type
{
    none          = -1,
    nil           = 0,
    boolean       = 1,
    lightuserdata = 2,
    number        = 3,
    string        = 4,
    table         = 5,
    function      = 6,
    userdata      = 7,
    coroutine     = 8,
    cdata         = 10,
};

enum class coroutine_status
{
    ok                  = 0,
    yield               = 1,
    error_runtime       = 2,
    error_syntax        = 3,
    error_memory        = 4,
    error_error_handler = 5,
};

class nil_t
{
public:
    explicit constexpr nil_t() = default;
};

constexpr nil_t nil;

constexpr int registry    = -10000;
constexpr int environment = -10001;
constexpr int globals     = -10002;

constexpr int upvalue(std::size_t index) { return globals - index - 1; }

int absolute(state, int) noexcept;

std::u8string_view to_u8string_view(type value);
std::u8string to_u8string(type value);

void reserve(state, std::size_t);

std::size_t top(state);

type typeof(state, int);

void check_argument(state s, std::size_t index);
void check_argument(state s, std::size_t index, type expected);
bool check_optional_argument(state s, std::size_t index, type expected);

std::size_t size(state, int);

bool equal(state, int, int);
bool raw_equal(state, int, int);
bool less(state, int, int);

coroutine_status status(state);

void save(state, writer&);
void save(state, std::ostream&);

std::size_t memory_usage(state);

void gc_start(state);
void gc_stop(state);
void gc_configure(state, float, float);
void gc_collect(state);
void gc_increment(state, std::size_t = 1);

void top(stack_guard const&, int);
void pop(stack_guard const&, std::size_t = 1);

void copy(stack_guard const&, int);

void push(stack_guard const&, std::nullptr_t) = delete;
void push(stack_guard const&, nil_t);
void push(stack_guard const&, bool);
void push(stack_guard const&, void*);
void push(stack_guard const&, double);
void push(stack_guard const&, std::int32_t);
void push(stack_guard const&, char8_t const*);
void push(stack_guard const&, std::u8string_view);
void push(stack_guard const&, std::span<std::byte const>);
void push(stack_guard const&, char const*)     = delete;
void push(stack_guard const&, wchar_t const*)  = delete;
void push(stack_guard const&, char16_t const*) = delete;
void push(stack_guard const&, char32_t const*) = delete;
void push(
    stack_guard const&, std::function<int(state)> const&, std::size_t = 0);
bool push(stack_guard const&, state);
bool push(stack_guard const&, stack_guard const&);

template<
    typename F,
    typename = std::enable_if_t<
        std::is_convertible_v<F, std::function<int(state)>> &&
        !std::is_same_v<std::function<int(state)>, std::remove_cvref_t<F>>>>
void push(stack_guard const& s, F&& function, std::size_t upvalues = 0)
{
    push(s, std::function<int(state)>{std::forward<F>(function)}, upvalues);
}

void create_table(stack_guard const&, std::size_t = 0, std::size_t = 0);
void* create_userdata(stack_guard const&, std::size_t);
state create_coroutine(stack_guard const&);

void get(stack_guard const&, int);
void set(stack_guard const&, int);
void get(stack_guard const&, int, u8zstring_view);
void set(stack_guard const&, int, u8zstring_view);
void raw_get(stack_guard const&, int);
void raw_set(stack_guard const&, int);
void raw_get(stack_guard const&, int, int);
void raw_set(stack_guard const&, int, int);
bool get_metatable(stack_guard const&, int);
void set_metatable(stack_guard const&, int);
void get_environment(stack_guard const&, int);
void set_environment(stack_guard const&, int);

template<typename T, typename U = std::decay_t<T>>
bool check(state, int);

void insert(stack_guard const&, int);
void remove(stack_guard const&, int);
void replace(stack_guard const&, int);

void concat(stack_guard const&, std::size_t);

void xmove(stack_guard const&, stack_guard const&, std::size_t);

void call(stack_guard const&, std::size_t, std::size_t = -1);
bool resume(stack_guard const&, std::size_t);

bool next(stack_guard const&, int);

void load(stack_guard const&, reader&, u8zstring_view);
void load(stack_guard const&, std::span<std::byte const>, u8zstring_view);
void load(stack_guard const&, std::u8string_view, u8zstring_view);
void load(stack_guard const&, std::u8string_view);
void load(stack_guard const&, std::istream&, u8zstring_view);

namespace detail
{
bool get_bool(state, int);
double get_number(state, int);
std::ptrdiff_t get_integer(state, int);
std::u8string_view get_string_view(state, int);
std::span<std::byte const> get_data_string_span(state, int);
void* get_userdata(state, int);
void const* get_pointer(state, int);
state get_coroutine(state, int);
}

template<typename>
class getter;

template<>
class getter<bool>
{
public:
    static type const lua_type = type::boolean;

    static bool get(state s, int index) { return detail::get_bool(s, index); }
};

template<>
class getter<float>
{
public:
    static type const lua_type = type::number;

    static float get(state s, int index)
    {
        return gsl::narrow_cast<float>(detail::get_number(s, index));
    }
};

template<>
class getter<double>
{
public:
    static type const lua_type = type::number;

    static double get(state s, int index)
    {
        return gsl::narrow_cast<double>(detail::get_number(s, index));
    }
};

template<>
class getter<long double>
{
public:
    static type const lua_type = type::number;

    static long double get(state s, int index)
    {
        return gsl::narrow_cast<long double>(detail::get_number(s, index));
    }
};

template<>
class getter<std::byte>
{
public:
    static type const lua_type = type::number;

    static std::byte get(state s, int index)
    {
        return gsl::narrow_cast<std::byte>(detail::get_integer(s, index));
    }
};

template<>
class getter<char>
{
public:
    static type const lua_type = type::number;

    static char get(state s, int index)
    {
        return gsl::narrow_cast<char>(detail::get_integer(s, index));
    }
};

template<>
class getter<signed char>
{
public:
    static type const lua_type = type::number;

    static signed char get(state s, int index)
    {
        return gsl::narrow_cast<signed char>(detail::get_integer(s, index));
    }
};

template<>
class getter<short int>
{
public:
    static type const lua_type = type::number;

    static short int get(state s, int index)
    {
        return gsl::narrow_cast<short int>(detail::get_integer(s, index));
    }
};

template<>
class getter<int>
{
public:
    static type const lua_type = type::number;

    static int get(state s, int index)
    {
        return gsl::narrow_cast<int>(detail::get_integer(s, index));
    }
};

template<>
class getter<long int>
{
public:
    static type const lua_type = type::number;

    static long int get(state s, int index)
    {
        return gsl::narrow_cast<long int>(detail::get_integer(s, index));
    }
};

template<>
class getter<long long int>
{
public:
    static type const lua_type = type::number;

    static long long int get(state s, int index)
    {
        return gsl::narrow_cast<long long int>(detail::get_number(s, index));
    }
};

template<>
class getter<unsigned char>
{
public:
    static type const lua_type = type::number;

    static unsigned char get(state s, int index)
    {
        return gsl::narrow_cast<unsigned char>(detail::get_integer(s, index));
    }
};

template<>
class getter<unsigned short int>
{
public:
    static type const lua_type = type::number;

    static unsigned short int get(state s, int index)
    {
        return gsl::narrow_cast<unsigned short int>(
            detail::get_integer(s, index));
    }
};

template<>
class getter<unsigned int>
{
public:
    static type const lua_type = type::number;

    static unsigned int get(state s, int index)
    {
        return gsl::narrow_cast<unsigned int>(detail::get_integer(s, index));
    }
};

template<>
class getter<unsigned long int>
{
public:
    static type const lua_type = type::number;

    static unsigned long int get(state s, int index)
    {
        return gsl::narrow_cast<unsigned long int>(
            detail::get_integer(s, index));
    }
};

template<>
class getter<unsigned long long int>
{
public:
    static type const lua_type = type::number;

    static unsigned long long int get(state s, int index)
    {
        return gsl::narrow_cast<unsigned long long int>(
            detail::get_number(s, index));
    }
};

template<>
class getter<char8_t>
{
public:
    static type const lua_type = type::number;

    static char8_t get(state s, int index)
    {
        return gsl::narrow_cast<char8_t>(detail::get_integer(s, index));
    }
};

template<>
class getter<char16_t>
{
public:
    static type const lua_type = type::number;

    static char16_t get(state s, int index)
    {
        return gsl::narrow_cast<char16_t>(detail::get_integer(s, index));
    }
};

template<>
class getter<char32_t>
{
public:
    static type const lua_type = type::number;

    static char32_t get(state s, int index)
    {
        return gsl::narrow_cast<char32_t>(detail::get_integer(s, index));
    }
};

template<>
class getter<wchar_t>
{
public:
    static type const lua_type = type::number;

    static wchar_t get(state s, int index)
    {
        return gsl::narrow_cast<wchar_t>(detail::get_integer(s, index));
    }
};

template<>
class getter<std::u8string_view>
{
public:
    static type const lua_type = type::string;

    static std::u8string_view get(state s, int index)
    {
        return detail::get_string_view(s, index);
    }
};

template<>
class getter<std::u8string>
{
public:
    static type const lua_type = type::string;

    static std::u8string get(state s, int index)
    {
        return std::u8string{detail::get_string_view(s, index)};
    }
};

template<>
class getter<std::vector<std::byte>>
{
public:
    static type const lua_type = type::string;

    static std::vector<std::byte> get(state s, int index)
    {
        auto const span = detail::get_data_string_span(s, index);
        return {span.begin(), span.end()};
    }
};

template<>
class getter<std::span<std::byte const>>
{
public:
    static type const lua_type = type::string;

    static std::span<std::byte const> get(state s, int index)
    {
        return detail::get_data_string_span(s, index);
    }
};

template<>
class getter<void*>
{
public:
    static type const lua_type = type::userdata;

    static void* get(state s, int index)
    {
        return detail::get_userdata(s, index);
    }
};

template<>
class getter<void const*>
{
public:
    static void const* get(state s, int index)
    {
        return detail::get_pointer(s, index);
    }
};

template<>
class getter<state>
{
public:
    static type const lua_type = type::coroutine;

    static state get(state s, int index)
    {
        return detail::get_coroutine(s, index);
    }
};

template<typename T>
class getter
{
public:
    static type const lua_type = type::userdata;

    static T* get(state s, int index)
    {
        return check<T>(s, index)
                   ? static_cast<T*>(getter<void*>::get(s, index))
                   : nullptr;
    }
};

template<typename T>
class getter<std::shared_ptr<T>>
{
public:
    static type const lua_type = type::userdata;

    static std::shared_ptr<T> get(state s, int index)
    {
        return check<std::shared_ptr<T>>(s, index)
                   ? *static_cast<std::shared_ptr<T>*>(
                         getter<void*>::get(s, index))
                   : std::shared_ptr<T>{};
    }
};

template<typename T>
class getter<std::weak_ptr<T>>
{
public:
    static type const lua_type = type::userdata;

    static std::weak_ptr<T> get(state s, int index)
    {
        return check<std::weak_ptr<T>>(s, index)
                   ? *static_cast<std::weak_ptr<T>*>(
                         getter<void*>::get(s, index))
                   : std::weak_ptr<T>{};
    }
};

template<typename T>
auto get(state s, int index)
{
    return getter<std::decay_t<T>>::get(s, index);
}

template<typename T>
auto get_argument(state s, std::size_t index)
{
    check_argument(s, index, getter<std::decay_t<T>>::lua_type);
    return getter<std::decay_t<T>>::get(s, index);
}

template<typename T, typename U>
auto get_optional_argument(
    state s, std::size_t index, U const& default_value = T{})
{
    return check_optional_argument(s, index, getter<std::decay_t<T>>::lua_type)
               ? getter<std::decay_t<T>>::get(s, index)
               : T(default_value);
}

template<typename T, typename U = std::decay_t<T>>
void push_metatable(stack_guard const& s, bool create = true)
{
    static_assert(std::is_destructible<U>::value, "type must be destructible.");

    static std::byte key;

    push(s, &key);
    raw_get(s, registry);
    if (typeof(s, -1) != type::table)
    {
        pop(s);
        if (create)
        {
            create_table(s, 0, 2);
            push(s, u8"__metatable");
            push(s, false);
            raw_set(s, -3);
            if (!std::is_trivially_destructible<U>::value)
            {
                push(s, u8"__gc");
                push(s, std::function<int(state)>{[](state st) {
                         if (!check<U>(st, 1))
                         {
                             throw std::runtime_error{
                                 "invalid userdata type; possible memory "
                                 "corruption"};
                         }
                         std::destroy_at(static_cast<U*>(get<void*>(st, 1)));
                         lua::stack_guard guard{st};
                         lua::push(guard, nil);
                         lua::set_metatable(guard, 1);
                         return 0;
                     }});
                raw_set(s, -3);
            }
            push(s, &key);
            copy(s, -2);
            raw_set(s, registry);
        }
        else
        {
            push(s, nil);
        }
    }
}

template<typename T, typename U = std::decay_t<T>, typename... A>
U* create(stack_guard const& s, A&&... args)
{
    auto ptr = create_userdata(s, sizeof(U));
    push_metatable<U>(s);
    set_metatable(s, -2);
    GSL_SUPPRESS(r.3) { return new (ptr) U{std::forward<A>(args)...}; }
}

template<typename T, typename U>
bool check(state s, int index)
{
    stack_guard guard{s};
    if (!get_metatable(guard, index))
    {
        return false;
    }
    push_metatable<U>(guard);
    return raw_equal(s, -2, -1);
}

}

#endif
