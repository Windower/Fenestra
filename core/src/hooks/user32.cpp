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

#include "hooks/user32.hpp"

#include "core.hpp"
#include "hooklib/hook.hpp"
#include "resource.hpp"
#include "utility.hpp"

#include <propsys.h>
#include <windows.h>

#include <propkey.h>
#include <propvarutil.h>
#include <windowsx.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <string_view>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::RegisterClassA)> RegisterClassA;
windower::hooklib::hook<decltype(::RegisterClassExW)> RegisterClassExW;
windower::hooklib::hook<decltype(::GetCursor)> GetCursor;
windower::hooklib::hook<decltype(::SetCursor)> SetCursor;
windower::hooklib::hook<decltype(::CreateDialogParamW)> CreateDialogParamW;
windower::hooklib::hook<decltype(::LoadIconA)> LoadIconA;
windower::hooklib::hook<decltype(::LoadIconW)> LoadIconW;
windower::hooklib::hook<decltype(::GetClassLongA)> GetClassLongA;
windower::hooklib::hook<decltype(::GetClassLongW)> GetClassLongW;
windower::hooklib::hook<decltype(::SetClassLongA)> SetClassLongA;
windower::hooklib::hook<decltype(::SetClassLongW)> SetClassLongW;
windower::hooklib::hook<decltype(::GetWindowLongA)> GetWindowLongA;
windower::hooklib::hook<decltype(::GetWindowLongW)> GetWindowLongW;
windower::hooklib::hook<decltype(::SetWindowLongA)> SetWindowLongA;
windower::hooklib::hook<decltype(::SetWindowLongW)> SetWindowLongW;
windower::hooklib::hook<decltype(::PeekMessageA)> PeekMessageA;
windower::hooklib::hook<decltype(::DispatchMessageA)> DispatchMessageA;
windower::hooklib::hook<decltype(::CreateWindowExA)> CreateWindowExA;
windower::hooklib::hook<decltype(::CreateWindowExW)> CreateWindowExW;
windower::hooklib::hook<decltype(::MoveWindow)> MoveWindow;
windower::hooklib::hook<decltype(::SetWindowTextA)> SetWindowTextA;
windower::hooklib::hook<decltype(::SetWindowTextW)> SetWindowTextW;
windower::hooklib::hook<decltype(::SetWindowsHookExA)> SetWindowsHookExA;
windower::hooklib::hook<decltype(::SetWindowsHookExW)> SetWindowsHookExW;
windower::hooklib::hook<decltype(::UnhookWindowsHookEx)> UnhookWindowsHookEx;

}

struct window_data
{
    ::WNDPROC wnd_proc = nullptr;
    ::LONG wnd_proc_a  = 0;
    ::LONG wnd_proc_w  = 0;
    ::MSG current      = {};
    bool update_title  = false;
    std::basic_string<::WCHAR> title;
};

constexpr auto window_data_size = (sizeof(window_data*) + alignof(::LONG) - 1) /
                                  alignof(::LONG) * alignof(::LONG);

auto ffxi_class_atom   = ::ATOM{};
auto pol_class_atom    = ::ATOM{};
auto hhook_counter     = ::LONG{};
auto current_cursor    = ::HCURSOR{};
bool cursor_overridden = false;

bool check_class(::HWND hwnd, ATOM expected) noexcept
{
    auto const atom = hooks::GetClassLongW(hwnd, GCW_ATOM);
    return atom && atom == expected;
}

std::basic_string_view<::CHAR>
get_atom_name(::ATOM atom, std::span<::CHAR> const buffer) noexcept
{
    return {buffer.data(), ::GetAtomNameA(atom, buffer.data(), buffer.size())};
}

std::basic_string_view<::WCHAR>
get_atom_name(::ATOM atom, std::span<::WCHAR> const buffer) noexcept
{
    return {buffer.data(), ::GetAtomNameW(atom, buffer.data(), buffer.size())};
}

template<typename T, std::size_t N>
bool check_class_name(T const* ptr, T const (&class_name)[N]) noexcept
{
    auto value = std::bit_cast<std::uintptr_t>(ptr);
    if (value != 0 && value <= 0xFFFF)
    {
        std::array<T, 256> buffer{};
        auto const atom = gsl::narrow_cast<::ATOM>(value);
        auto const name = get_atom_name(atom, buffer);
        return name == class_name;
    }
    return std::basic_string_view<T>{ptr} == class_name;
}

extern "C" ::LRESULT CALLBACK ffxi_wnd_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam) noexcept
{
    auto const data_value = hooks::GetWindowLongW(hwnd, 0);
    auto const data       = std::bit_cast<window_data*>(data_value);
    if (uMsg == WM_NCDESTROY)
    {
        ::SetWindowLongW(hwnd, 0, 0);
        auto const wnd_proc = data->wnd_proc;
        std::unique_ptr<window_data>{data}.reset();
        return ::CallWindowProcW(wnd_proc, hwnd, uMsg, wParam, lParam);
    }
    else
    {
        auto const time = gsl::narrow_cast<::DWORD>(::GetMessageTime());
        auto const pos  = ::GetMessagePos();
        auto const x    = GET_X_LPARAM(pos);
        auto const y    = GET_Y_LPARAM(pos);
        if (data->current.hwnd != hwnd || data->current.message != uMsg ||
            data->current.wParam != wParam || data->current.lParam != lParam ||
            data->current.pt.x != x || data->current.pt.y != y ||
            data->current.time != time)
        {
            auto& core = windower::core::instance();
            if (auto const result = core.process_message({
                    .hwnd    = hwnd,
                    .message = uMsg,
                    .wParam  = wParam,
                    .lParam  = lParam,
                    .time    = time,
                    .pt      = {.x = x, .y = y},
                }))
            {
                return *result;
            }
        }
        else
        {
            data->current = {};
        }
    }

    switch (uMsg)
    {
    default: break;
    case WM_SETTEXT:
        if (data->update_title)
        {
            data->update_title = false;
            break;
        }
        data->title = lParam ? std::bit_cast<::WCHAR*>(lParam) : L"";
        return TRUE;
    case WM_GETTEXT:
        if (lParam)
        {
            std::fill_n(std::bit_cast<::WCHAR*>(lParam), wParam, 0);
            return data->title.copy(std::bit_cast<::WCHAR*>(lParam), wParam);
        }
        return 0;
    case WM_GETTEXTLENGTH: return data->title.size();
    }

    return ::CallWindowProcW(data->wnd_proc, hwnd, uMsg, wParam, lParam);
}

void set_window_properties(::HWND hwnd)
{
    if (windower::library shell32{u8"shell32.dll"})
    {
        if (auto ptr = shell32.get_function(u8"SHGetPropertyStoreForWindow"))
        {
            using SHGetPropertyStoreForWindow =
                ::HRESULT(STDAPICALLTYPE*)(::HWND, ::IID const&, void**);

            winrt::com_ptr<::IPropertyStore> properties;
            if (SUCCEEDED(reinterpret_cast<SHGetPropertyStoreForWindow>(ptr)(
                    hwnd, IID_PPV_ARGS(properties.put()))))
            {
                GSL_SUPPRESS(type.7)
                {
                    ::PROPVARIANT value{};
                    if (SUCCEEDED(::InitPropVariantFromString(
                            L"Windower.Windower", &value)))
                    {
                        properties->SetValue(PKEY_AppUserModel_ID, value);
                        ::PropVariantClear(&value);
                    }
                    auto executable =
                        windower::windower_path() / u8"windower.exe";
                    if (SUCCEEDED(::InitPropVariantFromString(
                            executable.c_str(), &value)))
                    {
                        properties->SetValue(
                            PKEY_AppUserModel_RelaunchCommand, value);
                        ::PropVariantClear(&value);
                    }
                    if (SUCCEEDED(
                            ::InitPropVariantFromString(L"Windower", &value)))
                    {
                        properties->SetValue(
                            PKEY_AppUserModel_RelaunchDisplayNameResource,
                            value);
                        ::PropVariantClear(&value);
                    }
                }
            }
        }
    }
}

namespace callbacks
{

::ATOM WINAPI RegisterClassA(::WNDCLASSA const* lpWndClass) noexcept
{
    if (check_class_name(lpWndClass->lpszClassName, "FFXiClass"))
    {
        auto window_class = *lpWndClass;
        window_class.cbWndExtra += window_data_size;
        auto const result = hooks::RegisterClassA(&window_class);
        ffxi_class_atom   = result;
        return result;
    }
    return hooks::RegisterClassA(lpWndClass);
}

::ATOM WINAPI RegisterClassExW(::WNDCLASSEXW const* lpWndClass) noexcept
{
    if (check_class_name(lpWndClass->lpszClassName, L"PlayOnlineUS") ||
        check_class_name(lpWndClass->lpszClassName, L"PlayOnlineUS") ||
        check_class_name(lpWndClass->lpszClassName, L"PlayOnlineUS"))
    {
        auto const result = hooks::RegisterClassExW(lpWndClass);
        pol_class_atom    = result;
        return result;
    }
    return hooks::RegisterClassExW(lpWndClass);
}

::HCURSOR WINAPI GetCursor() noexcept
{
    if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        return current_cursor;
    }
    return hooks::GetCursor();
}

::HCURSOR WINAPI SetCursor(::HCURSOR hCursor) noexcept
{
    if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        if (!cursor_overridden)
        {
            hooks::SetCursor(hCursor);
        }
        return std::exchange(current_cursor, hCursor);
    }
    cursor_overridden = hCursor != nullptr;
    return hooks::SetCursor(cursor_overridden ? hCursor : current_cursor);
}

::HWND WINAPI CreateDialogParamW(
    ::HINSTANCE hInstance, ::LPCWSTR lpTemplateName, ::HWND hWndParent,
    ::DLGPROC lpDialogFunc, ::LPARAM dwInitParam) noexcept
{
    auto const hwnd = hooks::CreateDialogParamW(
        hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

    if (hInstance == ::GetModuleHandleW(nullptr))
    {
        if (std::bit_cast<std::uintptr_t>(lpTemplateName) == 103)
        {
            ::ShowWindow(hwnd, SW_HIDE);
        }
    }

    return hwnd;
}

::HICON WINAPI LoadIconA(::HINSTANCE hInstance, ::LPCSTR lpIconName) noexcept
{
    if (hInstance || (IS_INTRESOURCE(lpIconName) &&
                      std::bit_cast<::ULONG_PTR>(lpIconName) >= 0x7F00))
    {
        hInstance  = static_cast<::HINSTANCE>(windower::windower_module());
        lpIconName = MAKEINTRESOURCEA(ICON_MANDY);
    }

    return hooks::LoadIconA(hInstance, lpIconName);
}

::HICON WINAPI LoadIconW(::HINSTANCE hInstance, ::LPCWSTR lpIconName) noexcept
{
    if (hInstance || (IS_INTRESOURCE(lpIconName) &&
                      std::bit_cast<::ULONG_PTR>(lpIconName) >= 0x7F00))
    {
        hInstance  = static_cast<::HINSTANCE>(windower::windower_module());
        lpIconName = MAKEINTRESOURCEW(ICON_MANDY);
    }

    return hooks::LoadIconW(hInstance, lpIconName);
}

::DWORD WINAPI GetClassLongA(::HWND hWnd, int nIndex) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex == GCL_CBWNDEXTRA)
        {
            return hooks::GetClassLongA(hWnd, nIndex) - window_data_size;
        }
    }
    return hooks::GetClassLongA(hWnd, nIndex);
}

::DWORD WINAPI GetClassLongW(::HWND hWnd, int nIndex) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex == GCL_CBWNDEXTRA)
        {
            return hooks::GetClassLongW(hWnd, nIndex) - window_data_size;
        }
    }
    return hooks::GetClassLongW(hWnd, nIndex);
}

::DWORD WINAPI SetClassLongA(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex == GCL_CBWNDEXTRA)
        {
            auto const new_value = dwNewLong + window_data_size;
            auto const result = hooks::SetClassLongA(hWnd, nIndex, new_value);
            return result - window_data_size;
        }
    }
    return hooks::SetClassLongA(hWnd, nIndex, dwNewLong);
}

::DWORD WINAPI SetClassLongW(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex == GCL_CBWNDEXTRA)
        {
            auto const new_value = dwNewLong + window_data_size;
            auto const result = hooks::SetClassLongA(hWnd, nIndex, new_value);
            return result - window_data_size;
        }
    }
    return hooks::SetClassLongA(hWnd, nIndex, dwNewLong);
}

::LONG WINAPI GetWindowLongA(::HWND hWnd, int nIndex) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex >= 0)
        {
            nIndex += window_data_size;
        }
        else if (nIndex == GWL_WNDPROC)
        {
            auto const data_value = hooks::GetWindowLongW(hWnd, 0);
            if (auto const data = std::bit_cast<window_data*>(data_value))
            {
                return std::bit_cast<::LONG>(data->wnd_proc_a);
            }
        }
    }
    return hooks::GetWindowLongA(hWnd, nIndex);
}

::LONG WINAPI GetWindowLongW(::HWND hWnd, int nIndex) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex >= 0)
        {
            nIndex += window_data_size;
        }
        else if (nIndex == GWL_WNDPROC)
        {
            auto const data_value = hooks::GetWindowLongW(hWnd, 0);
            if (auto const data = std::bit_cast<window_data*>(data_value))
            {
                return std::bit_cast<::LONG>(data->wnd_proc_w);
            }
        }
    }
    return hooks::GetWindowLongW(hWnd, nIndex);
}

::LONG WINAPI SetWindowLongA(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex >= 0)
        {
            nIndex += window_data_size;
        }
        else if (nIndex == GWL_WNDPROC)
        {
            auto const data_value = hooks::GetWindowLongW(hWnd, 0);
            if (auto const data = std::bit_cast<window_data*>(data_value))
            {
                auto wnd_proc = std::bit_cast<::LONG>(data->wnd_proc);
                hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                auto result =
                    hooks::SetWindowLongA(hWnd, GWL_WNDPROC, dwNewLong);
                wnd_proc = hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                data->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);
                return result;
            }
        }
    }
    return hooks::SetWindowLongA(hWnd, nIndex, dwNewLong);
}

::LONG WINAPI SetWindowLongW(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
{
    if (check_class(hWnd, ffxi_class_atom))
    {
        if (nIndex >= 0)
        {
            nIndex += window_data_size;
        }
        else if (nIndex == GWL_WNDPROC)
        {
            auto const data_value = hooks::GetWindowLongW(hWnd, 0);
            if (auto const data = std::bit_cast<window_data*>(data_value))
            {
                auto wnd_proc = std::bit_cast<::LONG>(data->wnd_proc);
                hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                auto result =
                    hooks::SetWindowLongW(hWnd, GWL_WNDPROC, dwNewLong);
                wnd_proc = hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                data->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);
                return result;
            }
        }
    }
    return hooks::SetWindowLongW(hWnd, nIndex, dwNewLong);
}

::BOOL WINAPI PeekMessageA(
    ::LPMSG lpMsg, ::HWND hWnd, ::UINT wMsgFilterMin, ::UINT wMsgFilterMax,
    ::UINT wRemoveMsg) noexcept
{
    auto& core = windower::core::instance();

    auto msg         = ::MSG{};
    auto const flags = (wRemoveMsg & ~PM_REMOVE) | PM_NOREMOVE;
    while (::PeekMessageW(&msg, hWnd, wMsgFilterMin, wMsgFilterMax, flags))
    {
        if (core.process_message(msg))
        {
            ::PeekMessageW(
                &msg, hWnd, wMsgFilterMin, wMsgFilterMax,
                PM_REMOVE | PM_NOYIELD);
            ::TranslateMessage(&msg);
        }
        else
        {
            return hooks::PeekMessageA(
                lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax,
                wRemoveMsg | PM_NOYIELD);
        }
    }
    return FALSE;
}

::LRESULT WINAPI DispatchMessageA(const ::MSG* lpMsg) noexcept
{
    if (check_class(lpMsg->hwnd, ffxi_class_atom))
    {
        auto const data_value = hooks::GetWindowLongW(lpMsg->hwnd, 0);
        if (auto const data = std::bit_cast<window_data*>(data_value))
        {
            data->current = *lpMsg;
        }
    }
    return hooks::DispatchMessageA(lpMsg);
}

::HWND WINAPI CreateWindowExA(
    ::DWORD dwExStyle, ::LPCSTR lpClassName, ::LPCSTR lpWindowName,
    ::DWORD dwStyle, int x, int y, int nWidth, int nHeight, ::HWND hWndParent,
    ::HMENU hMenu, ::HINSTANCE hInstance, ::LPVOID lpParam) noexcept
{
    auto hwnd = hooks::CreateWindowExA(
        dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight,
        hWndParent, hMenu, hInstance, lpParam);

    if (check_class(hwnd, ffxi_class_atom))
    {
        auto ptr  = std::make_unique<window_data>();
        auto data = ptr.get();
        hooks::SetWindowLongW(hwnd, 0, std::bit_cast<::LONG>(ptr.release()));

        auto wnd_proc  = std::bit_cast<::LONG>(&ffxi_wnd_proc);
        wnd_proc       = hooks::SetWindowLongW(hwnd, GWL_WNDPROC, wnd_proc);
        data->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);

        auto const title_size = std::strlen(lpWindowName);
        data->title.resize(::MultiByteToWideChar(
            CP_ACP, 0, lpWindowName, title_size, nullptr, 0));
        ::MultiByteToWideChar(
            CP_ACP, 0, lpWindowName, title_size, data->title.data(),
            data->title.size());

        ::SetWindowTextW(hwnd, L"Final Fantasy XI");
        set_window_properties(hwnd);

        windower::core::instance().client_hwnd = hwnd;
    }

    return hwnd;
}

::HWND WINAPI CreateWindowExW(
    ::DWORD dwExStyle, ::LPCWSTR lpClassName, ::LPCWSTR lpWindowName,
    ::DWORD dwStyle, int x, int y, int nWidth, int nHeight, ::HWND hWndParent,
    ::HMENU hMenu, ::HINSTANCE hInstance, ::LPVOID lpParam) noexcept
{
    auto hwnd = hooks::CreateWindowExW(
        dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight,
        hWndParent, hMenu, hInstance, lpParam);

    if (check_class(hwnd, pol_class_atom))
    {
        set_window_properties(hwnd);
    }

    return hwnd;
}

::BOOL WINAPI MoveWindow(
    ::HWND hWnd, int X, int Y, int nWidth, int nHeight,
    ::BOOL bRepaint) noexcept
{
    if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto const& core = windower::core::instance();

        auto const display = core.settings.display_bounds;

        auto const x = display.location.x;
        auto const y = display.location.y;
        auto const w = display.size.width;
        auto const h = display.size.height;

        X = x + std::max(0, (w - nWidth) / 2);
        Y = y + std::max(0, (h - nHeight) / 2);
    }

    return hooks::MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
}

::BOOL WINAPI SetWindowTextA(::HWND hWnd, ::LPCSTR lpString) noexcept
{
    if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto const data_value = hooks::GetWindowLongW(hWnd, 0);
        if (auto const data = std::bit_cast<window_data*>(data_value))
        {
            data->update_title = true;
        }
    }
    return hooks::SetWindowTextA(hWnd, lpString);
}

::BOOL WINAPI SetWindowTextW(::HWND hWnd, ::LPCWSTR lpString) noexcept
{
    if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto const data_value = hooks::GetWindowLongW(hWnd, 0);
        if (auto const data = std::bit_cast<window_data*>(data_value))
        {
            data->update_title = true;
        }
    }
    return hooks::SetWindowTextW(hWnd, lpString);
}

::HHOOK WINAPI SetWindowsHookExA(
    int idHook, ::HOOKPROC lpfn, ::HINSTANCE hMod, ::DWORD dwThreadId) noexcept
{
    if ((idHook == WH_KEYBOARD_LL || idHook == WH_KEYBOARD) &&
        windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        return std::bit_cast<::HHOOK>(
            std::bit_cast<std::intptr_t>(windower::windower_module()) +
            ::InterlockedIncrement(&hhook_counter));
    }
    return hooks::SetWindowsHookExA(idHook, lpfn, hMod, dwThreadId);
}

::HHOOK WINAPI SetWindowsHookExW(
    int idHook, ::HOOKPROC lpfn, ::HINSTANCE hMod, ::DWORD dwThreadId) noexcept
{
    if ((idHook == WH_KEYBOARD_LL || idHook == WH_KEYBOARD) &&
        windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        return std::bit_cast<::HHOOK>(
            std::bit_cast<std::intptr_t>(windower::windower_module()) +
            ::InterlockedIncrement(&hhook_counter));
    }
    return hooks::SetWindowsHookExW(idHook, lpfn, hMod, dwThreadId);
}

::BOOL WINAPI UnhookWindowsHookEx(::HHOOK hhk) noexcept
{
    if (windower::is_windower_module(hhk))
    {
        return TRUE;
    }
    return hooks::UnhookWindowsHookEx(hhk);
}

}

}

void windower::user32::install()
{
    current_cursor = ::GetCursor();

    hooks::RegisterClassA = hooklib::make_hook(
        u8"user32.dll", u8"RegisterClassA", callbacks::RegisterClassA);
    hooks::RegisterClassExW = hooklib::make_hook(
        u8"user32.dll", u8"RegisterClassExW", callbacks::RegisterClassExW);

    hooks::GetCursor =
        hooklib::make_hook(u8"user32.dll", u8"GetCursor", callbacks::GetCursor);
    hooks::SetCursor =
        hooklib::make_hook(u8"user32.dll", u8"SetCursor", callbacks::SetCursor);

    hooks::CreateDialogParamW = hooklib::make_hook(
        u8"user32.dll", u8"CreateDialogParamW", callbacks::CreateDialogParamW);

    hooks::LoadIconA =
        hooklib::make_hook(u8"user32.dll", u8"LoadIconA", callbacks::LoadIconA);
    hooks::LoadIconW =
        hooklib::make_hook(u8"user32.dll", u8"LoadIconW", callbacks::LoadIconW);

    hooks::GetClassLongA = hooklib::make_hook(
        u8"user32.dll", u8"GetClassLongA", callbacks::GetClassLongA);
    hooks::GetClassLongW = hooklib::make_hook(
        u8"user32.dll", u8"GetClassLongW", callbacks::GetClassLongW);
    hooks::SetClassLongA = hooklib::make_hook(
        u8"user32.dll", u8"SetClassLongA", callbacks::SetClassLongA);
    hooks::SetClassLongW = hooklib::make_hook(
        u8"user32.dll", u8"SetClassLongW", callbacks::SetClassLongW);

    hooks::GetWindowLongA = hooklib::make_hook(
        u8"user32.dll", u8"GetWindowLongA", callbacks::GetWindowLongA);
    hooks::GetWindowLongW = hooklib::make_hook(
        u8"user32.dll", u8"GetWindowLongW", callbacks::GetWindowLongW);
    hooks::SetWindowLongA = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowLongA", callbacks::SetWindowLongA);
    hooks::SetWindowLongW = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowLongW", callbacks::SetWindowLongW);

    hooks::PeekMessageA = hooklib::make_hook(
        u8"user32.dll", u8"PeekMessageA", callbacks::PeekMessageA);

    hooks::DispatchMessageA = hooklib::make_hook(
        u8"user32.dll", u8"DispatchMessageA", callbacks::DispatchMessageA);

    hooks::CreateWindowExA = hooklib::make_hook(
        u8"user32.dll", u8"CreateWindowExA", callbacks::CreateWindowExA);

    hooks::MoveWindow = hooklib::make_hook(
        u8"user32.dll", u8"MoveWindow", callbacks::MoveWindow);

    hooks::SetWindowTextA = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowTextA", callbacks::SetWindowTextA);
    hooks::SetWindowTextW = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowTextW", callbacks::SetWindowTextW);

    hooks::SetWindowsHookExA = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowsHookExA", callbacks::SetWindowsHookExA);
    hooks::SetWindowsHookExW = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowsHookExW", callbacks::SetWindowsHookExW);
    hooks::UnhookWindowsHookEx = hooklib::make_hook(
        u8"user32.dll", u8"UnhookWindowsHookEx",
        callbacks::UnhookWindowsHookEx);
}

void windower::user32::uninstall() noexcept
{
    hooks::RegisterClassA      = {};
    hooks::RegisterClassExW    = {};
    hooks::GetCursor           = {};
    hooks::SetCursor           = {};
    hooks::CreateDialogParamW  = {};
    hooks::LoadIconA           = {};
    hooks::LoadIconW           = {};
    hooks::GetClassLongA       = {};
    hooks::GetClassLongW       = {};
    hooks::SetClassLongA       = {};
    hooks::SetClassLongW       = {};
    hooks::GetWindowLongA      = {};
    hooks::GetWindowLongW      = {};
    hooks::SetWindowLongA      = {};
    hooks::SetWindowLongW      = {};
    hooks::PeekMessageA        = {};
    hooks::DispatchMessageA    = {};
    hooks::CreateWindowExA     = {};
    hooks::CreateWindowExW     = {};
    hooks::MoveWindow          = {};
    hooks::SetWindowTextA      = {};
    hooks::SetWindowTextW      = {};
    hooks::SetWindowsHookExA   = {};
    hooks::SetWindowsHookExW   = {};
    hooks::UnhookWindowsHookEx = {};
}
