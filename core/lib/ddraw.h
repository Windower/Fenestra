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

#ifndef WINDOWER_DDRAW_H
#define WINDOWER_DDRAW_H

#include <windows.h>

#include <unknwn.h>

::GUID const IID_IDirectDraw  = {0x6C14DB80, 0xA733, 0x11CE, 0xA5, 0x21, 0x00,
                                 0x20,       0xAF,   0x0B,   0xE5, 0x60};
::GUID const IID_IDirectDraw7 = {0x15E65EC0, 0x3B9C, 0x11D2, 0xB9, 0x2F, 0x00,
                                 0x60,       0x97,   0x97,   0xEA, 0x5B};
::GUID const IID_IDirectDrawSurface = {
    0x6C14DB81, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60};
::GUID const IID_IDirectDrawSurface7 = {
    0x06675A80, 0x3B9B, 0x11D2, 0xB9, 0x2F, 0x00, 0x60, 0x97, 0x97, 0xEA, 0x5B};
::GUID const IID_IDirectDrawPalette = {
    0x6C14DB84, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60};
::GUID const IID_IDirectDrawClipper = {
    0x6C14DB85, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60};

#define DD_ROP_SPACE 8

#define MAX_DDDEVICEID_STRING 512

#define DDSCAPS_RESERVED1 0x00000001
#define DDSCAPS_ALPHA 0x00000002
#define DDSCAPS_BACKBUFFER 0x00000004
#define DDSCAPS_COMPLEX 0x00000008
#define DDSCAPS_FLIP 0x00000010
#define DDSCAPS_FRONTBUFFER 0x00000020
#define DDSCAPS_OFFSCREENPLAIN 0x00000040
#define DDSCAPS_OVERLAY 0x00000080
#define DDSCAPS_PALETTE 0x00000100
#define DDSCAPS_PRIMARYSURFACE 0x00000200
#define DDSCAPS_PRIMARYSURFACELEFT 0x00000400
#define DDSCAPS_SYSTEMMEMORY 0x00000800
#define DDSCAPS_TEXTURE 0x00001000
#define DDSCAPS_3DDEVICE 0x00002000
#define DDSCAPS_VIDEOMEMORY 0x00004000
#define DDSCAPS_VISIBLE 0x00008000
#define DDSCAPS_WRITEONLY 0x00010000
#define DDSCAPS_ZBUFFER 0x00020000
#define DDSCAPS_OWNDC 0x00040000
#define DDSCAPS_LIVEVIDEO 0x00080000
#define DDSCAPS_HWCODEC 0x00100000
#define DDSCAPS_MODEX 0x00200000
#define DDSCAPS_MIPMAP 0x00400000
#define DDSCAPS_RESERVED2 0x00800000
#define DDSCAPS_ALLOCONLOAD 0x04000000
#define DDSCAPS_VIDEOPORT 0x08000000
#define DDSCAPS_LOCALVIDMEM 0x10000000
#define DDSCAPS_NONLOCALVIDMEM 0x20000000
#define DDSCAPS_STANDARDVGAMODE 0x40000000
#define DDSCAPS_OPTIMIZED 0x80000000

struct IDirectDraw;
struct IDirectDraw7;
struct IDirectDrawClipper;
struct IDirectDrawPalette;
struct IDirectDrawSurface;
struct IDirectDrawSurface7;

struct DDCOLORKEY
{
    ::DWORD dwColorSpaceLowValue;
    ::DWORD dwColorSpaceHighValue;
};

using LPDDCOLORKEY = ::DDCOLORKEY*;

struct DDPIXELFORMAT
{
    ::DWORD dwSize;
    ::DWORD dwFlags;
    ::DWORD dwFourCC;
    union
    {
        ::DWORD dwRGBBitCount;
        ::DWORD dwYUVBitCount;
        ::DWORD dwZBufferBitDepth;
        ::DWORD dwAlphaBitDepth;
        ::DWORD dwLuminanceBitCount;
        ::DWORD dwBumpBitCount;
    };
    union
    {
        ::DWORD dwRBitMask;
        ::DWORD dwYBitMask;
        ::DWORD dwStencilBitDepth;
        ::DWORD dwLuminanceBitMask;
        ::DWORD dwBumpDuBitMask;
    };
    union
    {
        ::DWORD dwGBitMask;
        ::DWORD dwUBitMask;
        ::DWORD dwZBitMask;
        ::DWORD dwBumpDvBitMask;
    };
    union
    {
        ::DWORD dwBBitMask;
        ::DWORD dwVBitMask;
        ::DWORD dwStencilBitMask;
        ::DWORD dwBumpLuminanceBitMask;
    };
    union
    {
        ::DWORD dwRGBAlphaBitMask;
        ::DWORD dwYUVAlphaBitMask;
        ::DWORD dwLuminanceAlphaBitMask;
        ::DWORD dwRGBZBitMask;
        ::DWORD dwYUVZBitMask;
    };
};

using LPDDPIXELFORMAT = ::DDPIXELFORMAT*;

struct DDSCAPS
{
    ::DWORD dwCaps;
};

using LPDDSCAPS = ::DDSCAPS*;

struct DDSCAPS2
{
    ::DWORD dwCaps;
    ::DWORD dwCaps2;
    ::DWORD dwCaps3;
    union
    {
        ::DWORD dwCaps4;
        ::DWORD dwVolumeDepth;
    };
};

using LPDDSCAPS2 = ::DDSCAPS2*;

struct DDSURFACEDESC
{
    ::DWORD dwSize;
    ::DWORD dwFlags;
    ::DWORD dwHeight;
    ::DWORD dwWidth;
    union
    {
        ::LONG lPitch;
        ::DWORD dwLinearSize;
    };
    ::DWORD dwBackBufferCount;
    union
    {
        ::DWORD dwMipMapCount;
        ::DWORD dwZBufferBitDepth;
        ::DWORD dwRefreshRate;
    };
    ::DWORD dwAlphaBitDepth;
    ::DWORD dwReserved;
    ::LPVOID lpSurface;
    ::DDCOLORKEY ddckCKDestOverlay;
    ::DDCOLORKEY ddckCKDestBlt;
    ::DDCOLORKEY ddckCKSrcOverlay;
    ::DDCOLORKEY ddckCKSrcBlt;
    ::DDPIXELFORMAT ddpfPixelFormat;
    ::DDSCAPS ddsCaps;
};

using LPDDSURFACEDESC = ::DDSURFACEDESC*;

struct DDSURFACEDESC2
{
    ::DWORD dwSize;
    ::DWORD dwFlags;
    ::DWORD dwHeight;
    ::DWORD dwWidth;
    union
    {
        ::LONG lPitch;
        ::DWORD dwLinearSize;
    };
    union
    {
        ::DWORD dwBackBufferCount;
        ::DWORD dwDepth;
    };
    union
    {
        ::DWORD dwMipMapCount;
        ::DWORD dwZBufferBitDepth;
        ::DWORD dwRefreshRate;
    };
    ::DWORD dwAlphaBitDepth;
    ::DWORD dwReserved;
    ::LPVOID lpSurface;
    union
    {
        ::DDCOLORKEY ddckCKDestOverlay;
        ::DWORD dwEmptyFaceColor;
    };
    ::DDCOLORKEY ddckCKDestBlt;
    ::DDCOLORKEY ddckCKSrcOverlay;
    ::DDCOLORKEY ddckCKSrcBlt;
    union
    {
        ::DDPIXELFORMAT ddpfPixelFormat;
        ::DWORD dwFVF;
    };
    ::DDSCAPS2 ddsCaps;
    ::DWORD dwTextureStage;
};

using LPDDSURFACEDESC2 = ::DDSURFACEDESC2*;

struct DDCAPS_DX7
{
    ::DWORD dwSize;
    ::DWORD dwCaps;
    ::DWORD dwCaps2;
    ::DWORD dwCKeyCaps;
    ::DWORD dwFXCaps;
    ::DWORD dwFXAlphaCaps;
    ::DWORD dwPalCaps;
    ::DWORD dwSVCaps;
    ::DWORD dwAlphaBltConstBitDepths;
    ::DWORD dwAlphaBltPixelBitDepths;
    ::DWORD dwAlphaBltSurfaceBitDepths;
    ::DWORD dwAlphaOverlayConstBitDepths;
    ::DWORD dwAlphaOverlayPixelBitDepths;
    ::DWORD dwAlphaOverlaySurfaceBitDepths;
    ::DWORD dwZBufferBitDepths;
    ::DWORD dwVidMemTotal;
    ::DWORD dwVidMemFree;
    ::DWORD dwMaxVisibleOverlays;
    ::DWORD dwCurrVisibleOverlays;
    ::DWORD dwNumFourCCCodes;
    ::DWORD dwAlignBoundarySrc;
    ::DWORD dwAlignSizeSrc;
    ::DWORD dwAlignBoundaryDest;
    ::DWORD dwAlignSizeDest;
    ::DWORD dwAlignStrideAlign;
    ::DWORD dwRops[DD_ROP_SPACE];
    ::DDSCAPS ddsOldCaps;
    ::DWORD dwMinOverlayStretch;
    ::DWORD dwMaxOverlayStretch;
    ::DWORD dwMinLiveVideoStretch;
    ::DWORD dwMaxLiveVideoStretch;
    ::DWORD dwMinHwCodecStretch;
    ::DWORD dwMaxHwCodecStretch;
    ::DWORD dwReserved1;
    ::DWORD dwReserved2;
    ::DWORD dwReserved3;
    ::DWORD dwSVBCaps;
    ::DWORD dwSVBCKeyCaps;
    ::DWORD dwSVBFXCaps;
    ::DWORD dwSVBRops[DD_ROP_SPACE];
    ::DWORD dwVSBCaps;
    ::DWORD dwVSBCKeyCaps;
    ::DWORD dwVSBFXCaps;
    ::DWORD dwVSBRops[DD_ROP_SPACE];
    ::DWORD dwSSBCaps;
    ::DWORD dwSSBCKeyCaps;
    ::DWORD dwSSBFXCaps;
    ::DWORD dwSSBRops[DD_ROP_SPACE];
    ::DWORD dwMaxVideoPorts;
    ::DWORD dwCurrVideoPorts;
    ::DWORD dwSVBCaps2;
    ::DWORD dwNLVBCaps;
    ::DWORD dwNLVBCaps2;
    ::DWORD dwNLVBCKeyCaps;
    ::DWORD dwNLVBFXCaps;
    ::DWORD dwNLVBRops[DD_ROP_SPACE];
    ::DDSCAPS2 ddsCaps;
};

using LPDDCAPS_DX7 = ::DDCAPS_DX7*;
using DDCAPS       = ::DDCAPS_DX7;
using LPDDCAPS     = ::DDCAPS*;

struct DDDEVICEIDENTIFIER2
{
    char szDriver[MAX_DDDEVICEID_STRING];
    char szDescription[MAX_DDDEVICEID_STRING];
    ::LARGE_INTEGER liDriverVersion;
    ::DWORD dwVendorId;
    ::DWORD dwDeviceId;
    ::DWORD dwSubSysId;
    ::DWORD dwRevision;
    ::GUID guidDeviceIdentifier;
    ::DWORD dwWHQLLevel;
};

using LPDDDEVICEIDENTIFIER2 = ::DDDEVICEIDENTIFIER2*;

struct DDBLTFX
{
    ::DWORD dwSize;
    ::DWORD dwDDFX;
    ::DWORD dwROP;
    ::DWORD dwDDROP;
    ::DWORD dwRotationAngle;
    ::DWORD dwZBufferOpCode;
    ::DWORD dwZBufferLow;
    ::DWORD dwZBufferHigh;
    ::DWORD dwZBufferBaseDest;
    ::DWORD dwZDestConstBitDepth;
    union
    {
        ::DWORD dwZDestConst;
        ::IDirectDrawSurface* lpDDSZBufferDest;
    };
    ::DWORD dwZSrcConstBitDepth;
    union
    {
        ::DWORD dwZSrcConst;
        ::IDirectDrawSurface* lpDDSZBufferSrc;
    };
    ::DWORD dwAlphaEdgeBlendBitDepth;
    ::DWORD dwAlphaEdgeBlend;
    ::DWORD dwReserved;
    ::DWORD dwAlphaDestConstBitDepth;
    union
    {
        ::DWORD dwAlphaDestConst;
        ::IDirectDrawSurface* lpDDSAlphaDest;
    };
    ::DWORD dwAlphaSrcConstBitDepth;
    union
    {
        ::DWORD dwAlphaSrcConst;
        ::IDirectDrawSurface* lpDDSAlphaSrc;
    };
    union
    {
        ::DWORD dwFillColor;
        ::DWORD dwFillDepth;
        ::DWORD dwFillPixel;
        ::IDirectDrawSurface* lpDDSPattern;
    };
    ::DDCOLORKEY ddckDestColorkey;
    ::DDCOLORKEY ddckSrcColorkey;
};

using LPDDBLTFX = ::DDBLTFX*;

struct DDBLTBATCH
{
    ::RECT* lprDest;
    ::IDirectDrawSurface* lpDDSSrc;
    ::RECT* lprSrc;
    ::DWORD dwFlags;
    ::DDBLTFX* lpDDBltFx;
};

using LPDDBLTBATCH = ::DDBLTBATCH*;

struct DDOVERLAYFX
{
    ::DWORD dwSize;
    ::DWORD dwAlphaEdgeBlendBitDepth;
    ::DWORD dwAlphaEdgeBlend;
    ::DWORD dwReserved;
    ::DWORD dwAlphaDestConstBitDepth;
    union
    {
        ::DWORD dwAlphaDestConst;
        ::IDirectDrawSurface* lpDDSAlphaDest;
    };
    ::DWORD dwAlphaSrcConstBitDepth;
    union
    {
        ::DWORD dwAlphaSrcConst;
        ::IDirectDrawSurface* lpDDSAlphaSrc;
    };
    ::DDCOLORKEY dckDestColorkey;
    ::DDCOLORKEY dckSrcColorkey;
    ::DWORD dwDDFX;
    ::DWORD dwFlags;
};

using LPDDOVERLAYFX = ::DDOVERLAYFX*;

using LPDDENUMMODESCALLBACK  = ::HRESULT(CALLBACK*)(::DDSURFACEDESC*, void*);
using LPDDENUMMODESCALLBACK2 = ::HRESULT(CALLBACK*)(::DDSURFACEDESC2*, void*);
using LPDDENUMSURFACESCALLBACK =
    ::HRESULT(CALLBACK*)(::IDirectDrawSurface*, ::DDSURFACEDESC*, void*);
using LPDDENUMSURFACESCALLBACK7 =
    ::HRESULT(CALLBACK*)(::IDirectDrawSurface7*, ::DDSURFACEDESC2*, void*);

struct DECLSPEC_NOVTABLE IDirectDraw : public ::IUnknown
{
    DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW::ULONG STDMETHODCALLTYPE AddRef() override   = 0;
    DECLSPEC_NOTHROW::ULONG STDMETHODCALLTYPE Release() override  = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE Compact() = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    CreateClipper(::DWORD, ::IDirectDrawClipper**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE CreatePalette(
        ::DWORD, ::PALETTEENTRY*, ::IDirectDrawPalette**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    CreateSurface(::DDSURFACEDESC*, ::IDirectDrawSurface**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    DuplicateSurface(::IDirectDrawSurface*, ::IDirectDrawSurface**) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE EnumDisplayModes(
        ::DWORD, ::DDSURFACEDESC*, void*, ::LPDDENUMMODESCALLBACK) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE EnumSurfaces(
        ::DWORD, ::DDSURFACEDESC*, void*, ::LPDDENUMSURFACESCALLBACK)      = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE FlipToGDISurface() = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetCaps(::DDCAPS*, ::DDCAPS*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetDisplayMode(::DDSURFACEDESC*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetFourCCCodes(::DWORD*, ::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetGDISurface(::IDirectDrawSurface**) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetMonitorFrequency(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetScanLine(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    GetVerticalBlankStatus(::BOOL*)                                         = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE Initialize(::GUID*) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    RestoreDisplayMode() = 0;
    virtual DECLSPEC_NOTHROW::HRESULT
        STDMETHODCALLTYPE SetCooperativeLevel(::HWND, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT
        STDMETHODCALLTYPE SetDisplayMode(::DWORD, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW::HRESULT
        STDMETHODCALLTYPE WaitForVerticalBlank(::DWORD, ::HANDLE) = 0;
};

using LPDIRECTDRAW = ::IDirectDraw*;

struct DECLSPEC_NOVTABLE IDirectDraw7 : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override   = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override  = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Compact() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    CreateClipper(::DWORD, ::IDirectDrawClipper**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE CreatePalette(
        ::DWORD, ::PALETTEENTRY*, ::IDirectDrawPalette**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    CreateSurface(::DDSURFACEDESC2*, ::IDirectDrawSurface7**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    DuplicateSurface(::IDirectDrawSurface7*, ::IDirectDrawSurface7**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE EnumDisplayModes(
        ::DWORD, ::DDSURFACEDESC2*, void*, ::LPDDENUMMODESCALLBACK2) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE EnumSurfaces(
        ::DWORD, ::DDSURFACEDESC2*, void*, ::LPDDENUMSURFACESCALLBACK7)     = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE FlipToGDISurface() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCaps(::DDCAPS*, ::DDCAPS*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDisplayMode(::DDSURFACEDESC2*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetFourCCCodes(::DWORD*, ::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetGDISurface(::IDirectDrawSurface7**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetMonitorFrequency(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetScanLine(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetVerticalBlankStatus(::BOOL*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::GUID*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    RestoreDisplayMode() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetCooperativeLevel(::HWND, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
        SetDisplayMode(::DWORD, ::DWORD, ::DWORD, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE WaitForVerticalBlank(::DWORD, ::HANDLE) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetAvailableVidMem(::DDSCAPS2*, ::DWORD*, ::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetSurfaceFromDC(::HDC, ::IDirectDrawSurface7**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    RestoreAllSurfaces() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    TestCooperativeLevel() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDeviceIdentifier(::DDDEVICEIDENTIFIER2*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    StartModeTest(::SIZE*, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EvaluateMode(::DWORD, ::DWORD*) = 0;
};

using LPDIRECTDRAW7 = ::IDirectDraw7*;

struct DECLSPEC_NOVTABLE IDirectDrawClipper : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetClipList(::RECT*, ::RGNDATA*, ::DWORD*)                            = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE GetHWnd(::HWND*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::IDirectDraw* lpDD, ::DWORD dwFlags) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    IsClipListChanged(::BOOL*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetClipList(::RGNDATA*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetHWnd(::DWORD, ::HWND) = 0;
};

using LPDIRECTDRAWCLIPPER = ::IDirectDrawClipper*;

struct DECLSPEC_NOVTABLE IDirectDrawPalette : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                          = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override           = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override          = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE GetCaps(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetEntries(::DWORD, ::DWORD, ::DWORD, ::PALETTEENTRY*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::IDirectDraw*, ::DWORD, ::PALETTEENTRY*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetEntries(::DWORD, ::DWORD, ::DWORD, ::PALETTEENTRY*) = 0;
};

using LPDIRECTDRAWPALETTE = ::IDirectDrawPalette*;

struct DECLSPEC_NOVTABLE IDirectDrawSurface : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;

    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    AddAttachedSurface(::IDirectDrawSurface*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    AddOverlayDirtyRect(::RECT*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Blt(::RECT*, ::IDirectDrawSurface*, ::RECT*, ::DWORD, ::DDBLTFX*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    BltBatch(::DDBLTBATCH*, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    BltFast(::DWORD, ::DWORD, ::IDirectDrawSurface*, ::RECT*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    DeleteAttachedSurface(::DWORD, ::IDirectDrawSurface*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumAttachedSurfaces(void*, ::LPDDENUMSURFACESCALLBACK) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumOverlayZOrders(::DWORD, void*, ::LPDDENUMSURFACESCALLBACK) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Flip(::IDirectDrawSurface*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetAttachedSurface(::DDSCAPS*, ::IDirectDrawSurface**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE GetBltStatus(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCaps(::DDSCAPS*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCaps(::IDirectDrawClipper**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetColorKey(::DWORD, ::DDCOLORKEY*)                                = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE GetDC(::HDC*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE GetFlipStatus(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetOverlayPosition(::LONG*, ::LONG*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPalette(::IDirectDrawPalette**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPixelFormat(::DDPIXELFORMAT*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetSurfaceDesc(::DDSURFACEDESC*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::IDirectDraw*, ::DDSURFACEDESC*)                  = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE IsLost() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Lock(::RECT*, ::DDSURFACEDESC*, ::DWORD, ::HANDLE)                    = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE ReleaseDC(::HDC) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Restore()        = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetClipper(::IDirectDrawClipper*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetColorKey(::DWORD, ::DDCOLORKEY*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetOverlayPosition(::LONG, ::LONG) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetPalette(::IDirectDrawPalette*)                                  = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Unlock(void*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE UpdateOverlay(
        ::RECT*, ::IDirectDrawSurface*, ::RECT*, ::DWORD, ::DDOVERLAYFX*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE UpdateOverlayDisplay(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    UpdateOverlayZOrder(::DWORD, ::IDirectDrawSurface*) = 0;
};

using LPDIRECTDRAWSURFACE = ::IDirectDrawSurface*;

struct DECLSPEC_NOVTABLE IDirectDrawSurface7 : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;

    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    AddAttachedSurface(::IDirectDrawSurface7*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    AddOverlayDirtyRect(::RECT*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Blt(::RECT*, ::IDirectDrawSurface7*, ::RECT*, ::DWORD, ::DDBLTFX*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    BltBatch(::DDBLTBATCH*, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE BltFast(
        ::DWORD, ::DWORD, ::IDirectDrawSurface7*, ::RECT*, ::DWORD,
        ::DDBLTFX*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    DeleteAttachedSurface(::DWORD, ::IDirectDrawSurface7*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumAttachedSurfaces(void*, ::LPDDENUMSURFACESCALLBACK7) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumOverlayZOrders(::DWORD, void*, ::LPDDENUMSURFACESCALLBACK7) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Flip(::IDirectDrawSurface7*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetAttachedSurface(::DDSCAPS2*, ::IDirectDrawSurface7**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE GetBltStatus(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCaps(::DDSCAPS2*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCaps(::IDirectDrawClipper**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetColorKey(::DWORD, ::DDCOLORKEY*)                                = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE GetDC(::HDC*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE GetFlipStatus(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetOverlayPosition(::LONG*, ::LONG*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPalette(::IDirectDrawPalette**) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPixelFormat(::DDPIXELFORMAT*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetSurfaceDesc(::DDSURFACEDESC2*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::IDirectDraw*, ::DDSURFACEDESC2*)                 = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE IsLost() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Lock(::RECT*, ::DDSURFACEDESC2*, ::DWORD, ::HANDLE)                   = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE ReleaseDC(::HDC) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Restore()        = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetClipper(::IDirectDrawClipper*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetColorKey(::DWORD, ::DDCOLORKEY*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetOverlayPosition(::LONG, ::LONG) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetPalette(::IDirectDrawPalette*)                                    = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Unlock(::RECT*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE UpdateOverlay(
        ::RECT*, ::IDirectDrawSurface7*, ::RECT*, ::DWORD, ::DDOVERLAYFX*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE UpdateOverlayDisplay(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    UpdateOverlayZOrder(::DWORD, ::IDirectDrawSurface7*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDDInterface(void**)                                                 = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE PageLock(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE PageUnlock(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetSurfaceDesc(::DDSURFACEDESC2*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetPrivateData(::GUID const&, void*, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPrivateData(::GUID const&, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    FreePrivateData(::GUID const&) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetUniquenessValue(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    ChangeUniquenessValue() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetPriority(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetPriority(::DWORD*)                                                 = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE SetLOD(::DWORD)  = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE GetLOD(::DWORD*) = 0;
};

using LPDIRECTDRAWSURFACE7 = ::IDirectDrawSurface7*;

::HRESULT WINAPI
DirectDrawCreateEx(::GUID*, ::LPVOID*, ::IID const&, ::IUnknown*);

#endif
