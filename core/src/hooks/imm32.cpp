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

#include "hooks/imm32.hpp"

#include "core.hpp"
#include "hooklib/hook.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>

namespace
{
::HIMC override_context;
::HIMC client_context;

::LONG get_composition_buffer(
    ::HIMC context, ::DWORD index, std::vector<std::uint8_t>& buffer) noexcept
{
    auto size = ::ImmGetCompositionStringW(context, index, nullptr, 0);
    if (size >= 0)
    {
        buffer.resize(size);
        size = ::ImmGetCompositionStringW(context, index, buffer.data(), size);
        if (size >= 0)
        {
            return 0;
        }
    }
    return size;
}

std::vector<unsigned char> process_attributes(
    std::vector<std::uint8_t> char_buffer,
    std::vector<std::uint8_t> attr_buffer) noexcept
{
    std::vector<unsigned char> result;

    auto utf_16_begin = std::bit_cast<wchar_t*>(char_buffer.data());
    auto utf_16_end   = std::bit_cast<wchar_t*>(
        std::next(char_buffer.data(), char_buffer.size()));
    std::wstring utf_16{utf_16_begin, utf_16_end};

    int attr_index = 0;
    for (auto c : utf_16)
    {
        if (c >= 0xD800 && c <= 0xDFFF)
        {
            if (c >= 0xDC00)
            {
                continue;
            }
            c = 0xFFFD;
        }
        auto const count =
            windower::to_sjis_string(std::wstring_view{&c, 1}).size();
        auto attr  = gsl::at(attr_buffer, attr_index++);
        for (std::size_t n = 0; n < count; ++n)
        {
            result.push_back(attr);
        }
    }

    return result;
}

namespace hooks
{
windower::hooklib::hook<decltype(::ImmAssociateContext)> ImmAssociateContext;
windower::hooklib::hook<decltype(::ImmAssociateContextEx)>
    ImmAssociateContextEx;
windower::hooklib::hook<decltype(::ImmGetCandidateListA)> ImmGetCandidateListA;
windower::hooklib::hook<decltype(::ImmGetCompositionStringA)>
    ImmGetCompositionStringA;
}

namespace callbacks
{
::HIMC WINAPI ImmAssociateContext(::HWND hWnd, ::HIMC hIMC) noexcept
{
    if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        auto result    = client_context;
        client_context = hIMC;
        if (!override_context)
        {
            hooks::ImmAssociateContext(hWnd, hIMC);
        }
        return result;
    }
    else if (windower::is_windower_module(WINDOWER_RETURN_ADDRESS))
    {
        override_context = hIMC;
        if (!hIMC)
        {
            hIMC = client_context;
        }
    }
    return hooks::ImmAssociateContext(hWnd, hIMC);
}

::BOOL WINAPI
ImmAssociateContextEx(::HWND hWnd, ::HIMC hIMC, ::DWORD dwFlags) noexcept
{
    if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
    {
        client_context = hIMC;
        if (!override_context)
        {
            hooks::ImmAssociateContextEx(hWnd, hIMC, dwFlags);
        }
        return TRUE;
    }
    else if (windower::is_windower_module(WINDOWER_RETURN_ADDRESS))
    {
        override_context = hIMC;
        if (!hIMC)
        {
            hIMC = client_context;
        }
    }
    return hooks::ImmAssociateContextEx(hWnd, hIMC, dwFlags);
}

::DWORD WINAPI ImmGetCandidateListA(
    ::HIMC hIMC, ::DWORD dwIndex, ::LPCANDIDATELIST lpCandList,
    ::DWORD dwBufLen) noexcept
{
    if (!lpCandList && dwBufLen != 0)
    {
        return 0;
    }

    if (auto const size = ::ImmGetCandidateListW(hIMC, dwIndex, nullptr, 0))
    {
        std::vector<std::uint8_t> utf_16_buffer(size);
        auto utf_16_list =
            std::bit_cast<::CANDIDATELIST*>(utf_16_buffer.data());
        if (::ImmGetCandidateListW(hIMC, dwIndex, utf_16_list, size))
        {
            auto const count = utf_16_list->dwCount;
            std::vector<std::uint8_t> shift_jis_buffer(
                sizeof(::CANDIDATELIST) + (count - 1) * sizeof(::DWORD));
            auto shift_jis_list =
                std::bit_cast<::CANDIDATELIST*>(shift_jis_buffer.data());

            shift_jis_list->dwStyle     = utf_16_list->dwStyle;
            shift_jis_list->dwCount     = utf_16_list->dwCount;
            shift_jis_list->dwSelection = utf_16_list->dwSelection;
            shift_jis_list->dwPageStart = utf_16_list->dwPageStart;
            shift_jis_list->dwPageSize  = utf_16_list->dwPageSize;

            for (std::size_t i = 0; i < count; ++i)
            {
                shift_jis_list =
                    std::bit_cast<::CANDIDATELIST*>(shift_jis_buffer.data());

                gsl::at(shift_jis_list->dwOffset, i) = shift_jis_buffer.size();

                auto utf_16 = std::wstring{std::bit_cast<wchar_t*>(std::next(
                    utf_16_buffer.data(), gsl::at(utf_16_list->dwOffset, i)))};
                utf_16.erase(
                    std::remove_if(
                        utf_16.begin(), utf_16.end(),
                        [](auto c) { return c >= 0xD800 && c <= 0xDBFF; }),
                    utf_16.end());
                std::replace_if(
                    utf_16.begin(), utf_16.end(),
                    [](auto c) { return c >= 0xDC00 && c <= 0xDFFF; },
                    L'\uFFFD');

                auto shift_jis = windower::to_sjis_string(utf_16);

                shift_jis_buffer.insert(
                    std::end(shift_jis_buffer), std::begin(shift_jis),
                    std::end(shift_jis));
                shift_jis_buffer.push_back('\0');
            }

            if (dwBufLen == 0)
            {
                return shift_jis_buffer.size();
            }

            auto buffer_size =
                std::min(std::size_t(dwBufLen), shift_jis_buffer.size());

            shift_jis_list =
                std::bit_cast<::CANDIDATELIST*>(shift_jis_buffer.data());
            shift_jis_list->dwSize = buffer_size;

            std::copy_n(
                std::begin(shift_jis_buffer), buffer_size,
                reinterpret_cast<std::uint8_t*>(lpCandList));

            return count;
        }
    }
    return 0;
}

::LONG WINAPI ImmGetCompositionStringA(
    ::HIMC hIMC, ::DWORD dwIndex, ::LPVOID lpBuf, ::DWORD dwBufLen) noexcept
{
    switch (dwIndex)
    {
    case GCS_COMPREADSTR:
    case GCS_COMPSTR:
    case GCS_RESULTREADSTR:
    case GCS_RESULTSTR: {
        if (!lpBuf && dwBufLen != 0)
        {
            return IMM_ERROR_GENERAL;
        }

        std::vector<unsigned char> buffer;
        if (auto const result = get_composition_buffer(hIMC, dwIndex, buffer))
        {
            return result;
        }

        auto utf_16_begin = std::bit_cast<wchar_t*>(buffer.data());
        auto utf_16_end =
            std::bit_cast<wchar_t*>(std::next(buffer.data(), buffer.size()));
        std::wstring utf_16{utf_16_begin, utf_16_end};
        utf_16.erase(
            std::remove_if(
                utf_16.begin(), utf_16.end(),
                [](auto c) { return c >= 0xD800 && c <= 0xDBFF; }),
            utf_16.end());
        std::replace_if(
            utf_16.begin(), utf_16.end(),
            [](auto c) { return c >= 0xDC00 && c <= 0xDFFF; }, L'\uFFFD');
        auto shift_jis = windower::to_sjis_string(utf_16);

        if (dwBufLen == 0)
        {
            return shift_jis.size();
        }

        auto buffer_size = std::min(std::size_t(dwBufLen), shift_jis.size());
        std::copy_n(
            std::begin(shift_jis), buffer_size, static_cast<char*>(lpBuf));

        return buffer_size;
    }

    case GCS_COMPREADATTR: {
        std::vector<std::uint8_t> char_buffer;
        std::vector<std::uint8_t> attr_buffer;

        if (auto const result =
                get_composition_buffer(hIMC, GCS_COMPREADSTR, char_buffer))
        {
            return result;
        }

        if (auto const result =
                get_composition_buffer(hIMC, GCS_COMPREADATTR, attr_buffer))
        {
            return result;
        }

        std::vector<unsigned char> shift_jis_attr =
            process_attributes(char_buffer, attr_buffer);

        auto buffer_size =
            std::min(std::size_t(dwBufLen), shift_jis_attr.size());
        std::copy_n(
            std::begin(shift_jis_attr), buffer_size,
            static_cast<std::uint8_t*>(lpBuf));

        return buffer_size;
    }

    case GCS_COMPATTR: {
        std::vector<std::uint8_t> char_buffer;
        std::vector<std::uint8_t> attr_buffer;

        if (auto const result =
                get_composition_buffer(hIMC, GCS_COMPSTR, char_buffer))
        {
            return result;
        }

        if (auto const result =
                get_composition_buffer(hIMC, GCS_COMPATTR, attr_buffer))
        {
            return result;
        }

        std::vector<unsigned char> shift_jis_attr =
            process_attributes(char_buffer, attr_buffer);

        auto buffer_size =
            std::min(std::size_t(dwBufLen), shift_jis_attr.size());
        std::copy_n(
            std::begin(shift_jis_attr), buffer_size,
            static_cast<std::uint8_t*>(lpBuf));

        return buffer_size;
    }

    case GCS_CURSORPOS:
    case GCS_DELTASTART: {
        auto const pos = ::ImmGetCompositionStringW(hIMC, dwIndex, nullptr, 0);
        if (pos < 0)
        {
            return pos;
        }

        std::vector<std::uint8_t> buffer;
        if (auto const result =
                get_composition_buffer(hIMC, GCS_COMPSTR, buffer))
        {
            return result;
        }

        auto utf_16_begin = std::bit_cast<wchar_t*>(buffer.data());
        auto utf_16_end   = std::next(utf_16_begin, pos);
        std::wstring utf_16{utf_16_begin, utf_16_end};
        utf_16.erase(
            std::remove_if(
                utf_16.begin(), utf_16.end(),
                [](auto c) { return c >= 0xD800 && c <= 0xDBFF; }),
            utf_16.end());
        std::replace_if(
            utf_16.begin(), utf_16.end(),
            [](auto c) { return c >= 0xDC00 && c <= 0xDFFF; }, L'\uFFFD');
        auto shift_jis = windower::to_sjis_string(utf_16);

        return shift_jis.size();
    }

    default:
        return hooks::ImmGetCompositionStringA(hIMC, dwIndex, lpBuf, dwBufLen);
    }
}
}
}

::HIMC windower::imm32::ImmAssociateContext(::HWND hWnd, ::HIMC hIMC) noexcept
{
    if (hooks::ImmAssociateContext)
    {
        return hooks::ImmAssociateContext(hWnd, hIMC);
    }
    return ::ImmAssociateContext(hWnd, hIMC);
}

::BOOL windower::imm32::ImmAssociateContextEx(
    ::HWND hWnd, ::HIMC hIMC, ::DWORD dwFlags) noexcept
{
    if (hooks::ImmAssociateContextEx)
    {
        return hooks::ImmAssociateContextEx(hWnd, hIMC, dwFlags);
    }
    return ::ImmAssociateContextEx(hWnd, hIMC, dwFlags);
}

::DWORD windower::imm32::ImmGetCandidateListA(
    ::HIMC hIMC, ::DWORD dwIndex, ::LPCANDIDATELIST lpCandList,
    ::DWORD dwBufLen) noexcept
{
    if (hooks::ImmGetCandidateListA)
    {
        return hooks::ImmGetCandidateListA(hIMC, dwIndex, lpCandList, dwBufLen);
    }
    return ::ImmGetCandidateListA(hIMC, dwIndex, lpCandList, dwBufLen);
}

::LONG windower::imm32::ImmGetCompositionStringA(
    ::HIMC hIMC, ::DWORD dwIndex, ::LPVOID lpBuf, ::DWORD dwBufLen) noexcept
{
    if (hooks::ImmGetCompositionStringA)
    {
        return hooks::ImmGetCompositionStringA(hIMC, dwIndex, lpBuf, dwBufLen);
    }
    return ::ImmGetCompositionStringA(hIMC, dwIndex, lpBuf, dwBufLen);
}

void windower::imm32::install()
{
    if (!hooks::ImmGetCandidateListA)
    {
        hooks::ImmAssociateContext = hooklib::make_hook(
            u8"imm32.dll", u8"ImmAssociateContext",
            callbacks::ImmAssociateContext);
        hooks::ImmAssociateContextEx = hooklib::make_hook(
            u8"imm32.dll", u8"ImmAssociateContextEx",
            callbacks::ImmAssociateContextEx);
        hooks::ImmGetCandidateListA = hooklib::make_hook(
            u8"imm32.dll", u8"ImmGetCandidateListA",
            callbacks::ImmGetCandidateListA);
        hooks::ImmGetCompositionStringA = hooklib::make_hook(
            u8"imm32.dll", u8"ImmGetCompositionStringA",
            callbacks::ImmGetCompositionStringA);
    }
}

void windower::imm32::uninstall() noexcept
{
    hooks::ImmAssociateContext      = {};
    hooks::ImmAssociateContextEx    = {};
    hooks::ImmGetCandidateListA     = {};
    hooks::ImmGetCompositionStringA = {};
}
