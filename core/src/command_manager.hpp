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

#ifndef WINDOWER_COMMAND_MANAGER_HPP
#define WINDOWER_COMMAND_MANAGER_HPP

#include <array>
#include <compare>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace windower
{

enum class command_source : std::int32_t
{
    binding             = -2,
    console             = -1,
    client              = 0,
    user                = 1,
    macro               = 2,
    sub_target          = 3,
    sub_target_pc       = 4,
    sub_target_npc      = 5,
    sub_target_party    = 6,
    sub_target_alliance = 7,
};

class command_manager
{
public:
    using handler_type =
        std::function<void(std::vector<std::u8string>, command_source)>;

    enum class layer
    {
        client,
        core,
        addon,
        script,
    };

    static constexpr std::size_t unlimited =
        std::numeric_limits<std::size_t>::max();

    static void initialize() noexcept;
    static command_manager& instance() noexcept;

    static bool is_valid_name(std::u8string_view) noexcept;
    static void check_name(std::u8string_view);

    static std::u8string escape(std::u8string_view);
    static std::u8string unescape(std::u8string_view);
    static std::vector<std::u8string>
        get_arguments(std::u8string_view, std::size_t = unlimited);

    void register_command(
        layer, std::u8string_view, std::u8string_view, handler_type,
        bool = false, std::shared_ptr<void> const& = nullptr);
    void unregister_command(layer, std::u8string_view, std::u8string_view);

    void register_alias(std::u8string_view, std::u8string_view);
    void unregister_alias(std::u8string_view);

    void validate_command(std::u8string_view) const;
    void handle_command(std::u8string_view, command_source);

    void purge() noexcept;

private:
    struct descriptor
    {
        std::u8string component;
        std::u8string command;
        std::weak_ptr<void> tag;
        handler_type handler;
        bool raw    = false;
        layer layer = layer::addon;

        bool operator==(descriptor const&) const noexcept;
        std::strong_ordering operator<=>(descriptor const&) const noexcept;
    };

    struct name_view
    {
        std::optional<std::u8string_view> component;
        std::u8string_view command;

        name_view(descriptor const&) noexcept;
        name_view(std::u8string_view) noexcept;
        name_view(
            std::optional<std::u8string_view>, std::u8string_view) noexcept;

        bool operator==(name_view const&) const noexcept;
        std::weak_ordering operator<=>(name_view const&) const noexcept;
    };

    using u8range = std::pair<std::size_t, std::size_t>;

    std::shared_ptr<void> m_default_tag = std::make_shared<int>();
    std::array<std::vector<descriptor>, 4> m_commands;
    std::vector<std::pair<std::u8string, std::u8string>> m_aliases;

    command_manager() = default;

    std::optional<std::u8string_view> resolve_alias(std::u8string_view) const;

    descriptor const*
        find(std::u8string_view, std::optional<u8range>, u8range) const;
};

}

#endif
