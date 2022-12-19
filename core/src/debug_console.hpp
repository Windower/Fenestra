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

#ifndef WINDOWER_DEBUG_CONSOLE_HPP
#define WINDOWER_DEBUG_CONSOLE_HPP

#include <atomic>
#include <mutex>
#include <thread>

namespace windower
{

class debug_console
{
public:
    static void initialize(bool = false);
    static debug_console& instance() noexcept;

    debug_console(debug_console const&) = delete;
    debug_console(debug_console&&)      = delete;

    debug_console& operator=(debug_console const&) = delete;
    debug_console& operator=(debug_console&&) = delete;

    bool open() const noexcept;
    void open(bool);

private:
    std::atomic<bool> m_open = false;
    std::thread m_thread;
    std::mutex m_mutex;

    debug_console() = default;
    ~debug_console() noexcept;

    void run();
};

}

#endif
