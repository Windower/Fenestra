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

#ifndef WINDOWER_HOOKS_ADVAPI32_HPP
#define WINDOWER_HOOKS_ADVAPI32_HPP

#include <windows.h>

namespace windower::advapi32
{

::LSTATUS RegEnumValueA(
    ::HKEY, ::DWORD, ::LPSTR, ::LPDWORD, ::LPDWORD, ::LPDWORD, ::LPBYTE,
    ::LPDWORD) noexcept;
::LSTATUS RegEnumValueW(
    ::HKEY, ::DWORD, ::LPWSTR, ::LPDWORD, ::LPDWORD, ::LPDWORD, ::LPBYTE,
    ::LPDWORD) noexcept;
::LSTATUS RegGetValueA(
    ::HKEY, ::LPCSTR, ::LPCSTR, ::DWORD, ::LPDWORD, ::PVOID,
    ::LPDWORD) noexcept;
::LSTATUS RegGetValueW(
    ::HKEY, ::LPCWSTR, ::LPCWSTR, ::DWORD, ::LPDWORD, ::PVOID,
    ::LPDWORD) noexcept;
::LSTATUS RegQueryValueExA(
    ::HKEY, ::LPCSTR, ::LPDWORD, ::LPDWORD, ::LPBYTE, ::LPDWORD) noexcept;
::LSTATUS RegQueryValueExW(
    ::HKEY, ::LPCWSTR, ::LPDWORD, ::LPDWORD, ::LPBYTE, ::LPDWORD) noexcept;

void install();
void uninstall() noexcept;

}

#endif
