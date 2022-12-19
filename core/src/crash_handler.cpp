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

#include "crash_handler.hpp"

#include "cloak.hpp"
#include "guid.hpp"
#include "library.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <crtdbg.h>
#include <dbghelp.h>
#include <objbase.h>
#include <stdlib.h>

#include <gsl/gsl>

#include <array>
#include <bit>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace
{

constexpr auto default_dump_type =
#if defined(_DEBUG)
    windower::dump_type::full;
#else
    windower::dump_type::basic;
#endif

std::wstring quote_argument(std::wstring_view argument)
{
    std::wstring result;
    if (!argument.empty() &&
        argument.find_first_of(L" \t\n\v\"") == std::wstring::npos)
    {
        result.append(argument);
    }
    else
    {
        result.reserve(argument.size() + 2);
        result.push_back(L'"');
        auto mark            = argument.begin();
        auto backslash_count = 0;
        for (auto it = argument.begin(); it != argument.end(); ++it)
        {
            if (*it == L'\\')
            {
                ++backslash_count;
            }
            else
            {
                if (*it == L'"')
                {
                    result.append(mark, it);
                    result.append(backslash_count + 1, L'\\');
                    result.push_back(L'"');
                    mark = ++it;
                }
                backslash_count = 0;
            }
        }
        result.append(mark, argument.end());
        result.append(backslash_count, L'\\');
        result.push_back(L'"');
    }
    return result;
}

std::wstring get_module_name(::HMODULE library)
{
    std::vector<wchar_t> buffer(MAX_PATH);
    auto size = ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    while (size == buffer.size())
    {
        buffer.resize(buffer.size() * 2);
        size = ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    }
    if (size == 0)
    {
        throw std::system_error{
            gsl::narrow_cast<int>(::GetLastError()), std::system_category()};
    }
    std::filesystem::path path{buffer.begin(), buffer.begin() + size};
    return path.filename().wstring();
}

std::wstring get_signature(::EXCEPTION_POINTERS& exception)
{
    auto const address = exception.ExceptionRecord->ExceptionAddress;
    auto const library = static_cast<::HMODULE>(windower::module_for(address));

    auto const code        = exception.ExceptionRecord->ExceptionCode;
    auto const module_name = get_module_name(library);
    auto const offset      = std::bit_cast<std::uintptr_t>(address) -
                        std::bit_cast<std::uintptr_t>(library);

    std::wstring result;
    auto hex_code = windower::to_u8string(code, 16);
    result.append(hex_code.size() >= 8 ? 0 : 8 - hex_code.size(), L'0');
    result.append(hex_code.begin(), hex_code.end());
    result.append(1, L'@');
    result.append(module_name);
    result.append(1, L'+');
    auto hex_offset = windower::to_u8string(offset, 16);
    result.append(hex_offset.size() >= 8 ? 0 : 8 - hex_offset.size(), L'0');
    result.append(hex_offset.begin(), hex_offset.end());
    return result;
}

std::filesystem::path generate_unique_name()
{
    std::filesystem::path result;
    result += windower::guid::generate().string();
    result += u8".dmp";
    return result;
}

void write_dump(
    std::filesystem::path const& path, windower::dump_type type,
    void* exception)
{
    std::filesystem::create_directories(path.parent_path());
    auto file = ::CreateFileW(
        path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION info = {};
        info.ExceptionPointers = static_cast<::EXCEPTION_POINTERS*>(exception);
        info.ThreadId          = ::GetCurrentThreadId();
        auto guard             = windower::uncloak();
        ::MiniDumpWriteDump(
            ::GetCurrentProcess(), ::GetCurrentProcessId(), file,
            gsl::narrow_cast<::MINIDUMP_TYPE>(type), &info, nullptr, nullptr);
        ::CloseHandle(file);
    }
}

}

extern "C"
{
    static ::LONG CALLBACK
    unhandled_exception_handler(::PEXCEPTION_POINTERS ExceptionInfo)
    {
        if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord ||
            ExceptionInfo->ExceptionRecord->ExceptionCode !=
                DBG_PRINTEXCEPTION_C)
        {
            windower::crash_handler::instance().crash(ExceptionInfo);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    static void abort_handler(int)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }

#if defined(_MSC_VER)
    static void pure_call_handler()
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }

    static void invalid_parameter_handler(
        [[maybe_unused]] wchar_t const* expression,
        [[maybe_unused]] wchar_t const* function,
        [[maybe_unused]] wchar_t const* file,
        [[maybe_unused]] unsigned int line,
        [[maybe_unused]] std::uintptr_t reserved)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
    static int crt_error_handler(
        [[maybe_unused]] int reportType, [[maybe_unused]] wchar_t* message,
        [[maybe_unused]] int* returnValue)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }
#endif
}

void windower::crash_handler::initialize() { instance(); }

windower::crash_handler& windower::crash_handler::instance()
{
    static crash_handler instance{
        std::filesystem::temp_directory_path() / u8"Windower"};
    return instance;
}

std::filesystem::path windower::crash_handler::write_dump(
    std::filesystem::path const& path, windower::dump_type type)
{
    auto temp = path / generate_unique_name();
    ::EXCEPTION_RECORD record{};
    ::CONTEXT context{};
    ::RtlCaptureContext(&context);
    ::EXCEPTION_POINTERS exception{&record, &context};
    ::write_dump(temp, type, &exception);
    return temp;
}

windower::crash_handler::crash_handler(std::filesystem::path const& path) :
    m_path{path}, m_full_path{path / ::generate_unique_name()},
    m_full_path_quoted{::quote_argument(m_full_path.wstring())},
    m_dump_type{::default_dump_type}
{
    ::SetUnhandledExceptionFilter(::unhandled_exception_handler);
    std::signal(SIGABRT, ::abort_handler);

#if defined(_MSC_VER)
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    _set_purecall_handler(::pure_call_handler);
    _set_invalid_parameter_handler(::invalid_parameter_handler);
    _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, ::crt_error_handler);
    _CrtSetReportMode(_CRT_WARN, 0);
    _CrtSetReportMode(_CRT_ERROR, 0);
    _CrtSetReportMode(_CRT_ASSERT, 0);
#endif

    if (library kernel32{u8"kernel32.dll"})
    {
        auto get_policy = kernel32.get_function<::BOOL WINAPI(::LPDWORD)>(
            u8"GetProcessUserModeExceptionPolicy");
        auto set_policy = kernel32.get_function<::BOOL WINAPI(::DWORD)>(
            u8"SetProcessUserModeExceptionPolicy");
        if (get_policy && set_policy)
        {
            ::DWORD flags;
            if (get_policy(&flags))
            {
                set_policy(flags & ~0x1);
            }
        }
    }
}

std::filesystem::path const& windower::crash_handler::dump_path() const noexcept
{
    return m_path;
}

void windower::crash_handler::dump_path(std::filesystem::path const& path)
{
    auto temp_path             = path;
    auto temp_full_path        = path / generate_unique_name();
    auto temp_full_path_quoted = quote_argument(temp_full_path.wstring());

    m_path             = std::move(temp_path);
    m_full_path        = std::move(temp_full_path);
    m_full_path_quoted = std::move(temp_full_path_quoted);
}

windower::dump_type windower::crash_handler::default_dump_type() const noexcept
{
    return m_dump_type;
}

void windower::crash_handler::default_dump_type(
    windower::dump_type dump_type) noexcept
{
    m_dump_type = dump_type;
}

std::filesystem::path windower::crash_handler::write_dump() const
{
    return write_dump(m_path, m_dump_type);
}

std::filesystem::path
windower::crash_handler::write_dump(windower::dump_type dump_type) const
{
    return write_dump(m_path, dump_type);
}

std::filesystem::path
windower::crash_handler::write_dump(std::filesystem::path const& path) const
{
    return write_dump(path, m_dump_type);
}

void windower::crash_handler::crash(void* exception) const
{
    ::write_dump(m_full_path, m_dump_type, exception);

    auto reporter = windower_path() / u8"windower.exe";
    auto args     = quote_argument(reporter.wstring()) + L" report-crash " +
                m_full_path_quoted;
    if (auto ptr = static_cast<::EXCEPTION_POINTERS*>(exception))
    {
        args.append(L" --signature ");
        args.append(quote_argument(get_signature(*ptr)));
    }

    ::PROCESS_INFORMATION process_info = {};
    ::STARTUPINFOW startup_info        = {};
    startup_info.cb                    = sizeof startup_info;
    auto const result                  = ::CreateProcessW(
                         reporter.c_str(), args.data(), nullptr, nullptr, false, 0, nullptr,
                         nullptr, &startup_info, &process_info);
    if (result)
    {
        ::CloseHandle(process_info.hProcess);
        ::CloseHandle(process_info.hThread);
    }

    constexpr auto access = SYNCHRONIZE | PROCESS_TERMINATE;
    auto const pid        = ::GetCurrentProcessId();
    auto const handle     = ::OpenProcess(access, false, pid);
    ::TerminateProcess(handle, EXIT_FAILURE);
    ::WaitForSingleObject(handle, INFINITE);
}
