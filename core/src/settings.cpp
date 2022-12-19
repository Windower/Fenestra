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

#include "settings.hpp"

#include "library.hpp"
#include "settings_channel.hpp"

#include <windows.h>

#include <shellscalingapi.h>

#include <algorithm>
#include <array>

extern "C"
{
    static ::BOOL CALLBACK
        enum_display_monitors_callback(::HMONITOR, ::HDC, ::LPRECT, ::LPARAM);
}

namespace
{

struct enum_display_monitors_block
{
    std::array<::WCHAR, 32> name;
    ::HMONITOR handle;
};

::HMONITOR get_display(windower::zstring_view name)
{
    enum_display_monitors_block block = {};
    if (!name.empty() &&
        ::MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, name.c_str(), name.size() + 1,
            block.name.data(), block.name.size()))
    {
        ::EnumDisplayMonitors(
            nullptr, nullptr, ::enum_display_monitors_callback,
            reinterpret_cast<::LPARAM>(&block));
        if (block.handle)
        {
            return block.handle;
        }
    }
    return ::MonitorFromPoint({}, MONITOR_DEFAULTTOPRIMARY);
}

windower::rectangle get_display_bounds(::HMONITOR display)
{
    ::MONITORINFO info;
    info.cbSize = sizeof info;
    if (::GetMonitorInfoW(display, &info))
    {
        windower::rectangle result;
        result.location.x  = info.rcMonitor.left;
        result.location.y  = info.rcMonitor.top;
        result.size.width  = info.rcMonitor.right - info.rcMonitor.left;
        result.size.height = info.rcMonitor.bottom - info.rcMonitor.top;
        return result;
    }

    // This should be relatively rare. If, for some reason, we can't
    // retrieve the monitor's bounds, we need a fallback. The user
    // can always override this by explicitly setting the resolution
    // in the launcher. According to Windower 4 analytics, as of
    // October 2015, 1280x720 will work for at least 74% of our users.
    return {{}, {1280u, 720u}};
}

float get_dpi_scale(::HMONITOR display)
{
    if (windower::library shcore{"shcore.dll"})
    {
        using GetDpiForMonitor =
            ::HRESULT(WINAPI)(::HMONITOR, ::MONITOR_DPI_TYPE, ::UINT*, ::UINT*);

        if (auto get_dpi_for_monitor =
                shcore.get_function<GetDpiForMonitor>(u8"GetDpiForMonitor"))
        {
            ::UINT dpi_x;
            ::UINT dpi_y;
            auto const result =
                get_dpi_for_monitor(display, MDT_DEFAULT, &dpi_x, &dpi_y);
            if (SUCCEEDED(result))
            {
                return std::round(dpi_x / 96.f * 20.f) / 20.f;
            }
        }
    }
    if (auto dc = ::GetDC(nullptr))
    {
        auto const scale =
            std::round(::GetDeviceCaps(dc, LOGPIXELSX) / 96.f * 20.f) / 20.f;
        ::ReleaseDC(nullptr, dc);
        return scale;
    }
    return 1.f;
}

}

extern "C"
{
    static ::BOOL CALLBACK enum_display_monitors_callback(
        ::HMONITOR monitor, ::HDC, ::LPRECT, ::LPARAM data)
    {
        auto info   = ::MONITORINFOEXW{};
        info.cbSize = sizeof info;
        if (::GetMonitorInfoW(monitor, &info))
        {
            auto block = std::bit_cast<enum_display_monitors_block*>(data);
            if (::CompareStringEx(
                    LOCALE_NAME_INVARIANT, NORM_IGNORECASE,
                    static_cast<::LPCWCH>(info.szDevice),
                    sizeof info.szDevice / sizeof gsl::at(info.szDevice, 0),
                    block->name.data(), block->name.size(), nullptr, nullptr,
                    0) == CSTR_EQUAL)
            {
                block->handle = monitor;
                return FALSE;
            }
        }
        return TRUE;
    }
}

void windower::settings::load()
{
    settings_channel s;

    debug          = s.get(u8"debug", false);
    developer_mode = s.get(u8"developer_mode", false);

    display_device_name = s.get(u8"display_device_name", u8"");
    auto display        = get_display(to_zstring_view(display_device_name));

    window_type = s.get(u8"window_type", window_type::borderless);

    display_bounds = get_display_bounds(display);

    if (window_type == window_type::window)
    {
        window_bounds.size.width  = s.get(u8"width", 1280);
        window_bounds.size.height = s.get(u8"height", 720);
    }
    else
    {
        window_bounds.size.width = s.get(u8"width", display_bounds.size.width);
        window_bounds.size.height =
            s.get(u8"height", display_bounds.size.height);
    }

    if (window_type == window_type::full_screen)
    {
        window_bounds.location = display_bounds.location;
    }
    else
    {
        window_bounds.location.x = s.get(
            u8"x_position",
            display_bounds.location.x +
                std::max(
                    0, (display_bounds.size.width - window_bounds.size.width) /
                           2));
        window_bounds.location.y = s.get(
            u8"y_position",
            display_bounds.location.y + std::max(
                                            0, (display_bounds.size.height -
                                                window_bounds.size.height) /
                                                   2));
    }

    auto samples_per_pixel = std::min(4.f, s.get(u8"samples_per_pixel", 1.f));

    auto const log_s = std::log2(samples_per_pixel);
    auto const log_w =
        (log_s - 1.f + std::abs(std::abs(std::fmod(log_s, 2.f)) - 1.f)) / 2.f;
    auto const log_h = log_s - log_w;

    render_size.width =
        std::max(1, std::int32_t(window_bounds.size.width * std::exp2(log_w)));
    render_size.height =
        std::max(1, std::int32_t(window_bounds.size.height * std::exp2(log_h)));

    auto ui_scale = std::max(
        1.f, s.get(
                 u8"ui_scale", window_type == window_type::full_screen
                                   ? 1.f
                                   : get_dpi_scale(display)));

    ui_size.width =
        std::max(1, std::int32_t(window_bounds.size.width / ui_scale));
    ui_size.height =
        std::max(1, std::int32_t(window_bounds.size.height / ui_scale));

    hardware_mouse = (window_type == window_type::full_screen) ||
                     s.get(u8"hardware_mouse", true);

    max_sounds                = s.get(u8"max_sounds", 32u);
    play_sound_when_unfocused = s.get(u8"play_sound_when_unfocused", false);

    mipmapping      = s.get(u8"mipmapping", 0u);
    bump_mapping    = s.get(u8"bump_mapping", false);
    map_compression = s.get(u8"map_compression", false);
    texture_compression =
        s.get(u8"texture_compression", texture_compression::uncompressed);
    environment_animation =
        s.get(u8"environment_animation", environment_animation::smooth);
    font_type = s.get(u8"font_type", font_type::uncompressed);

    gamma = s.get(u8"gamma", 2.2f);

    driver_stability = s.get(u8"driver_stability", false);

    play_intro = s.get(u8"play_intro", false);

    verbose_logging = s.get(u8"verbose_logging", debug);

    settings_path = s.get(u8"settings_path", u8"");
    user_path     = s.get(u8"user_path", u8"");
    temp_path     = s.get(u8"temp_path", u8"");

    command_line_args = s.get(u8"command_line_args", u8"");
}
