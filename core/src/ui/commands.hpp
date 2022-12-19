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

#ifndef WINDOWER_UI_COMMANDS_HPP
#define WINDOWER_UI_COMMANDS_HPP

#include "ui/command_buffer.hpp"
#include "ui/data_buffer_traits.hpp"
#include "ui/rectangle.hpp"

#include <d3d8.h>

#include <cstddef>
#include <cstdint>

namespace windower::ui
{

class set_texture_command
{
public:
    using pointer = ::IDirect3DBaseTexture8*;
    using const_pointer = ::IDirect3DBaseTexture8 const*;

    static bool check_state(command_buffer&, const_pointer) noexcept;

    set_texture_command(pointer) noexcept;

    void execute(::IDirect3DDevice8*) const noexcept;
    void stitch(pointer) noexcept;

private:
    pointer m_ptr;
};

class set_texture_wrap_command
{
public:
    set_texture_wrap_command(bool) noexcept;

    void execute(::IDirect3DDevice8*) const noexcept;
    void stitch(bool) noexcept;

private:
    bool m_wrap;
};

template<typename T>
class set_data_buffer_command
{
public:
    using pointer = typename data_buffer_traits<T>::pointer;
    using const_pointer = typename data_buffer_traits<T>::const_pointer;

    static bool
    check_state(command_buffer& commands, const_pointer ptr) noexcept
    {
        auto const result =
            !commands.has_state(data_buffer_traits<T>::state_id, ptr);
        if (result)
        {
            commands.state(data_buffer_traits<T>::state_id, ptr);
        }
        return result;
    }

    set_data_buffer_command(pointer ptr) noexcept : m_ptr{ptr} {}

    void execute(::IDirect3DDevice8* d3d_device) const noexcept
    {
        Expects(d3d_device != nullptr);

        data_buffer_traits<T>::set_buffer(d3d_device, m_ptr);
    }

    void stitch(pointer ptr) noexcept { m_ptr = ptr; }

private:
    pointer m_ptr;
};

class set_clip_command
{
public:
    set_clip_command(rectangle const&) noexcept;

    void execute(::IDirect3DDevice8*) const noexcept;
    void stitch(rectangle const&) noexcept;

private:
    ::DWORD m_x;
    ::DWORD m_y;
    ::DWORD m_width;
    ::DWORD m_height;
};

template<::D3DPRIMITIVETYPE Primitive>
class draw_primitive_command_base
{
public:
    draw_primitive_command_base(
        std::uint16_t vertex_offset, std::uint16_t vertex_count,
        std::uint16_t index_offset, std::uint16_t primitive_count) noexcept :
        m_vertex_offset{vertex_offset},
        m_vertex_count{vertex_count}, m_index_offset{index_offset},
        m_primitive_count{primitive_count}
    {}

    void execute(::IDirect3DDevice8* d3d_device) const noexcept
    {
        Expects(d3d_device != nullptr);

        d3d_device->DrawIndexedPrimitive(
            Primitive, m_vertex_offset, m_vertex_count, m_index_offset,
            m_primitive_count);
    }

protected:
    std::uint16_t m_vertex_offset{0};
    std::uint16_t m_vertex_count{0};
    std::uint16_t m_index_offset{0};
    std::uint16_t m_primitive_count{0};
};

class draw_line_strip_command :
    public draw_primitive_command_base<::D3DPT_LINESTRIP>
{
public:
    draw_line_strip_command(
        std::uint16_t vertex_offset, std::uint16_t vertex_count,
        std::uint16_t index_offset, std::uint16_t index_count) noexcept;
};

class draw_triangle_list_command :
    public draw_primitive_command_base<::D3DPT_TRIANGLELIST>
{
public:
    draw_triangle_list_command(
        std::uint16_t vertex_offset, std::uint16_t vertex_count,
        std::uint16_t index_offset, std::uint16_t index_count) noexcept;

    void stitch(
        std::uint16_t vertex_offset, std::uint16_t vertex_count,
        std::uint16_t index_offset, std::uint16_t index_count) noexcept;
};

class begin_mask_command
{
public:
    begin_mask_command() noexcept = default;

    void execute(::IDirect3DDevice8* d3d_device) const noexcept;
};

class apply_mask_command
{
public:
    apply_mask_command() noexcept = default;

    void execute(::IDirect3DDevice8* d3d_device) const noexcept;
};

class end_mask_command
{
public:
    end_mask_command() noexcept = default;

    void execute(::IDirect3DDevice8* d3d_device) const noexcept;
};

}

#endif
