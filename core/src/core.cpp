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

#include "core.hpp"

#include "addon/error.hpp"
#include "command_handlers.hpp"
#include "command_manager.hpp"
#include "crash_handler.hpp"
#include "debug_console.hpp"
#include "hooks/advapi32.hpp"
#include "hooks/d3d8.hpp"
#include "hooks/ddraw.hpp"
#include "hooks/dinput8.hpp"
#include "hooks/ffximain.hpp"
#include "hooks/imm32.hpp"
#include "hooks/kernel32.hpp"
#include "hooks/user32.hpp"
#include "settings.hpp"
#include "ui/user_interface.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <gsl/gsl>

#include <mutex>

namespace
{
std::mutex output_mutex;

template<typename E>
void get_type_name(std::u8string& result, E const& exception)
{
    std::string_view const type_name = typeid(exception).name();
    std::transform(
        type_name.begin(), type_name.end(), std::back_inserter(result),
        [](auto c) noexcept { return gsl::narrow_cast<char8_t>(c); });
}

void get_error_message(
    std::u8string& result, windower::lua::error const& exception)
{
    std::string_view const what = exception.what();
    result.reserve(result.size() + what.size());
    std::copy(what.begin(), what.end(), std::back_inserter(result));

    if (windower::core::instance().settings.verbose_logging &&
        exception.has_stack_trace())
    {
        for (auto const& frame : exception.stack_trace())
        {
            result.append(1, u8'\n');
            if (!frame.type.empty())
            {
                result.append(1, u8'(');
                result.append(frame.type);
                result.append(1, u8')');
            }
            result.append(frame.name.empty() ? frame.name : u8"<unknown>");
            if (!frame.source.value.empty())
            {
                result.append(u8"\n  ");
                if (frame.source.type == u8"string")
                {
                    if (gsl::at(frame.source.value, 0) == u8'=')
                    {
                        result.append(
                            frame.source.value.begin() + 1,
                            frame.source.value.end());
                    }
                    else
                    {
                        result.append(u8"[string]");
                        result.append(frame.source.value);
                    }
                }
                else
                {
                    if (frame.source.type != u8"file")
                    {
                        result.append(1, u8'[');
                        result.append(frame.source.type);
                        result.append(1, u8']');
                    }
                    result.append(frame.source.value);
                }

                if (frame.source.line != 0)
                {
                    result.append(1, u8':');
                    result.append(windower::to_u8string(frame.source.line));
                }
            }
            if (!frame.locals.empty())
            {
                result.append(u8"\n  locals:");
                for (auto const& l : frame.locals)
                {
                    result.append(u8"\n    ");
                    result.append(l.name);
                    result.append(u8" = [");
                    result.append(l.type);
                    result.append(u8"]");
                    result.append(l.value);
                }
            }
            if (!frame.upvalues.empty())
            {
                result.append(u8"\n  upvalues:");
                for (auto const& u : frame.upvalues)
                {
                    result.append(u8"\n    ");
                    result.append(u.name);
                    result.append(u8" = [");
                    result.append(u.type);
                    result.append(u8"]");
                    result.append(u.value);
                }
            }
        }
    }
}

template<typename E>
void get_error_message(std::u8string& result, E const& exception)
{
    if (auto ptr = dynamic_cast<windower::lua::error const*>(&exception))
    {
        get_error_message(result, *ptr);
    }
    else
    {
        get_type_name(result, exception);
        result.append(u8"\n");
        auto const what = windower::to_u8string(exception.what());
        result.append(what);
        if (auto windower_error =
                dynamic_cast<windower::windower_error const*>(&exception))
        {
            result.append(u8": ");
            result.append(windower_error->message());
        }
    }
}

template<typename E>
void get_error_message(
    std::u8string& result, std::size_t level, E const& exception)
{
    get_error_message(result, exception);
    if (auto nested = dynamic_cast<std::nested_exception const*>(&exception))
    {
        if (auto nested_ptr = nested->nested_ptr())
        {
            try
            {
                std::rethrow_exception(nested_ptr);
            }
            catch (std::exception const& e)
            {
                result.append(u8"\nnested exception [");
                result.append(windower::to_u8string(level + 1));
                result.append(u8"]: \n");
                get_error_message(result, level + 1, e);
            }
        }
    }
}

template<typename E>
std::u8string get_error_message(E const& exception)
{
    std::u8string result;
    get_error_message(result, 0, exception);
    return result;
}

std::u8string
process_output(std::u8string_view component, std::u8string_view text)
{
    if (component.empty())
    {
        component = u8"core";
    }
    auto const count = std::count(text.begin(), text.end(), u8'\n') + 1;
    std::u8string temp;
    temp.reserve(temp.size() + (component.size() + 3) * count);
    temp.append(1, u8'[');
    temp.append(component);
    temp.append(1, u8']');
    temp.append(1, u8' ');
    std::u8string::size_type start = 0;
    std::u8string::size_type pos;
    while ((pos = text.find(u8'\n', start)) != std::u8string::npos)
    {
        temp.append(text, start, pos - start + 1);
        temp.append(1, u8'[');
        temp.append(component);
        temp.append(1, u8']');
        temp.append(1, u8' ');
        start = pos + 1;
    }
    temp.append(text, start, std::u8string::npos);
    return temp;
}

template<typename T>
bool try_pop(std::queue<T>& queue, std::mutex& mutex, T& result)
{
    std::lock_guard<std::mutex> guard{mutex};
    if (queue.empty())
    {
        return false;
    }
    result = std::move(queue.front());
    queue.pop();
    return true;
}
}

void windower::core::initialize() noexcept { instance(); }

windower::core& windower::core::instance() noexcept
{
    static core instance;
    return instance;
}

windower::core::core() noexcept
{
    kernel32::install();
    user32::install();
    advapi32::install();
    imm32::install();
    d3d8::install();
    dinput8::install();
    ddraw::install();

    settings.load();
    crash_handler::instance().dump_path(settings.temp_path);

    run_on_next_frame([]() mutable {
        debug_console::initialize(core::instance().settings.debug);

        core::instance().package_manager =
            std::make_unique<windower::package_manager>();
        core::instance().package_manager->update_all();

        auto& cmd = command_manager::instance();

        cmd.register_command(
            command_manager::layer::core, u8"", u8"install",
            command_handlers::install);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"uninstall",
            command_handlers::uninstall);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"update",
            command_handlers::update);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"updateall",
            command_handlers::updateall);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"load",
            command_handlers::load);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unload",
            command_handlers::unload);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"reload",
            command_handlers::reload);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"unloadall",
            command_handlers::unloadall);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"reloadall",
            command_handlers::reloadall);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"alias",
            command_handlers::alias, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unalias",
            command_handlers::unalias);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"bind",
            command_handlers::bind, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unbind",
            command_handlers::unbind, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"listbinds",
            command_handlers::listbinds, true);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"exec",
            command_handlers::exec);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"eval",
            command_handlers::eval, true);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"reset",
            command_handlers::reset);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"pkg", command_handlers::pkg);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"nextwindow",
            command_handlers::nextwindow);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"prevwindow",
            command_handlers::prevwindow);
    });
}

void windower::core::output(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    if (source < command_source::client)
    {
        auto w_text =
            to_wstring(process_output(component, text)).append(1, L'\n');

        {
            std::lock_guard<std::mutex> lock{output_mutex};
            auto written       = ::DWORD{};
            auto output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
            ::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            if (::GetConsoleScreenBufferInfo(output_handle, &buffer_info) &&
                buffer_info.dwCursorPosition.X != 0)
            {
                w_text.insert(w_text.begin(), L'\n');
            }

            ::WriteConsoleW(
                output_handle, w_text.data(), w_text.size(), &written, nullptr);
        }

        ::OutputDebugStringW(w_text.c_str());
    }
    else
    {
        instance().run_on_next_frame([text = process_output(component, text)] {
            ffximain::add_to_chat(text);
        });
    }
}

void windower::core::error(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    if (source < command_source::client)
    {
        auto w_text =
            to_wstring(process_output(component, text)).append(1, L'\n');

        {
            std::lock_guard<std::mutex> lock{output_mutex};
            auto error_handle = ::GetStdHandle(STD_ERROR_HANDLE);
            ::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            if (::GetConsoleScreenBufferInfo(error_handle, &buffer_info))
            {
                if (buffer_info.dwCursorPosition.X != 0)
                {
                    w_text.insert(w_text.begin(), L'\n');
                }
            }
            else
            {
                buffer_info.wAttributes =
                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            }

            ::SetConsoleTextAttribute(
                error_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
            auto written = ::DWORD{};
            ::WriteConsoleW(
                error_handle, w_text.data(), w_text.size(), &written, nullptr);
            ::SetConsoleTextAttribute(error_handle, buffer_info.wAttributes);
        }

        ::OutputDebugStringW(w_text.c_str());
    }
    else
    {
        instance().run_on_next_frame([text = process_output(component, text)] {
            ffximain::add_to_chat(text, 0xA7);
        });
    }
}

void windower::core::error(
    std::u8string_view component, std::exception const& exception,
    command_source source)
{
    error(component, get_error_message(exception), source);
}

void windower::core::error(
    std::u8string_view component, std::exception_ptr exception,
    command_source source)
{
    if (exception)
    {
        try
        {
            std::rethrow_exception(exception);
        }
        catch (std::exception const& e)
        {
            error(component, e, source);
        }
    }
    else
    {
        error(component, u8"unknown error", source);
    }
}

void windower::core::update() noexcept
{
    if (!m_updated)
    {
        m_updated = true;
        scheduler::next_frame();
        script_environment.run_until_idle();
        if (addon_manager)
        {
            addon_manager->run_until_idle();
        }
        std::function<void()> function;
        while (try_pop(m_queued_functions, m_queued_functions_mutex, function))
        {
            try
            {
                function();
            }
            catch (std::exception const&)
            {
                error(u8"");
            }
        }
    }
}

void windower::core::begin_frame() noexcept
{
    ui.begin_frame();
    m_updated = false;
}

void windower::core::end_frame() noexcept { ui.end_frame(); }

void windower::core::run_on_next_frame(std::function<void()> function)
{
    std::lock_guard<std::mutex> guard{m_queued_functions_mutex};
    m_queued_functions.emplace(std::move(function));
}

std::optional<::LRESULT>
windower::core::process_message(::MSG const& msg) noexcept
{
    if (auto const result = ui.process_message(msg))
    {
        return result;
    }
    if (auto const result = binding_manager.process_message(msg))
    {
        return result;
    }
    return std::nullopt;
}
