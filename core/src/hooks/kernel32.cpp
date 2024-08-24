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

#include "hooks/kernel32.hpp"

#include "cloak.hpp"
#include "core.hpp"
#include "handle.hpp"
#include "hooklib/hook.hpp"
#include "settings_channel.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <algorithm>
#include <bit>
#include <codecvt>
#include <vector>

namespace
{
::DWORD priority = THREAD_PRIORITY_NORMAL;

::LPTOP_LEVEL_EXCEPTION_FILTER unhandled_exception_filter = nullptr;

namespace hooks
{
windower::hooklib::hook<decltype(::GetACP)> GetACP;
windower::hooklib::hook<decltype(::LoadLibraryA)> LoadLibraryA;
windower::hooklib::hook<decltype(::LoadLibraryW)> LoadLibraryW;
windower::hooklib::hook<decltype(::CreateMutexA)> CreateMutexA;
windower::hooklib::hook<decltype(::CreateMutexW)> CreateMutexW;
windower::hooklib::hook<decltype(::OpenMutexA)> OpenMutexA;
windower::hooklib::hook<decltype(::OpenMutexW)> OpenMutexW;
windower::hooklib::hook<decltype(::GetPriorityClass)> GetPriorityClass;
windower::hooklib::hook<decltype(::SetPriorityClass)> SetPriorityClass;
windower::hooklib::hook<decltype(::CreateProcessW)> CreateProcessW;
windower::hooklib::hook<decltype(::SetUnhandledExceptionFilter)>
    SetUnhandledExceptionFilter;
};

namespace callbacks
{
::UINT WINAPI GetACP() noexcept { return 932; }

::HMODULE WINAPI LoadLibraryA(::LPCSTR lpFileName) noexcept
{
    if (lpFileName && ::CompareStringA(
                          LOCALE_INVARIANT, NORM_IGNORECASE, lpFileName, -1,
                          "hook.dll", 8) == CSTR_EQUAL)
    {
        return nullptr;
    }
    else if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto guard = windower::uncloak();
        return hooks::LoadLibraryA(lpFileName);
    }
    return hooks::LoadLibraryA(lpFileName);
}

::HMODULE WINAPI LoadLibraryW(::LPCWSTR lpFileName) noexcept
{
    if (lpFileName && ::CompareStringW(
                          LOCALE_INVARIANT, NORM_IGNORECASE, lpFileName, -1,
                          L"hook.dll", 8) == CSTR_EQUAL)
    {
        return nullptr;
    }
    else if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto guard = windower::uncloak();
        return hooks::LoadLibraryW(lpFileName);
    }
    return hooks::LoadLibraryW(lpFileName);
}

::HANDLE WINAPI CreateMutexA(
    ::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner,
    ::LPCSTR lpName) noexcept
{
    if (lpName)
    {
        std::string name = lpName;
        name.append(std::to_string(::GetCurrentProcessId()));
        return hooks::CreateMutexA(
            lpMutexAttributes, bInitialOwner, name.c_str());
    }
    return hooks::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
}

::HANDLE WINAPI CreateMutexW(
    ::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner,
    ::LPCWSTR lpName) noexcept
{
    if (lpName)
    {
        std::wstring name = lpName;
        name.append(std::to_wstring(::GetCurrentProcessId()));
        return hooks::CreateMutexW(
            lpMutexAttributes, bInitialOwner, name.c_str());
    }
    return hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
}

::HANDLE WINAPI OpenMutexA(
    ::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
{
    if (lpName)
    {
        std::string name = lpName;
        name.append(std::to_string(::GetCurrentProcessId()));
        return hooks::OpenMutexA(dwDesiredAccess, bInheritHandle, name.c_str());
    }
    return hooks::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName);
}

::HANDLE WINAPI OpenMutexW(
    ::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
{
    if (lpName)
    {
        std::wstring name = lpName;
        name.append(std::to_wstring(::GetCurrentProcessId()));
        return hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, name.c_str());
    }
    return hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
}

::DWORD WINAPI GetPriorityClass(::HANDLE hProcess) noexcept
{
    if (::GetProcessId(hProcess) == ::GetCurrentProcessId())
    {
        return priority;
    }
    return hooks::GetPriorityClass(hProcess);
}

::BOOL WINAPI
SetPriorityClass(::HANDLE hProcess, ::DWORD dwPriorityClass) noexcept
{
    if (::GetProcessId(hProcess) == ::GetCurrentProcessId())
    {
        ::InterlockedExchange(&priority, dwPriorityClass);
        return TRUE;
    }
    return hooks::SetPriorityClass(hProcess, dwPriorityClass);
}

::BOOL WINAPI CreateProcessW(
    ::LPCWSTR lpApplicationName, ::LPWSTR lpCommandLine,
    ::LPSECURITY_ATTRIBUTES lpProcessAttributes,
    ::LPSECURITY_ATTRIBUTES lpThreadAttributes, ::BOOL bInheritHandles,
    ::DWORD dwCreationFlags, ::LPVOID lpEnvironment,
    ::LPCWSTR lpCurrentDirectory, ::LPSTARTUPINFOW lpStartupInfo,
    ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    if (lpApplicationName && lpCommandLine)
    {
        auto name = std::max(
            std::wcsrchr(lpApplicationName, '\\'),
            std::wcsrchr(lpApplicationName, '/'));
        name = name ? std::next(name, 1) : lpApplicationName;
        if (std::wcscmp(name, L"startpol.exe") == 0)
        {
            STARTUPINFO startup{};
            startup.cb = sizeof startup;

            auto path = windower::windower_path() / u8"windower.exe";
            auto u8_args =
                u8R"(")" + path.u8string() + u8R"(" )" +
                windower::core::instance().settings.command_line_args;
            auto args = windower::to_wstring(u8_args);

            return hooks::CreateProcessW(
                path.c_str(), args.data(), nullptr, nullptr, false, 0, nullptr,
                nullptr, &startup, lpProcessInformation);
        }
    }
    return hooks::CreateProcessW(
        lpApplicationName, lpCommandLine, lpProcessAttributes,
        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

::LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(
    ::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept
{
    return std::bit_cast<::LPTOP_LEVEL_EXCEPTION_FILTER>(
        ::InterlockedExchangePointer(
            std::bit_cast<void**>(&unhandled_exception_filter),
            static_cast<void*>(lpTopLevelExceptionFilter)));
}

}

}

::UINT windower::kernel32::GetACP() noexcept
{
    if (hooks::GetACP)
    {
        return hooks::GetACP();
    }
    return ::GetACP();
}

::HANDLE windower::kernel32::CreateMutexA(
    ::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner,
    ::LPCSTR lpName) noexcept
{
    if (hooks::CreateMutexA)
    {
        return hooks::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
    }
    return ::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
}

::HANDLE windower::kernel32::CreateMutexW(
    ::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner,
    ::LPCWSTR lpName) noexcept
{
    if (hooks::CreateMutexW)
    {
        return hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
    }
    return ::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
}

::HANDLE windower::kernel32::OpenMutexA(
    ::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
{
    if (hooks::OpenMutexA)
    {
        return hooks::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName);
    }
    return ::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName);
}

::HANDLE windower::kernel32::OpenMutexW(
    ::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
{
    if (hooks::OpenMutexW)
    {
        return hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
    }
    return ::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
}

::DWORD windower::kernel32::GetPriorityClass(::HANDLE hProcess) noexcept
{
    if (hooks::GetPriorityClass)
    {
        return hooks::GetPriorityClass(hProcess);
    }
    return ::GetPriorityClass(hProcess);
}

::BOOL windower::kernel32::SetPriorityClass(
    ::HANDLE hProcess, ::DWORD dwPriorityClass) noexcept
{
    if (hooks::SetPriorityClass)
    {
        return hooks::SetPriorityClass(hProcess, dwPriorityClass);
    }
    return ::SetPriorityClass(hProcess, dwPriorityClass);
}

::BOOL windower::kernel32::CreateProcessA(
    ::LPCSTR lpApplicationName, ::LPSTR lpCommandLine,
    ::LPSECURITY_ATTRIBUTES lpProcessAttributes,
    ::LPSECURITY_ATTRIBUTES lpThreadAttributes, ::BOOL bInheritHandles,
    ::DWORD dwCreationFlags, ::LPVOID lpEnvironment,
    ::LPCSTR lpCurrentDirectory, ::LPSTARTUPINFOA lpStartupInfo,
    ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    return ::CreateProcessA(
        lpApplicationName, lpCommandLine, lpProcessAttributes,
        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

::BOOL windower::kernel32::CreateProcessW(
    ::LPCWSTR lpApplicationName, ::LPWSTR lpCommandLine,
    ::LPSECURITY_ATTRIBUTES lpProcessAttributes,
    ::LPSECURITY_ATTRIBUTES lpThreadAttributes, ::BOOL bInheritHandles,
    ::DWORD dwCreationFlags, ::LPVOID lpEnvironment,
    ::LPCWSTR lpCurrentDirectory, ::LPSTARTUPINFOW lpStartupInfo,
    ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    if (hooks::CreateProcessW)
    {
        return hooks::CreateProcessW(
            lpApplicationName, lpCommandLine, lpProcessAttributes,
            lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
            lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    return ::CreateProcessW(
        lpApplicationName, lpCommandLine, lpProcessAttributes,
        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

::LPTOP_LEVEL_EXCEPTION_FILTER windower::kernel32::SetUnhandledExceptionFilter(
    ::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept
{
    if (hooks::SetUnhandledExceptionFilter)
    {
        return hooks::SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
    }
    return ::SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
}

void windower::kernel32::install()
{
    if (!hooks::GetACP)
    {
        priority = ::GetPriorityClass(::GetCurrentProcess());

        hooks::GetACP =
            hooklib::make_hook(u8"kernel32.dll", u8"GetACP", callbacks::GetACP);
        hooks::LoadLibraryA = hooklib::make_hook(
            u8"kernel32.dll", u8"LoadLibraryA", callbacks::LoadLibraryA);
        hooks::LoadLibraryW = hooklib::make_hook(
            u8"kernel32.dll", u8"LoadLibraryW", callbacks::LoadLibraryW);
        hooks::CreateMutexA = hooklib::make_hook(
            u8"kernel32.dll", u8"CreateMutexA", callbacks::CreateMutexA);
        hooks::CreateMutexW = hooklib::make_hook(
            u8"kernel32.dll", u8"CreateMutexW", callbacks::CreateMutexW);
        hooks::OpenMutexA = hooklib::make_hook(
            u8"kernel32.dll", u8"OpenMutexA", callbacks::OpenMutexA);
        hooks::OpenMutexW = hooklib::make_hook(
            u8"kernel32.dll", u8"OpenMutexW", callbacks::OpenMutexW);
        hooks::GetPriorityClass = hooklib::make_hook(
            u8"kernel32.dll", u8"GetPriorityClass",
            callbacks::GetPriorityClass);
        hooks::SetPriorityClass = hooklib::make_hook(
            u8"kernel32.dll", u8"SetPriorityClass",
            callbacks::SetPriorityClass);
        hooks::SetUnhandledExceptionFilter = hooklib::make_hook(
            u8"kernel32.dll", u8"SetUnhandledExceptionFilter",
            callbacks::SetUnhandledExceptionFilter);

        // This hook causes problems when child process debugging is used.
        // It's only used to re-inject after a PlayOnline Viewer update.
        if (!::IsDebuggerPresent())
        {
            hooks::CreateProcessW = hooklib::make_hook(
                u8"kernel32.dll", u8"CreateProcessW",
                callbacks::CreateProcessW);
        }
    }
}

void windower::kernel32::uninstall() noexcept
{
    hooks::GetACP                      = {};
    hooks::CreateMutexA                = {};
    hooks::CreateMutexW                = {};
    hooks::OpenMutexA                  = {};
    hooks::OpenMutexW                  = {};
    hooks::GetPriorityClass            = {};
    hooks::SetPriorityClass            = {};
    hooks::CreateProcessW              = {};
    hooks::SetUnhandledExceptionFilter = {};
}
