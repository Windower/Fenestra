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

#ifndef WINDOWER_WRAPPERS_DIRECT_INPUT_DEVICE_HPP
#define WINDOWER_WRAPPERS_DIRECT_INPUT_DEVICE_HPP

#include <windows.h>
#include <dinput.h>

namespace windower
{

class direct_input_device : public ::IDirectInputDevice8A
{
public:
    direct_input_device() = delete;
    direct_input_device(::IDirectInputDevice8A*) noexcept;
    direct_input_device(direct_input_device const&) = delete;
    direct_input_device(direct_input_device&&) = delete;
    virtual ~direct_input_device() = 0;

    direct_input_device& operator=(direct_input_device const&) = delete;
    direct_input_device& operator=(direct_input_device&&) = delete;

    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    ::HRESULT STDMETHODCALLTYPE GetCapabilities(::DIDEVCAPS*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumObjects(
        ::LPDIENUMDEVICEOBJECTSCALLBACKA, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetProperty(::GUID const&, ::DIPROPHEADER*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetProperty(::GUID const&, ::DIPROPHEADER const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Acquire() noexcept override;
    ::HRESULT STDMETHODCALLTYPE Unacquire() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceState(::DWORD, void*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDeviceData(
        ::DWORD, ::DIDEVICEOBJECTDATA*, ::DWORD*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetDataFormat(::DIDATAFORMAT const*) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE SetEventNotification(::HANDLE) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        SetCooperativeLevel(::HWND, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetObjectInfo(
        ::DIDEVICEOBJECTINSTANCEA*, ::DWORD, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceInfo(::DIDEVICEINSTANCEA*) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE RunControlPanel(::HWND, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Initialize(::HINSTANCE, ::DWORD, ::GUID const&) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateEffect(
        ::GUID const&, ::DIEFFECT const*, ::IDirectInputEffect**,
        ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    EnumEffects(::LPDIENUMEFFECTSCALLBACKA, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetEffectInfo(::DIEFFECTINFOA*, ::GUID const&) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetForceFeedbackState(::DWORD*) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE SendForceFeedbackCommand(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(
        ::LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, void*,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Escape(::DIEFFESCAPE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Poll() noexcept override;
    ::HRESULT STDMETHODCALLTYPE SendDeviceData(
        ::DWORD, ::DIDEVICEOBJECTDATA const*, ::DWORD*,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumEffectsInFile(
        ::CHAR const*, ::LPDIENUMEFFECTSINFILECALLBACK, void*,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE WriteEffectToFile(
        ::CHAR const*, ::DWORD, ::DIFILEEFFECT*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE BuildActionMap(
        ::DIACTIONFORMATA*, ::CHAR const*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetActionMap(::DIACTIONFORMATA*, ::CHAR const*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetImageInfo(::DIDEVICEIMAGEINFOHEADERA*) noexcept override;

private:
    ::IDirectInputDevice8A* m_impl;
    ::ULONG m_count = 0;
};

}

#endif
