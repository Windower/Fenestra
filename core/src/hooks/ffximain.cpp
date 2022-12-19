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

#include "hooks/ffximain.hpp"

#include "addon/addon.hpp"
#include "addon/modules/chat.hpp"
#include "addon/modules/event.hpp"
#include "addon/modules/packet.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "hooklib/hook.hpp"
#include "scanner.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <tuple>
#include <vector>

using namespace windower::signature_literals;

namespace
{

std::shared_ptr<int> command_tag;

struct chat_log;
struct renderer;

struct chat_mode
{
    void const* __vfptr;
    void* _unknown;
    char8_t command_prefix[128];
    char8_t previous_command_prefix[128];
    std::uint32_t mode_id;
    std::uint32_t previous_mode_id;
};

struct autotranslate_dictionary;

struct cipher_data
{
    std::uint8_t key[16];
    std::uint32_t p_array[18];
    std::uint32_t s_boxes[4][256];
    std::uint32_t _unknown_1[16];
    std::uint32_t _unknown_2;
    std::uint16_t counter;
    std::uint16_t _padding;
};

struct menu_entry
{
    char8_t type[8];
    char8_t name[8];
    std::byte _unknown_10_1F[16];
    void const* const* data;
    std::uint32_t _unknown_24_27;
    std::uint32_t _unknown_28_2B;
};

std::vector<std::byte> temp_buffer{};
std::uint16_t last_out_counter;

constexpr std::size_t udp_packet_header_size = 28;
constexpr std::size_t packet_max_size        = 508;

namespace signatures
{

constexpr windower::signature chat_log_ptr =
    u8"884C2409884C240A518D4C240C518B0D"_sig;
constexpr windower::signature chat_mode_ptr =
    u8"8D4C24086A006A016A0050518B0D????????E8????????8B0D"_sig;
constexpr windower::signature autotranslate_ptr =
    u8"8BFE83C9FF33C06A00F2AEF7D1496A01518B0D&"_sig;
constexpr windower::signature menu_ptr = u8"5355568B30A0"_sig;

constexpr windower::signature add_to_chat =
    u8"&81EC44080000535556578BE96A086A00"_sig;
constexpr windower::signature input_command =
    u8"&81EC040400008B8C2408040000555657"_sig;
constexpr windower::signature decode_packet =
    u8"&8B44241881ECB805000053558BAC24C8050000563BE857"_sig;
constexpr windower::signature encode_packet =
    u8"&518B4C24145333C0558B6C24144956578D14CD0000000033FF85ED89542410"_sig;
constexpr windower::signature autotranslate_lookup =
    u8"&5153558B6C24145685ED578BF1C644241000C644241100C64424"_sig;
constexpr windower::signature draw_scene = u8"&568BF18B86500D0000"_sig;

}

namespace callbacks
{

bool add_to_chat(
    chat_log const*, windower::sjis_char const*, std::uint8_t&, bool,
    std::int32_t, void*);
void input_command(windower::sjis_char const*, windower::command_source);
std::size_t decode_packet(
    std::byte*, std::size_t, cipher_data const (&)[2], void const*,
    std::byte const*, std::size_t);
std::size_t encode_packet(
    std::byte const*, std::size_t, std::byte*, std::size_t, void const*);
std::size_t lookup_autotranslate(
    autotranslate_dictionary const*, std::uint32_t, char*,
    std::size_t) noexcept;

void draw_scene(renderer const*) noexcept;

}

namespace hooks
{

chat_log const* const* chat_log_ptr               = nullptr;
chat_mode const* const* chat_mode_ptr             = nullptr;
autotranslate_dictionary const* autotranslate_ptr = nullptr;
menu_entry const* menu_ptr                        = nullptr;

windower::hooklib::hook<
    bool(windower::sjis_char const*, std::uint8_t&, bool, std::int32_t, void*),
    chat_log const>
    add_to_chat;
windower::hooklib::hook<decltype(callbacks::input_command)> input_command;
windower::hooklib::hook<decltype(callbacks::decode_packet)> decode_packet;
windower::hooklib::hook<decltype(callbacks::encode_packet)> encode_packet;
windower::hooklib::hook<
    std::size_t(std::uint32_t, char*, std::size_t),
    autotranslate_dictionary const>
    lookup_autotranslate;
windower::hooklib::hook<void(), renderer const> draw_scene;

}

template<std::size_t N>
std::string_view lookup_autotranslate_impl(
    char32_t code_point, std::array<char, N>& shift_jis_buffer) noexcept
{
    using namespace windower;

    if (!hooks::autotranslate_ptr)
    {
        if (auto const temp =
                **scan(u8"ffximain.dll", signatures::autotranslate_ptr))
        {
            hooks::autotranslate_ptr = *(temp + 0x7E6C);
        }
        else
        {
            return {};
        }
    }

    std::uint32_t code = 0;
    if (code_point >= U'\U000F0000' && code_point <= U'\U000F7FFF')
    {
        if ((code_point & 0xFF00) != 0 && (code_point & 0x00FF) != 0)
        {
            code = code_point & 0xFFFF | 0x02020000;
        }
    }
    else if (code_point == U'\U000F8000')
    {
        code = 0x1402FFFF;
    }
    else if (code_point >= U'\U000F8001' && code_point <= U'\U000FFFFD')
    {
        code_point -= U'\U000F8001';
        code = code_point | ((code_point & 0xFF00) == 0 ? 0x1502FF00
                             : (code_point & 0xFF) == 0 ? 0x160200FF
                                                        : 0x13020000);
    }
    else if (code_point == U'\U00100000')
    {
        code = 0x0702FFFF;
    }
    else if (code_point >= U'\U00100001' && code_point <= U'\U0010FFFD')
    {
        code &= 0xFFFF;
        code = code_point | ((code_point & 0xFF00) == 0 ? 0x0902FF00
                             : (code_point & 0xFF) == 0 ? 0x0A0200FF
                                                        : 0x07020000);
    }

    if (code == 0)
    {
        return {};
    }

    auto size = hooks::lookup_autotranslate(
        hooks::autotranslate_ptr, change_endian(code), shift_jis_buffer.data(),
        shift_jis_buffer.size());
    return {shift_jis_buffer.data(), size <= N ? size : 0};
}

namespace callbacks
{

bool add_to_chat(
    chat_log const* object, windower::sjis_char const* text_shift_jis,
    std::uint8_t& type, bool indented, std::int32_t unknown_1, void* unknown_2)
{
    using namespace windower;

    if (!text_shift_jis || *text_shift_jis == '\0' || unknown_2)
    {
        return hooks::add_to_chat(
            object, text_shift_jis, type, indented, unknown_1, unknown_2);
    }

    auto result =
        trigger_text_added(to_u8string(text_shift_jis), type, indented);

    if (result.blocked())
    {
        return false;
    }

    if (result.unchanged())
    {
        return hooks::add_to_chat(
            object, text_shift_jis, type, indented, unknown_1, unknown_2);
    }

    auto result_type = result.type();

    if (result.text().empty())
    {
        constexpr std::array<sjis_char, 2> blank{0x20, 0x00};
        return hooks::add_to_chat(
            object, blank.data(), result_type, result.indented(), unknown_1,
            unknown_2);
    }

    auto result_text_shift_jis = windower::to_sjis_string(result.text());

    return hooks::add_to_chat(
        object, result_text_shift_jis.c_str(), result_type, result.indented(),
        unknown_1, unknown_2);
}

void input_command(
    windower::sjis_char const* shift_jis_command,
    windower::command_source source)
{
    using namespace windower;

    auto utf8_command = to_u8string(shift_jis_command);
    auto is_command   = false;

    auto it        = std::size_t{};
    auto const end = utf8_command.size();

    auto const first = next_code_point(utf8_command, it);
    if (first == U'/' && it != end &&
        !is_whitespace(next_code_point(utf8_command, it)))
    {
        is_command = true;
    }
    else if (first >= U'\U000F0000' && first <= U'\U000F7FFD')
    {
        std::array<char, 450> buffer{};
        auto const shift_jis_at_phrase =
            lookup_autotranslate_impl(first, buffer);
        if (!shift_jis_at_phrase.empty() && shift_jis_at_phrase.front() == '/')
        {
            is_command      = true;
            auto command    = to_u8string(shift_jis_at_phrase);
            auto const mark = it;
            if (!is_whitespace(next_code_point(utf8_command, it)))
            {
                command += u8' ';
            }
            utf8_command.replace(0, mark, command);
        }
    }

    if (!is_command)
    {
        if (!hooks::chat_mode_ptr)
        {
            hooks::input_command(shift_jis_command, source);
            return;
        }
        utf8_command.insert(0, &(*hooks::chat_mode_ptr)->command_prefix[0]);
    }

    try
    {
        command_manager::instance().handle_command(utf8_command, source);
    }
    catch (...)
    {
        core::error(u8"", std::current_exception(), source);
    }
}

std::size_t decode_packet(
    std::byte* output_ptr, std::size_t output_size,
    cipher_data const (&cipher_data)[2], void const* decompression_tree,
    std::byte const* input_ptr, std::size_t input_size)
{
    using namespace windower;

    struct udp_header
    {
        std::uint16_t server_counter;
        std::uint16_t client_counter;
        std::uint32_t _unknown_04_07;
        std::uint32_t timestamp;
        std::uint32_t _unknown_0C_0F;
        std::uint32_t _unknown_10_13;
        std::uint32_t _unknown_14_17;
        std::uint32_t _unknown_18_1B;
    };

    static_assert(sizeof(udp_header) == 28);
    static_assert(std::is_trivially_copyable_v<udp_header>);

    if (temp_buffer.size() < output_size)
    {
        temp_buffer.clear();
        temp_buffer.resize(output_size);
    }
    auto decoded_size = hooks::decode_packet(
        temp_buffer.data(), temp_buffer.size(), cipher_data, decompression_tree,
        input_ptr, input_size);
    std::span<std::byte const> input{temp_buffer.data(), decoded_size};
    std::span<std::byte> output{output_ptr, output_size};

    udp_header header{};
    auto const header_buffer = std::as_writable_bytes(std::span{&header, 1});
    if (input.size() < header_buffer.size())
    {
        std::copy(input.begin(), input.end(), output.begin());
        return input.size();
    }
    std::copy_n(input.begin(), header_buffer.size(), header_buffer.begin());
    std::copy(header_buffer.begin(), header_buffer.end(), output.begin());

    input  = input.subspan(header_buffer.size());
    output = output.subspan(header_buffer.size());

    auto const counter   = header.server_counter;
    auto const timestamp = header.timestamp;
    auto& queue          = *core::instance().incoming_packet_queue;
    auto const result =
        queue.process_buffer(input, counter, timestamp, output.size());
    std::copy(result.begin(), result.end(), output.begin());

    return header_buffer.size() + result.size();
}

std::size_t encode_packet(
    std::byte const* input_ptr, std::size_t input_size, std::byte* output_ptr,
    std::size_t output_size, void const* compression_table)
{
    using namespace windower;

    // auto const output_bytes_upper_bound = (output_size - 1) * 8;
    auto const output_bytes_lower_bound = (output_size - 1) * 2 / 3;

    auto const input = std::span{input_ptr, input_size};

    auto const counter =
        !input.empty()
            ? std::to_integer<std::uint16_t>(gsl::at(input, 2)) +
                  std::to_integer<std::uint16_t>(gsl::at(input, 3)) * 0x100
            : last_out_counter + 1;
    last_out_counter  = gsl::narrow_cast<std::uint16_t>(counter);
    auto& queue       = *core::instance().outgoing_packet_queue;
    auto const result = queue.process_buffer(
        input, last_out_counter, 0, output_bytes_lower_bound);

    return hooks::encode_packet(
        result.data(), result.size(), output_ptr, output_size,
        compression_table);
}

std::size_t lookup_autotranslate(
    autotranslate_dictionary const* object, std::uint32_t code, char* buffer,
    std::size_t buffer_size) noexcept
{
    return hooks::lookup_autotranslate(object, code, buffer, buffer_size);
}

void draw_scene(renderer const* renderer) noexcept
{
    hooks::draw_scene(renderer);

    auto& core = windower::core::instance();

    core.update();
    core.ui.render(windower::ui::layer::world);
}

}

void client_command(
    std::vector<std::u8string> utf8_args, windower::command_source source)
{
    using namespace windower;

    if (source <= command_source::console)
    {
        source = command_source::user;
    }
    if (hooks::input_command)
    {
        hooks::input_command(
            to_sjis_string(command_manager::unescape(utf8_args.at(0))).c_str(),
            source);
    }
}

}

#include <numeric>

void windower::ffximain::install()
{
    if (!command_tag)
    {
        command_tag = std::make_shared<int>();

        hooks::chat_log_ptr = scan(u8"ffximain.dll", signatures::chat_log_ptr);
        hooks::chat_mode_ptr =
            scan(u8"ffximain.dll", signatures::chat_mode_ptr);
        hooks::menu_ptr = scan(u8"ffximain.dll", signatures::menu_ptr);

        hooks::add_to_chat = hooklib::make_hook_thiscall(
            scan(u8"ffximain.dll", signatures::add_to_chat),
            callbacks::add_to_chat);
        hooks::input_command = hooklib::make_hook(
            scan(u8"ffximain.dll", signatures::input_command),
            callbacks::input_command);
        hooks::decode_packet = hooklib::make_hook(
            scan(u8"ffximain.dll", signatures::decode_packet),
            callbacks::decode_packet);
        hooks::encode_packet = hooklib::make_hook(
            scan(u8"ffximain.dll", signatures::encode_packet),
            callbacks::encode_packet);
        hooks::lookup_autotranslate = hooklib::make_hook_thiscall(
            scan(u8"ffximain.dll", signatures::autotranslate_lookup),
            callbacks::lookup_autotranslate);
        hooks::draw_scene = hooklib::make_hook_thiscall(
            scan(u8"ffximain.dll", signatures::draw_scene),
            callbacks::draw_scene);

        static constexpr auto resource_name = L"client-commands";
        auto const library = static_cast<::HMODULE>(windower_module());
        if (auto hrsrc = ::FindResourceW(library, resource_name, RT_RCDATA))
        {
            if (auto resource = ::LoadResource(library, hrsrc))
            {
                static constexpr std::u8string_view name_chars =
                    u8"abcdefghijklmnopqrstuvwxyz"
                    u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    u8"0123456789?_-";

                std::u8string_view const command_list{
                    static_cast<char8_t const*>(::LockResource(resource)),
                    ::SizeofResource(library, hrsrc)};

                auto& commmand_manager = windower::command_manager::instance();
                auto first             = std::u8string_view::size_type{};
                auto last = command_list.find_first_not_of(name_chars);
                while (last != std::u8string_view::npos)
                {
                    if (first != last)
                    {
                        commmand_manager.register_command(
                            command_manager::layer::client, {},
                            command_list.substr(first, last - first),
                            client_command, true, command_tag);
                    }
                    first = last + 1;
                    last  = command_list.find_first_not_of(name_chars, first);
                }
            }
        }
    }
}

void windower::ffximain::uninstall() noexcept
{
    command_tag.reset();
    command_manager::instance().purge();

    hooks::encode_packet        = {};
    hooks::decode_packet        = {};
    hooks::input_command        = {};
    hooks::add_to_chat          = {};
    hooks::lookup_autotranslate = {};
    hooks::draw_scene           = {};

    hooks::chat_log_ptr      = nullptr;
    hooks::chat_mode_ptr     = nullptr;
    hooks::autotranslate_ptr = nullptr;
}

std::u8string windower::ffximain::lookup_autotranslate(char32_t code_point)
{
    std::array<char, 450> shift_jis_buffer{};
    auto const view = lookup_autotranslate_impl(code_point, shift_jis_buffer);
    return to_u8string(view);
}

void windower::ffximain::add_to_chat(
    std::u8string_view text, std::uint8_t type, bool indented)
{
    if (hooks::chat_log_ptr && *hooks::chat_log_ptr && hooks::add_to_chat)
    {
        auto result = trigger_text_added(text, type, indented);

        if (result.blocked())
        {
            return;
        }

        if (result.unchanged())
        {
            if (text.empty())
            {
                constexpr std::array<sjis_char, 2> blank{0x20, 0x00};
                hooks::add_to_chat(
                    *hooks::chat_log_ptr, blank.data(), type, indented, 1,
                    nullptr);
            }
            else
            {
                auto result_text_shift_jis = windower::to_sjis_string(text);
                hooks::add_to_chat(
                    *hooks::chat_log_ptr, result_text_shift_jis.c_str(), type,
                    indented, 1, nullptr);
            }
        }
        else
        {
            auto result_type = result.type();
            if (auto const result_text = result.text(); result_text.empty())
            {
                constexpr std::array<sjis_char, 2> blank{0x20, 0x00};
                hooks::add_to_chat(
                    *hooks::chat_log_ptr, blank.data(), result_type,
                    result.indented(), 1, nullptr);
            }
            else
            {
                auto result_text_shift_jis =
                    windower::to_sjis_string(result_text);
                hooks::add_to_chat(
                    *hooks::chat_log_ptr, result_text_shift_jis.c_str(),
                    result_type, result.indented(), 1, nullptr);
            }
        }
    }
}

void const* windower::ffximain::menu(
    std::u8string_view name, std::u8string_view type) noexcept
{
    for (auto it = hooks::menu_ptr; it && it->data; std::advance(it, 1))
    {
        auto const name_ptr  = static_cast<char8_t const*>(it->name);
        auto const name_size = std::size(it->name);
        auto const type_ptr  = static_cast<char8_t const*>(it->type);
        auto const type_size = std::size(it->type);

        if (std::u8string_view{name_ptr, name_size} == name &&
            std::u8string_view{type_ptr, type_size} == type)
        {
            return *it->data;
        }
    }
    return nullptr;
}
