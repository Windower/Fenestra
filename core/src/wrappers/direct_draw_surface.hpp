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

#ifndef WINDOWER_WRAPPERS_DIRECT_DRAW_SURFACE7_HPP
#define WINDOWER_WRAPPERS_DIRECT_DRAW_SURFACE7_HPP

#include "wrappers/direct_draw.hpp"

#include <windows.h>

#include <ddraw.h>

namespace windower
{

class direct_draw_surface final : public ::IDirectDrawSurface7
{
public:
    direct_draw_surface() = delete;
    direct_draw_surface(::IDirectDrawSurface7*, direct_draw*) noexcept;
    direct_draw_surface(direct_draw_surface const&) = delete;
    direct_draw_surface(direct_draw_surface&&)      = delete;
    ~direct_draw_surface();

    direct_draw_surface& operator=(direct_draw_surface const&) = delete;
    direct_draw_surface& operator=(direct_draw_surface&&)      = delete;

    // IUnknown methods
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDirectDrawSurface7 methods
    ::HRESULT STDMETHODCALLTYPE
    AddAttachedSurface(::IDirectDrawSurface7*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE AddOverlayDirtyRect(::RECT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Blt(::RECT*, ::IDirectDrawSurface7*, ::RECT*, ::DWORD,
        ::DDBLTFX*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    BltBatch(::DDBLTBATCH*, ::DWORD, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE BltFast(
        ::DWORD, ::DWORD, ::IDirectDrawSurface7*, ::RECT*, ::DWORD,
        ::DDBLTFX*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    DeleteAttachedSurface(::DWORD, ::IDirectDrawSurface7*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    EnumAttachedSurfaces(void*, ::LPDDENUMSURFACESCALLBACK7) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumOverlayZOrders(
        ::DWORD, void*, ::LPDDENUMSURFACESCALLBACK7) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Flip(::IDirectDrawSurface7*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetAttachedSurface(::DDSCAPS2*, ::IDirectDrawSurface7**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetBltStatus(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetCaps(::DDSCAPS2*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetCaps(::IDirectDrawClipper**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetColorKey(::DWORD, ::DDCOLORKEY*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDC(::HDC*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetFlipStatus(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetOverlayPosition(::LONG*, ::LONG*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPalette(::IDirectDrawPalette**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPixelFormat(::DDPIXELFORMAT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetSurfaceDesc(::DDSURFACEDESC2*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Initialize(::IDirectDraw*, ::DDSURFACEDESC2*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE IsLost() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Lock(::RECT*, ::DDSURFACEDESC2*, ::DWORD, ::HANDLE) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ReleaseDC(::HDC) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Restore() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetClipper(::IDirectDrawClipper*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetColorKey(::DWORD, ::DDCOLORKEY*) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE SetOverlayPosition(::LONG, ::LONG) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetPalette(::IDirectDrawPalette*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Unlock(::RECT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE UpdateOverlay(
        ::RECT*, ::IDirectDrawSurface7*, ::RECT*, ::DWORD,
        ::DDOVERLAYFX*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE UpdateOverlayDisplay(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    UpdateOverlayZOrder(::DWORD, ::IDirectDrawSurface7*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDDInterface(void**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE PageLock(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE PageUnlock(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetSurfaceDesc(::DDSURFACEDESC2*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetPrivateData(::GUID const&, void*, ::DWORD, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPrivateData(::GUID const&, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    FreePrivateData(::GUID const&) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetUniquenessValue(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ChangeUniquenessValue() noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetPriority(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetPriority(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetLOD(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetLOD(::DWORD*) noexcept override;

private:
    ::IDirectDrawSurface7* m_impl;
    direct_draw* m_parent;
    ::ULONG m_count = 0;

    friend class direct_draw;
};

}

#endif
