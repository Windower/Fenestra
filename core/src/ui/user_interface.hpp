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

#ifndef WINDOWER_UI_USER_INTERFACE_HPP
#define WINDOWER_UI_USER_INTERFACE_HPP

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/layer.hpp"

#include <windows.h>

#include <d3d8.h>

#include <gsl/gsl>

#include <memory>
#include <optional>

namespace windower
{

class user_interface
{
public:
    explicit operator bool() const noexcept;

    ui::context* context() noexcept;

    void initialize(
        ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
        ui::dimension const& screen_size, ui::dimension const& ui_size,
        ui::dimension const& render_size) noexcept;
    void reset() noexcept;

    void activate_next_window() noexcept;
    void activate_previous_window() noexcept;

    std::optional<::LRESULT>
    process_message(::MSG const& message) const noexcept;
    void begin_frame() noexcept;
    void end_frame() noexcept;
    void render(ui::layer layer) noexcept;

private:
    std::unique_ptr<ui::context> m_context;
};

}

#endif
