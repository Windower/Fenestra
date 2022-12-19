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

#include "ui/user_interface.hpp"

#include "ui/context.hpp"
#include "ui/dimension.hpp"

#include <d3d8.h>

#include <gsl/gsl>

#include <memory>

namespace windower
{

user_interface::operator bool() const noexcept { return m_context != nullptr; }

void user_interface::initialize(
    ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
    ui::dimension const& screen_size, ui::dimension const& ui_size,
    ui::dimension const& render_size) noexcept
{
    m_context = std::make_unique<ui::context>(
        hwnd, d3d_device, screen_size, ui_size, render_size);
}

void user_interface::reset() noexcept { m_context.reset(); }

void user_interface::activate_next_window() noexcept
{
    if (m_context)
    {
        m_context->activate_next_window();
    }
}

void user_interface::activate_previous_window() noexcept
{
    if (m_context)
    {
        m_context->activate_previous_window();
    }
}

std::optional<::LRESULT>
user_interface::process_message(::MSG const& message) const noexcept
{
    return m_context ? m_context->process_message(message) : std::nullopt;
}

void user_interface::begin_frame() noexcept
{
    if (m_context)
    {
        m_context->begin_frame();
    }
}

void user_interface::end_frame() noexcept
{
    if (m_context)
    {
        m_context->end_frame();
    }
}

void user_interface::render(ui::layer layer) noexcept
{
    if (m_context)
    {
        m_context->render(layer);
    }
}

ui::context* user_interface::context() noexcept { return m_context.get(); }

}
