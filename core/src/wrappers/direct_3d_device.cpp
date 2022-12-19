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

#include "wrappers/direct_3d_device.hpp"

#include "addon/addon_manager.hpp"
#include "addon/script_environment.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "hooks/ffximain.hpp"
#include "wrappers/direct_3d.hpp"

#include <windows.h>

#include <d3d8.h>

#include <cstddef>
#include <memory>
#include <string>

windower::direct_3d_device::direct_3d_device(
    ::IDirect3DDevice8* impl, ::HWND hwnd, direct_3d* parent) :
    m_impl{impl},
    m_parent{parent}
{
    m_parent->AddRef();

    AddRef();

    auto& core = core::instance();
    core.incoming_packet_queue =
        std::make_unique<packet_queue>(packet_direction::incoming);
    core.outgoing_packet_queue =
        std::make_unique<packet_queue>(packet_direction::outgoing);

    ffximain::install();

    core.run_on_next_frame([impl, hwnd] {
        auto& core = core::instance();

        core.ui.initialize(
            hwnd, impl,
            {gsl::narrow_cast<float>(core.settings.window_bounds.size.width),
             gsl::narrow_cast<float>(core.settings.window_bounds.size.height)},
            {gsl::narrow_cast<float>(core.settings.ui_size.width),
             gsl::narrow_cast<float>(core.settings.ui_size.height)},
            {gsl::narrow_cast<float>(core.settings.render_size.width),
             gsl::narrow_cast<float>(core.settings.render_size.height)});

        core.script_environment.reset();
        core.addon_manager = std::make_unique<addon_manager>();

        std::vector<std::u8string> names;
        for (auto const& p :
             core.package_manager->installed_packages(package_type::service))
        {
            names.push_back(p->name());
        }
        core.addon_manager->load(names);
        core.script_environment.execute(u8"init");
    });
}

windower::direct_3d_device::~direct_3d_device()
{
    auto& core = core::instance();
    core.script_environment.reset();
    core.addon_manager = nullptr;
    core.ui.reset();

    ffximain::uninstall();

    core.incoming_packet_queue = nullptr;
    core.outgoing_packet_queue = nullptr;

    m_impl->Release();
    m_impl = nullptr;

    m_parent->Release();
    m_parent = nullptr;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::QueryInterface(
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
        ::IUnknown* const ptr = this;
        *ppvObj               = ptr;
        return S_OK;
    }
    else if (::IsEqualGUID(riid, ::IID_IDirect3DDevice8))
    {
        AddRef();
        ::IDirect3DDevice8* const ptr = this;
        *ppvObj                       = ptr;
        return S_OK;
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_3d_device::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_3d_device::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::TestCooperativeLevel() noexcept
{
    return m_impl->TestCooperativeLevel();
}

::UINT STDMETHODCALLTYPE
windower::direct_3d_device::GetAvailableTextureMem() noexcept
{
    return m_impl->GetAvailableTextureMem();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ResourceManagerDiscardBytes(::DWORD Bytes) noexcept
{
    return m_impl->ResourceManagerDiscardBytes(Bytes);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDirect3D(::IDirect3D8** ppD3D8) noexcept
{
    if (ppD3D8)
    {
        m_parent->AddRef();
        *ppD3D8 = m_parent;
        return S_OK;
    }
    return D3DERR_INVALIDCALL;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDeviceCaps(::D3DCAPS8* pCaps) noexcept
{
    return m_impl->GetDeviceCaps(pCaps);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDisplayMode(::D3DDISPLAYMODE* pMode) noexcept
{
    return m_impl->GetDisplayMode(pMode);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetCreationParameters(
    ::D3DDEVICE_CREATION_PARAMETERS* pParameters) noexcept
{
    return m_impl->GetCreationParameters(pParameters);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetCursorProperties(
    ::UINT XHotSpot, ::UINT YHotSpot,
    ::IDirect3DSurface8* pCursorBitmap) noexcept
{
    return m_impl->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE windower::direct_3d_device::SetCursorPosition(
    int X, int Y, ::DWORD Flags) noexcept
{
    return m_impl->SetCursorPosition(X, Y, Flags);
}

::BOOL STDMETHODCALLTYPE
windower::direct_3d_device::ShowCursor(::BOOL bShow) noexcept
{
    return m_impl->ShowCursor(bShow);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CreateAdditionalSwapChain(
    ::D3DPRESENT_PARAMETERS* pPresentationParameters,
    ::IDirect3DSwapChain8** pSwapChain) noexcept
{
    return m_impl->CreateAdditionalSwapChain(
        pPresentationParameters, pSwapChain);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Reset(
    ::D3DPRESENT_PARAMETERS* pPresentationParameters) noexcept
{
    return m_impl->Reset(pPresentationParameters);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Present(
    ::RECT const* pSourceRect, ::RECT const* pDestRect,
    ::HWND hDestWindowOverride, RGNDATA const* pDirtyRegion) noexcept
{
    auto& core = core::instance();

    m_impl->BeginScene();

    core.update();
    core.end_frame();
    core.ui.render(windower::ui::layer::screen);
    core.ui.render(windower::ui::layer::layout);

    m_impl->EndScene();

    m_frame_in_progress = false;

    return m_impl->Present(
        pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetBackBuffer(
    ::UINT BackBuffer, ::D3DBACKBUFFER_TYPE Type,
    ::IDirect3DSurface8** ppBackBuffer) noexcept
{
    return m_impl->GetBackBuffer(BackBuffer, Type, ppBackBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRasterStatus(
    ::D3DRASTER_STATUS* pRasterStatus) noexcept
{
    return m_impl->GetRasterStatus(pRasterStatus);
}

void STDMETHODCALLTYPE windower::direct_3d_device::SetGammaRamp(
    ::DWORD Flags, ::D3DGAMMARAMP const* pRamp) noexcept
{
    return m_impl->SetGammaRamp(Flags, pRamp);
}

void STDMETHODCALLTYPE
windower::direct_3d_device::GetGammaRamp(::D3DGAMMARAMP* pRamp) noexcept
{
    return m_impl->GetGammaRamp(pRamp);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateTexture(
    ::UINT Width, ::UINT Height, ::UINT Levels, ::DWORD Usage,
    ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DTexture8** ppTexture) noexcept
{
    return m_impl->CreateTexture(
        Width, Height, Levels, Usage, Format, Pool, ppTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVolumeTexture(
    ::UINT Width, ::UINT Height, ::UINT Depth, ::UINT Levels, ::DWORD Usage,
    ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DVolumeTexture8** ppVolumeTexture) noexcept
{
    return m_impl->CreateVolumeTexture(
        Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateCubeTexture(
    ::UINT EdgeLength, ::UINT Levels, ::DWORD Usage, ::D3DFORMAT Format,
    ::D3DPOOL Pool, ::IDirect3DCubeTexture8** ppCubeTexture) noexcept
{
    return m_impl->CreateCubeTexture(
        EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVertexBuffer(
    ::UINT Length, ::DWORD Usage, ::DWORD FVF, ::D3DPOOL Pool,
    ::IDirect3DVertexBuffer8** ppVertexBuffer) noexcept
{
    return m_impl->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateIndexBuffer(
    ::UINT Length, ::DWORD Usage, ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DIndexBuffer8** ppIndexBuffer) noexcept
{
    return m_impl->CreateIndexBuffer(
        Length, Usage, Format, Pool, ppIndexBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateRenderTarget(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::D3DMULTISAMPLE_TYPE MultiSample, ::BOOL Lockable,
    ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateRenderTarget(
        Width, Height, Format, MultiSample, Lockable, ppSurface);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CreateDepthStencilSurface(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::D3DMULTISAMPLE_TYPE MultiSample, ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateDepthStencilSurface(
        Width, Height, Format, MultiSample, ppSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateImageSurface(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateImageSurface(Width, Height, Format, ppSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CopyRects(
    ::IDirect3DSurface8* pSourceSurface, ::RECT const* pSourceRectsArray,
    ::UINT cRects, ::IDirect3DSurface8* pDestinationSurface,
    ::POINT const* pDestPointsArray) noexcept
{
    return m_impl->CopyRects(
        pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface,
        pDestPointsArray);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::UpdateTexture(
    ::IDirect3DBaseTexture8* pSourceTexture,
    ::IDirect3DBaseTexture8* pDestinationTexture) noexcept
{
    return m_impl->UpdateTexture(pSourceTexture, pDestinationTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetFrontBuffer(
    ::IDirect3DSurface8* pDestSurface) noexcept
{
    return m_impl->GetFrontBuffer(pDestSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetRenderTarget(
    ::IDirect3DSurface8* pRenderTarget,
    ::IDirect3DSurface8* pNewZStencil) noexcept
{
    return m_impl->SetRenderTarget(pRenderTarget, pNewZStencil);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRenderTarget(
    ::IDirect3DSurface8** ppRenderTarget) noexcept
{
    return m_impl->GetRenderTarget(ppRenderTarget);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetDepthStencilSurface(
    ::IDirect3DSurface8** ppZStencilSurface) noexcept
{
    return m_impl->GetDepthStencilSurface(ppZStencilSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::BeginScene() noexcept
{
    auto const result = m_impl->BeginScene();
    if (!m_frame_in_progress)
    {
        m_frame_in_progress = true;
        core::instance().begin_frame();
    }
    return result;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::EndScene() noexcept
{
    return m_impl->EndScene();
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Clear(
    ::DWORD Count, ::D3DRECT const* pRects, ::DWORD Flags, ::D3DCOLOR Color,
    float Z, ::DWORD Stencil) noexcept
{
    return m_impl->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX const* pMatrix) noexcept
{
    return m_impl->SetTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX* pMatrix) noexcept
{
    return m_impl->GetTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::MultiplyTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX const* pMatrix) noexcept
{
    return m_impl->MultiplyTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetViewport(
    ::D3DVIEWPORT8 const* pViewport) noexcept
{
    return m_impl->SetViewport(pViewport);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetViewport(::D3DVIEWPORT8* pViewport) noexcept
{
    return m_impl->GetViewport(pViewport);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetMaterial(
    ::D3DMATERIAL8 const* pMaterial) noexcept
{
    return m_impl->SetMaterial(pMaterial);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetMaterial(::D3DMATERIAL8* pMaterial) noexcept
{
    return m_impl->GetMaterial(pMaterial);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetLight(
    ::DWORD Index, ::D3DLIGHT8 const* pLight) noexcept
{
    return m_impl->SetLight(Index, pLight);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetLight(
    ::DWORD Index, ::D3DLIGHT8* pLight) noexcept
{
    return m_impl->GetLight(Index, pLight);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::LightEnable(::DWORD Index, ::BOOL Enable) noexcept
{
    return m_impl->LightEnable(Index, Enable);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetLightEnable(
    ::DWORD Index, ::BOOL* pEnable) noexcept
{
    return m_impl->GetLightEnable(Index, pEnable);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetClipPlane(
    ::DWORD Index, float const* pPlane) noexcept
{
    return m_impl->SetClipPlane(Index, pPlane);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetClipPlane(::DWORD Index, float* pPlane) noexcept
{
    return m_impl->GetClipPlane(Index, pPlane);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetRenderState(
    ::D3DRENDERSTATETYPE State, ::DWORD Value) noexcept
{
    return m_impl->SetRenderState(State, Value);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRenderState(
    ::D3DRENDERSTATETYPE State, ::DWORD* pValue) noexcept
{
    return m_impl->GetRenderState(State, pValue);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::BeginStateBlock() noexcept
{
    return m_impl->BeginStateBlock();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::EndStateBlock(::DWORD* pToken) noexcept
{
    return m_impl->EndStateBlock(pToken);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ApplyStateBlock(::DWORD Token) noexcept
{
    return m_impl->ApplyStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CaptureStateBlock(::DWORD Token) noexcept
{
    return m_impl->CaptureStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeleteStateBlock(::DWORD Token) noexcept
{
    return m_impl->DeleteStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateStateBlock(
    ::D3DSTATEBLOCKTYPE Type, ::DWORD* pToken) noexcept
{
    return m_impl->CreateStateBlock(Type, pToken);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetClipStatus(
    ::D3DCLIPSTATUS8 const* pClipStatus) noexcept
{
    return m_impl->SetClipStatus(pClipStatus);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetClipStatus(
    ::D3DCLIPSTATUS8* pClipStatus) noexcept
{
    return m_impl->GetClipStatus(pClipStatus);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTexture(
    ::DWORD Stage, ::IDirect3DBaseTexture8** ppTexture) noexcept
{
    return m_impl->GetTexture(Stage, ppTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTexture(
    ::DWORD Stage, ::IDirect3DBaseTexture8* pTexture) noexcept
{
    return m_impl->SetTexture(Stage, pTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTextureStageState(
    ::DWORD Stage, ::D3DTEXTURESTAGESTATETYPE Type, ::DWORD* pValue) noexcept
{
    return m_impl->GetTextureStageState(Stage, Type, pValue);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTextureStageState(
    ::DWORD Stage, ::D3DTEXTURESTAGESTATETYPE Type, ::DWORD Value) noexcept
{
    return m_impl->SetTextureStageState(Stage, Type, Value);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ValidateDevice(::DWORD* pNumPasses) noexcept
{
    return m_impl->ValidateDevice(pNumPasses);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetInfo(
    ::DWORD DevInfoID, void* pDevInfoStruct, ::DWORD DevInfoStructSize) noexcept
{
    return m_impl->GetInfo(DevInfoID, pDevInfoStruct, DevInfoStructSize);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetPaletteEntries(
    ::UINT PaletteNumber, ::PALETTEENTRY const* pEntries) noexcept
{
    return m_impl->SetPaletteEntries(PaletteNumber, pEntries);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPaletteEntries(
    ::UINT PaletteNumber, ::PALETTEENTRY* pEntries) noexcept
{
    return m_impl->GetPaletteEntries(PaletteNumber, pEntries);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetCurrentTexturePalette(
    ::UINT PaletteNumber) noexcept
{
    return m_impl->SetCurrentTexturePalette(PaletteNumber);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetCurrentTexturePalette(
    ::UINT* PaletteNumber) noexcept
{
    return m_impl->GetCurrentTexturePalette(PaletteNumber);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawPrimitive(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT StartVertex,
    ::UINT PrimitiveCount) noexcept
{
    return m_impl->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawIndexedPrimitive(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT minIndex, ::UINT NumVertices,
    ::UINT startIndex, ::UINT primCount) noexcept
{
    return m_impl->DrawIndexedPrimitive(
        PrimitiveType, minIndex, NumVertices, startIndex, primCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawPrimitiveUP(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT PrimitiveCount,
    void const* pVertexStreamZeroData, ::UINT VertexStreamZeroStride) noexcept
{
    return m_impl->DrawPrimitiveUP(
        PrimitiveType, PrimitiveCount, pVertexStreamZeroData,
        VertexStreamZeroStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawIndexedPrimitiveUP(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT MinVertexIndex,
    ::UINT NumVertexIndices, ::UINT PrimitiveCount, void const* pIndexData,
    ::D3DFORMAT IndexDataFormat, void const* pVertexStreamZeroData,
    ::UINT VertexStreamZeroStride) noexcept
{
    return m_impl->DrawIndexedPrimitiveUP(
        PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount,
        pIndexData, IndexDataFormat, pVertexStreamZeroData,
        VertexStreamZeroStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::ProcessVertices(
    ::UINT SrcStartIndex, ::UINT DestIndex, ::UINT VertexCount,
    ::IDirect3DVertexBuffer8* pDestBuffer, ::DWORD Flags) noexcept
{
    return m_impl->ProcessVertices(
        SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVertexShader(
    ::DWORD const* pDeclaration, ::DWORD const* pFunction, ::DWORD* pHandle,
    ::DWORD Usage) noexcept
{
    return m_impl->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetVertexShader(::DWORD Handle) noexcept
{
    return m_impl->SetVertexShader(Handle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetVertexShader(::DWORD* pHandle) noexcept
{
    return m_impl->GetVertexShader(pHandle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeleteVertexShader(::DWORD Handle) noexcept
{
    return m_impl->DeleteVertexShader(Handle);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetVertexShaderConstant(
    ::DWORD Register, void const* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->SetVertexShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetVertexShaderConstant(
    ::DWORD Register, void* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->GetVertexShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetVertexShaderDeclaration(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetVertexShaderDeclaration(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetVertexShaderFunction(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetVertexShaderFunction(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetStreamSource(
    ::UINT StreamNumber, ::IDirect3DVertexBuffer8* pStreamData,
    ::UINT Stride) noexcept
{
    return m_impl->SetStreamSource(StreamNumber, pStreamData, Stride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetStreamSource(
    ::UINT StreamNumber, ::IDirect3DVertexBuffer8** ppStreamData,
    ::UINT* pStride) noexcept
{
    return m_impl->GetStreamSource(StreamNumber, ppStreamData, pStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetIndices(
    ::IDirect3DIndexBuffer8* pIndexData, ::UINT BaseVertexIndex) noexcept
{
    return m_impl->SetIndices(pIndexData, BaseVertexIndex);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetIndices(
    ::IDirect3DIndexBuffer8** ppIndexData, ::UINT* pBaseVertexIndex) noexcept
{
    return m_impl->GetIndices(ppIndexData, pBaseVertexIndex);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreatePixelShader(
    ::DWORD const* pFunction, ::DWORD* pHandle) noexcept
{
    return m_impl->CreatePixelShader(pFunction, pHandle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetPixelShader(::DWORD Handle) noexcept
{
    return m_impl->SetPixelShader(Handle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetPixelShader(::DWORD* pHandle) noexcept
{
    return m_impl->GetPixelShader(pHandle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeletePixelShader(::DWORD Handle) noexcept
{
    return m_impl->DeletePixelShader(Handle);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetPixelShaderConstant(
    ::DWORD Register, void const* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->SetPixelShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPixelShaderConstant(
    ::DWORD Register, void* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->GetPixelShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPixelShaderFunction(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetPixelShaderFunction(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawRectPatch(
    ::UINT Handle, float const* pNumSegs,
    ::D3DRECTPATCH_INFO const* pRectPatchInfo) noexcept
{
    return m_impl->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawTriPatch(
    ::UINT Handle, float const* pNumSegs,
    ::D3DTRIPATCH_INFO const* pTriPatchInfo) noexcept
{
    return m_impl->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeletePatch(::UINT Handle) noexcept
{
    return m_impl->DeletePatch(Handle);
}
