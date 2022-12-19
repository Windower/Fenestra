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

#include "window_manager.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/window.hpp"
#include "utility.hpp"

#include <gsl/gsl>

#include <algorithm>
#include <functional>
#include <ranges>
#include <span>
#include <utility>

namespace windower::ui
{

window* window_manager::get(
    context const& ctx, std::uintptr_t id, layer layer, float depth,
    window const* parent) noexcept
{
    namespace range = std::ranges;

    if (auto const it = range::find(
            m_windows, id, [](auto const& w) { return w.first.m_id; });
        it != m_windows.end())
    {
        if (it->first.m_layer != layer)
        {
            auto const old_layer = it->first.m_layer;
            auto const old_index = to_underlying(old_layer);
            std::erase(gsl::at(m_z_order, old_index), &it->first);

            auto const new_layer = layer;
            auto const new_index = to_underlying(new_layer);
            if (old_index < new_index)
            {
                gsl::at(m_z_order, new_index).push_back(&it->first);
            }
            else
            {
                auto& z_order = gsl::at(m_z_order, new_index);
                z_order.insert(z_order.begin(), &it->first);
            }
        }
        it->first.m_layer  = layer;
        it->first.m_depth  = depth;
        it->first.m_parent = parent;
        it->second         = true;
        return &it->first;
    }

    std::pair<window, bool>* entry = nullptr;
    if (m_free_list.empty())
    {
        entry = &m_windows.emplace_back();
    }
    else
    {
        entry = &gsl::at(m_windows, m_free_list.back());
        m_free_list.pop_back();
    }
    entry->first.m_id      = id;
    entry->first.m_layer   = layer;
    entry->first.m_depth   = depth;
    entry->first.m_parent  = parent;
    entry->second          = true;
    auto const layer_index = to_underlying(layer);
    gsl::at(m_z_order, layer_index).push_back(&entry->first);
    activate_window(ctx, &entry->first);
    return &entry->first;
}

window const* window_manager::hot_window() const noexcept
{
    return m_hot_window;
}

window const* window_manager::active_window() const noexcept
{
    return m_active_window;
}

void window_manager::update(context& ctx) noexcept
{
    for (std::size_t i = 0; i < m_windows.size(); ++i)
    {
        auto& entry = gsl::at(m_windows, i);
        entry.first.m_commands.clear();
        if (!entry.second && entry.first.m_id != 0)
        {
            auto const layer_index = to_underlying(entry.first.m_layer);
            std::erase(gsl::at(m_z_order, layer_index), &entry.first);
            entry.first.m_id = 0;
            m_free_list.push_back(i);
        }
        entry.second = false;
    }

    for (auto& z_order : m_z_order)
    {
        std::stable_sort(
            z_order.begin(), z_order.end(),
            [](auto lhs, auto rhs) noexcept { return *lhs < *rhs; });
    }

    update_hot_window(ctx);
    if (auto const& mouse = ctx.mouse();
        mouse.was_pressed(mouse_button::left) ||
        mouse.was_pressed(mouse_button::right) ||
        mouse.was_pressed(mouse_button::middle) ||
        mouse.was_pressed(mouse_button::x1) ||
        mouse.was_pressed(mouse_button::x2))
    {
        activate_window(ctx, m_hot_window);
    }
}

window const* window_manager::update_hot_window(context& ctx) noexcept
{
    namespace range = std::ranges;
    namespace view  = std::ranges::views;

    m_hot_window = nullptr;

    auto const& mouse = ctx.mouse().position();
    for (auto const& layer : m_z_order)
    {
        if (auto const it = range::find_if(
                layer,
                [&](auto w) noexcept {
                    if (!interactable(ctx, w))
                    {
                        return false;
                    }

                    auto const scale   = ctx.scale_factor(w->layer());
                    auto const& bounds = w->bounds();

                    auto const x0 = std::round(scale.x * bounds.x0);
                    auto const y0 = std::round(scale.y * bounds.y0);
                    auto const x1 = std::round(scale.x * bounds.x1);
                    auto const y1 = std::round(scale.y * bounds.y1);

                    return is_inside(mouse, {x0, y0, x1, y1});
                });
            it != layer.end())
        {
            m_hot_window = *it;
            break;
        }
    }

    return m_hot_window;
}
std::span<window const* const>
window_manager::z_order(layer layer) const noexcept
{
    return gsl::at(m_z_order, to_underlying(layer));
}

void window_manager::activate_next(context& ctx) noexcept
{
    activate_next(ctx, true, true, std::ranges::less{}, [](vector const& p) {
        return p.x * p.x + 5 * p.y * p.y;
    });
}

void window_manager::activate_next_horizontal(context& ctx) noexcept
{
    activate_next(ctx, true, false, std::ranges::less{}, [](vector const& p) {
        return p.x * p.x + 10 * p.y * p.y;
    });
}

void window_manager::activate_next_vertical(context& ctx) noexcept
{
    activate_next(ctx, false, true, std::ranges::less{}, [](vector const& p) {
        return 10 * p.x * p.x + p.y * p.y;
    });
}

void window_manager::activate_previous(context& ctx) noexcept
{
    activate_next(
        ctx, true, true, std::ranges::greater_equal{},
        [](vector const& p) { return p.x * p.x + 5 * p.y * p.y; });
}

void window_manager::activate_previous_horizontal(context& ctx) noexcept
{
    activate_next(
        ctx, true, false, std::ranges::greater_equal{},
        [](vector const& p) { return p.x * p.x + 10 * p.y * p.y; });
}

void window_manager::activate_previous_vertical(context& ctx) noexcept
{
    activate_next(
        ctx, false, true, std::ranges::greater_equal{},
        [](vector const& p) { return 10 * p.x * p.x + p.y * p.y; });
}

std::optional<::LRESULT>
window_manager::process_message(::MSG const& message) noexcept
{
    return m_active_window && m_active_window->m_message_handler
               ? m_active_window->m_message_handler(*m_active_window, message)
               : std::nullopt;
}

vector window_manager::position(
    context const& ctx, gsl::not_null<window const*> window,
    vector const& origin, bool wrap_x, bool wrap_y) const noexcept
{
    auto const screen   = ctx.screen_size();
    auto const zoom     = window->zoom_factor();
    auto const scale    = ctx.scale_factor(window->layer());
    auto const position = window->bounds().position();

    auto x = position.x * scale.x * zoom - origin.x;
    auto y = position.y * scale.y * zoom - origin.y;

    if (wrap_x)
    {
        x -= std::floor(x / screen.width) * screen.width;
    }

    if (wrap_y)
    {
        y -= std::floor(y / screen.height) * screen.height;
    }

    return {x, y};
}

bool window_manager::interactable(
    context const& ctx, gsl::not_null<window const*> window) const noexcept
{
    return window->interactable() &&
           (!ctx.layout_mode() ||
            to_underlying(window->layer()) <= to_underlying(layer::layout));
}

bool window_manager::activate_window(
    context const& ctx, window const* window) noexcept
{
    namespace range = std::ranges;

    if (window)
    {
        auto& layer = gsl::at(m_z_order, to_underlying(window->m_layer));

        auto const it      = range::find(layer, window);
        auto const ptr     = it != range::end(layer) ? *it : nullptr;
        auto const changed = (!ptr || interactable(ctx, ptr)) &&
                             std::exchange(m_active_window, ptr) != ptr;

        if (changed && ptr)
        {
            range::stable_partition(
                range::begin(layer),
                range::begin(range::stable_partition(
                    layer, [ptr](auto e) noexcept { return *ptr <= *e; })),
                [ptr](auto e) noexcept { return *ptr < *e; });
        }

        return changed;
    }
    else
    {
        return std::exchange(m_active_window, nullptr) != nullptr;
    }
}

}
