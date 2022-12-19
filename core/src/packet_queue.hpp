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

#ifndef WINDOWER_PACKET_QUEUE_HPP
#define WINDOWER_PACKET_QUEUE_HPP

#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{

enum class packet_direction
{
    incoming,
    outgoing,
};

class packet_queue
{
public:
    packet_queue(packet_queue const&) = delete;
    packet_queue(packet_queue&&) = default;
    packet_queue(packet_direction direction) : m_direction{direction} {}

    void queue(
        std::uint16_t id, std::vector<std::byte> data,
        std::u8string injected_by);

    std::span<std::byte> temp_buffer(std::size_t);
    std::span<std::byte const> process_buffer(
        std::span<std::byte const>, std::uint16_t, std::uint32_t, std::size_t);

private:
    struct packet
    {
        packet(std::uint16_t, std::vector<std::byte>, std::u8string_view);

        std::uint16_t id;
        std::vector<std::byte> data;
        std::u8string injected_by;
    };

    packet_direction const m_direction;
    std::deque<packet> m_queue;
    std::vector<std::byte> m_output_buffer;

    std::size_t peek_size() const noexcept;
    void process_packet(
        std::span<std::byte>&, std::uint16_t, std::uint16_t, std::uint32_t,
        std::span<std::byte const>, std::u8string_view = {}) const;
};

}

#endif
