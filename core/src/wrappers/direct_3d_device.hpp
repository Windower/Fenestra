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

#ifndef WINDOWER_WRAPPERS_DIRECT_3D_DEVICE_HPP
#define WINDOWER_WRAPPERS_DIRECT_3D_DEVICE_HPP

#include "wrappers/direct_3d.hpp"

#include <windows.h>

#include <d3d8.h>

namespace windower
{

class direct_3d_device final : public ::IDirect3DDevice8
{
public:
    direct_3d_device() = delete;
    direct_3d_device(::IDirect3DDevice8*, ::HWND, direct_3d*);
    direct_3d_device(direct_3d_device const&) = delete;
    direct_3d_device(direct_3d_device&&)      = delete;
    virtual ~direct_3d_device();

    direct_3d_device& operator=(direct_3d_device const&) = delete;
    direct_3d_device& operator=(direct_3d_device&&)      = delete;

    // IUnknown methods
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDirect3DDevice8 methods
    ::HRESULT STDMETHODCALLTYPE TestCooperativeLevel() noexcept override;
    ::UINT STDMETHODCALLTYPE GetAvailableTextureMem() noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        ResourceManagerDiscardBytes(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDirect3D(::IDirect3D8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDeviceCaps(::D3DCAPS8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDisplayMode(::D3DDISPLAYMODE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetCreationParameters(::D3DDEVICE_CREATION_PARAMETERS*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetCursorProperties(::UINT, ::UINT, ::IDirect3DSurface8*) noexcept override;
    void STDMETHODCALLTYPE
    SetCursorPosition(int, int, ::DWORD) noexcept override;
    ::BOOL STDMETHODCALLTYPE ShowCursor(::BOOL) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(
        ::D3DPRESENT_PARAMETERS*, ::IDirect3DSwapChain8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    Reset(::D3DPRESENT_PARAMETERS*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE Present(
        ::RECT const*, ::RECT const*, ::HWND,
        ::RGNDATA const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetBackBuffer(
        ::UINT, ::D3DBACKBUFFER_TYPE, ::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetRasterStatus(::D3DRASTER_STATUS*) noexcept override;
    void STDMETHODCALLTYPE
    SetGammaRamp(::DWORD, ::D3DGAMMARAMP const*) noexcept override;
    void STDMETHODCALLTYPE GetGammaRamp(::D3DGAMMARAMP*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateTexture(
        ::UINT, ::UINT, ::UINT, ::DWORD, ::D3DFORMAT, ::D3DPOOL,
        ::IDirect3DTexture8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateVolumeTexture(
        ::UINT, ::UINT, ::UINT, ::UINT, ::DWORD, ::D3DFORMAT, ::D3DPOOL,
        ::IDirect3DVolumeTexture8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateCubeTexture(
        ::UINT, ::UINT, ::DWORD, ::D3DFORMAT, ::D3DPOOL,
        ::IDirect3DCubeTexture8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateVertexBuffer(
        ::UINT, ::DWORD, ::DWORD, ::D3DPOOL,
        ::IDirect3DVertexBuffer8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateIndexBuffer(
        ::UINT, ::DWORD, ::D3DFORMAT, ::D3DPOOL,
        ::IDirect3DIndexBuffer8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateRenderTarget(
        ::UINT, ::UINT, ::D3DFORMAT, ::D3DMULTISAMPLE_TYPE, ::BOOL,
        ::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(
        ::UINT, ::UINT, ::D3DFORMAT, ::D3DMULTISAMPLE_TYPE,
        ::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateImageSurface(
        ::UINT, ::UINT, ::D3DFORMAT, ::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CopyRects(
        ::IDirect3DSurface8*, ::RECT const*, ::UINT, ::IDirect3DSurface8*,
        ::POINT const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE UpdateTexture(
        ::IDirect3DBaseTexture8*, ::IDirect3DBaseTexture8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetFrontBuffer(::IDirect3DSurface8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetRenderTarget(
        ::IDirect3DSurface8*, ::IDirect3DSurface8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetRenderTarget(::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDepthStencilSurface(::IDirect3DSurface8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE BeginScene() noexcept override;
    ::HRESULT STDMETHODCALLTYPE EndScene() noexcept override;
    ::HRESULT STDMETHODCALLTYPE Clear(
        ::DWORD, ::D3DRECT const*, ::DWORD, ::D3DCOLOR, float,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetTransform(::D3DTRANSFORMSTATETYPE, ::D3DMATRIX const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetTransform(::D3DTRANSFORMSTATETYPE, ::D3DMATRIX*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE MultiplyTransform(
        ::D3DTRANSFORMSTATETYPE, ::D3DMATRIX const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetViewport(::D3DVIEWPORT8 const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetViewport(::D3DVIEWPORT8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetMaterial(::D3DMATERIAL8 const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetMaterial(::D3DMATERIAL8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetLight(::DWORD, ::D3DLIGHT8 const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetLight(::DWORD, ::D3DLIGHT8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE LightEnable(::DWORD, ::BOOL) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetLightEnable(::DWORD, ::BOOL*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetClipPlane(::DWORD, float const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetClipPlane(::DWORD, float*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        SetRenderState(::D3DRENDERSTATETYPE, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetRenderState(::D3DRENDERSTATETYPE, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE BeginStateBlock() noexcept override;
    ::HRESULT STDMETHODCALLTYPE EndStateBlock(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ApplyStateBlock(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CaptureStateBlock(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DeleteStateBlock(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    CreateStateBlock(::D3DSTATEBLOCKTYPE, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetClipStatus(::D3DCLIPSTATUS8 const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetClipStatus(::D3DCLIPSTATUS8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetTexture(::DWORD, ::IDirect3DBaseTexture8**) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetTexture(::DWORD, ::IDirect3DBaseTexture8*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetTextureStageState(
        ::DWORD, ::D3DTEXTURESTAGESTATETYPE, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetTextureStageState(
        ::DWORD, ::D3DTEXTURESTAGESTATETYPE, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ValidateDevice(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetInfo(::DWORD, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetPaletteEntries(::UINT, ::PALETTEENTRY const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPaletteEntries(::UINT, ::PALETTEENTRY*) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE SetCurrentTexturePalette(::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetCurrentTexturePalette(::UINT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
        DrawPrimitive(::D3DPRIMITIVETYPE, ::UINT, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(
        ::D3DPRIMITIVETYPE, ::UINT, ::UINT, ::UINT, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(
        ::D3DPRIMITIVETYPE, ::UINT, void const*, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(
        ::D3DPRIMITIVETYPE, ::UINT, ::UINT, ::UINT, void const*, ::D3DFORMAT,
        void const*, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ProcessVertices(
        ::UINT, ::UINT, ::UINT, ::IDirect3DVertexBuffer8*,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateVertexShader(
        ::DWORD const*, ::DWORD const*, ::DWORD*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetVertexShader(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetVertexShader(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DeleteVertexShader(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetVertexShaderConstant(::DWORD, void const*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetVertexShaderConstant(::DWORD, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetVertexShaderDeclaration(::DWORD, void*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetVertexShaderFunction(::DWORD, void*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetStreamSource(
        ::UINT, ::IDirect3DVertexBuffer8*, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetStreamSource(
        ::UINT, ::IDirect3DVertexBuffer8**, ::UINT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetIndices(::IDirect3DIndexBuffer8*, ::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetIndices(::IDirect3DIndexBuffer8**, ::UINT*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    CreatePixelShader(::DWORD const*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE SetPixelShader(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetPixelShader(::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DeletePixelShader(::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    SetPixelShaderConstant(::DWORD, void const*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPixelShaderConstant(::DWORD, void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetPixelShaderFunction(::DWORD, void*, ::DWORD*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DrawRectPatch(
        ::UINT, float const*, ::D3DRECTPATCH_INFO const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DrawTriPatch(
        ::UINT, float const*, ::D3DTRIPATCH_INFO const*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE DeletePatch(::UINT) noexcept override;

private:
    ::IDirect3DDevice8* m_impl;
    direct_3d* m_parent;
    ::ULONG m_count          = 0;
    bool m_frame_in_progress = false;
};

}

#endif
