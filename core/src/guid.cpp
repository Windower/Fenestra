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

#include "guid.hpp"

#include <guiddef.h>
#include <objbase.h>

#include <gsl/gsl>

#include <algorithm>
#include <array>
#include <ranges>
#include <string>

windower::guid windower::guid::generate() noexcept
{
    auto guid = ::GUID{};
    if (SUCCEEDED(::CoCreateGuid(&guid)))
    {
        return guid;
    }
    return {};
}

std::span<std::byte const> windower::guid::raw() const noexcept
{
    return std::as_bytes(std::span{&m_guid, 1});
}

std::u8string windower::guid::string() const noexcept
{
    namespace range = std::ranges;
    namespace view  = std::ranges::views;

    std::array<::OLECHAR, 39> buffer{};
    if (::StringFromGUID2(m_guid, buffer.data(), buffer.size()) != 39)
    {
        return u8"00000000-0000-0000-0000-000000000000";
    }

    std::u8string result;
    range::transform(
        buffer | view::drop(1) | view::take(36), std::back_inserter(result),
        [](auto c) { return gsl::narrow_cast<char8_t>(c); });
    return result;
}
