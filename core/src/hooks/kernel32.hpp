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

#ifndef WINDOWER_HOOKS_KERNEL32_HPP
#define WINDOWER_HOOKS_KERNEL32_HPP

#include <windows.h>

namespace windower::kernel32
{

::UINT GetACP() noexcept;
::HMODULE LoadLibraryA(::LPCSTR);
::HMODULE LoadLibraryW(::LPCWSTR);
::HANDLE CreateMutexA(::LPSECURITY_ATTRIBUTES, ::BOOL, ::LPCSTR) noexcept;
::HANDLE CreateMutexW(::LPSECURITY_ATTRIBUTES, ::BOOL, ::LPCWSTR) noexcept;
::HANDLE OpenMutexA(::DWORD, ::BOOL, ::LPCSTR) noexcept;
::HANDLE OpenMutexW(::DWORD, ::BOOL, ::LPCWSTR) noexcept;
::DWORD GetPriorityClass(::HANDLE) noexcept;
::BOOL SetPriorityClass(::HANDLE, ::DWORD) noexcept;
::BOOL CreateProcessA(
    ::LPCSTR, ::LPSTR, ::LPSECURITY_ATTRIBUTES, ::LPSECURITY_ATTRIBUTES, ::BOOL,
    ::DWORD, ::PVOID, ::LPCSTR, ::LPSTARTUPINFOA,
    ::LPPROCESS_INFORMATION) noexcept;
::BOOL CreateProcessW(
    ::LPCWSTR, ::LPWSTR, ::LPSECURITY_ATTRIBUTES, ::LPSECURITY_ATTRIBUTES,
    ::BOOL, ::DWORD, ::PVOID, ::LPCWSTR, ::LPSTARTUPINFOW,
    ::LPPROCESS_INFORMATION) noexcept;
::LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(
    ::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept;

void install();
void uninstall() noexcept;

}

#endif
