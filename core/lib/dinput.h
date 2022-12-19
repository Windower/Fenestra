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

#ifndef WINDOWER_DINPUT_H
#define WINDOWER_DINPUT_H

#include <windows.h>

#include <unknwn.h>

::GUID const IID_IDirectInput8 = {0xBF798030, 0x483A, 0x4DA2, 0xAA, 0x99, 0x5D,
                                  0x64,       0xED,   0x36,   0x97, 0x00};
::GUID const GUID_SysKeyboard  = {0x6F1D2B61, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44,
                                  0x45,       0x53,   0x54,   0x00, 0x00};

#define DI_OK S_OK
#define DI_NOTATTACHED S_FALSE
#define DI_BUFFEROVERFLOW S_FALSE
#define DI_PROPNOEFFECT S_FALSE
#define DI_NOEFFECT S_FALSE
#define DI_POLLEDDEVICE ::HRESULT(0x00000002L)
#define DI_DOWNLOADSKIPPED ::HRESULT(0x00000003L)
#define DI_EFFECTRESTARTED ::HRESULT(0x00000004L)
#define DI_TRUNCATED ::HRESULT(0x00000008L)
#define DI_SETTINGSNOTSAVED ::HRESULT(0x0000000BL)
#define DI_TRUNCATEDANDRESTARTED ::HRESULT(0x0000000CL)
#define DI_WRITEPROTECT ::HRESULT(0x00000013L)

#define DIERR_OLDDIRECTINPUTVERSION                                            \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_OLD_WIN_VERSION)
#define DIERR_BETADIRECTINPUTVERSION                                           \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_RMODE_APP)
#define DIERR_BADDRIVERVER                                                     \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_BAD_DRIVER_LEVEL)
#define DIERR_DEVICENOTREG REGDB_E_CLASSNOTREG
#define DIERR_NOTFOUND                                                         \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)
#define DIERR_OBJECTNOTFOUND                                                   \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)
#define DIERR_INVALIDPARAM E_INVALIDARG
#define DIERR_NOINTERFACE E_NOINTERFACE
#define DIERR_GENERIC E_FAIL
#define DIERR_OUTOFMEMORY E_OUTOFMEMORY
#define DIERR_UNSUPPORTED E_NOTIMPL
#define DIERR_NOTINITIALIZED                                                   \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_NOT_READY)
#define DIERR_ALREADYINITIALIZED                                               \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_ALREADY_INITIALIZED)
#define DIERR_NOAGGREGATION CLASS_E_NOAGGREGATION
#define DIERR_OTHERAPPHASPRIO E_ACCESSDENIED
#define DIERR_INPUTLOST                                                        \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_READ_FAULT)
#define DIERR_ACQUIRED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_BUSY)
#define DIERR_NOTACQUIRED                                                      \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INVALID_ACCESS)
#define DIERR_READONLY E_ACCESSDENIED
#define DIERR_HANDLEEXISTS E_ACCESSDENIED
#define DIERR_INSUFFICIENTPRIVS 0x80040200L
#define DIERR_DEVICEFULL 0x80040201L
#define DIERR_MOREDATA 0x80040202L
#define DIERR_NOTDOWNLOADED 0x80040203L
#define DIERR_HASEFFECTS 0x80040204L
#define DIERR_NOTEXCLUSIVEACQUIRED 0x80040205L
#define DIERR_INCOMPLETEEFFECT 0x80040206L
#define DIERR_NOTBUFFERED 0x80040207L
#define DIERR_EFFECTPLAYING 0x80040208L
#define DIERR_UNPLUGGED 0x80040209L
#define DIERR_REPORTFULL 0x8004020AL
#define DIERR_MAPFILEFAIL 0x8004020BL

#define MAKEDIPROP(prop) (*reinterpret_cast<::GUID const*>(prop))

#define DIPROP_BUFFERSIZE MAKEDIPROP(1)
#define DIPROP_AXISMODE MAKEDIPROP(2)
#define DIPROP_GRANULARITY MAKEDIPROP(3)
#define DIPROP_RANGE MAKEDIPROP(4)
#define DIPROP_DEADZONE MAKEDIPROP(5)
#define DIPROP_SATURATION MAKEDIPROP(6)
#define DIPROP_FFGAIN MAKEDIPROP(7)
#define DIPROP_FFLOAD MAKEDIPROP(8)
#define DIPROP_AUTOCENTER MAKEDIPROP(9)
#define DIPROP_CALIBRATIONMODE MAKEDIPROP(10)
#define DIPROP_CALIBRATION MAKEDIPROP(11)
#define DIPROP_GUIDANDPATH MAKEDIPROP(12)
#define DIPROP_INSTANCENAME MAKEDIPROP(13)
#define DIPROP_PRODUCTNAME MAKEDIPROP(14)
#define DIPROP_JOYSTICKID MAKEDIPROP(15)
#define DIPROP_GETPORTDISPLAYNAME MAKEDIPROP(16)
#define DIPROP_PHYSICALRANGE MAKEDIPROP(18)
#define DIPROP_LOGICALRANGE MAKEDIPROP(19)
#define DIPROP_KEYNAME MAKEDIPROP(20)
#define DIPROP_CPOINTS MAKEDIPROP(21)
#define DIPROP_APPDATA MAKEDIPROP(22)
#define DIPROP_SCANCODE MAKEDIPROP(23)
#define DIPROP_VIDPID MAKEDIPROP(24)
#define DIPROP_USERNAME MAKEDIPROP(25)
#define DIPROP_TYPENAME MAKEDIPROP(26)

#define DIPROPAXISMODE_ABS 0
#define DIPROPAXISMODE_REL 1

#define DIPROPAUTOCENTER_OFF 0
#define DIPROPAUTOCENTER_ON 1

#define DIPROPCALIBRATIONMODE_COOKED 0
#define DIPROPCALIBRATIONMODE_RAW 1

#define DIPH_DEVICE 0
#define DIPH_BYOFFSET 1
#define DIPH_BYID 2
#define DIPH_BYUSAGE 3

struct IDirectInput8A;
struct IDirectInputDevice8A;
struct IDirectInputEffect;

#ifndef D3DCOLOR_DEFINED
#    define D3DCOLOR_DEFINED
using D3DCOLOR = ::DWORD;
#endif

struct DIDEVICEINSTANCEA
{
    ::DWORD dwSize;
    ::GUID guidInstance;
    ::GUID guidProduct;
    ::DWORD dwDevType;
    ::CHAR tszInstanceName[MAX_PATH];
    ::CHAR tszProductName[MAX_PATH];
    ::GUID guidFFDriver;
    ::WORD wUsagePage;
    ::WORD wUsage;
};

using LPDIDEVICEINSTANCEA  = ::DIDEVICEINSTANCEA*;
using LPCDIDEVICEINSTANCEA = ::DIDEVICEINSTANCEA const*;

struct DIACTIONA
{
    ::UINT_PTR uAppData;
    ::DWORD dwSemantic;
    ::DWORD dwFlags;
    union
    {
        ::CHAR const* lptszActionName;
        ::UINT uResIdString;
    };
    ::GUID guidInstance;
    ::DWORD dwObjID;
    ::DWORD dwHow;
};

using LPDIACTIONA  = ::DIACTIONA*;
using LPCDIACTIONA = ::DIACTIONA const*;

struct DIACTIONFORMATA
{
    ::DWORD dwSize;
    ::DWORD dwActionSize;
    ::DWORD dwDataSize;
    ::DWORD dwNumActions;
    ::DIACTIONA* rgoAction;
    ::GUID guidActionMap;
    ::DWORD dwGenre;
    ::DWORD dwBufferSize;
    ::LONG lAxisMin;
    ::LONG lAxisMax;
    ::HINSTANCE hInstString;
    ::FILETIME ftTimeStamp;
    ::DWORD dwCRC;
    ::CHAR tszActionMap[MAX_PATH];
};

using LPDIACTIONFORMATA  = ::DIACTIONFORMATA*;
using LPCDIACTIONFORMATA = ::DIACTIONFORMATA const*;

struct DICOLORSET
{
    ::DWORD dwSize;
    ::D3DCOLOR cTextFore;
    ::D3DCOLOR cTextHighlight;
    ::D3DCOLOR cCalloutLine;
    ::D3DCOLOR cCalloutHighlight;
    ::D3DCOLOR cBorder;
    ::D3DCOLOR cControlFill;
    ::D3DCOLOR cHighlightFill;
    ::D3DCOLOR cAreaFill;
};

using LPDICOLORSET  = ::DICOLORSET*;
using LPCDICOLORSET = ::DICOLORSET const*;

struct DICONFIGUREDEVICESPARAMSA
{
    ::DWORD dwSize;
    ::DWORD dwcUsers;
    ::CHAR* lptszUserNames;
    ::DWORD dwcFormats;
    ::DIACTIONFORMATA* lprgFormats;
    ::HWND hwnd;
    ::DICOLORSET dics;
    ::IUnknown* lpUnkDDSTarget;
};

using LPDICONFIGUREDEVICESPARAMSA  = ::DICONFIGUREDEVICESPARAMSA*;
using LPCDICONFIGUREDEVICESPARAMSA = ::DICONFIGUREDEVICESPARAMSA const*;

struct DIDEVCAPS
{
    ::DWORD dwSize;
    ::DWORD dwFlags;
    ::DWORD dwDevType;
    ::DWORD dwAxes;
    ::DWORD dwButtons;
    ::DWORD dwPOVs;
    ::DWORD dwFFSamplePeriod;
    ::DWORD dwFFMinTimeResolution;
    ::DWORD dwFirmwareRevision;
    ::DWORD dwHardwareRevision;
    ::DWORD dwFFDriverVersion;
};

using LPDIDEVCAPS = ::DIDEVCAPS*;

struct DIDEVICEOBJECTINSTANCEA
{
    ::DWORD dwSize;
    ::GUID guidType;
    ::DWORD dwOfs;
    ::DWORD dwType;
    ::DWORD dwFlags;
    ::CHAR tszName[MAX_PATH];
    ::DWORD dwFFMaxForce;
    ::DWORD dwFFForceResolution;
    ::WORD wCollectionNumber;
    ::WORD wDesignatorIndex;
    ::WORD wUsagePage;
    ::WORD wUsage;
    ::DWORD dwDimension;
    ::WORD wExponent;
    ::WORD wReportId;
};

using LPDIDEVICEOBJECTINSTANCEA  = ::DIDEVICEOBJECTINSTANCEA*;
using LPCDIDEVICEOBJECTINSTANCEA = ::DIDEVICEOBJECTINSTANCEA const*;

struct DIPROPHEADER
{
    ::DWORD dwSize;
    ::DWORD dwHeaderSize;
    ::DWORD dwObj;
    ::DWORD dwHow;
};

using LPDIPROPHEADER  = ::DIPROPHEADER*;
using LPCDIPROPHEADER = ::DIPROPHEADER const*;

struct DIPROPDWORD
{
    ::DIPROPHEADER diph;
    ::DWORD dwData;
};

using LPDIPROPDWORD  = ::DIPROPDWORD*;
using LPCDIPROPDWORD = ::DIPROPDWORD const*;

struct DIDEVICEOBJECTDATA
{
    ::DWORD dwOfs;
    ::DWORD dwData;
    ::DWORD dwTimeStamp;
    ::DWORD dwSequence;
    ::UINT_PTR uAppData;
};

using LPDIDEVICEOBJECTDATA  = ::DIDEVICEOBJECTDATA*;
using LPCDIDEVICEOBJECTDATA = ::DIDEVICEOBJECTDATA const*;

struct DIOBJECTDATAFORMAT
{
    ::GUID const* pguid;
    ::DWORD dwOfs;
    ::DWORD dwType;
    ::DWORD dwFlags;
};

using LPDIOBJECTDATAFORMAT  = ::DIOBJECTDATAFORMAT*;
using LPCDIOBJECTDATAFORMAT = ::DIOBJECTDATAFORMAT const*;

struct DIDATAFORMAT
{
    ::DWORD dwSize;
    ::DWORD dwObjSize;
    ::DWORD dwFlags;
    ::DWORD dwDataSize;
    ::DWORD dwNumObjs;
    ::DIOBJECTDATAFORMAT* rgodf;
};

using LPDIDATAFORMAT  = ::DIDATAFORMAT*;
using LPCDIDATAFORMAT = ::DIDATAFORMAT const*;

struct DIENVELOPE
{
    ::DWORD dwSize;
    ::DWORD dwAttackLevel;
    ::DWORD dwAttackTime;
    ::DWORD dwFadeLevel;
    ::DWORD dwFadeTime;
};

using LPDIENVELOPE  = ::DIENVELOPE*;
using LPCDIENVELOPE = ::DIENVELOPE const*;

struct DIEFFECT
{
    ::DWORD dwSize;
    ::DWORD dwFlags;
    ::DWORD dwDuration;
    ::DWORD dwSamplePeriod;
    ::DWORD dwGain;
    ::DWORD dwTriggerButton;
    ::DWORD dwTriggerRepeatInterval;
    ::DWORD cAxes;
    ::DWORD* rgdwAxes;
    ::LONG* rglDirection;
    ::DIENVELOPE* lpEnvelope;
    ::DWORD cbTypeSpecificParams;
    void* lpvTypeSpecificParams;
    ::DWORD dwStartDelay;
};

using LPDIEFFECT  = ::DIEFFECT*;
using LPCDIEFFECT = ::DIEFFECT const*;

struct DIEFFESCAPE
{
    ::DWORD dwSize;
    ::DWORD dwCommand;
    void* lpvInBuffer;
    ::DWORD cbInBuffer;
    void* lpvOutBuffer;
    ::DWORD cbOutBuffer;
};

using LPDIEFFESCAPE = ::DIEFFESCAPE*;

struct DIEFFECTINFOA
{
    ::DWORD dwSize;
    ::GUID guid;
    ::DWORD dwEffType;
    ::DWORD dwStaticParams;
    ::DWORD dwDynamicParams;
    ::CHAR tszName[MAX_PATH];
};

using LPDIEFFECTINFOA  = ::DIEFFECTINFOA*;
using LPCDIEFFECTINFOA = ::DIEFFECTINFOA const*;

struct DIFILEEFFECT
{
    ::DWORD dwSize;
    ::GUID GuidEffect;
    ::DIEFFECT const* lpDiEffect;
    ::CHAR szFriendlyName[MAX_PATH];
};

using LPDIFILEEFFECT  = ::DIFILEEFFECT*;
using LPCDIFILEEFFECT = ::DIFILEEFFECT const*;

struct DIDEVICEIMAGEINFOA
{
    ::CHAR tszImagePath[MAX_PATH];
    ::DWORD dwFlags;
    ::DWORD dwViewID;
    ::RECT rcOverlay;
    ::DWORD dwObjID;
    ::DWORD dwcValidPts;
    ::POINT rgptCalloutLine[5];
    ::RECT rcCalloutRect;
    ::DWORD dwTextAlign;
};

using LPDIDEVICEIMAGEINFOA  = ::DIDEVICEIMAGEINFOA*;
using LPCDIDEVICEIMAGEINFOA = ::DIDEVICEIMAGEINFOA const*;

struct DIDEVICEIMAGEINFOHEADERA
{
    ::DWORD dwSize;
    ::DWORD dwSizeImageInfo;
    ::DWORD dwcViews;
    ::DWORD dwcButtons;
    ::DWORD dwcAxes;
    ::DWORD dwcPOVs;
    ::DWORD dwBufferSize;
    ::DWORD dwBufferUsed;
    ::DIDEVICEIMAGEINFOA* lprgImageInfoArray;
};

using LPDIDEVICEIMAGEINFOHEADERA  = ::DIDEVICEIMAGEINFOHEADERA*;
using LPCDIDEVICEIMAGEINFOHEADERA = ::DIDEVICEIMAGEINFOHEADERA const*;

struct DIMOUSESTATE
{
    ::LONG lX;
    ::LONG lY;
    ::LONG lZ;
    ::BYTE rgbButtons[4];
};

using LPDIMOUSESTATE = ::DIMOUSESTATE*;

struct DIMOUSESTATE2
{
    ::LONG lX;
    ::LONG lY;
    ::LONG lZ;
    ::BYTE rgbButtons[8];
};

using LPDIMOUSESTATE2 = ::DIMOUSESTATE2*;

using LPDIENUMDEVICESCALLBACKA =
    ::BOOL(CALLBACK*)(::DIDEVICEINSTANCEA const*, void*);
using LPDIENUMDEVICESBYSEMANTICSCBA = ::BOOL(CALLBACK*)(
    ::DIDEVICEINSTANCEA const*, ::IDirectInputDevice8A*, ::DWORD, ::DWORD,
    void*);
using LPDICONFIGUREDEVICESCALLBACK = ::BOOL(CALLBACK*)(::IUnknown*, void*);
using LPDIENUMDEVICEOBJECTSCALLBACKA =
    ::BOOL(CALLBACK*)(::DIDEVICEOBJECTINSTANCEA const*, void*);
using LPDIENUMEFFECTSCALLBACKA =
    ::BOOL(CALLBACK*)(::DIEFFECTINFOA const*, void*);
using LPDIENUMCREATEDEFFECTOBJECTSCALLBACK =
    ::BOOL(CALLBACK*)(::IDirectInputEffect*, void*);
using LPDIENUMEFFECTSINFILECALLBACK =
    ::BOOL(CALLBACK*)(::DIFILEEFFECT const*, void*);

struct DECLSPEC_NOVTABLE IDirectInputEffect : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;
    virtual DECLSPEC_NOTHROW::HRESULT STDMETHODCALLTYPE
    Initialize(::HINSTANCE, ::DWORD, ::GUID const&) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetEffectGuid(::GUID*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetParameters(::DIEFFECT*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetParameters(::DIEFFECT const*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE Start(::DWORD, ::DWORD)               = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Stop() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetEffectStatus(::DWORD*)                                       = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Download() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Unload()   = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Escape(::DIEFFESCAPE*) = 0;
};

using LPDIRECTINPUTEFFECT = ::IDirectInputEffect*;

struct DECLSPEC_NOVTABLE IDirectInputDevice8A : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetCapabilities(::DIDEVCAPS*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumObjects(::LPDIENUMDEVICEOBJECTSCALLBACKA, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetProperty(::GUID const&, ::DIPROPHEADER*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetProperty(::GUID const&, ::DIPROPHEADER const*)                = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Acquire()   = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Unacquire() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDeviceState(::DWORD, void*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDeviceData(::DWORD, ::DIDEVICEOBJECTDATA*, ::DWORD*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetDataFormat(::DIDATAFORMAT const*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetEventNotification(::HANDLE) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SetCooperativeLevel(::HWND, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetObjectInfo(::DIDEVICEOBJECTINSTANCEA*, ::DWORD, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDeviceInfo(::DIDEVICEINSTANCEA*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE RunControlPanel(::HWND, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Initialize(::HINSTANCE, ::DWORD, ::GUID const&) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE CreateEffect(
        ::GUID const&, ::DIEFFECT const*, ::IDirectInputEffect**,
        ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumEffects(::LPDIENUMEFFECTSCALLBACKA, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetEffectInfo(::DIEFFECTINFOA*, ::GUID const&) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetForceFeedbackState(::DWORD*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE SendForceFeedbackCommand(::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumCreatedEffectObjects(
        ::LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    Escape(::DIEFFESCAPE*)                                      = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE Poll() = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SendDeviceData(::DWORD, ::DIDEVICEOBJECTDATA const*, ::DWORD*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE EnumEffectsInFile(
        ::CHAR const*, ::LPDIENUMEFFECTSINFILECALLBACK, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    WriteEffectToFile(::CHAR const*, ::DWORD, ::DIFILEEFFECT*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    BuildActionMap(::DIACTIONFORMATA*, ::CHAR const*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    SetActionMap(::DIACTIONFORMATA*, ::CHAR const*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetImageInfo(::DIDEVICEIMAGEINFOHEADERA*) = 0;
};

using LPDIRECTINPUTDEVICE8A = ::IDirectInputDevice8A*;

struct DECLSPEC_NOVTABLE IDirectInput8A : public ::IUnknown
{
    DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) override                 = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE AddRef() override  = 0;
    DECLSPEC_NOTHROW ::ULONG STDMETHODCALLTYPE Release() override = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    CreateDevice(::GUID const&, ::IDirectInputDevice8A**, ::IUnknown*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    EnumDevices(::DWORD, ::LPDIENUMDEVICESCALLBACKA, void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    GetDeviceStatus(::GUID const&) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE RunControlPanel(::HWND, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT
        STDMETHODCALLTYPE Initialize(::HINSTANCE, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE
    FindDevice(::GUID const&, ::CHAR const*, ::GUID*) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(
        ::CHAR const*, ::DIACTIONFORMATA*, ::LPDIENUMDEVICESBYSEMANTICSCBA,
        void*, ::DWORD) = 0;
    virtual DECLSPEC_NOTHROW ::HRESULT STDMETHODCALLTYPE ConfigureDevices(
        ::LPDICONFIGUREDEVICESCALLBACK, ::DICONFIGUREDEVICESPARAMSA*, ::DWORD,
        void*) = 0;
};

using LPDIRECTINPUT8A = ::IDirectInput8A*;

::HRESULT WINAPI
DirectInput8Create(::HINSTANCE, ::DWORD, ::IID const&, void**, ::IUnknown*);

#endif
