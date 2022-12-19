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

#include "wrappers/direct_draw_surface.hpp"

#include "core.hpp"

#include <windows.h>

#include <ddraw.h>

windower::direct_draw_surface::direct_draw_surface(
    ::IDirectDrawSurface7* impl, direct_draw* parent) noexcept :
    m_impl{impl},
    m_parent{parent}
{
    m_parent->AddRef();

    AddRef();
}

windower::direct_draw_surface::~direct_draw_surface()
{
    m_impl->Release();
    m_impl = nullptr;

    m_parent->Release();
    m_parent = nullptr;
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::QueryInterface(
    IID const& riid, void** ppvObj) noexcept
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
    if (::IsEqualGUID(riid, ::IID_IDirectDrawSurface7))
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_draw_surface::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_draw_surface::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::AddAttachedSurface(
    ::IDirectDrawSurface7* lpDDSurface) noexcept
{
    return m_impl->AddAttachedSurface(lpDDSurface);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::AddOverlayDirtyRect(::RECT* lpRect) noexcept
{
    return m_impl->AddOverlayDirtyRect(lpRect);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::Blt(
    ::RECT* lpDestRect, ::IDirectDrawSurface7* lpDDSrcSurface,
    ::RECT* lpSrcRect, ::DWORD dwFlags, ::DDBLTFX* lpDDBltFx) noexcept
{
    if (lpDestRect && lpDDSrcSurface)
    {
        core::instance().update();
        ::HDC dc;
        if (SUCCEEDED(lpDDSrcSurface->GetDC(&dc)))
        {
            // TODO: render update gui
            lpDDSrcSurface->ReleaseDC(dc);
        }
    }
    return m_impl->Blt(
        lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::BltBatch(
    ::DDBLTBATCH* lpDDBltBatch, ::DWORD dwCount, ::DWORD dwFlags) noexcept
{
    return m_impl->BltBatch(lpDDBltBatch, dwCount, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::BltFast(
    ::DWORD dwX, ::DWORD dwY, ::IDirectDrawSurface7* lpDDSrcSurface,
    ::RECT* lpSrcRect, ::DWORD dwFlags, ::DDBLTFX* lpDDBltFx) noexcept
{
    return m_impl->BltFast(
        dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::DeleteAttachedSurface(
    ::DWORD dwFlags, ::IDirectDrawSurface7* lpDDSAttachedSurface) noexcept
{
    return m_impl->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::EnumAttachedSurfaces(
    void* lpContext,
    ::LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback) noexcept
{
    return m_impl->EnumAttachedSurfaces(lpContext, lpEnumSurfacesCallback);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::EnumOverlayZOrders(
    ::DWORD dwFlags, void* lpContext,
    ::LPDDENUMSURFACESCALLBACK7 lpfnCallback) noexcept
{
    return m_impl->EnumOverlayZOrders(dwFlags, lpContext, lpfnCallback);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::Flip(
    ::IDirectDrawSurface7* lpDDSurfaceTargetOverride, ::DWORD dwFlags) noexcept
{
    core::instance().update();
    DDSCAPS2 caps{};
    caps.dwCaps = DDSCAPS_BACKBUFFER;

    ::IDirectDrawSurface7* back_buffer = nullptr;
    if (SUCCEEDED(m_impl->GetAttachedSurface(&caps, &back_buffer)) &&
        back_buffer != nullptr)
    {
        ::HDC dc = nullptr;
        if (SUCCEEDED(back_buffer->GetDC(&dc)))
        {
            // TODO: render update gui
            back_buffer->ReleaseDC(dc);
        }
        back_buffer->Release();
    }

    return m_impl->Flip(lpDDSurfaceTargetOverride, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetAttachedSurface(
    ::DDSCAPS2* lpDDSCaps,
    ::IDirectDrawSurface7** lplpDDAttachedSurface) noexcept
{
    return m_impl->GetAttachedSurface(lpDDSCaps, lplpDDAttachedSurface);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetBltStatus(::DWORD dwFlags) noexcept
{
    return m_impl->GetBltStatus(dwFlags);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetCaps(::DDSCAPS2* lpDDSCaps) noexcept
{
    return m_impl->GetCaps(lpDDSCaps);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetCaps(
    ::IDirectDrawClipper** lplpDDClipper) noexcept
{
    return m_impl->GetCaps(lplpDDClipper);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetColorKey(
    ::DWORD dwFlags, ::DDCOLORKEY* lpDDColorKey) noexcept
{
    return m_impl->GetColorKey(dwFlags, lpDDColorKey);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetDC(::HDC* lphDC) noexcept
{
    return m_impl->GetDC(lphDC);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetFlipStatus(::DWORD dwFlags) noexcept
{
    return m_impl->GetFlipStatus(dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetOverlayPosition(
    ::LONG* lplX, ::LONG* lplY) noexcept
{
    return m_impl->GetOverlayPosition(lplX, lplY);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetPalette(
    ::IDirectDrawPalette** lplpDDPalette) noexcept
{
    return m_impl->GetPalette(lplpDDPalette);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetPixelFormat(
    ::DDPIXELFORMAT* lpDDPixelFormat) noexcept
{
    return m_impl->GetPixelFormat(lpDDPixelFormat);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetSurfaceDesc(
    ::DDSURFACEDESC2* lpDDSurfaceDesc) noexcept
{
    return m_impl->GetSurfaceDesc(lpDDSurfaceDesc);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::Initialize(
    ::IDirectDraw* lpDD, ::DDSURFACEDESC2* lpDDSurfaceDesc) noexcept
{
    return m_impl->Initialize(lpDD, lpDDSurfaceDesc);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::IsLost() noexcept
{
    return m_impl->IsLost();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::Lock(
    ::RECT* lpDestRect, ::DDSURFACEDESC2* lpDDSurfaceDesc, ::DWORD dwFlags,
    ::HANDLE hEvent) noexcept
{
    return m_impl->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::ReleaseDC(::HDC hDC) noexcept
{
    return m_impl->ReleaseDC(hDC);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::Restore() noexcept
{
    return m_impl->Restore();
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::SetClipper(
    ::IDirectDrawClipper* lpDDClipper) noexcept
{
    return m_impl->SetClipper(lpDDClipper);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::SetColorKey(
    ::DWORD dwFlags, ::DDCOLORKEY* lpDDColorKey) noexcept
{
    return m_impl->SetColorKey(dwFlags, lpDDColorKey);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::SetOverlayPosition(::LONG lX, ::LONG lY) noexcept
{
    return m_impl->SetOverlayPosition(lX, lY);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::SetPalette(
    ::IDirectDrawPalette* lpDDPalette) noexcept
{
    return m_impl->SetPalette(lpDDPalette);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::Unlock(::RECT* lpRect) noexcept
{
    return m_impl->Unlock(lpRect);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::UpdateOverlay(
    ::RECT* lpSrcRect, ::IDirectDrawSurface7* lpDDDestSurface,
    ::RECT* lpDestRect, ::DWORD dwFlags, ::DDOVERLAYFX* lpDDOverlayFx) noexcept
{
    return m_impl->UpdateOverlay(
        lpSrcRect, lpDDDestSurface, lpDestRect, dwFlags, lpDDOverlayFx);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::UpdateOverlayDisplay(::DWORD dwFlags) noexcept
{
    return m_impl->UpdateOverlayDisplay(dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::UpdateOverlayZOrder(
    ::DWORD dwFlags, ::IDirectDrawSurface7* lpDDSReference) noexcept
{
    return m_impl->UpdateOverlayZOrder(dwFlags, lpDDSReference);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetDDInterface(void** lplpDD) noexcept
{
    return m_impl->GetDDInterface(lplpDD);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::PageLock(::DWORD dwFlags) noexcept
{
    return m_impl->PageLock(dwFlags);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::PageUnlock(::DWORD dwFlags) noexcept
{
    return m_impl->PageUnlock(dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::SetSurfaceDesc(
    ::DDSURFACEDESC2* lpDDsd2, ::DWORD dwFlags) noexcept
{
    return m_impl->SetSurfaceDesc(lpDDsd2, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::SetPrivateData(
    ::GUID const& guidTag, void* lpData, ::DWORD cbSize,
    ::DWORD dwFlags) noexcept
{
    return m_impl->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_draw_surface::GetPrivateData(
    ::GUID const& guidTag, void* lpBuffer, ::DWORD lpcbBufferSize) noexcept
{
    return m_impl->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::FreePrivateData(::GUID const& guidTag) noexcept
{
    return m_impl->FreePrivateData(guidTag);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetUniquenessValue(::DWORD* lpValue) noexcept
{
    return m_impl->GetUniquenessValue(lpValue);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::ChangeUniquenessValue() noexcept
{
    return m_impl->ChangeUniquenessValue();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::SetPriority(::DWORD dwPriority) noexcept
{
    return m_impl->SetPriority(dwPriority);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetPriority(::DWORD* lpdwPriority) noexcept
{
    return m_impl->GetPriority(lpdwPriority);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::SetLOD(::DWORD dwMaxLOD) noexcept
{
    return m_impl->SetLOD(dwMaxLOD);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_draw_surface::GetLOD(::DWORD* lpdwMaxLOD) noexcept
{
    return m_impl->GetLOD(lpdwMaxLOD);
}
