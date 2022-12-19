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

#ifndef WINDOWER_UI_DATA_BUFFER_TRAITS_HPP
#define WINDOWER_UI_DATA_BUFFER_TRAITS_HPP

#include "ui/command_buffer.hpp"
#include "ui/vertex.hpp"

#include <d3d8.h>
#include <winrt/base.h>

#include <cstddef>
#include <cstdint>

namespace windower::ui
{

template<typename>
class data_buffer_traits;

template<>
class data_buffer_traits<vertex>
{
public:
    using com_interface = ::IDirect3DVertexBuffer8;
    using pointer = com_interface*;
    using const_pointer = com_interface const*;
    using com_pointer = winrt::com_ptr<com_interface>;

    static constexpr command_buffer::state_id state_id =
        command_buffer::state_id::vertex_buffer;

    static com_pointer allocate(
        gsl::not_null<::IDirect3DDevice8*> d3d_device,
        std::size_t size) noexcept;
    static void set_buffer(
        gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept;
};

template<>
class data_buffer_traits<std::uint16_t>
{
public:
    using com_interface = ::IDirect3DIndexBuffer8;
    using pointer = com_interface*;
    using const_pointer = com_interface const*;
    using com_pointer = winrt::com_ptr<com_interface>;

    static constexpr command_buffer::state_id state_id =
        command_buffer::state_id::index_buffer;

    static com_pointer allocate(
        gsl::not_null<::IDirect3DDevice8*> d3d_device,
        std::size_t size) noexcept;
    static void set_buffer(
        gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept;
};

}

#endif
