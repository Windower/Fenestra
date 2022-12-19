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

#ifndef WINDOWER_WRAPPERS_DIRECT_3D_HPP
#define WINDOWER_WRAPPERS_DIRECT_3D_HPP

#include <windows.h>
#include <d3d8.h>

namespace windower
{

class direct_3d final : public ::IDirect3D8
{
public:
    direct_3d() = delete;
    direct_3d(::IDirect3D8*) noexcept;
    direct_3d(direct_3d const&) = delete;
    direct_3d(direct_3d&&) = delete;
    virtual ~direct_3d();

    direct_3d& operator=(direct_3d const&) = delete;
    direct_3d& operator=(direct_3d&&) = delete;

    // IUnknown methods
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDirect3D8 methods
    ::HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void*) noexcept override;
    ::UINT STDMETHODCALLTYPE GetAdapterCount() noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(
        ::UINT, ::DWORD, ::D3DADAPTER_IDENTIFIER8*) noexcept override;
    ::UINT STDMETHODCALLTYPE GetAdapterModeCount(::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    EnumAdapterModes(::UINT, ::UINT, ::D3DDISPLAYMODE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetAdapterDisplayMode(::UINT, ::D3DDISPLAYMODE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceType(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::D3DFORMAT,
        ::BOOL) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceFormat(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::DWORD, ::D3DRESOURCETYPE,
        ::D3DFORMAT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::BOOL,
        ::D3DMULTISAMPLE_TYPE) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::D3DFORMAT,
        ::D3DFORMAT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceCaps(::UINT, ::D3DDEVTYPE, ::D3DCAPS8*) noexcept override;
    ::HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateDevice(
        ::UINT, ::D3DDEVTYPE, ::HWND, ::DWORD, ::D3DPRESENT_PARAMETERS*,
        ::IDirect3DDevice8**) noexcept override;

private:
    ::IDirect3D8* m_impl;
    ::ULONG m_count = 0;
};

}

#endif
