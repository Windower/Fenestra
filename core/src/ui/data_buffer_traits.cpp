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

#include "ui/data_buffer_traits.hpp"

#include "ui/context.hpp"
#include "ui/vertex.hpp"

#include <gsl/gsl>

#include <cstdint>

namespace windower::ui
{

data_buffer_traits<vertex>::com_pointer data_buffer_traits<vertex>::allocate(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, std::size_t size) noexcept
{
    com_pointer result;
    d3d_device->CreateVertexBuffer(
        size * sizeof(vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, ::D3DPOOL_DEFAULT,
        result.put());
    return result;
}

void data_buffer_traits<vertex>::set_buffer(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept
{
    d3d_device->SetStreamSource(0, ptr, sizeof(vertex));
}

data_buffer_traits<std::uint16_t>::com_pointer
data_buffer_traits<std::uint16_t>::allocate(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, std::size_t size) noexcept
{
    com_pointer result;
    d3d_device->CreateIndexBuffer(
        size * sizeof(std::uint16_t), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        ::D3DFMT_INDEX16, ::D3DPOOL_DEFAULT, result.put());
    return result;
}

void data_buffer_traits<std::uint16_t>::set_buffer(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept
{
    d3d_device->SetIndices(ptr, 0);
}

}
