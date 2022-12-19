--[[
Copyright © Windower Dev Team

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"),to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

local ffi = require('ffi')
local string = require('string')

local args = {...}

local original_loader = args[1]

local date_info = ffi.typeof [[struct
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
}]]

local size_t_ref = ffi.typeof('size_t[1]')

local get_date_info_t = ffi.typeof('$(*)(double, char const*, size_t)',
                                   date_info)
local format_date_t = ffi.typeof(
                          'char const*(*)(char const*, size_t, double, char const*, size_t, size_t*)')
local delete_string_t = ffi.typeof('void(*)(char const*)')
local get_current_time_t = ffi.typeof('double(*)()')
local get_current_time_zoned_t = ffi.typeof('double(*)(char const*, size_t)')
local get_time_from_date_info_t = ffi.typeof('double(*)($ const*)', date_info)

local get_date_info = ffi.cast(get_date_info_t, args[2])
local format_date = ffi.cast(format_date_t, args[3])
local delete_string = ffi.cast(delete_string_t, args[4])
local get_current_time = ffi.cast(get_current_time_t, args[5])
local get_current_time_zoned = ffi.cast(get_current_time_zoned_t, args[6])
local get_time_from_date_info = ffi.cast(get_time_from_date_info_t, args[7])

local string_gsub = string.gsub

local date = function(format, time, time_zone)
    if format == nil then
        format = '%c'
    elseif type(format) ~= 'string' then
        error('bad argument #1 to \'date\' (string expected, got ' ..
                  type(format) .. ')', 2)
    end

    if time == nil then
        time = get_current_time()
    elseif type(time) ~= 'number' then
        error(
            'bad argument #2 to \'date\' (number expected, got ' .. type(time) ..
                ')', 2)
    end

    if time_zone ~= nil and type(time_zone) ~= 'string' then
        error('bad argument #3 to \'date\' (string expected, got ' ..
                  type(time_zone) .. ')', 2)
    end

    if string.byte(format, 1) == 0x21 then time_zone = 'Etc/UTC' end

    if format == '*t' or format == '!*t' then
        local info = get_date_info(time, time_zone,
                                   time_zone == nil and 0 or #time_zone)

        if not info.isok then error('time zone not found', 2) end

        local result = {
            year = info.year,
            month = info.month,
            day = info.day,
            wday = info.wday,
            yday = info.yday,

            hour = info.hour,
            min = info.min,
            sec = info.sec,
            nanosec = info.nanosec,

            isdst = info.isdst,
            isleap_year = info.isleap_year,

            time_zone = ffi.string(info.time_zone, info.time_zone_length)
        }
        return result
    else
        local length = size_t_ref()
        format = '{0:%%' .. string_gsub(format, '}', '}}') .. '}'
        local ptr = ffi.gc(format_date(format, #format, time, time_zone,
                                       time_zone == nil and 0 or #time_zone,
                                       length), delete_string)
        if ptr == nil then
            if length[0] == 0 then
                error('time zone not found', 2)
            elseif length[0] == 1 then
                error('invalid format string', 2)
            end
            error('<unknown format error: ' .. length[0] .. '>', 2)
        end

        return ffi.string(ptr, length[0])
    end
end

local time = function(t)
    if t == nil then
        return get_current_time()
    else
        local t_type = type(t)
        if t_type == 'string' then
            return get_current_time_zoned(t, #t)
        elseif t_type ~= 'table' then
            error('bad argument #1 to \'time\' (table expected, got ' ..
                      type(format) .. ')', 2)
        end
    end

    local info = date_info()

    local day = t.day
    if day == nil then
        error('field \'day\' missing in date table', 2)
    else
        day = tonumber(day)
        if day == nil or day % 1 ~= 0 then
            error('field \'day\' is not an integer', 2)
        end
    end

    local month = t.month
    if month == nil then
        error('field \'month\' missing in date table', 2)
    else
        month = tonumber(month)
        if month == nil or month % 1 ~= 0 then
            error('field \'month\' is not an integer', 2)
        end
    end

    local year = t.year
    if year == nil then
        error('field \'year\' missing in date table', 2)
    else
        year = tonumber(year)
        if year == nil or year % 1 ~= 0 then
            error('field \'year\' is not an integer', 2)
        end
    end

    local sec = t.sec
    if sec == nil then
        sec = 0
    else
        sec = tonumber(sec)
        if sec == nil or sec % 1 ~= 0 then
            error('field \'sec\' is not an integer', 2)
        end
    end

    local min = t.min
    if min == nil then
        min = 0
    else
        min = tonumber(min)
        if min == nil or min % 1 ~= 0 then
            error('field \'min\' is not an integer', 2)
        end
    end

    local hour = t.hour
    if hour == nil then
        hour = 0
    else
        hour = tonumber(hour)
        if hour == nil or hour % 1 ~= 0 then
            error('field \'hour\' is not an integer', 2)
        end
    end

    local nanosec = t.nanosec
    if nanosec == nil then
        nanosec = 0
    else
        nanosec = tonumber(nanosec)
        if nanosec == nil or nanosec % 1 ~= 0 then
            error('field \'nanosec\' is not an integer', 2)
        end
    end

    local time_zone = t.time_zone
    if time_zone ~= nil and type(time_zone) ~= 'string' then
        error('field \'time_zone\' is not a string', 2)
    end

    info.year = year
    info.month = month
    info.day = day

    info.hour = hour
    info.min = min
    info.sec = sec
    info.nanosec = nanosec

    info.time_zone = time_zone
    info.time_zone_length = time_zone and #t.time_zone or 0

    local result = get_time_from_date_info(info)
    if result ~= result then error('time zone not found', 2) end

    return result
end

local difftime = function(t1, t2)
    if type(t1) ~= 'number' then
        error('bad argument #1 to \'difftime\' (number expected, got ' ..
                  type(t1) .. ')', 2)
    end

    if type(t2) ~= 'number' then
        error('bad argument #2 to \'difftime\' (number expected, got ' ..
                  type(t2) .. ')', 2)
    end

    return t2 - t1
end

local loader = function(name)
    local module = original_loader(name)

    module.date = date
    module.time = time
    module.difftime = difftime
end

return loader
