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

#include "cloak.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "crash_handler.hpp"
#include "library.hpp"
#include "unicode.hpp"

#include <windows.h>

#include <shellscalingapi.h>

namespace
{
void enable_process_dpi_awareness()
{
    if (windower::library shcore{u8"shcore.dll"})
    {
        if (auto ptr = shcore.get_function(u8"SetProcessDpiAwareness"))
        {
            using SetProcessDpiAwareness =
                ::HRESULT(WINAPI*)(::PROCESS_DPI_AWARENESS);

            if (SUCCEEDED(reinterpret_cast<SetProcessDpiAwareness>(ptr)(
                    PROCESS_PER_MONITOR_DPI_AWARE)))
            {
                return;
            }
        }
    }
    ::SetProcessDPIAware();
}
}

extern "C"
{
    static ::DWORD WINAPI initialize_thread(::LPVOID) noexcept(false)
    {
        windower::crash_handler::initialize();

        // SE seems to make no attempt to detect any injected dlls.
        // However, just in case, we'll pin ourselves in place and remove
        // ourselves from the PEB.
        windower::pin_and_cloak();

        enable_process_dpi_awareness();

        windower::command_manager::initialize();
        windower::core::initialize();

        return EXIT_SUCCESS;
    }

    ::BOOL WINAPI DllMain(::HINSTANCE hinstDLL, ::DWORD fdwReason, ::LPVOID)
    {
        switch (fdwReason)
        {
        default: break;
        case DLL_PROCESS_ATTACH:
            return ::CreateThread(
                       nullptr, 0, &::initialize_thread, hinstDLL, 0, nullptr)
                     ? TRUE
                     : FALSE;
        }
        return TRUE;
    }
}
