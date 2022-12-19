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

#ifndef WINDOWER_UI_CONTEXT_HPP
#define WINDOWER_UI_CONTEXT_HPP

#include "hooks/ffximain.hpp"
#include "ui/command_buffer.hpp"
#include "ui/cursor.hpp"
#include "ui/data_buffer.hpp"
#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "ui/mouse.hpp"
#include "ui/static_any.hpp"
#include "ui/text_layout_engine.hpp"
#include "ui/text_rasterizer.hpp"
#include "ui/texture_cache.hpp"
#include "ui/texture_token.hpp"
#include "ui/transform.hpp"
#include "ui/vector.hpp"
#include "ui/vertex.hpp"
#include "ui/window.hpp"
#include "ui/window_manager.hpp"

#include <windows.h>

#include <d3d8.h>
#include <dwrite.h>
#include <wincodec.h>

#include <gsl/gsl>

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

namespace windower::ui
{

enum class system_color : std::uint8_t
{
    transparent                         = 0,
    white                               = 1,
    black                               = 2,
    error                               = 3,
    layout_active_title                 = 4,
    layout_active_title_stroke          = 5,
    layout_active_hidden_title          = 6,
    layout_active_hidden_title_stroke   = 7,
    layout_inactive_title               = 8,
    layout_inactive_title_stroke        = 9,
    layout_inactive_hidden_title        = 10,
    layout_inactive_hidden_title_stroke = 11,
    color_picker_highlight              = 12,
    accent                              = 128,
    window_title                        = 129,
    window_title_inactive               = 130,
    label                               = 131,
    label_disabled                      = 132,
    button                              = 133,
    button_disabled                     = 134,
    link                                = 135,
    link_disabled                       = 136,
};

enum class system_cursor : std::int8_t
{
    none           = -1,
    normal         = 0,
    hot            = 1,
    north          = 2,
    north_east     = 3,
    east           = 4,
    south_east     = 5,
    south          = 6,
    south_west     = 7,
    west           = 8,
    north_west     = 9,
    north_alt      = 10,
    north_east_alt = 11,
    east_alt       = 12,
    south_east_alt = 13,
    south_alt      = 14,
    south_west_alt = 15,
    west_alt       = 16,
    north_west_alt = 17,
};

class context
{
public:
    context(context const&) noexcept = default;
    context(context&&) noexcept      = default;
    context(
        ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
        dimension const& screen_size, dimension const& ui_size,
        dimension const& render_size) noexcept;

    ~context() noexcept;

    context& operator=(context const&) noexcept = default;
    context& operator=(context&&) noexcept = default;

    dimension const& screen_size() const noexcept;

    std::u8string_view skin() const noexcept;
    void skin(std::u8string_view skin) noexcept;

    rectangle const& bounds() const noexcept;
    void bounds(rectangle const& bounds) noexcept;

    window const* current_window() const noexcept;

    window const* active_window() const noexcept;
    bool is_active(window const&) const noexcept;

    void activate_next_window() noexcept;
    void activate_previous_window() noexcept;

    void use_id(id id) noexcept;
    bool hit_test(id id) const noexcept;
    bool hit_test(id id, rectangle const& bounds) const noexcept;

    bool is_inactive() const noexcept;
    bool is_active(id id) const noexcept;
    bool activate(id id) noexcept;
    bool deactivate(id id) noexcept;

    template<typename T>
    std::remove_cvref_t<T>* active_state(id id) noexcept;
    template<typename T>
    std::remove_cvref_t<T>* active_state(id id, T&& state) noexcept;
    template<typename T, typename... A>
    std::remove_cvref_t<T>* emplace_active_state(id id, A&&... args) noexcept;
    bool clear_active_state(id id) noexcept;

    id focused_id() const noexcept;

    bool is_blurred() const noexcept;
    bool is_focused(id id) const noexcept;
    bool focus(id id) noexcept;
    bool blur(id id) noexcept;

    template<typename T, typename... A>
    std::remove_cvref_t<T>* focus(id id, A&&... args) noexcept
    {
        return m_window_stack.back().window->focus<T>(
            id, std::forward<A>(args)...);
    }

    template<typename T>
    std::remove_cvref_t<T>* focus_state(id id) const noexcept
    {
        return m_window_stack.back().window->focus_state<T>(id);
    }

    template<typename T>
    std::remove_cvref_t<T>* focus_state(id id, T&& state) noexcept
    {
        return m_window_stack.back().window->focus_state(
            id, std::forward<T>(state));
    }

    template<typename T, typename... A>
    std::remove_cvref_t<T>* emplace_focus_state(id id, A&&... args) noexcept
    {
        return m_window_stack.back().window->emplace_focus_state<T>(
            id, std::forward<T>(args)...);
    }

    vector get_scroll(id id) noexcept;
    vector get_scroll(id id, rectangle const& bounds) noexcept;

    bool layout_mode() const noexcept;
    void layout_mode(bool enabled) noexcept;

    color system_color(system_color color) const noexcept;

    bool enabled() const noexcept;
    void enabled(bool enabled) noexcept;
    void push_enabled(bool enabled) noexcept;
    void push_enabled() noexcept;
    void pop_enabled() noexcept;

    transform current_transform() const noexcept;
    void push_transform(transform const& transform) noexcept;
    void pop_transform() noexcept;

    void push_clip(rectangle const& clip) noexcept;
    void pop_clip() noexcept;

    void push_offset(vector const& vector) noexcept;
    void pop_offset() noexcept;

    window& begin_window(std::intptr_t id, layer layer, float depth) noexcept;
    void end_window() noexcept;

    std::optional<vector> to_widget(vector const&) const noexcept;

    vector origin() const noexcept;
    float zoom_factor() const noexcept;
    vector scale_factor() const noexcept;
    vector scale_factor(layer layer) const noexcept;
    float scale_factor_uniform() const noexcept;
    float scale_factor_uniform(layer layer) const noexcept;
    std::tuple<float, float> depth() const noexcept;
    bool interactable() const noexcept;
    bool interactable(layer layer) const noexcept;

    void set_texture(texture_token texture) noexcept;

    void draw_triangle_list(
        std::initializer_list<vertex> vertices,
        std::initializer_list<std::uint16_t> indices,
        bool reflected = false) noexcept;
    void draw_triangle_list(
        std::span<vertex const> vertices,
        std::span<std::uint16_t const> indices,
        bool reflected = false) noexcept;

    void set_cursor(system_cursor cursor) noexcept;

    std::optional<::LRESULT> process_message(::MSG const& message) noexcept;

    void begin_frame() noexcept;
    void end_frame() noexcept;
    void render(layer layer) noexcept;

    mouse& mouse() noexcept;
    texture_cache& texture_cache() noexcept;
    text_layout_engine& text_layout_engine() noexcept;
    text_rasterizer& text_rasterizer() noexcept;

    gsl::not_null<::IDirect3DDevice8*> d3d_device() noexcept;
    gsl::not_null<::IWICImagingFactory*> wic_factory() noexcept;
    gsl::not_null<::IDWriteFactory*> dwrite_factory() noexcept;

private:
    class layer_descriptor
    {
    public:
        vector scale_factor;
        bool interactable;
        dimension size;
    };

    class window_context
    {
    public:
        window* window = nullptr;
        std::vector<transform> transform_stack;
        std::vector<vector> offset_stack;
        std::vector<rectangle> clip_stack;
        std::vector<bool> enabled_stack;
        bool fully_clipped = false;
    };

    ::HWND m_hwnd;

    gsl::not_null<::IDirect3DDevice8*> m_d3d_device;
    winrt::com_ptr<::IWICImagingFactory> m_wic_factory;
    winrt::com_ptr<::IDWriteFactory> m_dwrite_factory;

    ::DWORD m_default_state;
    ::DWORD m_previous_state;

    ui::mouse m_mouse;
    ui::texture_cache m_texture_cache;
    ui::text_layout_engine m_text_layout_engine;
    ui::text_rasterizer m_text_rasterizer;

    std::array<layer_descriptor, 3> m_layers{};

    window m_screen;
    window m_layout_grid;
    window_manager m_window_manager;
    std::vector<window_context> m_window_stack;

    data_buffer<vertex> m_vertex_buffer;
    data_buffer<std::uint16_t> m_index_buffer;

    std::u8string m_skin;
    std::array<color, 256> m_colors  = {};
    std::array<cursor, 18> m_cursors = {};

    rectangle m_bounds;

    std::optional<system_cursor> m_cursor;

    id m_active_id;
    id m_last_active_id;
    id m_active_state_id;
    static_any<256> m_active_state;
    bool m_active_id_used;

    id m_scroll_id;
    id m_scroll_requested_id;

    bool m_layout_mode = false;

    shared_window_config const* m_client_shared = nullptr;
    window_config const* m_client_log1          = nullptr;
    window_config const* m_client_log2          = nullptr;

    layer_descriptor& descriptor(layer) noexcept;
    layer_descriptor const& descriptor(layer) const noexcept;
    command_buffer& commands() noexcept;

    void draw_layout_grid() noexcept;
    void load_colors(
        std::u8string_view skin, std::size_t offset = 0,
        std::size_t count = 128) noexcept;

    void process_mouse_message(::MSG const& message) noexcept;
};

template<typename T>
std::remove_cvref_t<T>* context::active_state(id id) noexcept
{
    if (m_active_state_id == id)
    {
        return m_active_state.value<T>();
    }
    return nullptr;
}

template<typename T>
std::remove_cvref_t<T>* context::active_state(id id, T&& state) noexcept
{
    if (is_active(id))
    {
        m_active_state_id = id;
        return &m_active_state.emplace<T>(std::forward<T>(state));
    }
    return nullptr;
}

template<typename T, typename... A>
std::remove_cvref_t<T>*
context::emplace_active_state(id id, A&&... args) noexcept
{
    if (is_active(id))
    {
        m_active_state_id = id;
        return &m_active_state.emplace<T>(std::forward<A>(args)...);
    }
    return nullptr;
}

}

#endif
