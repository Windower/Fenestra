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

#include "wrappers/direct_input.hpp"

#include "wrappers/direct_input_keyboard.hpp"

#include <windows.h>

#include <dinput.h>
#include <winrt/base.h>

#include <memory>

windower::direct_input::direct_input(::IDirectInput8A* impl) noexcept :
    m_impl{impl}
{
    direct_input::AddRef();
}

windower::direct_input::~direct_input()
{
    m_impl->Release();
    m_impl = nullptr;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input::QueryInterface(IID const& riid, void** ppvObj) noexcept
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

::ULONG STDMETHODCALLTYPE windower::direct_input::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_input::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::CreateDevice(
    ::GUID const& rguid, ::IDirectInputDevice8A** lplpDirectInputDevice,
    ::IUnknown* pUnkOuter) noexcept
{
    if (!lplpDirectInputDevice)
    {
        return E_POINTER;
    }
    *lplpDirectInputDevice = nullptr;

    winrt::com_ptr<::IDirectInputDevice8A> ptr;
    auto const result = m_impl->CreateDevice(rguid, ptr.put(), pUnkOuter);
    if (result == S_OK)
    {
        if (::IsEqualGUID(rguid, GUID_SysKeyboard))
        {
            *lplpDirectInputDevice =
                std::make_unique<direct_input_keyboard>(ptr.detach()).release();
        }
    }
    return result;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::EnumDevices(
    ::DWORD dwDevType, ::LPDIENUMDEVICESCALLBACKA lpCallback, void* pvRef,
    ::DWORD dwFlags) noexcept
{
    return m_impl->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_input::GetDeviceStatus(::GUID const& rguidInstance) noexcept
{
    return m_impl->GetDeviceStatus(rguidInstance);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::RunControlPanel(
    ::HWND hwndOwner, ::DWORD dwFlags) noexcept
{
    return m_impl->RunControlPanel(hwndOwner, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::Initialize(
    ::HINSTANCE hinst, ::DWORD dwVersion) noexcept
{
    return m_impl->Initialize(hinst, dwVersion);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::FindDevice(
    ::GUID const& rguidClass, ::CHAR const* ptszName,
    ::GUID* pguidInstance) noexcept
{
    return m_impl->FindDevice(rguidClass, ptszName, pguidInstance);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::EnumDevicesBySemantics(
    ::CHAR const* ptszUserName, ::DIACTIONFORMATA* lpdiActionFormat,
    ::LPDIENUMDEVICESBYSEMANTICSCBA lpCallback, void* pvRef,
    ::DWORD dwFlags) noexcept
{
    return m_impl->EnumDevicesBySemantics(
        ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_input::ConfigureDevices(
    ::LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
    ::DICONFIGUREDEVICESPARAMSA* lpdiCDParams, ::DWORD dwFlags,
    void* pvRefData) noexcept
{
    return m_impl->ConfigureDevices(
        lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}
