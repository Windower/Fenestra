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

#ifndef WINDOWER_WRAPPERS_DIRECT_DRAW7_HPP
#define WINDOWER_WRAPPERS_DIRECT_DRAW7_HPP

#include <windows.h>

#include <ddraw.h>

namespace windower
{

class direct_draw final : public ::IDirectDraw7
{
public:
    direct_draw() = delete;
    direct_draw(::IDirectDraw7*) noexcept;
    direct_draw(direct_draw const&) = delete;
    direct_draw(direct_draw&&)      = delete;
    ~direct_draw();

    direct_draw& operator=(direct_draw const&) = delete;
    direct_draw& operator=(direct_draw&&)      = delete;

    // IUnknown methods
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDirectDraw7 methods
    ::HRESULT STDMETHODCALLTYPE Compact() noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateClipper(
        ::DWORD, ::IDirectDrawClipper**, ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreatePalette(
        ::DWORD, ::PALETTEENTRY*, ::IDirectDrawPalette**,
        ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateSurface(
        ::DDSURFACEDESC2*, ::IDirectDrawSurface7**,
        ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DuplicateSurface(
        ::IDirectDrawSurface7*, ::IDirectDrawSurface7**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumDisplayModes(
        ::DWORD, ::DDSURFACEDESC2*, void*,
        ::LPDDENUMMODESCALLBACK2) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumSurfaces(
        ::DWORD, ::DDSURFACEDESC2*, void*,
        ::LPDDENUMSURFACESCALLBACK7) noexcept override;
    ::HRESULT STDMETHODCALLTYPE FlipToGDISurface() noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetCaps(::DDCAPS*, ::DDCAPS*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDisplayMode(::DDSURFACEDESC2*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetFourCCCodes(::DWORD*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetGDISurface(::IDirectDrawSurface7**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetMonitorFrequency(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetScanLine(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetVerticalBlankStatus(::BOOL*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Initialize(::GUID*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE RestoreDisplayMode() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        SetCooperativeLevel(::HWND, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetDisplayMode(
        ::DWORD, ::DWORD, ::DWORD, ::DWORD, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        WaitForVerticalBlank(::DWORD, ::HANDLE) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetAvailableVidMem(::DDSCAPS2*, ::DWORD*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetSurfaceFromDC(::HDC, ::IDirectDrawSurface7**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE RestoreAllSurfaces() noexcept override;
    ::HRESULT STDMETHODCALLTYPE TestCooperativeLevel() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceIdentifier(::DDDEVICEIDENTIFIER2*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    StartModeTest(::SIZE*, ::DWORD, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    EvaluateMode(::DWORD, ::DWORD*) noexcept override;

private:
    ::IDirectDraw7* m_impl;
    ::ULONG m_count = 0;
};

}

#endif
