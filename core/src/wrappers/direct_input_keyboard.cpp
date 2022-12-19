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

#include "wrappers/direct_input_keyboard.hpp"

#include "wrappers/direct_input_device.hpp"
#include "core.hpp"

#include <dinput.h>

#include <vector>

windower::direct_input_keyboard::direct_input_keyboard(
    ::IDirectInputDevice8A* impl) noexcept :
    direct_input_device{impl}
{}

::HRESULT STDMETHODCALLTYPE windower::direct_input_keyboard::GetDeviceState(
    ::DWORD cbData, void* lpvData) noexcept
{
    if (!lpvData)
    {
        return E_POINTER;
    }

    auto keys = static_cast<::BYTE*>(lpvData);
    auto& state = core::instance().binding_manager.client_state();
    std::copy_n(
        state.begin(), std::min(state.size(), std::size_t(cbData)), keys);

    return DI_OK;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_keyboard::GetDeviceData(
    ::DWORD, ::DIDEVICEOBJECTDATA* rgdod, ::DWORD* pdwInOut, ::DWORD) noexcept
{
    if (!rgdod || !pdwInOut)
    {
        return E_POINTER;
    }

    *pdwInOut = INFINITE;
    direct_input_device::GetDeviceData(
        sizeof(::DIDEVICEOBJECTDATA), nullptr, pdwInOut, 0);
    *pdwInOut = 0;
    return DI_OK;
}
