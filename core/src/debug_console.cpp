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

#include "debug_console.hpp"

#include "command_manager.hpp"
#include "core.hpp"
#include "handle.hpp"
#include "resource.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <array>
#include <cstdio>
#include <mutex>
#include <thread>

void windower::debug_console::initialize(bool open) { instance().open(open); }

windower::debug_console& windower::debug_console::instance() noexcept
{
    static debug_console instance;
    return instance;
}

windower::debug_console::~debug_console() noexcept { open(false); }

bool windower::debug_console::open() const noexcept { return m_open; }

void windower::debug_console::open(bool open)
{
    std::lock_guard<std::mutex> lock{m_mutex};
    if (m_open.exchange(open) != open)
    {
        if (open)
        {
            m_thread = std::thread{&debug_console::run, this};
        }
        else
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }
    }
}

template<typename, typename>
class test_exception : public std::exception
{};

void windower::debug_console::run()
{
    if (::GetConsoleCP() == 0)
    {
        ::AllocConsole();

        // disable close button
        ::DeleteMenu(
            ::GetSystemMenu(::GetConsoleWindow(), false), SC_CLOSE,
            MF_BYCOMMAND);
        ::SetConsoleTitleW(
            reinterpret_cast<::LPCWSTR>(u"Windower Debug Console"));
    }

    ::FILE* file;
    ::freopen_s(&file, "CONIN$", "r", stdin);
    ::freopen_s(&file, "CONOUT$", "w", stdout);
    ::freopen_s(&file, "CONOUT$", "w", stderr);

    auto input_handle = ::GetStdHandle(STD_INPUT_HANDLE);

    handle signal = ::CreateEventW(nullptr, false, false, nullptr);

    std::wstring command;
    while (m_open)
    {
        auto output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
        ::SetConsoleTextAttribute(
            output_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        ::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
        if (::GetConsoleScreenBufferInfo(output_handle, &buffer_info) &&
            buffer_info.dwCursorPosition.X != 0)
        {
            ::DWORD written;
            ::WriteConsoleW(output_handle, L"\n> ", 2, &written, nullptr);
        }
        else
        {
            ::DWORD written;
            ::WriteConsoleW(output_handle, L"> ", 2, &written, nullptr);
        }

        do
        {
            ::DWORD read = 0;
            std::array<wchar_t, 256> buffer = {};
            if (::ReadConsoleW(
                    input_handle, buffer.data(), buffer.size(), &read,
                    nullptr) &&
                read > 0)
            {
                command.insert(
                    command.end(), buffer.begin(), buffer.begin() + read);
                if (command.size() >= 2 && *(command.end() - 2) == L'\r' &&
                    *(command.end() - 1) == L'\n')
                {
                    command.erase(command.end() - 2, command.end());
                    core::instance().run_on_next_frame(
                        [command = to_u8string(command),
                         signal  = ::HANDLE(signal)]() {
                            try
                            {
                                command_manager::instance().handle_command(
                                    command, command_source::console);
                            }
                            catch (std::exception const&)
                            {
                                core::error(u8"");
                            }
                            ::SetEvent(signal);
                        });
                    command.clear();
                    ::WaitForSingleObject(signal, INFINITE);
                    // HACK: give asynchronous error messages time to output
                    ::Sleep(10);
                    break;
                }
            }
        }
        while (true);
    }

    ::FreeConsole();
}
