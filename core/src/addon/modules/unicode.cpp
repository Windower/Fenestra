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

#include "unicode.hpp"

#include "../../unicode.hpp"
#include "addon/lua.hpp"
#include "addon/modules/unicode.lua.hpp"
#include "hooks/ffximain.hpp"

#include <lua.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace
{

extern "C"
{
    void delete_u8string(void const* ptr) noexcept
    {
        auto const deleter = std::unique_ptr<std::u8string const>{
            static_cast<std::u8string const*>(ptr)};
    }

    void const* u8string_data(void const* ptr) noexcept
    {
        return static_cast<std::u8string const*>(ptr)->data();
    }

    std::size_t u8string_length(void const* ptr) noexcept
    {
        return static_cast<std::u8string const*>(ptr)->size();
    }

    void delete_wstring(void const* ptr) noexcept
    {
        auto const deleter = std::unique_ptr<std::wstring const>{
            static_cast<std::wstring const*>(ptr)};
    }

    void const* wstring_data(void const* ptr) noexcept
    {
        return static_cast<std::wstring const*>(ptr)->data();
    }

    std::size_t wstring_length(void const* ptr) noexcept
    {
        return static_cast<std::wstring const*>(ptr)->size();
    }

    void delete_sjis_string(void const* ptr) noexcept
    {
        auto const deleter = std::unique_ptr<windower::sjis_string const>{
            static_cast<windower::sjis_string const*>(ptr)};
    }

    void const* sjis_string_data(void const* ptr) noexcept
    {
        return static_cast<windower::sjis_string const*>(ptr)->data();
    }

    std::size_t sjis_string_length(void const* ptr) noexcept
    {
        return static_cast<windower::sjis_string const*>(ptr)->size();
    }

    void const* to_wstring_native(char8_t const* str, std::size_t size) noexcept
    {
        auto ptr = std::make_unique<std::wstring>();
        *ptr     = windower::to_wstring({str, size});
        return ptr.release();
    }

    void const*
    from_wstring_native(wchar_t const* str, std::size_t size) noexcept
    {
        auto ptr = std::make_unique<std::u8string>();
        *ptr     = windower::to_u8string({str, size});
        return ptr.release();
    }

    void const*
    to_sjis_string_native(char8_t const* str, std::size_t size) noexcept
    {
        auto ptr = std::make_unique<windower::sjis_string>();
        *ptr     = windower::to_sjis_string({str, size});
        return ptr.release();
    }

    void const* from_sjis_string_native(
        windower::sjis_char const* str, size_t size) noexcept
    {
        auto ptr = std::make_unique<std::u8string>();
        *ptr     = windower::to_u8string({str, size});
        return ptr.release();
    }

    void const* lookup_autotranslate(char32_t code_point) noexcept
    {
        auto ptr = std::make_unique<std::u8string>();
        *ptr     = windower::ffximain::lookup_autotranslate(code_point);
        return ptr.release();
    }
}

}

int windower::load_unicode_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_unicode_source, u8"core.unicode");
    lua::push(guard, &delete_u8string);
    lua::push(guard, &u8string_data);
    lua::push(guard, &u8string_length);
    lua::push(guard, &delete_wstring);
    lua::push(guard, &wstring_data);
    lua::push(guard, &wstring_length);
    lua::push(guard, &delete_sjis_string);
    lua::push(guard, &sjis_string_data);
    lua::push(guard, &sjis_string_length);
    lua::push(guard, &to_wstring_native);
    lua::push(guard, &from_wstring_native);
    lua::push(guard, &to_sjis_string_native);
    lua::push(guard, &from_sjis_string_native);
    lua::push(guard, &lookup_autotranslate);
    lua::call(guard, 14);

    return guard.release();
}
