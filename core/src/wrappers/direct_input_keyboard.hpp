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

#ifndef WINDOWER_WRAPPERS_DIRECT_INPUT_KEYBOARD_HPP
#define WINDOWER_WRAPPERS_DIRECT_INPUT_KEYBOARD_HPP

#include "direct_input_device.hpp"

#include <dinput.h>

namespace windower
{

class direct_input_keyboard : public direct_input_device
{
public:
    direct_input_keyboard() = delete;
    direct_input_keyboard(::IDirectInputDevice8A*) noexcept;
    direct_input_keyboard(direct_input_keyboard const&) = delete;
    direct_input_keyboard(direct_input_keyboard&&) = delete;
    ~direct_input_keyboard() override = default;

    direct_input_keyboard& operator=(direct_input_keyboard const&) = delete;
    direct_input_keyboard& operator=(direct_input_keyboard&&) = delete;

    ::HRESULT STDMETHODCALLTYPE
    GetDeviceState(::DWORD, void*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDeviceData(
        ::DWORD, ::DIDEVICEOBJECTDATA*, ::DWORD*, ::DWORD) noexcept override;
};

}

#endif
