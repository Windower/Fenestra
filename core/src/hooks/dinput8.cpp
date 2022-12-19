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

#include "hooks/dinput8.hpp"

#include "hooklib/hook.hpp"
#include "wrappers/direct_input.hpp"

#include <windows.h>

#include <dinput.h>
#include <winrt/base.h>

#include <memory>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::DirectInput8Create)> DirectInput8Create;

}

namespace callbacks
{

::HRESULT WINAPI DirectInput8Create(
    ::HINSTANCE hinst, ::DWORD dwVersion, ::IID const& riidltf,
    ::LPVOID* ppvOut, ::LPUNKNOWN punkOuter) noexcept
{
    using namespace windower;

    if (!ppvOut)
    {
        return E_POINTER;
    }
    *ppvOut = nullptr;

    winrt::com_ptr<::IDirectInput8A> ptr;
    auto const result = hooks::DirectInput8Create(
        hinst, dwVersion, riidltf, ptr.put_void(), punkOuter);
    if (result == S_OK)
    {
        *ppvOut = std::make_unique<direct_input>(ptr.detach()).release();
    }
    return result;
}

}

}

::HRESULT windower::dinput8::DirectInput8Create(
    ::HINSTANCE hinst, ::DWORD dwVersion, ::IID const& riidltf,
    ::LPVOID* ppvOut, ::LPUNKNOWN punkOuter) noexcept
{
    return hooks::DirectInput8Create(
        hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

void windower::dinput8::install()
{
    if (!hooks::DirectInput8Create)
    {
        hooks::DirectInput8Create = hooklib::make_hook(
            u8"dinput8.dll", u8"DirectInput8Create",
            callbacks::DirectInput8Create);
    }
}

void windower::dinput8::uninstall() noexcept { hooks::DirectInput8Create = {}; }
