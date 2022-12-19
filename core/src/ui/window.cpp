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

#include "ui/window.hpp"

#include "ui/command_buffer.hpp"
#include "ui/layer.hpp"
#include "ui/rectangle.hpp"
#include "widget\window.hpp"

#include <gsl/gsl>

namespace windower::ui
{

bool window::operator==(window const& other) const noexcept
{
    return m_id == other.m_id;
}

std::partial_ordering window::operator<=>(window const& other) const noexcept
{
    if (m_id == other.m_id)
    {
        return std::partial_ordering::equivalent;
    }

    auto const layer_a = gsl::narrow_cast<std::size_t>(m_layer);
    auto const layer_b = gsl::narrow_cast<std::size_t>(other.m_layer);
    if (auto const cmp = layer_a <=> layer_b;
        cmp != std::strong_ordering::equal)
    {
        return cmp;
    }

    if (auto const cmp = m_depth <=> other.m_depth;
        cmp != std::partial_ordering::equivalent &&
        cmp != std::partial_ordering::unordered)
    {
        return cmp;
    }

    if (is_descendent_of(other))
    {
        return std::partial_ordering::greater;
    }

    if (other.is_descendent_of(*this))
    {
        return std::partial_ordering::less;
    }

    return std::partial_ordering::unordered;
}

void window::bounds(rectangle const& bounds) noexcept { m_bounds = bounds; }

rectangle const& window::bounds() const noexcept { return m_bounds; }

void window::origin(vector const& origin) noexcept { m_origin = origin; }

vector const& window::origin() const noexcept { return m_origin; }

void window::zoom_factor(float bounds) noexcept { m_zoom_factor = bounds; }

float window::zoom_factor() const noexcept { return m_zoom_factor; }

void window::interactable(bool interactable) noexcept
{
    m_interactable = interactable;
}

bool window::interactable() const noexcept { return m_interactable; }

layer window::layer() const noexcept { return m_layer; }

float window::depth() const noexcept { return m_depth; }

command_buffer& window::commands() noexcept { return m_commands; }

command_buffer const& window::commands() const noexcept { return m_commands; }

bool window::valid(std::size_t layer_index) const noexcept
{
    return gsl::narrow_cast<std::size_t>(m_layer) == layer_index;
}

bool window::is_descendent_of(window const& ancestor) const noexcept
{
    auto current_ptr        = this;
    auto const ancestor_ptr = &ancestor;
    while ((current_ptr = current_ptr->m_parent) != nullptr)
    {
        if (current_ptr == ancestor_ptr)
        {
            return true;
        }
    }
    return false;
}

id window::focused_id() const noexcept { return m_focused_id; }

bool window::is_blurred() const noexcept { return m_focused_id == no_id; }

bool window::is_focused(id id) const noexcept { return m_focused_id == id; }

bool window::focus(id id) noexcept
{
    if (!is_focused(id))
    {
        m_message_handler = nullptr;
        m_focus_state.clear();
        m_focused_id = id;
        return true;
    }
    return false;
}

bool window::blur(id id) noexcept
{
    if (is_focused(id))
    {
        m_message_handler = nullptr;
        m_focus_state.clear();
        m_focused_id = no_id;
        return true;
    }
    return false;
}

}
