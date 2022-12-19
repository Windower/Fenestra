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

#ifndef WINDOWER_BINDING_MANAGER_HPP
#define WINDOWER_BINDING_MANAGER_HPP

#include <windows.h>

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace windower
{

class key_handle
{
public:
    bool operator==(key_handle const&) const noexcept = default;

private:
    std::uint8_t m_device_id;
    std::uint8_t m_key_id;

    friend class binding_manager;
};

class binding_manager
{
public:
    enum class flag
    {
        up   = 0,
        chat = 1,
    };

    std::vector<key_handle> register_device(
        std::u8string_view device_name, std::vector<std::u8string> const& keys);
    void unregister_device(std::u8string_view device_name);

    bool key_state(key_handle key) const noexcept;
    bool key_state(key_handle key, bool pressed);

    bool key_state(std::uint8_t key) const noexcept;
    bool key_state(std::uint8_t key, bool pressed);

    void lost_focus();

    void bind(std::u8string_view bind_string);
    void bind(std::u8string_view bind_string, std::u8string_view command);
    void bind(
        std::u8string_view bind_string, std::function<bool()> const& handler,
        std::u8string_view = u8"");

    void unbind(std::u8string_view bind_string);

    std::vector<std::pair<std::u8string, std::u8string>> get_binds() const;

    std::array<std::uint8_t, 256> const& client_state() const noexcept;

    std::optional<::LRESULT> process_message(::MSG const&) noexcept;

private:
    static constexpr std::size_t max_devices = 64;
    static constexpr std::size_t max_flags   = 32;

    class binding
    {
    public:
        std::bitset<max_devices> device_mask;
        std::vector<key_handle> keys;
        std::bitset<max_flags> predicate_mask;
        std::bitset<max_flags> predicate;
        std::function<bool()> handler;
        std::u8string handler_display;

        bool operator==(binding const&) const;
        bool operator<(binding const&) const noexcept;
    };

    class device_descriptor
    {
    public:
        std::uint8_t id = 0;
        std::u8string name;
        std::map<std::u8string, std::tuple<std::uint8_t, std::u8string>> keys;
    };

    static flag get_flag(std::u8string_view, std::size_t, std::size_t);
    static std::u8string_view get_flag_name(std::size_t) noexcept;

    std::array<bool, 256> m_system_device                = {};
    std::array<std::uint8_t, 256> m_system_device_client = {};
    std::array<std::tuple<bool, std::vector<std::uint8_t>>, max_devices - 1>
        m_devices;
    std::bitset<max_flags> m_flags;
    std::vector<binding> m_bindings;
    std::map<std::u8string, device_descriptor> m_descriptors;

    std::tuple<binding, std::size_t>
    create_binding(std::u8string_view, bool) const;
    void insert_binding(binding&&);

    std::u8string get_name(key_handle) const;
    key_handle find(std::u8string_view, std::size_t, std::size_t) const;
    key_handle find(
        std::u8string_view, std::size_t, std::size_t, std::size_t,
        std::size_t) const;
};

}

#endif
