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

#ifndef WINDOWER_GUID_HPP
#define WINDOWER_GUID_HPP

#include <guiddef.h>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>

namespace windower
{

class guid
{
public:
    static guid generate() noexcept;

    constexpr guid() noexcept = default;
    constexpr guid(::GUID const& guid) noexcept : m_guid{guid} {}

    constexpr bool operator==(guid const& other) const noexcept
    {
        return std::ranges::equal(raw(), other.raw());
    }

    constexpr ::GUID const& get() const noexcept { return m_guid; }
    constexpr ::GUID* put() noexcept { return &m_guid; }

    std::span<std::byte const> raw() const noexcept;
    std::u8string string() const noexcept;

private:
    ::GUID m_guid = {};
};

}

#endif
