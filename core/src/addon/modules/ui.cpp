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

#include "addon/modules/ui.hpp"

#include "addon/lua.hpp"
#include "addon/modules/ui.lua.hpp"
#include "addon/unsafe.hpp"
#include "core.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/button.hpp"
#include "ui/widget/check.hpp"
#include "ui/widget/color_picker.hpp"
#include "ui/widget/edit.hpp"
#include "ui/widget/image_button.hpp"
#include "ui/widget/label.hpp"
#include "ui/widget/link.hpp"
#include "ui/widget/progress.hpp"
#include "ui/widget/radio.hpp"
#include "ui/widget/scroll_panel.hpp"
#include "ui/widget/slider.hpp"
#include "ui/widget/window.hpp"

#include <lua.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace windower
{
namespace ui::wrappers
{
namespace
{

extern "C"
{
    context* get_context() { return core::instance().ui.context(); }

    bool direct_to_screen(context const& ctx)
    {
        return ctx.current_window() == nullptr;
    }

    std::uint64_t
    make_id(context const& ctx, void const* instance, std::uint32_t id)
    {
        void const* scope = ctx.current_window();
        if (!scope)
        {
            scope = instance;
        }
        return gsl::narrow_cast<std::uint64_t>(ui::make_id(scope, id));
    }

    std::int32_t get_system_color(context const& ctx, std::uint8_t index)
    {
        return gsl::narrow_cast<std::int32_t>(
            ctx.system_color(system_color{index}));
    }

    bool begin_window(context& ctx, widget::window_state& state)
    {
        return widget::begin_window(ctx, state);
    }

    void end_window(context& ctx) { widget::end_window(ctx); }

    void begin_scroll_panel(
        context& ctx, std::uint64_t id, widget::scroll_panel_state& state)
    {
        widget::begin_scroll_panel(ctx, ui::id{id}, state);
    }

    void end_scroll_panel(context& ctx) { widget::end_scroll_panel(ctx); }

    void begin_scope(context& ctx) { ctx.push_enabled(); }

    void end_scope(context& ctx) { ctx.pop_enabled(); }

    void set_enabled(context& ctx, bool enabled) { ctx.enabled(enabled); }

    void set_bounds(context& ctx, float x0, float y0, float x1, float y1)
    {
        ctx.bounds({x0, y0, x1, y1});
    }

    widget::button_state button(
        context& ctx, std::uint64_t id, char8_t const* text_data,
        std::size_t text_size, bool checked)
    {
        return widget::button(ctx, ui::id{id}, {text_data, text_size}, checked);
    }

    widget::button_state check(
        context& ctx, std::uint64_t id, char8_t const* text_data,
        std::size_t text_size, int8_t checked)
    {
        return widget::check(
            ctx, ui::id{id}, {text_data, text_size},
            checked == -1 ? std::nullopt : std::optional{checked != 0});
    }

    std::int32_t
    color_picker(context& ctx, std::uint64_t id, std::int32_t value, bool alpha)
    {
        return gsl::narrow_cast<std::int32_t>(
            widget::color_picker(ctx, ui::id{id}, color{value}, alpha));
    }

    void edit(
        context& ctx, std::uint64_t id, widget::edit_state& state,
        char8_t const* text_data, std::size_t text_size)
    {
        // HACK: the text pointer in the state is only valid as long as the
        // widget has focus.
        if (!ctx.is_focused(ui::id{id}))
        {
            state.text({text_data, text_size});
        }

        widget::edit(ctx, ui::id{id}, state);
    }

    widget::button_state image_button(
        context& ctx, id id, widget::image_button_descriptor const& descriptor)
    {
        return widget::image_button(ctx, ui::id{id}, descriptor);
    }

    void label(context& ctx, char8_t const* text_data, std::size_t text_size)
    {
        return widget::label(ctx, {text_data, text_size});
    }

    widget::button_state
    link(context& ctx, id id, char8_t const* text_data, std::size_t text_size)
    {
        return widget::link(ctx, ui::id{id}, {text_data, text_size});
    }

    void progress(
        context& ctx, widget::progress_entry const* entries_data,
        std::size_t entries_size, direction direction)
    {
        widget::progress(
            ctx,
            std::span<widget::progress_entry const>{entries_data, entries_size},
            direction);
    }

    widget::button_state radio(
        context& ctx, std::uint64_t id, char8_t const* text_data,
        std::size_t text_size, bool checked)
    {
        auto const result =
            widget::radio(ctx, ui::id{id}, {text_data, text_size}, checked);
        return result;
    }

    float slider(
        context& ctx, std::uint64_t id, float value, float min, float max,
        std::int32_t fill_color, direction direction)
    {
        return widget::slider(
            ctx, ui::id{id}, value, min, max, color{fill_color}, direction);
    }

    void set_texture(
        context& ctx, void const* texture_data, std::size_t texture_size)
    {
        if (texture_data == nullptr)
        {
            primitive::set_texture(ctx, no_texture);
        }
        else if (texture_size != 0)
        {
            auto const name = std::u8string_view{
                static_cast<char8_t const*>(texture_data), texture_size};
            primitive::set_texture(ctx, name);
        }
        else
        {
            auto const& image = *static_cast<ffxi_image const*>(texture_data);
            primitive::set_texture(ctx, image);
        }
    }

    void rectangle_flat(
        context& ctx, ui::rectangle const& bounds, std::int32_t color)
    {
        primitive::rectangle(ctx, bounds, ui::color{color});
    }

    void rectangle_patch(
        context& ctx, ui::rectangle const& bounds, void const* texture_data,
        std::size_t texture_size, patch const& patch, std::int32_t color)
    {
        set_texture(ctx, texture_data, texture_size);
        primitive::rectangle(ctx, bounds, patch, ui::color{color});
    }

    void rectangle_nine_patch(
        context& ctx, ui::rectangle const& bounds, void const* texture_data,
        std::size_t texture_size, nine_patch const& patch, std::int32_t color)
    {
        set_texture(ctx, texture_data, texture_size);
        primitive::rectangle(ctx, bounds, patch, ui::color{color});
    }
}

}
}

int load_ui_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_ui_source, u8"core.ui");
    lua::push(guard, lua::unsafe::unwrap(s));
    lua::push(guard, &ui::wrappers::get_context);
    lua::push(guard, &ui::wrappers::direct_to_screen);
    lua::push(guard, &ui::wrappers::make_id);
    lua::push(guard, &ui::wrappers::get_system_color);
    lua::push(guard, &ui::wrappers::begin_window);
    lua::push(guard, &ui::wrappers::end_window);
    lua::push(guard, &ui::wrappers::begin_scroll_panel);
    lua::push(guard, &ui::wrappers::end_scroll_panel);
    lua::push(guard, &ui::wrappers::begin_scope);
    lua::push(guard, &ui::wrappers::end_scope);
    lua::push(guard, &ui::wrappers::set_enabled);
    lua::push(guard, &ui::wrappers::set_bounds);
    lua::push(guard, &ui::wrappers::button);
    lua::push(guard, &ui::wrappers::check);
    lua::push(guard, &ui::wrappers::color_picker);
    lua::push(guard, &ui::wrappers::edit);
    lua::push(guard, &ui::wrappers::image_button);
    lua::push(guard, &ui::wrappers::label);
    lua::push(guard, &ui::wrappers::link);
    lua::push(guard, &ui::wrappers::progress);
    lua::push(guard, &ui::wrappers::radio);
    lua::push(guard, &ui::wrappers::slider);
    lua::push(guard, &ui::wrappers::rectangle_flat);
    lua::push(guard, &ui::wrappers::rectangle_patch);
    lua::push(guard, &ui::wrappers::rectangle_nine_patch);
    lua::call(guard, guard.size() - 1);

    return guard.release();
}

}
