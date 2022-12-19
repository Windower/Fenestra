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

#include "scanner.hpp"

#include "library.hpp"

#include <windows.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>

namespace
{

std::span<::IMAGE_SECTION_HEADER const>
get_sections(windower::library const& library) noexcept
{
    std::byte const* const base = library;

    auto dos_header = std::bit_cast<::IMAGE_DOS_HEADER const*>(base);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return {};
    }

    auto nt_header = std::bit_cast<::IMAGE_NT_HEADERS const*>(
        std::next(base, dos_header->e_lfanew));
    if (nt_header->Signature != IMAGE_NT_SIGNATURE ||
        nt_header->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
    {
        return {};
    }

    return std::span<::IMAGE_SECTION_HEADER const>{
        std::bit_cast<::IMAGE_SECTION_HEADER const*>(std::next(
            base, dos_header->e_lfanew + sizeof(nt_header->Signature) +
                      sizeof(nt_header->FileHeader) +
                      nt_header->FileHeader.SizeOfOptionalHeader)),
        nt_header->FileHeader.NumberOfSections};
}

std::span<std::byte> get_data(
    windower::library const& library,
    ::IMAGE_SECTION_HEADER const& section) noexcept
{
    if ((section.Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0)
    {
        return {};
    }
    std::byte* const base = library;
    auto const ptr        = std::next(base, section.VirtualAddress);
    auto const size       = section.Misc.VirtualSize;
    return std::span<std::byte>{ptr, size};
}

std::span<std::byte>::iterator
match(std::span<std::byte> data, windower::signature const& sig)
{
    auto const sig_data = sig.data();
    auto const sig_mask = sig.mask();
    auto it             = data.begin();
    auto end            = std::prev(data.end(), sig.size());
    while (true)
    {
        it = std::find(it, end, *sig_data.begin());
        if (it == end)
        {
            return data.end();
        }
        auto const is_match = std::equal(
            sig_data.begin() + 1, sig_data.end(), it + 1,
            [mask_it = sig_mask.begin() + 1](auto a, auto b) mutable {
                return a == (b & *mask_it++);
            });
        if (is_match)
        {
            return it;
        }
        ++it;
    }
}

}

void windower::scan(
    library const& library, signature const& sig,
    std::span<address> results) noexcept
{
    if (library)
    {
        for (auto const& section : get_sections(library))
        {
            auto section_data = get_data(library, section);
            auto it           = match(section_data, sig);
            while (!results.empty() && it != section_data.end())
            {
                auto result = address{&*it};
                result += sig.offset();
                *results.begin() = sig.dereference() ? *result : result;
                results          = results.subspan(1);
                section_data     = section_data.subspan(
                        std::distance(section_data.begin(), it));
                it = match(section_data, sig);
            }
            if (results.empty())
            {
                return;
            }
        }
    }
    std::fill(results.begin(), results.end(), nullptr);
}
