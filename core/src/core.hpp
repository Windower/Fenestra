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

#ifndef WINDOWER_CORE_HPP
#define WINDOWER_CORE_HPP

#include "addon/addon_manager.hpp"
#include "addon/package_manager.hpp"
#include "addon/script_environment.hpp"
#include "binding_manager.hpp"
#include "command_manager.hpp"
#include "downloader.hpp"
#include "packet_queue.hpp"
#include "settings.hpp"
#include "ui/user_interface.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>

namespace windower
{

class core
{
public:
    static void initialize() noexcept;
    static core& instance() noexcept;

    static void output(
        std::u8string_view component, std::u8string_view text,
        command_source source = command_source::console);
    static void error(
        std::u8string_view component, std::u8string_view text,
        command_source source = command_source::console);
    static void error(
        std::u8string_view component, std::exception const& exception,
        command_source source = command_source::console);
    static void error(
        std::u8string_view component,
        std::exception_ptr exception = std::current_exception(),
        command_source source        = command_source::console);

    settings settings;

    downloader downloader;
    binding_manager binding_manager;
    script_environment script_environment;
    user_interface ui;
    std::unique_ptr<packet_queue> incoming_packet_queue;
    std::unique_ptr<packet_queue> outgoing_packet_queue;
    std::unique_ptr<package_manager> package_manager;
    std::unique_ptr<addon_manager> addon_manager;

    void* client_hwnd;

    bool render_ui   = false;
    int render_phase = 0;

    core(core const&) = delete;
    core& operator=(core const&) = delete;

    void begin_frame() noexcept;
    void update() noexcept;
    void end_frame() noexcept;

    void run_on_next_frame(std::function<void()> function);
    std::optional<::LRESULT> process_message(::MSG const& msg) noexcept;

private:
    std::mutex m_queued_functions_mutex;
    std::queue<std::function<void()>> m_queued_functions;
    bool m_updated = false;

    core() noexcept;
};

}

#endif
