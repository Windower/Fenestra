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

#include "wrappers/direct_draw.hpp"

#include "wrappers/direct_draw_surface.hpp"

#include <windows.h>

#include <ddraw.h>
#include <winrt/base.h>

#include <memory>

namespace
{

::GUID constexpr IID_IDirect3D7 = {
    0xF5049E77,
    0x4861,
    0x11D2,
    {0xA4, 0x07, 0x00, 0xA0, 0xC9, 0x06, 0x29, 0xA8}};
}

windower::direct_draw::direct_draw(::IDirectDraw7* impl) noexcept : m_impl{impl}
{
    AddRef();
}

windower::direct_draw::~direct_draw()
{
    m_impl->Release();
    m_impl = nullptr;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::QueryInterface(IID const& riid, void** ppvObj) noexcept
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
    if (::IsEqualGUID(riid, ::IID_IDirectDraw7))
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    if (::IsEqualGUID(riid, IID_IDirect3D7))
    {
        return m_impl->QueryInterface(riid, ppvObj);
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_draw::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_draw::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::Compact() noexcept
{
    return m_impl->Compact();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::CreateClipper(
    ::DWORD dwFlags, ::IDirectDrawClipper** lplpDDClipper,
    ::IUnknown* pUnkOuter) noexcept
{
    return m_impl->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::CreatePalette(
    ::DWORD dwFlags, ::PALETTEENTRY* lpDDColorArray,
    ::IDirectDrawPalette** lplpDDPalette, ::IUnknown* pUnkOuter) noexcept
{
    return m_impl->CreatePalette(
        dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::CreateSurface(
    ::DDSURFACEDESC2* lpDDSurfaceDesc2, ::IDirectDrawSurface7** lplpDDSurface,
    ::IUnknown* pUnkOuter) noexcept
{
    if (!lpDDSurfaceDesc2 || !lplpDDSurface)
    {
        return E_POINTER;
    }
    *lplpDDSurface = nullptr;

    winrt::com_ptr<::IDirectDrawSurface7> ptr;
    auto const result =
        m_impl->CreateSurface(lpDDSurfaceDesc2, ptr.put(), pUnkOuter);
    if (SUCCEEDED(result))
    {
        if (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        {
            *lplpDDSurface =
                std::make_unique<direct_draw_surface>(ptr.detach(), this)
                    .release();
        }
        else
        {
            *lplpDDSurface = ptr.detach();
        }
    }
    return result;
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::DuplicateSurface(
    ::IDirectDrawSurface7* lpDDSurface,
    ::IDirectDrawSurface7** lplpDupDDSurface) noexcept
{
    if (auto const surface = dynamic_cast<direct_draw_surface*>(lpDDSurface))
    {
        if (!lplpDupDDSurface)
        {
            return E_POINTER;
        }
        *lplpDupDDSurface = nullptr;

        winrt::com_ptr<::IDirectDrawSurface7> ptr;
        auto const result =
            m_impl->DuplicateSurface(surface->m_impl, ptr.put());
        if (SUCCEEDED(result))
        {
            *lplpDupDDSurface =
                std::make_unique<direct_draw_surface>(ptr.detach(), this)
                    .release();
        }
        return result;
    }

    return m_impl->DuplicateSurface(lpDDSurface, lplpDupDDSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::EnumDisplayModes(
    ::DWORD dwFlags, ::DDSURFACEDESC2* lpDDSurfaceDesc2, void* lpContext,
    ::LPDDENUMMODESCALLBACK2 lpEnumModesCallback) noexcept
{
    return m_impl->EnumDisplayModes(
        dwFlags, lpDDSurfaceDesc2, lpContext, lpEnumModesCallback);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::EnumSurfaces(
    ::DWORD dwFlags, ::DDSURFACEDESC2* lpDDSD2, void* lpContext,
    ::LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback) noexcept
{
    return m_impl->EnumSurfaces(
        dwFlags, lpDDSD2, lpContext, lpEnumSurfacesCallback);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::FlipToGDISurface() noexcept
{
    return m_impl->FlipToGDISurface();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetCaps(
    ::DDCAPS* lpDDDriverCaps, ::DDCAPS* lpDDHELCaps) noexcept
{
    return m_impl->GetCaps(lpDDDriverCaps, lpDDHELCaps);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetDisplayMode(
    ::DDSURFACEDESC2* lpDDSurfaceDesc2) noexcept
{
    return m_impl->GetDisplayMode(lpDDSurfaceDesc2);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetFourCCCodes(
    ::DWORD* lpNumCodes, ::DWORD* lpCodes) noexcept
{
    return m_impl->GetFourCCCodes(lpNumCodes, lpCodes);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetGDISurface(
    ::IDirectDrawSurface7** lplpGDIDDSSurface) noexcept
{
    return m_impl->GetGDISurface(lplpGDIDDSSurface);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::GetMonitorFrequency(::DWORD* lpdwFrequency) noexcept
{
    return m_impl->GetMonitorFrequency(lpdwFrequency);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::GetScanLine(::DWORD* lpdwScanLine) noexcept
{
    return m_impl->GetScanLine(lpdwScanLine);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::GetVerticalBlankStatus(::BOOL* lpbIsInVB) noexcept
{
    return m_impl->GetVerticalBlankStatus(lpbIsInVB);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::Initialize(::GUID* lpGUID) noexcept
{
    return m_impl->Initialize(lpGUID);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::RestoreDisplayMode() noexcept
{
    return m_impl->RestoreDisplayMode();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::SetCooperativeLevel(
    ::HWND hWnd, ::DWORD dwFlags) noexcept
{
    return m_impl->SetCooperativeLevel(hWnd, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::SetDisplayMode(
    ::DWORD dwWidth, ::DWORD dwHeight, ::DWORD dwBPP, ::DWORD dwRefreshRate,
    ::DWORD dwFlags) noexcept
{
    return m_impl->SetDisplayMode(
        dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::WaitForVerticalBlank(
    ::DWORD dwFlags, ::HANDLE hEvent) noexcept
{
    return m_impl->WaitForVerticalBlank(dwFlags, hEvent);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetAvailableVidMem(
    ::DDSCAPS2* lpDDSCaps2, ::DWORD* lpdwTotal, ::DWORD* lpdwFree) noexcept
{
    return m_impl->GetAvailableVidMem(lpDDSCaps2, lpdwTotal, lpdwFree);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetSurfaceFromDC(
    ::HDC hdc, ::IDirectDrawSurface7** lpDDS) noexcept
{
    return m_impl->GetSurfaceFromDC(hdc, lpDDS);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::RestoreAllSurfaces() noexcept
{
    return m_impl->RestoreAllSurfaces();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw::TestCooperativeLevel() noexcept
{
    return m_impl->TestCooperativeLevel();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::GetDeviceIdentifier(
    ::DDDEVICEIDENTIFIER2* lpdddi, ::DWORD dwFlags) noexcept
{
    return m_impl->GetDeviceIdentifier(lpdddi, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::StartModeTest(
    ::SIZE* lpModesToTest, ::DWORD dwNumEntries, ::DWORD dwFlags) noexcept
{
    return m_impl->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw::EvaluateMode(
    ::DWORD dwFlags, ::DWORD* pSecondsUntilTimeout) noexcept
{
    return m_impl->EvaluateMode(dwFlags, pSecondsUntilTimeout);
}
