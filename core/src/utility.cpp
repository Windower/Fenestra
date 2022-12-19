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

#include "utility.hpp"

#include "core.hpp"
#include "handle.hpp"
#include "hooks/kernel32.hpp"
#include "library.hpp"
#include "unicode.hpp"

#include <windows.h>

#include <shlobj.h>

#include <gsl/gsl>

#include <array>
#include <charconv>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <vector>

#if !defined(_MSC_VER)
#    include <codecvt>
#    include <sstream>
#endif

extern "C" ::IMAGE_DOS_HEADER __ImageBase;

void* windower::module_for(void const* ptr) noexcept
{
    ::HMODULE library       = nullptr;
    ::DWORD constexpr flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    if (!::GetModuleHandleExW(flags, static_cast<::LPCWSTR>(ptr), &library))
    {
        return nullptr;
    }
    return library;
}

void* windower::windower_module() noexcept { return &__ImageBase; }

bool windower::is_windower_module(void const* ptr) noexcept
{
    auto const begin = std::bit_cast<std::byte const*>(&__ImageBase);
    auto const end   = std::next(
          begin, std::bit_cast<::IMAGE_NT_HEADERS const*>(
                   std::next(begin, __ImageBase.e_lfanew))
                     ->OptionalHeader.SizeOfImage);
    return begin <= ptr && ptr < end;
}

bool windower::is_game_module(void const* ptr) noexcept
{
    static constexpr auto const modules = std::array<::WCHAR const*, 10>{
        L"ffximain.dll",   nullptr,        L"polcore.dll",
        L"polcoreeu.dll",  L"polhook.dll", L"app.dll",
        L"appeu.dll",      L"ffxi.dll",    L"ffxiresource.dll",
        L"ffxiversion.dll"};

    auto const module = module_for(ptr);
    return module != nullptr &&
           std::find_if(modules.begin(), modules.end(), [module](auto name) {
               return ::GetModuleHandleW(name) == module;
           }) != modules.end();
}

std::filesystem::path windower::windower_path()
{
    namespace fs = std::filesystem;

    auto buffer  = std::array<::WCHAR, MAX_PATH + 1>{};
    auto library = static_cast<::HMODULE>(windower_module());
    ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    buffer.back() = L'\0';
    return fs::canonical(fs::path{buffer.data()}.remove_filename());
}

std::filesystem::path windower::settings_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.settings_path;

    if (result.empty())
    {
        ::WCHAR* wstr      = nullptr;
        auto const deleter = gsl::finally([&]() { ::CoTaskMemFree(wstr); });
        if (SUCCEEDED(::SHGetKnownFolderPath(
                FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &wstr)))
        {
            result = fs::path{wstr} / u8"Windower";
        }
    }

    return result;
}

std::filesystem::path windower::user_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.user_path;

    if (result.empty())
    {
        ::WCHAR* wstr      = nullptr;
        auto const deleter = gsl::finally([&]() { ::CoTaskMemFree(wstr); });
        if (SUCCEEDED(::SHGetKnownFolderPath(
                FOLDERID_SavedGames, KF_FLAG_CREATE, nullptr, &wstr)))
        {
            result = fs::path{wstr} / u8"Windower";
        }
    }

    return result;
}

std::filesystem::path windower::temp_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.temp_path;

    if (result.empty())
    {
        result = fs::temp_directory_path() / u8"Windower";
    }

    return result;
}

std::filesystem::path windower::client_path()
{
    namespace fs = std::filesystem;

    fs::path result;

    auto handle = ::GetModuleHandleW(L"ffximain.dll");
    if (handle)
    {
        auto buffer = std::array<::WCHAR, MAX_PATH + 1>{};
        ::GetModuleFileNameW(handle, buffer.data(), buffer.size());
        buffer.back() = L'\0';
        result = fs::canonical(fs::path{buffer.data()}.remove_filename());
    }

    return result;
}

// This function is completely non-portable, and there's really no way to
// implement it without SEH.
#if defined(_MSC_VER)
namespace
{

constexpr auto MS_VC_EXCEPTION = ::DWORD{0x406D1388};

#    pragma pack(push, 8)
struct threadname_info
{
    ::DWORD dwType = 0x1000;
    ::LPCSTR szName;
    ::DWORD dwThreadID = 0xFFFFFFFF;
    ::DWORD dwFlags    = 0;
};
#    pragma pack(pop)

void set_thread_name_seh(windower::zstring_view name)
{
    __try
    {
        threadname_info info{.szName = name.c_str()};
        ::RaiseException(
            MS_VC_EXCEPTION, 0, sizeof info / sizeof(::ULONG_PTR),
            reinterpret_cast<::ULONG_PTR*>(&info));
    }
    __except (
        ::GetExceptionCode() == MS_VC_EXCEPTION ? EXCEPTION_EXECUTE_HANDLER
                                                : EXCEPTION_CONTINUE_SEARCH)
    {
        __noop();
    }
}

}

void windower::set_thread_name(u8zstring_view name)
{
    if (auto const kernel32 = library{u8"kernel32.dll"})
    {
        if (auto const SetThreadDescription =
                kernel32.get_function<decltype(::SetThreadDescription)>(
                    u8"SetThreadDescription"))
        {
            auto result = SetThreadDescription(
                ::GetCurrentThread(), to_wstring(name).c_str());
            if (SUCCEEDED(result))
            {
                return;
            }
        }
    }
    set_thread_name_seh(to_zstring_view(name));
}
#endif

std::string_view windower::to_string_view(std::u8string_view value) noexcept
{
    static_assert(char_is_ascii());

    [[gsl::suppress(type .1)]]
    {
        return {reinterpret_cast<char const*>(value.data()), value.size()};
    }
}

windower::zstring_view windower::to_zstring_view(u8zstring_view value) noexcept
{
    static_assert(char_is_ascii());

    [[gsl::suppress(type .1)]]
    {
        return {reinterpret_cast<char const*>(value.data()), value.size()};
    }
}

std::u8string windower::to_u8string(std::string_view value)
{
    static_assert(char_is_ascii());

    return {value.begin(), value.end()};
}

std::u8string windower::to_u8string(signed char value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<signed char>::digits10 + 1;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(signed short int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<signed short int>::digits10 + 1;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(signed int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<signed int>::digits10 + 1;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(signed long int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<signed long int>::digits10 + 1;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(signed long long int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<signed long long int>::digits10 + 1;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(unsigned char value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits = std::numeric_limits<unsigned char>::digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(unsigned short int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<unsigned short int>::digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(unsigned int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits = std::numeric_limits<unsigned int>::digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(unsigned long int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<unsigned long int>::digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(unsigned long long int value, int base)
{
    static_assert(char_is_ascii());

    static constexpr auto digits =
        std::numeric_limits<unsigned long long int>::digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(float value)
{
    static_assert(char_is_ascii());

    static constexpr auto digits = std::numeric_limits<float>::max_digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value);
    return {begin, result.ptr};
}

std::u8string windower::to_u8string(double value)
{
    static_assert(char_is_ascii());

    static constexpr auto digits = std::numeric_limits<double>::max_digits10;

    auto buffer       = std::array<char, digits>{};
    auto const begin  = buffer.data();
    auto const end    = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value);
    return {begin, result.ptr};
}

std::string windower::to_string(std::u8string_view value)
{
    static_assert(char_is_ascii());

    return {value.begin(), value.end()};
}

std::size_t
windower::parse(std::u8string_view s, std::int8_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::int16_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::int32_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::int64_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::uint8_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::uint16_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::uint32_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::u8string_view s, std::uint64_t& value, int base) noexcept
{
    return parse(to_string_view(s), value, base);
}

std::size_t
windower::parse(std::string_view s, std::int8_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::int16_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::int32_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::int64_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::uint8_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::uint16_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::uint32_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::size_t
windower::parse(std::string_view s, std::uint64_t& value, int base) noexcept
{
    auto const begin  = s.data();
    auto const end    = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range
             ? gsl::narrow_cast<std::size_t>(result.ptr - s.data())
             : 0;
}

std::ostream&
windower::hex_dump(std::ostream& stream, std::span<std::byte const> buffer)
{
    auto line      = buffer.begin();
    auto const end = buffer.end();

    format_guard format{stream};

    stream.setf(std::ios::hex, std::ios_base::basefield);
    stream.fill('0');
    for (; std::distance(line, end) >= 16; std::advance(line, 16))
    {
        stream << &*line << ':' << ' ';
        for (auto i = 0; i < 16; ++i)
        {
            stream.width(2);
            stream << std::to_integer<std::uint8_t>(*std::next(line, i)) << ' ';
        }
        stream << ' ';
        for (auto i = 0; i < 16; ++i)
        {
            auto const c = std::to_integer<char>(*std::next(line, i));
            stream << (c >= 0x20 && c <= 0x7E ? c : '.');
        }
        stream << std::endl;
    }

    auto const extra = std::distance(line, end);
    if (extra)
    {
        stream << &*line << ':' << ' ';
        for (auto i = 0; i < extra; ++i)
        {
            stream.width(2);
            stream << std::to_integer<std::uint8_t>(*std::next(line, i)) << ' ';
        }
        for (int i = extra; i < 16; ++i)
        {
            stream << ' ' << ' ' << ' ';
        }
        stream << ' ';
        for (auto i = 0; i < extra; ++i)
        {
            auto const c = std::to_integer<char>(*std::next(line, i));
            stream << (c >= 0x20 && c <= 0x7E ? c : '.');
        }
        for (int i = extra; i < 16; ++i)
        {
            stream << ' ';
        }
        stream << std::endl;
    }

    return stream;
}

[[noreturn]] void windower::throw_system_error(std::uint32_t error)
{
    throw std::system_error{
        gsl::narrow_cast<int>(error), std::system_category()};
}

[[noreturn]] void windower::throw_system_error()
{
    throw_system_error(::GetLastError());
}

[[noreturn]] void windower::fail_fast() noexcept
{
    WINDOWER_DEBUG_BREAK;
    std::_Exit(-1);
}
