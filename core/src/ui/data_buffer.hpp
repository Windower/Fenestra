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

#ifndef WINDOWER_UI_DATA_BUFFER_HPP
#define WINDOWER_UI_DATA_BUFFER_HPP

#include "ui/command_buffer.hpp"
#include "ui/data_buffer_traits.hpp"

#include <d3d8.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace windower::ui
{

template<typename T>
class data_segment
{
public:
    std::span<T> data;
    std::size_t offset;
};

template<typename T>
class data_buffer
{
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);

public:
    static constexpr std::size_t segment_capacity = 0x4000 / sizeof(T);
    static constexpr command_buffer::state_id state_id =
        data_buffer_traits<T>::state_id;

    void clear() noexcept
    {
        m_buffers.erase(m_buffers.begin() + m_next, m_buffers.end());
        m_next = 0;
    }

    void finalize() noexcept
    {
        if (m_current_buffer)
        {
            m_current_buffer->Unlock();
        }
        m_current_buffer = nullptr;
        m_data = {};
    }

    data_segment<T> allocate(
        ::IDirect3DDevice8* d3d_device, command_buffer& command_buffer,
        std::size_t count) noexcept
    {
        return allocate(d3d_device, command_buffer, count, count);
    }

    data_segment<T> allocate(
        ::IDirect3DDevice8* d3d_device, command_buffer& command_buffer,
        std::size_t min_count, std::size_t max_count) noexcept
    {
        if (m_data.size() <= min_count)
        {
            finalize();
            allocate_next(d3d_device);
            command_buffer.emplace<set_data_buffer_command<T>>(
                m_current_buffer);
        }

        command_buffer.emplace<set_data_buffer_command<T>>(m_current_buffer);
        auto const count = std::min(max_count, m_data.size());
        auto const data = m_data.subspan(0, count);
        auto const offset = segment_capacity - m_data.size();
        m_data = m_data.subspan(count);
        return {data, offset};
    }

    data_segment<T>
    allocate_and_set(::IDirect3DDevice8* d3d_device, std::size_t count) noexcept
    {
        return allocate_and_set(d3d_device, count, count);
    }

    data_segment<T> allocate_and_set(
        ::IDirect3DDevice8* d3d_device, std::size_t min_count,
        std::size_t max_count) noexcept
    {
        if (m_data.size() <= min_count)
        {
            finalize();
            allocate_next(d3d_device);
            data_buffer_traits<T>::set_buffer(d3d_device, m_current_buffer);
        }

        auto const count = std::min(max_count, m_data.size());
        auto const data = m_data.subspan(0, count);
        auto const offset = segment_capacity - m_data.size();
        m_data = m_data.subspan(count);
        return {data, offset};
    }

private:
    std::span<T> m_data;
    typename data_buffer_traits<T>::pointer m_current_buffer = nullptr;
    std::vector<typename data_buffer_traits<T>::com_pointer> m_buffers;
    std::size_t m_next = 0;

    void allocate_next(::IDirect3DDevice8* d3d_device) noexcept
    {
        struct helper
        {
            T data[segment_capacity];
        };

        if (m_next >= m_buffers.size())
        {
            m_buffers.push_back(
                data_buffer_traits<T>::allocate(d3d_device, segment_capacity));
        }
        m_current_buffer = gsl::at(m_buffers, m_next).get();
        ::BYTE* ptr = nullptr;
        m_current_buffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
        m_data = std::span<T>{(new (ptr) helper)->data};
        ++m_next;
    }
};

}

#endif
