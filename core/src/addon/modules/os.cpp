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

#include "os.hpp"

#include "addon/lua.hpp"
#include "addon/modules/os.lua.hpp"

#include <chrono>
#include <memory>

extern "C"
{
    struct date_info
    {
        int year;
        int month;
        int day;
        int wday;
        int yday;

        int hour;
        int min;
        int sec;
        int nanosec;

        bool isok;
        bool isleap_year;
        bool isdst;

        char const* time_zone;
        size_t time_zone_length;
    };

    static date_info get_date_info(
        double lua_time, char const* time_zone_name,
        std::size_t time_zone_name_length)
    {
        try
        {
            auto const time_zone =
                time_zone_name ? std::chrono::locate_zone(
                                     {time_zone_name, time_zone_name_length})
                               : std::chrono::current_zone();

            auto const time_point =
                std::chrono::sys_time{std::chrono::duration<double>{lua_time}};

            auto const zoned = std::chrono::zoned_time{time_zone, time_point};
            auto const local = zoned.get_local_time();
            auto const date_point = floor<std::chrono::days>(local);
            auto const date       = std::chrono::year_month_day{date_point};
            auto const time       = std::chrono::hh_mm_ss{
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    local - date_point)};

            auto info = date_info{};

            info.year  = int{date.year()};
            info.month = unsigned{date.month()};
            info.day   = unsigned{date.day()};
            info.wday  = std::chrono::weekday{date_point}.iso_encoding();
            info.yday =
                (date_point -
                 std::chrono::local_days{std::chrono::year_month_day{
                     date.year(), std::chrono::month{1}, std::chrono::day{1}}})
                    .count();

            info.hour = time.hours().count();
            info.min  = time.minutes().count();
            info.sec  = int(time.seconds().count());
            info.nanosec =
                int(std::chrono::nanoseconds{time.subseconds()}.count());

            info.isok        = true;
            info.isleap_year = date.year().is_leap();
            info.isdst       = zoned.get_info().save != std::chrono::minutes{};

            info.time_zone        = zoned.get_time_zone()->name().data();
            info.time_zone_length = zoned.get_time_zone()->name().size();

            return info;
        }
        catch (std::runtime_error const&)
        {
            date_info info{};
            info.isok = false;
            return info;
        }
    }

    static char const* format_date(
        char const* format_str, std::size_t format_str_length, double lua_time,
        char const* time_zone_name, std::size_t time_zone_name_length,
        std::size_t& result_length)
    {
        try
        {
            auto const time_zone =
                time_zone_name ? std::chrono::locate_zone(
                                     {time_zone_name, time_zone_name_length})
                               : std::chrono::current_zone();

            if (format_str_length > 0 && *format_str == u8'!')
            {
                --format_str_length;
                ++format_str;
            }

            auto const time_point =
                std::chrono::sys_time{std::chrono::duration<double>{lua_time}};

            auto const zoned = std::chrono::zoned_time{
                time_zone, floor<std::chrono::seconds>(time_point)};
            auto formatted_string = std::string{};
            try
            {
                formatted_string = std::vformat(
                    std::string_view{format_str, format_str_length},
                    std::make_format_args(zoned));
            }
            catch (std::format_error const&)
            {
                result_length = 1;
                return nullptr;
            }

            auto formatted_string_view = std::string_view{formatted_string};
            formatted_string_view.remove_prefix(1);
            result_length = formatted_string_view.size();
            auto result   = std::make_unique<char[]>(result_length);
            std::copy_n(
                formatted_string_view.begin(), result_length, result.get());

            return result.release();
        }
        catch (std::runtime_error const&)
        {
            result_length = 0;
            return nullptr;
        }
    }

    static void delete_string(char const* string)
    {
        auto const deleter = std::unique_ptr<char const []> { string };
    }

    static double get_current_time()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static double get_current_time_zoned(
        char const* time_zone_name, std::size_t time_zone_name_length)
    {
        try
        {
            auto const time_zone =
                time_zone_name ? std::chrono::locate_zone(
                                     {time_zone_name, time_zone_name_length})
                               : std::chrono::current_zone();
            auto const zoned = std::chrono::zoned_time{
                time_zone, std::chrono::system_clock::now()};
            auto const local = zoned.get_local_time();
            return std::chrono::duration_cast<std::chrono::duration<double>>(
                       local.time_since_epoch())
                .count();
        }
        catch (std::runtime_error const&)
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    static double get_time_from_date_info(date_info const* lua_date)
    {
        try
        {
            auto const time_zone =
                lua_date->time_zone
                    ? std::chrono::locate_zone(
                          {lua_date->time_zone, lua_date->time_zone_length})
                    : std::chrono::current_zone();

            auto const local =
                std::chrono::local_days{
                    std::chrono::year_month_day{
                        std::chrono::year{lua_date->year},
                        std::chrono::month{0}, std::chrono::day{0}} +
                    std::chrono::months{lua_date->month}} +
                std::chrono::days{lua_date->day} +
                std::chrono::hours{lua_date->hour} +
                std::chrono::minutes{lua_date->min} +
                std::chrono::seconds{lua_date->sec};

            auto const zoned = std::chrono::zoned_time{time_zone, local};
            auto const time  = zoned.get_sys_time() +
                              std::chrono::duration<double>{
                                  std::chrono::nanoseconds{lua_date->nanosec}};

            return time.time_since_epoch().count();
        }
        catch (std::runtime_error const&)
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
}

int windower::preload_os_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::push(guard, u8"_PRELOAD");
    lua::raw_get(guard, lua::registry);
    lua::push(guard, u8"os");
    lua::load(guard, lua_os_source, u8"os");
    lua::push(guard, u8"os");
    lua::raw_get(guard, -4);
    lua::push(guard, &get_date_info);
    lua::push(guard, &format_date);
    lua::push(guard, &delete_string);
    lua::push(guard, &get_current_time);
    lua::push(guard, &get_current_time_zoned);
    lua::push(guard, &get_time_from_date_info);
    lua::call(guard, 7);
    lua::raw_set(guard, -3);
    lua::pop(guard);

    return guard.release();
}
