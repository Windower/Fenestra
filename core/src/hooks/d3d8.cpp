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

#include "hooks/d3d8.hpp"

#include "hooklib/hook.hpp"
#include "wrappers/direct_3d.hpp"

#include <windows.h>

#include <d3d8.h>

#include <memory>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::Direct3DCreate8)> Direct3DCreate8;

}

namespace callbacks
{

::IDirect3D8* WINAPI Direct3DCreate8(::UINT SDKVersion) noexcept
{
    using namespace windower;

    ::MemoryBarrier();
    if (auto ptr = hooks::Direct3DCreate8(SDKVersion))
    {
        return std::make_unique<direct_3d>(ptr).release();
    }
    return nullptr;
}

}

}

::IDirect3D8* windower::d3d8::Direct3DCreate8(::UINT SDKVersion) noexcept
{
    return hooks::Direct3DCreate8(SDKVersion);
}

void windower::d3d8::install()
{
    if (!hooks::Direct3DCreate8)
    {
        hooks::Direct3DCreate8 = hooklib::make_hook(
            u8"d3d8.dll", u8"Direct3DCreate8", callbacks::Direct3DCreate8);
    }
}

void windower::d3d8::uninstall() noexcept { hooks::Direct3DCreate8 = {}; }
