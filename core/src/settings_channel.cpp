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

#include "settings_channel.hpp"

#include "handle.hpp"
#include "utility.hpp"

#include <windows.h>

#include <pugixml.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

windower::settings_channel::settings_channel() noexcept
{
    std::wostringstream s;
    s << LR"(Windower.Settings)";
    s << L'[' << std::hex << std::uppercase << std::setw(8)
      << std::setfill(L'0') << ::GetCurrentProcessId() << L']';
    s << LR"(.Data)";
    auto const name = s.str();

    handle const file =
        ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (!file)
    {
        throw_system_error();
    }

    auto const* ptr = ::MapViewOfFile(file, FILE_MAP_READ, 0, 0, 0);
    auto const view = static_cast<std::uint64_t const*>(ptr);
    if (!view)
    {
        throw_system_error();
    }

    auto const size = *view;
    m_document.load_buffer(std::next(view), gsl::narrow<std::size_t>(size));
    m_root = m_document.child("settings");
}

windower::settings_channel::~settings_channel()
{
    std::wostringstream s;
    s << LR"(Windower.Settings)";
    s << L'[' << std::hex << std::uppercase << std::setw(8)
      << std::setfill(L'0') << ::GetCurrentProcessId() << L']';
    s << LR"(.Flag)";
    auto const name = s.str();

    handle const flag = ::OpenEventW(EVENT_MODIFY_STATE, false, name.c_str());
    if (!flag)
    {
        fail_fast();
    }

    ::SetEvent(flag);
}

auto windower::settings_channel::get(
    u8zstring_view key, std::u8string_view def) const -> std::u8string
{
    auto key_ptr = to_zstring_view(key).c_str();
    if (auto const node = m_root.child(key_ptr).text())
    {
        std::string_view const view = node.get();
        return {view.begin(), view.end()};
    }
    return std::u8string{def};
}

auto windower::settings_channel::get(
    u8zstring_view key, char8_t const* def) const -> std::u8string
{
    return get(key, std::u8string_view{def});
}

auto windower::settings_channel::get(u8zstring_view key, bool def) const -> bool
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_bool(def);
}

auto windower::settings_channel::get(u8zstring_view key, float def) const
    -> float
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_float(def);
}

auto windower::settings_channel::get(u8zstring_view key, double def) const
    -> double
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_double(def);
}

auto windower::settings_channel::get(u8zstring_view key, char def) const -> char
{
    auto key_ptr = to_zstring_view(key).c_str();
    if constexpr (std::is_signed_v<char>)
    {
        return gsl::narrow_cast<char>(m_root.child(key_ptr).text().as_int(def));
    }
    else
    {
        return gsl::narrow_cast<char>(
            m_root.child(key_ptr).text().as_uint(def));
    }
}

auto windower::settings_channel::get(u8zstring_view key, signed char def) const
    -> signed char
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<signed char>(
        m_root.child(key_ptr).text().as_int(def));
}

auto windower::settings_channel::get(u8zstring_view key, short def) const
    -> short
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<short>(m_root.child(key_ptr).text().as_int(def));
}

auto windower::settings_channel::get(u8zstring_view key, int def) const -> int
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_int(def);
}

auto windower::settings_channel::get(u8zstring_view key, long def) const
    -> long int
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<long>(m_root.child(key_ptr).text().as_llong(def));
}

auto windower::settings_channel::get(u8zstring_view key, long long def) const
    -> long long int
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_llong(def);
}

auto windower::settings_channel::get(
    u8zstring_view key, unsigned char def) const -> unsigned char
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<unsigned char>(
        m_root.child(key_ptr).text().as_uint(def));
}

auto windower::settings_channel::get(
    u8zstring_view key, unsigned short def) const -> unsigned short
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<unsigned short>(
        m_root.child(key_ptr).text().as_uint(def));
}

auto windower::settings_channel::get(u8zstring_view key, unsigned def) const
    -> unsigned
{
    auto key_ptr = to_zstring_view(key).c_str();
    return m_root.child(key_ptr).text().as_uint(def);
}

auto windower::settings_channel::get(
    u8zstring_view key, unsigned long def) const -> unsigned long
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<unsigned long>(
        m_root.child(key_ptr).text().as_llong(def));
}

auto windower::settings_channel::get(
    u8zstring_view key, unsigned long long def) const -> unsigned long long
{
    auto key_ptr = to_zstring_view(key).c_str();
    return gsl::narrow_cast<unsigned long long>(
        m_root.child(key_ptr).text().as_llong(def));
}
