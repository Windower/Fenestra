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

#include "hooks/ddraw.hpp"

#include "hooklib/hook.hpp"
#include "wrappers/direct_draw.hpp"

#include <windows.h>

#include <ddraw.h>
#include <winrt/base.h>

#include <memory>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::DirectDrawCreateEx)> DirectDrawCreateEx;

}

namespace callbacks
{

::HRESULT WINAPI DirectDrawCreateEx(
    ::GUID* lpGUID, ::LPVOID* lplpDD, ::IID const& iid,
    ::IUnknown* pUnkOuter) noexcept
{
    using namespace windower;

    if (!lplpDD)
    {
        return E_POINTER;
    }
    *lplpDD = nullptr;

    ::MemoryBarrier();
    winrt::com_ptr<::IDirectDraw7> ptr;
    auto const result =
        hooks::DirectDrawCreateEx(lpGUID, ptr.put_void(), iid, pUnkOuter);
    if (result == S_OK)
    {
        *lplpDD = std::make_unique<direct_draw>(ptr.detach()).release();
    }
    return result;
}

}

}

::HRESULT windower::ddraw::DirectDrawCreateEx(
    ::GUID* lpGUID, ::LPVOID* lplpDD, ::IID const& iid,
    ::IUnknown* pUnkOuter) noexcept
{
    return hooks::DirectDrawCreateEx(lpGUID, lplpDD, iid, pUnkOuter);
}

void windower::ddraw::install()
{
    if (!hooks::DirectDrawCreateEx)
    {
        hooks::DirectDrawCreateEx = hooklib::make_hook(
            u8"ddraw.dll", u8"DirectDrawCreateEx",
            callbacks::DirectDrawCreateEx);
    }
}

void windower::ddraw::uninstall() noexcept { hooks::DirectDrawCreateEx = {}; }
