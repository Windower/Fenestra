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

#ifndef WINDOWER_UI_WINDOW_HPP
#define WINDOWER_UI_WINDOW_HPP

#include "ui/command_buffer.hpp"
#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "ui/rectangle.hpp"
#include "ui/static_any.hpp"
#include "ui/vector.hpp"

#include <windows.h>

#include <compare>
#include <optional>
#include <vector>

namespace windower::ui
{

class context;
class window_manager;
class window;

template<typename T>
concept message_processor = requires(T value, ::MSG const& msg)
{
    // clang-format off
    { value.process_message(msg) } noexcept ->
        std::convertible_to<std::optional<::LRESULT>>;
    // clang-format on
};

class window
{
public:
    bool operator==(window const&) const noexcept;
    std::partial_ordering operator<=>(window const&) const noexcept;

    void bounds(rectangle const& bounds) noexcept;
    rectangle const& bounds() const noexcept;

    void origin(vector const& origin) noexcept;
    vector const& origin() const noexcept;

    void zoom_factor(float zoom_factor) noexcept;
    float zoom_factor() const noexcept;

    void interactable(bool interactable) noexcept;
    bool interactable() const noexcept;

    layer layer() const noexcept;
    float depth() const noexcept;
    command_buffer& commands() noexcept;
    command_buffer const& commands() const noexcept;

    bool valid(std::size_t) const noexcept;

    bool is_descendent_of(window const& ancestor) const noexcept;

    id focused_id() const noexcept;

    bool is_blurred() const noexcept;
    bool is_focused(id id) const noexcept;
    bool focus(id id) noexcept;
    bool blur(id id) noexcept;

    template<typename T, typename... A>
    std::remove_cvref_t<T>* focus(id id, A&&... args) noexcept
    {
        focus(id);
        auto const state = &m_focus_state.emplace<T>(std::forward<A>(args)...);
        if constexpr (message_processor<T>)
        {
            m_message_handler =
                [](window& wnd,
                   ::MSG const& msg) noexcept -> std::optional<::LRESULT> {
                if (auto state = wnd.m_focus_state.value<T>())
                {
                    return {state->process_message(msg)};
                }
                return std::nullopt;
            };
        }
        return state;
    }

    template<typename T>
    std::remove_cvref_t<T>* focus_state(id id) noexcept
    {
        if (is_focused(id))
        {
            return m_focus_state.value<T>();
        }
        return nullptr;
    }

    template<typename T>
    std::remove_cvref_t<T>* focus_state(id id, T&& state) noexcept
    {
        if (is_focused(id))
        {
            return &m_focus_state.emplace<T>(std::forward<T>(state));
        }
        return nullptr;
    }

    template<typename T, typename... A>
    std::remove_cvref_t<T>* emplace_focus_state(id id, A&&... args) noexcept
    {
        if (is_focused(id))
        {
            return &m_focus_state.emplace<T>(std::forward<T>(args)...);
        }
        return nullptr;
    }

private:
    std::uintptr_t m_id    = 0;
    float m_depth          = 0.f;
    window const* m_parent = nullptr;
    ui::command_buffer m_commands;
    ui::layer m_layer = ui::layer::screen;

    rectangle m_bounds;
    vector m_origin;
    float m_zoom_factor = 1.f;
    bool m_interactable = true;

    id m_focused_id;
    bool m_focused_id_used = false;
    static_any<256> m_focus_state;
    std::optional<::LRESULT> (*m_message_handler)(
        window&, ::MSG const&) noexcept = nullptr;

    friend class context;
    friend class window_manager;
};

}

#endif
