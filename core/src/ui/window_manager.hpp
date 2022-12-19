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

#ifndef WINDOWER_UI_WINDOW_MANAGER_HPP
#define WINDOWER_UI_WINDOW_MANAGER_HPP

#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "ui/vector.hpp"
#include "ui/window.hpp"

#include <gsl/gsl>

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>

namespace windower::ui
{

class context;

class window_manager
{
public:
    window*
    get(context const& ctx, std::uintptr_t id, layer layer, float depth,
        window const* parent) noexcept;

    window const* hot_window() const noexcept;
    window const* active_window() const noexcept;

    void update(context& ctx) noexcept;
    window const* update_hot_window(context& ctx) noexcept;

    std::span<window const* const> z_order(layer layer) const noexcept;

    void activate_next(context& ctx) noexcept;
    void activate_next_horizontal(context& ctx) noexcept;
    void activate_next_vertical(context& ctx) noexcept;
    void activate_previous(context& ctx) noexcept;
    void activate_previous_horizontal(context& ctx) noexcept;
    void activate_previous_vertical(context& ctx) noexcept;

    std::optional<::LRESULT> process_message(::MSG const& message) noexcept;

private:
    std::deque<std::pair<window, bool>> m_windows;
    std::array<std::vector<window*>, 3> m_z_order;
    std::vector<std::size_t> m_free_list;
    window* m_hot_window    = nullptr;
    window* m_active_window = nullptr;

    vector position(
        context const& ctx, gsl::not_null<window const*> window,
        vector const& origin = {}, bool wrap_x = false,
        bool wrap_y = false) const noexcept;
    bool interactable(
        context const& ctx, gsl::not_null<window const*> window) const noexcept;
    bool activate_window(context const&, window const*) noexcept;

    template<typename P, typename C>
    void activate_next(
        context& ctx, bool wrap_x, bool wrap_y, C comparison,
        P projection) noexcept
    {
        namespace range = std::ranges;
        namespace view  = std::ranges::views;

        auto windows = m_z_order | view::join | view::filter([&](auto w) {
                           return w != m_active_window && interactable(ctx, w);
                       });

        if (!range::empty(windows))
        {
            if (!m_active_window)
            {
                if (auto const it = range::begin(windows);
                    it != range::end(windows))
                {
                    activate_window(ctx, *it);
                }
            }
            else
            {
                auto const origin = position(ctx, m_active_window);
                if (auto const it = range::min_element(
                        windows, comparison,
                        [&](auto w) {
                            return projection(
                                position(ctx, w, origin, wrap_x, wrap_y));
                        });
                    it != range::end(windows))
                {
                    activate_window(ctx, *it);
                }
            }
        }
    }
};

}

#endif
