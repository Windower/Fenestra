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

#include "packet_queue.hpp"

#include "addon/modules/packet.hpp"

namespace
{

struct packet_header
{
public:
    packet_header() noexcept = default;
    packet_header(
        std::uint16_t id, std::size_t size, std::uint16_t counter) noexcept :
        m_packed_id_size{id & 0x1FF
                         | (size + sizeof(packet_header)) << 7 & 0xFE00},
        m_counter{counter}
    {}

    std::size_t size() const noexcept
    {
        return (m_packed_id_size >> 7 & 0x1FC) - sizeof(packet_header);
    }
    std::uint16_t id() const noexcept { return m_packed_id_size & 0x1FF; }
    std::uint16_t counter() const noexcept { return m_counter; }

private:
    std::uint16_t m_packed_id_size;
    std::uint16_t m_counter;
};

static_assert(sizeof(packet_header) == 4);

constexpr auto max_packet_size = 508;

template<typename T>
T read(std::span<std::byte const>& input)
{
    static_assert(std::is_trivially_copyable_v<T>);

    T value;
    auto const span = std::as_writable_bytes(std::span{&value, 1});
    std::copy_n(input.begin(), span.size(), span.begin());
    input = input.subspan(sizeof value);
    return value;
}

std::span<std::byte const>
read(std::span<std::byte const>& input, std::size_t size) noexcept
{
    auto const value = input.subspan(0, size);
    input = input.subspan(size);
    return value;
}

std::span<std::byte const>
write(std::span<std::byte>& output, std::span<std::byte const> value)
{
    std::copy_n(value.begin(), value.size(), output.begin());
    output = output.subspan(value.size());
    return value;
}

template<typename T>
T const& write(std::span<std::byte>& output, T const& value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    write(output, std::as_bytes(std::span{&value, 1}));
    return value;
}

}

void windower::packet_queue::queue(
    std::uint16_t id, std::vector<std::byte> data, std::u8string injected_by)
{
    m_queue.emplace_back(id, std::move(data), std::move(injected_by));
}

std::span<std::byte const> windower::packet_queue::process_buffer(
    std::span<std::byte const> input, std::uint16_t counter,
    std::uint32_t timestamp, std::size_t output_size)
{
    if (m_output_buffer.size() < output_size)
    {
        m_output_buffer.clear();
        m_output_buffer.resize(output_size);
    }
    auto output = std::span{m_output_buffer};

    while (input.size() >= sizeof(packet_header))
    {
        auto const header = read<packet_header>(input);
        auto const size = header.size();
        if (size > input.size())
        {
            continue;
        }
        auto const id = header.id();
        auto const data = read(input, size);
        if (data.size() + sizeof header <= output.size())
        {
            process_packet(output, id, counter, timestamp, data);
        }
        else
        {
            m_queue.emplace_front(
                id, std::vector<std::byte>{data.begin(), data.end()},
                std::u8string{});
            core::error(
                u8"",
                u8"WARNING!!! Client packet was delayed due to buffer "
                u8"overflow.");
        }
    }

    while (!m_queue.empty() && peek_size() <= output.size())
    {
        auto const& packet = m_queue.front();
        process_packet(
            output, packet.id, counter, timestamp, packet.data,
            packet.injected_by);
        m_queue.pop_front();
    }

    return {m_output_buffer.data(), m_output_buffer.size() - output.size()};
}

std::size_t windower::packet_queue::peek_size() const noexcept
{
    return m_queue.front().data.size() + sizeof(packet_header);
}

void windower::packet_queue::process_packet(
    std::span<std::byte>& output, std::uint16_t id, std::uint16_t counter,
    std::uint32_t timestamp, std::span<std::byte const> data,
    std::u8string_view injected_by) const
{
    using namespace windower;

    auto const result = trigger_packet(
        m_direction == packet_direction::incoming, id, counter, timestamp, data,
        injected_by);

    if (result.blocked())
    {
        return;
    }

    if (result.unchanged())
    {
        write(output, packet_header{id, data.size(), counter});
        write(output, data);
        return;
    }

    auto const result_id = result.id();
    auto const result_data = result.data();

    if (result_data.size() + sizeof(packet_header) > output.size())
    {
        core::error(
            u8"", u8"WARNING!!! Oversized packet modifications were dropped.");
        write(output, packet_header{id, data.size(), counter});
        write(output, data);
        return;
    }

    write(output, packet_header{result_id, result_data.size(), counter});
    write(output, result_data);
}

windower::packet_queue::packet::packet(
    std::uint16_t id, std::vector<std::byte> data,
    std::u8string_view injected_by) :
    id{id},
    data{data}, injected_by{injected_by}
{}
