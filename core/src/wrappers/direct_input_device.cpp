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

#include "wrappers/direct_input_device.hpp"

#include <windows.h>
#include <dinput.h>

windower::direct_input_device::direct_input_device(
    ::IDirectInputDevice8A* impl) noexcept :
    m_impl{impl}
{
    direct_input_device::AddRef();
}

windower::direct_input_device::~direct_input_device()
{
    m_impl->Release();
    m_impl = nullptr;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::QueryInterface(
    ::IID const& riid, void** ppvObj) noexcept
{
    if (!ppvObj)
    {
        return E_POINTER;
    }
    *ppvObj = nullptr;

    if (::IsEqualGUID(riid, ::IID_IUnknown))
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    if (::IsEqualGUID(riid, ::IID_IDirectInput8))
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_input_device::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_input_device::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetCapabilities(
    ::DIDEVCAPS* lpDIDevCaps) noexcept
{
    return m_impl->GetCapabilities(lpDIDevCaps);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::EnumObjects(
    ::LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, void* pvRef,
    ::DWORD dwFlags) noexcept
{
    return m_impl->EnumObjects(lpCallback, pvRef, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetProperty(
    ::GUID const& rguidProp, ::DIPROPHEADER* pdiph) noexcept
{
    return m_impl->GetProperty(rguidProp, pdiph);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::SetProperty(
    ::GUID const& rguidProp, ::DIPROPHEADER const* pdiph) noexcept
{
    return m_impl->SetProperty(rguidProp, pdiph);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::Acquire() noexcept
{
    return m_impl->Acquire();
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::Unacquire() noexcept
{
    return m_impl->Unacquire();
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetDeviceState(
    ::DWORD cbData, void* lpvData) noexcept
{
    return m_impl->GetDeviceState(cbData, lpvData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetDeviceData(
    ::DWORD cbObjectData, ::DIDEVICEOBJECTDATA* rgdod, ::DWORD* pdwInOut,
    ::DWORD dwFlags) noexcept
{
    return m_impl->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::SetDataFormat(
    ::DIDATAFORMAT const* lpdf) noexcept
{
    return m_impl->SetDataFormat(lpdf);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input_device::SetEventNotification(::HANDLE hEvent) noexcept
{
    return m_impl->SetEventNotification(hEvent);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::SetCooperativeLevel(
    ::HWND hwnd, ::DWORD dwFlags) noexcept
{
    return m_impl->SetCooperativeLevel(hwnd, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetObjectInfo(
    ::DIDEVICEOBJECTINSTANCEA* pdidoi, ::DWORD dwObj, ::DWORD dwHow) noexcept
{
    return m_impl->GetObjectInfo(pdidoi, dwObj, dwHow);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetDeviceInfo(
    ::DIDEVICEINSTANCEA* pdidi) noexcept
{
    return m_impl->GetDeviceInfo(pdidi);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::RunControlPanel(
    ::HWND hwndOwner, ::DWORD dwFlags) noexcept
{
    return m_impl->RunControlPanel(hwndOwner, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::Initialize(
    ::HINSTANCE hinst, ::DWORD dwVersion, ::GUID const& rguid) noexcept
{
    return m_impl->Initialize(hinst, dwVersion, rguid);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::CreateEffect(
    ::GUID const& rguid, ::DIEFFECT const* lpeff, ::IDirectInputEffect** ppdeff,
    ::LPUNKNOWN punkOuter) noexcept
{
    return m_impl->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::EnumEffects(
    ::LPDIENUMEFFECTSCALLBACKA lpCallback, void* pvRef,
    ::DWORD dwEffType) noexcept
{
    return m_impl->EnumEffects(lpCallback, pvRef, dwEffType);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetEffectInfo(
    ::DIEFFECTINFOA* pdei, ::GUID const& rguid) noexcept
{
    return m_impl->GetEffectInfo(pdei, rguid);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input_device::GetForceFeedbackState(::DWORD* pdwOut) noexcept
{
    return m_impl->GetForceFeedbackState(pdwOut);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input_device::SendForceFeedbackCommand(
    ::DWORD dwFlags) noexcept
{
    return m_impl->SendForceFeedbackCommand(dwFlags);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input_device::EnumCreatedEffectObjects(
    ::LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, void* pvRef,
    ::DWORD fl) noexcept
{
    return m_impl->EnumCreatedEffectObjects(lpCallback, pvRef, fl);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input_device::Escape(::DIEFFESCAPE* pesc) noexcept
{
    return m_impl->Escape(pesc);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::Poll() noexcept
{
    return m_impl->Poll();
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::SendDeviceData(
    ::DWORD cbObjectData, ::DIDEVICEOBJECTDATA const* rgdod, ::DWORD* pdwInOut,
    ::DWORD fl) noexcept
{
    return m_impl->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::EnumEffectsInFile(
    ::CHAR const* lpszFileName, ::LPDIENUMEFFECTSINFILECALLBACK pec,
    void* pvRef, ::DWORD dwFlags) noexcept
{
    return m_impl->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::WriteEffectToFile(
    ::CHAR const* lpszFileName, ::DWORD dwEntries, ::DIFILEEFFECT* rgDiFileEft,
    ::DWORD dwFlags) noexcept
{
    return m_impl->WriteEffectToFile(
        lpszFileName, dwEntries, rgDiFileEft, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::BuildActionMap(
    ::DIACTIONFORMATA* lpdiaf, ::CHAR const* lpszUserName,
    ::DWORD dwFlags) noexcept
{
    return m_impl->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::SetActionMap(
    ::DIACTIONFORMATA* lpdiActionFormat, ::CHAR const* lptszUserName,
    ::DWORD dwFlags) noexcept
{
    return m_impl->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_device::GetImageInfo(
    ::DIDEVICEIMAGEINFOHEADERA* lpdiDevImageInfoHeader) noexcept
{
    return m_impl->GetImageInfo(lpdiDevImageInfoHeader);
}
