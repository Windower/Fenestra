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

#include "hooks/advapi32.hpp"

#include "core.hpp"
#include "hooklib/hook.hpp"
#include "utility.hpp"

#include <windows.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cwchar>
#include <string>
#include <vector>

namespace
{

using NTSTATUS = ::LONG;
using NtQueryKeyFunc =
    NTSTATUS(WINAPI*)(::HANDLE, ::DWORD, ::PVOID, ::ULONG, ::PULONG);

struct KEY_NAME_INFORMATION
{
    ::ULONG NameLength;
    ::WCHAR Name[512];
};

std::array<wchar_t const*, 6> const ffxi_keys = {
    LR"(\REGISTRY\MACHINE\SOFTWARE\Wow6432Node\PlayOnlineUS\SquareEnix\FinalFantasyXI)",
    LR"(\REGISTRY\MACHINE\SOFTWARE\Wow6432Node\PlayOnline\Square\FinalFantasyXI)",
    LR"(\REGISTRY\MACHINE\SOFTWARE\Wow6432Node\PlayOnlineEU\SquareEnix\FinalFantasyXI)",

    LR"(\REGISTRY\MACHINE\SOFTWARE\PlayOnlineUS\SquareEnix\FinalFantasyXI)",
    LR"(\REGISTRY\MACHINE\SOFTWARE\PlayOnline\Square\FinalFantasyXI)",
    LR"(\REGISTRY\MACHINE\SOFTWARE\PlayOnlineEU\SquareEnix\FinalFantasyXI)",
};

bool is_ffxi_key(::HKEY key, wchar_t const* sub_key = nullptr) noexcept
{
    static auto const NtQueryKey = std::bit_cast<NtQueryKeyFunc>(
        ::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "NtQueryKey"));

    KEY_NAME_INFORMATION name_info = {};
    ::ULONG size                   = 0;
    if (NtQueryKey(key, 3, &name_info, sizeof name_info, &size) ==
        0 /* STATUS_SUCCESS */)
    {
        std::wstring name{
            static_cast<::WCHAR*>(name_info.Name),
            name_info.NameLength / sizeof(::WCHAR)};
        if (sub_key)
        {
            name += L'\\';
            name += sub_key;
        }

        for (auto test : ffxi_keys)
        {
            if (::CompareStringW(
                    LOCALE_INVARIANT, NORM_IGNORECASE, name.c_str(),
                    name.size(), test, std::wcslen(test)) == CSTR_EQUAL)
            {
                return true;
            }
        }
    }
    return false;
}

template<typename T, std::size_t N>
bool check(T const* value, char const (&test)[N]) noexcept
{
    for (std::size_t i = 0; i < N; ++i)
    {
        if (*std::next(value, i) != gsl::at(test, i))
        {
            return false;
        }
    }
    return true;
}

template<typename T, typename U>
T convert(U value) noexcept
{
    return gsl::narrow_cast<T>(value);
}

template<typename T>
constexpr T convert(float value) noexcept
{
    return *std::bit_cast<T*>(&value);
}

template<typename T>
bool set_value(::LPDWORD type, ::LPVOID data, ::LPDWORD size, T value) noexcept
{
    if (type)
    {
        *type = REG_DWORD;
    }
    if (size)
    {
        *size = sizeof(::DWORD);
    }
    if (data)
    {
        *static_cast<::DWORD*>(data) = convert<::DWORD>(value);
    }
    return true;
}

template<typename T>
bool get_ffxi_setting(
    T value, ::LPDWORD type, ::LPVOID data, ::LPDWORD size) noexcept
{
    if (value)
    {
        if (check(value, "0000"))
        {
            // General > Enable MIP mapping

            // Can only set to 0 or 1 in the official config tool,
            // values greater than 1 increase the number of mip levels
            // used.
            return set_value(
                type, data, size,
                windower::core::instance().settings.mipmapping);
        }
        else if (check(value, "0001"))
        {
            // Screen Size > Overlay Graphics Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.window_bounds.size.width);
        }
        else if (check(value, "0002"))
        {
            // Screen Size > Overlay Graphics Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.window_bounds.size.height);
        }
        else if (check(value, "0003"))
        {
            // Screen Size > Background Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.render_size.width);
        }
        else if (check(value, "0004"))
        {
            // Screen Size > Background Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.render_size.height);
        }
        else if (check(value, "0007"))
        {
            // Sound > Enable Sound
            return set_value(
                type, data, size,
                windower::core::instance().settings.max_sounds > 0);
        }
        else if (check(value, "0011"))
        {
            // Effects > Environmental Animation
            return set_value(
                type, data, size,
                windower::core::instance().settings.environment_animation);
        }
        else if (check(value, "0017"))
        {
            // Effects > Enable bump mapping
            return set_value(
                type, data, size,
                windower::core::instance().settings.bump_mapping);
        }
        else if (check(value, "0018"))
        {
            // Textures > Texture Compression
            return set_value(
                type, data, size,
                windower::core::instance().settings.texture_compression);
        }
        else if (check(value, "0019"))
        {
            // Textures > On-Screen Maps
            return set_value(
                type, data, size,
                windower::core::instance().settings.map_compression);
        }
        // else if (check(value, "0020"))
        //{
        //     // unknown function
        // }
        else if (check(value, "0021"))
        {
            // Misc. > Enable hardware mouse cursor
            return set_value(
                type, data, size,
                windower::core::instance().settings.hardware_mouse);
        }
        else if (check(value, "0022"))
        {
            // Misc. > Show opening movie
            return set_value(
                type, data, size,
                windower::core::instance().settings.play_intro);
        }
        // else if (check(value, "0023"))
        //{
        //     // Misc. > Simplified character creation visuals
        // }
        // else if (check(value, "0024"))
        //{
        //     // unknown function
        // }
        else if (check(value, "0028"))
        {
            // General > Gamma Base

            // This value is an offset from an unspecified base gamma.
            // Because of limitations in DirectX 8, gamma correction
            // can only be correctly applied in full screen mode. In
            // window, and borderless window modes this is emulated by
            // adjusting the color of every vertex. The base gamma used
            // is also different in different modes, in full screen
            // 1.5 is used, and otherwise the system's display gamma
            // setting is used (we assume a system gamma of 2.2 here).
            auto gamma_correction = windower::core::instance().settings.gamma;
            if (windower::core::instance().settings.window_type ==
                windower::window_type::full_screen)
            {
                gamma_correction -= 1.5f;
            }
            else
            {
                gamma_correction -= 2.2f;
            }
            return set_value(type, data, size, gamma_correction);
        }
        else if (check(value, "0029"))
        {
            // Sound > SoundEffectNum
            return set_value(
                type, data, size,
                windower::core::instance().settings.max_sounds);
        }
        // else if (check(value, "0030"))
        //{
        //     // unknown function
        // }
        // else if (check(value, "0031"))
        //{
        //     // unknown function
        //     // default value: 0.006f
        // }
        // else if (check(value, "0032"))
        //{
        //     // unknown function
        // }
        // else if (check(value, "0033"))
        //{
        //     // unknown function
        // }
        else if (check(value, "0034"))
        {
            // General > Unlabled mode combo box

            // 0 = Full Screen
            // 1 = Window
            // 3 = Borderless Window
            switch (windower::core::instance().settings.window_type)
            {
            case windower::window_type::full_screen:
                return set_value(type, data, size, 0);

            case windower::window_type::window:
                return set_value(type, data, size, 1);

            default:
            case windower::window_type::borderless:
                return set_value(type, data, size, 3);
            }
        }
        else if (check(value, "0035"))
        {
            // Sound > Always On
            return set_value(
                type, data, size,
                windower::core::instance().settings.play_sound_when_unfocused);
        }
        else if (check(value, "0036"))
        {
            // Textures > Fonts
            return set_value(
                type, data, size,
                windower::core::instance().settings.font_type);
        }
        else if (check(value, "0037"))
        {
            // Screen Size > Menu Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.ui_size.width);
        }
        else if (check(value, "0038"))
        {
            // Screen Size > Menu Resolution
            return set_value(
                type, data, size,
                windower::core::instance().settings.ui_size.height);
        }
        // else if (check(value, "0039"))
        //{
        //     // unknown function
        //     // default value: 1
        // }
        else if (check(value, "0040"))
        {
            // Misc. > Graphics Stabilization
            return set_value(
                type, data, size,
                windower::core::instance().settings.driver_stability);
        }
        // else if (check(value, "0041"))
        //{
        //     // Misc. > New UI
        // }
        // else if (check(value, "0042"))
        //{
        //     // Misc. 2 > Directory
        // }
        // else if (check(value, "0043"))
        //{
        //     // Misc. 2 > Take a screenshot in your screen's resolution.
        // }
    }
    return false;
}

namespace hooks
{

windower::hooklib::hook<decltype(::RegEnumValueA)> RegEnumValueA;
windower::hooklib::hook<decltype(::RegEnumValueW)> RegEnumValueW;
windower::hooklib::hook<decltype(::RegGetValueA)> RegGetValueA;
windower::hooklib::hook<decltype(::RegGetValueW)> RegGetValueW;
windower::hooklib::hook<decltype(::RegQueryValueExA)> RegQueryValueExA;
windower::hooklib::hook<decltype(::RegQueryValueExW)> RegQueryValueExW;

}

namespace callbacks
{

::LSTATUS WINAPI RegEnumValueA(
    ::HKEY hKey, ::DWORD dwIndex, ::LPSTR lpValueName, ::LPDWORD lpcchValueName,
    ::LPDWORD lpReserved, ::LPDWORD lpType, ::LPBYTE lpData,
    ::LPDWORD lpcbData) noexcept
{
    auto const result = hooks::RegEnumValueA(
        hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData,
        lpcbData);
    if (result == ERROR_SUCCESS && is_ffxi_key(hKey))
    {
        get_ffxi_setting(lpValueName, lpType, lpData, lpcbData);
    }
    return result;
}

::LSTATUS WINAPI RegEnumValueW(
    ::HKEY hKey, ::DWORD dwIndex, ::LPWSTR lpValueName,
    ::LPDWORD lpcchValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    auto const result = hooks::RegEnumValueW(
        hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData,
        lpcbData);
    if (result == ERROR_SUCCESS && is_ffxi_key(hKey))
    {
        get_ffxi_setting(lpValueName, lpType, lpData, lpcbData);
    }
    return result;
}

::LSTATUS WINAPI RegGetValueA(
    ::HKEY hkey, ::LPCSTR lpSubKey, ::LPCSTR lpValue, ::DWORD dwFlags,
    ::LPDWORD pdwType, ::PVOID pvData, ::LPDWORD pcbData) noexcept
{
    auto const size =
        ::MultiByteToWideChar(CP_THREAD_ACP, 0, lpSubKey, -1, nullptr, 0);
    if (size == 0)
    {
        return ::GetLastError();
    }

    std::vector<::WCHAR> buffer(size);
    ::MultiByteToWideChar(
        CP_THREAD_ACP, 0, lpSubKey, -1, buffer.data(), buffer.size());

    if (is_ffxi_key(hkey, buffer.data()))
    {
        if (get_ffxi_setting(lpValue, pdwType, pvData, pcbData))
        {
            return ERROR_SUCCESS;
        }
    }

    return hooks::RegGetValueA(
        hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

::LSTATUS WINAPI RegGetValueW(
    ::HKEY hkey, ::LPCWSTR lpSubKey, ::LPCWSTR lpValue, ::DWORD dwFlags,
    ::LPDWORD pdwType, ::PVOID pvData, ::LPDWORD pcbData) noexcept
{
    if (is_ffxi_key(hkey, lpSubKey))
    {
        if (get_ffxi_setting(lpValue, pdwType, pvData, pcbData))
        {
            return ERROR_SUCCESS;
        }
    }

    return hooks::RegGetValueW(
        hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

::LSTATUS WINAPI RegQueryValueExA(
    ::HKEY hKey, ::LPCSTR lpValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    if (is_ffxi_key(hKey))
    {
        if (get_ffxi_setting(lpValueName, lpType, lpData, lpcbData))
        {
            return ERROR_SUCCESS;
        }
    }

    return hooks::RegQueryValueExA(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

::LSTATUS WINAPI RegQueryValueExW(
    ::HKEY hKey, ::LPCWSTR lpValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    if (is_ffxi_key(hKey))
    {
        if (get_ffxi_setting(lpValueName, lpType, lpData, lpcbData))
        {
            return ERROR_SUCCESS;
        }
    }

    return hooks::RegQueryValueExW(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}
}

}

::LSTATUS windower::advapi32::RegEnumValueA(
    ::HKEY hKey, ::DWORD dwIndex, ::LPSTR lpValueName, ::LPDWORD lpcchValueName,
    ::LPDWORD lpReserved, ::LPDWORD lpType, ::LPBYTE lpData,
    ::LPDWORD lpcbData) noexcept
{
    if (hooks::RegEnumValueA)
    {
        return hooks::RegEnumValueA(
            hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType,
            lpData, lpcbData);
    }
    return ::RegEnumValueA(
        hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData,
        lpcbData);
}

::LSTATUS windower::advapi32::RegEnumValueW(
    ::HKEY hKey, ::DWORD dwIndex, ::LPWSTR lpValueName,
    ::LPDWORD lpcchValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    if (hooks::RegEnumValueW)
    {
        return hooks::RegEnumValueW(
            hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType,
            lpData, lpcbData);
    }
    return ::RegEnumValueW(
        hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData,
        lpcbData);
}

::LSTATUS windower::advapi32::RegGetValueA(
    ::HKEY hkey, ::LPCSTR lpSubKey, ::LPCSTR lpValue, ::DWORD dwFlags,
    ::LPDWORD pdwType, ::PVOID pvData, ::LPDWORD pcbData) noexcept
{
    if (hooks::RegGetValueA)
    {
        return hooks::RegGetValueA(
            hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }
    return ::RegGetValueA(
        hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

::LSTATUS windower::advapi32::RegGetValueW(
    ::HKEY hkey, ::LPCWSTR lpSubKey, ::LPCWSTR lpValue, ::DWORD dwFlags,
    ::LPDWORD pdwType, ::PVOID pvData, ::LPDWORD pcbData) noexcept
{
    if (hooks::RegGetValueW)
    {
        return hooks::RegGetValueW(
            hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }
    return ::RegGetValueW(
        hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

::LSTATUS windower::advapi32::RegQueryValueExA(
    ::HKEY hKey, ::LPCSTR lpValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    if (hooks::RegQueryValueExA)
    {
        return hooks::RegQueryValueExA(
            hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    }
    return ::RegQueryValueExA(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

::LSTATUS windower::advapi32::RegQueryValueExW(
    ::HKEY hKey, ::LPCWSTR lpValueName, ::LPDWORD lpReserved, ::LPDWORD lpType,
    ::LPBYTE lpData, ::LPDWORD lpcbData) noexcept
{
    if (hooks::RegQueryValueExW)
    {
        return hooks::RegQueryValueExW(
            hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    }
    return ::RegQueryValueExW(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

void windower::advapi32::install()
{
    if (!hooks::RegEnumValueA)
    {
        hooks::RegEnumValueA = hooklib::make_hook(
            u8"advapi32.dll", u8"RegEnumValueA", callbacks::RegEnumValueA);
        hooks::RegEnumValueW = hooklib::make_hook(
            u8"advapi32.dll", u8"RegEnumValueW", callbacks::RegEnumValueW);
        hooks::RegGetValueA = hooklib::make_hook(
            u8"advapi32.dll", u8"RegGetValueA", callbacks::RegGetValueA);
        hooks::RegGetValueW = windower::hooklib::make_hook(
            u8"advapi32.dll", u8"RegGetValueW", callbacks::RegGetValueW);
        hooks::RegQueryValueExA = hooklib::make_hook(
            u8"advapi32.dll", u8"RegQueryValueExA",
            callbacks::RegQueryValueExA);
        hooks::RegQueryValueExW = hooklib::make_hook(
            u8"advapi32.dll", u8"RegQueryValueExW",
            callbacks::RegQueryValueExW);
    }
}

void windower::advapi32::uninstall() noexcept
{
    hooks::RegEnumValueA    = {};
    hooks::RegEnumValueW    = {};
    hooks::RegGetValueA     = {};
    hooks::RegGetValueW     = {};
    hooks::RegQueryValueExA = {};
    hooks::RegQueryValueExW = {};
}
