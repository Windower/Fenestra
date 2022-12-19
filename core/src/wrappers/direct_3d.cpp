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

#include "wrappers/direct_3d.hpp"

#include "core.hpp"
#include "wrappers/direct_3d_device.hpp"

#include <windows.h>

#include <d3d8.h>
#include <winrt/base.h>

namespace
{

::UINT fix_adapter(gsl::not_null<::IDirect3D8*> d3d8, ::UINT index) noexcept
{
    auto const& core    = windower::core::instance();
    auto const* monitor = ::MonitorFromPoint(
        {core.settings.display_bounds.location.x,
         core.settings.display_bounds.location.y},
        MONITOR_DEFAULTTOPRIMARY);

    auto result = index;
    for (::UINT i = 0; i < d3d8->GetAdapterCount(); ++i)
    {
        auto const* temp = d3d8->GetAdapterMonitor(i);
        if (temp == monitor)
        {
            result = i;
            break;
        }
    }

    return index == 0 ? result : index <= result ? index - 1 : index;
}

}

windower::direct_3d::direct_3d(::IDirect3D8* impl) noexcept : m_impl{impl}
{
    AddRef();
}

windower::direct_3d::~direct_3d()
{
    m_impl->Release();
    m_impl = nullptr;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d::QueryInterface(IID const& riid, void** ppvObj) noexcept
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
    else if (::IsEqualGUID(riid, ::IID_IDirect3D8))
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_3d::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_3d::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d::RegisterSoftwareDevice(void* pInitializeFunction) noexcept
{
    return m_impl->RegisterSoftwareDevice(pInitializeFunction);
}

::UINT STDMETHODCALLTYPE windower::direct_3d::GetAdapterCount() noexcept
{
    return m_impl->GetAdapterCount();
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::GetAdapterIdentifier(
    ::UINT Adapter, ::DWORD Flags,
    ::D3DADAPTER_IDENTIFIER8* pIdentifier) noexcept
{
    return m_impl->GetAdapterIdentifier(
        fix_adapter(m_impl, Adapter), Flags, pIdentifier);
}

::UINT STDMETHODCALLTYPE
windower::direct_3d::GetAdapterModeCount(::UINT Adapter) noexcept
{
    return m_impl->GetAdapterModeCount(fix_adapter(m_impl, Adapter));
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::EnumAdapterModes(
    ::UINT Adapter, ::UINT Mode, ::D3DDISPLAYMODE* pMode) noexcept
{
    return m_impl->EnumAdapterModes(fix_adapter(m_impl, Adapter), Mode, pMode);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::GetAdapterDisplayMode(
    ::UINT Adapter, ::D3DDISPLAYMODE* pMode) noexcept
{
    return m_impl->GetAdapterDisplayMode(fix_adapter(m_impl, Adapter), pMode);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::CheckDeviceType(
    ::UINT Adapter, ::D3DDEVTYPE CheckType, ::D3DFORMAT DisplayFormat,
    ::D3DFORMAT BackBufferFormat, ::BOOL Windowed) noexcept
{
    return m_impl->CheckDeviceType(
        fix_adapter(m_impl, Adapter), CheckType, DisplayFormat,
        BackBufferFormat, Windowed);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::CheckDeviceFormat(
    ::UINT Adapter, ::D3DDEVTYPE DeviceType, ::D3DFORMAT AdapterFormat,
    ::DWORD Usage, ::D3DRESOURCETYPE RType, ::D3DFORMAT CheckFormat) noexcept
{
    return m_impl->CheckDeviceFormat(
        fix_adapter(m_impl, Adapter), DeviceType, AdapterFormat, Usage, RType,
        CheckFormat);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::CheckDeviceMultiSampleType(
    ::UINT Adapter, ::D3DDEVTYPE DeviceType, ::D3DFORMAT SurfaceFormat,
    ::BOOL Windowed, ::D3DMULTISAMPLE_TYPE MultiSampleType) noexcept
{
    return m_impl->CheckDeviceMultiSampleType(
        fix_adapter(m_impl, Adapter), DeviceType, SurfaceFormat, Windowed,
        MultiSampleType);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::CheckDepthStencilMatch(
    ::UINT Adapter, ::D3DDEVTYPE DeviceType, ::D3DFORMAT AdapterFormat,
    ::D3DFORMAT RenderTargetFormat, ::D3DFORMAT DepthStencilFormat) noexcept
{
    return m_impl->CheckDepthStencilMatch(
        fix_adapter(m_impl, Adapter), DeviceType, AdapterFormat,
        RenderTargetFormat, DepthStencilFormat);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::GetDeviceCaps(
    ::UINT Adapter, ::D3DDEVTYPE DeviceType, ::D3DCAPS8* pCaps) noexcept
{
    return m_impl->GetDeviceCaps(
        fix_adapter(m_impl, Adapter), DeviceType, pCaps);
}

::HMONITOR STDMETHODCALLTYPE
windower::direct_3d::GetAdapterMonitor(::UINT Adapter) noexcept
{
    return m_impl->GetAdapterMonitor(fix_adapter(m_impl, Adapter));
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d::CreateDevice(
    ::UINT Adapter, ::D3DDEVTYPE DeviceType, ::HWND hFocusWindow,
    ::DWORD BehaviorFlags, ::D3DPRESENT_PARAMETERS* pPresentationParameters,
    ::IDirect3DDevice8** ppReturnedDeviceInterface) noexcept
{
    if (!ppReturnedDeviceInterface)
    {
        return E_POINTER;
    }
    *ppReturnedDeviceInterface = nullptr;

    winrt::com_ptr<::IDirect3DDevice8> ptr;
    auto const result = m_impl->CreateDevice(
        fix_adapter(m_impl, Adapter), DeviceType, hFocusWindow,
        BehaviorFlags | D3DCREATE_FPU_PRESERVE, pPresentationParameters,
        ptr.put());
    if (SUCCEEDED(result))
    {
        *ppReturnedDeviceInterface =
            std::make_unique<direct_3d_device>(ptr.detach(), hFocusWindow, this)
                .release();
    }
    return result;
}
