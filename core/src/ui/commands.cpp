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

#include "ui/commands.hpp"

#include "ui/command_buffer.hpp"
#include "ui/rectangle.hpp"

#include <d3d8.h>

#include <gsl/gsl>

#include <cstdint>

namespace windower::ui
{

bool set_texture_command::check_state(
    command_buffer& commands, const_pointer ptr) noexcept
{
    auto const result =
        !commands.has_state(command_buffer::state_id::texture, ptr);
    if (result)
    {
        commands.state(command_buffer::state_id::texture, ptr);
    }
    return result;
}

set_texture_command::set_texture_command(pointer ptr) noexcept : m_ptr{ptr} {}

void set_texture_command::execute(::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    d3d_device->SetTexture(0, m_ptr);
}

void set_texture_command::stitch(pointer ptr) noexcept { m_ptr = ptr; }

set_texture_wrap_command::set_texture_wrap_command(bool wrap) noexcept :
    m_wrap{wrap}
{}

void set_texture_wrap_command::execute(
    ::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    auto const value = m_wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;

    d3d_device->SetTextureStageState(0, D3DTSS_ADDRESSU, value);
    d3d_device->SetTextureStageState(0, D3DTSS_ADDRESSV, value);
    d3d_device->SetTextureStageState(0, D3DTSS_ADDRESSW, value);
}

void set_texture_wrap_command::stitch(bool wrap) noexcept { m_wrap = wrap; }

set_clip_command::set_clip_command(rectangle const& clip_rect) noexcept :
    m_x{gsl::narrow_cast<std::uint32_t>(clip_rect.x0)},
    m_y{gsl::narrow_cast<std::uint32_t>(clip_rect.y0)},
    m_width{gsl::narrow_cast<std::uint32_t>(clip_rect.width())},
    m_height{gsl::narrow_cast<std::uint32_t>(clip_rect.height())}
{}

void set_clip_command::execute(::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    ::D3DVIEWPORT8 viewport{m_x, m_y, m_width, m_height, 0.f, 1.f};
    d3d_device->SetViewport(&viewport);
}

void set_clip_command::stitch(rectangle const& clip_rect) noexcept
{
    m_x      = gsl::narrow_cast<std::uint32_t>(clip_rect.x0);
    m_y      = gsl::narrow_cast<std::uint32_t>(clip_rect.y0);
    m_width  = gsl::narrow_cast<std::uint32_t>(clip_rect.width());
    m_height = gsl::narrow_cast<std::uint32_t>(clip_rect.height());
}

draw_line_strip_command::draw_line_strip_command(
    std::uint16_t vertex_offset, std::uint16_t vertex_count,
    std::uint16_t index_offset, std::uint16_t index_count) noexcept :
    draw_primitive_command_base{
        vertex_offset, vertex_count, index_offset,
        gsl::narrow_cast<std::uint16_t>(index_count - 1)}
{}

draw_triangle_list_command::draw_triangle_list_command(
    std::uint16_t vertex_offset, std::uint16_t vertex_count,
    std::uint16_t index_offset, std::uint16_t index_count) noexcept :
    draw_primitive_command_base{
        vertex_offset, vertex_count, index_offset,
        gsl::narrow_cast<std::uint16_t>(index_count / 3)}
{
    Expects(index_count % 3 == 0);
}

void draw_triangle_list_command::stitch(
    std::uint16_t vertex_offset, std::uint16_t vertex_count,
    std::uint16_t index_offset, std::uint16_t index_count) noexcept
{
    Expects(index_count % 3 == 0);
    Expects(m_vertex_offset + m_vertex_count == vertex_offset);
    Expects(m_index_offset + m_primitive_count * 3 == index_offset);

    this->m_vertex_count += vertex_count;
    this->m_primitive_count += index_count / 3;
}

void begin_mask_command::execute(::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    d3d_device->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 0.f, 0);
    d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    d3d_device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
}

void apply_mask_command::execute(::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
    d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
}

void end_mask_command::execute(::IDirect3DDevice8* d3d_device) const noexcept
{
    Expects(d3d_device != nullptr);

    d3d_device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

}
