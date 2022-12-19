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

#ifndef WINDOWER_WRAPPERS_DIRECT_INPUT_HPP
#define WINDOWER_WRAPPERS_DIRECT_INPUT_HPP

#include <windows.h>
#include <dinput.h>

namespace windower
{

class direct_input final : public ::IDirectInput8A
{
public:
    direct_input() = delete;
    direct_input(::IDirectInput8A*) noexcept;
    direct_input(direct_input const&) = delete;
    direct_input(direct_input&&) = delete;
    ~direct_input();

    direct_input& operator=(direct_input const&) = delete;
    direct_input& operator=(direct_input&&) = delete;

    // IUnknown methods
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDirectInput8A methods
    ::HRESULT STDMETHODCALLTYPE CreateDevice(
        ::GUID const&, ::IDirectInputDevice8A**, ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumDevices(
        ::DWORD, ::LPDIENUMDEVICESCALLBACKA, ::LPVOID,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceStatus(::GUID const&) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE RunControlPanel(::HWND, ::DWORD) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE Initialize(::HINSTANCE, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    FindDevice(::GUID const&, ::CHAR const*, ::GUID*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(
        ::CHAR const*, ::DIACTIONFORMATA*, ::LPDIENUMDEVICESBYSEMANTICSCBA,
        void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ConfigureDevices(
        ::LPDICONFIGUREDEVICESCALLBACK, ::DICONFIGUREDEVICESPARAMSA*, ::DWORD,
        void*) noexcept override;

private:
    ::IDirectInput8A* m_impl;
    ::ULONG m_count = 0;
};

}

#endif
