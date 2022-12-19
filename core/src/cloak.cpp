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

#include "cloak.hpp"

#include "utility.hpp"

#include <windows.h>

#include <intrin.h>

#include <gsl/gsl>

#include <bit>

namespace
{

struct UNICODE_STRING
{
    ::USHORT Length;
    ::USHORT MaximumLength;
    ::PWSTR Buffer;
};

struct PEB_LDR_DATA
{
    ::ULONG Length;
    ::BOOLEAN Initialized;
    ::PVOID SsHandle;
    ::LIST_ENTRY InLoadOrderModuleList;
    ::LIST_ENTRY InMemoryOrderModuleList;
    ::LIST_ENTRY InInitializationOrderModuleList;
};

struct LDR_DATA_TABLE_ENTRY
{
    ::LIST_ENTRY InLoadOrderLinks;
    ::LIST_ENTRY InMemoryOrderLinks;
    ::LIST_ENTRY InInitializationOrderLinks;
    ::PVOID DllBase;
    ::PVOID EntryPoint;
    ::ULONG SizeOfImage;
    ::UNICODE_STRING FullDllName;
    ::UNICODE_STRING BaseDllName;
    ::ULONG Flags;
    ::USHORT ObsoleteLoadCount;
    ::USHORT TlsIndex;
    ::LIST_ENTRY HashLinks;
};

::LDR_DATA_TABLE_ENTRY* cloaked_entry = nullptr;

void const* pin(void const* module)
{
    ::HMODULE dummy = nullptr;
    ::GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        static_cast<::LPCWSTR>(module), &dummy);
    return module;
}

::PEB_LDR_DATA* get_peb_ldr_data() noexcept
{
    return *std::bit_cast<::PEB_LDR_DATA**>(__readfsdword(0x30) + 0x0C);
}

void link(::LIST_ENTRY& node, ::LIST_ENTRY& new_node) noexcept
{
    new_node.Flink        = node.Flink;
    new_node.Blink        = &node;
    node.Flink            = &new_node;
    new_node.Flink->Blink = &new_node;
}

void unlink(::LIST_ENTRY& node) noexcept
{
    node.Flink->Blink = node.Blink;
    node.Blink->Flink = node.Flink;
    node.Flink        = &node;
    node.Blink        = &node;
}

}

void windower::pin_and_cloak() noexcept
{
    if (!cloaked_entry)
    {
        auto module       = pin(windower_module());
        auto peb_ldr_data = get_peb_ldr_data();
        auto node         = peb_ldr_data->InLoadOrderModuleList.Flink;
        while (node != &peb_ldr_data->InLoadOrderModuleList)
        {
            auto entry = std::bit_cast<::LDR_DATA_TABLE_ENTRY*>(node);
            if (entry->DllBase == module)
            {
                unlink(entry->InLoadOrderLinks);
                unlink(entry->InMemoryOrderLinks);
                unlink(entry->InInitializationOrderLinks);
                unlink(entry->HashLinks);
                cloaked_entry = entry;
                return;
            }
            node = node->Flink;
        }
    }
}

windower::cloak_guard windower::uncloak() noexcept
{
    if (cloaked_entry)
    {
        auto peb_ldr_data = get_peb_ldr_data();
        link(
            *peb_ldr_data->InLoadOrderModuleList.Blink,
            cloaked_entry->InLoadOrderLinks);
        link(
            *peb_ldr_data->InMemoryOrderModuleList.Blink,
            cloaked_entry->InMemoryOrderLinks);
        link(
            *peb_ldr_data->InInitializationOrderModuleList.Blink,
            cloaked_entry->InInitializationOrderLinks);

        // Not sure what to do about this...
        // GetModuleHandle doesn't work without it, but we don't use it.
        // link(???, cloaked_entry->HashLinks);
    }
    return {};
}

windower::cloak_guard::~cloak_guard()
{
    if (cloaked_entry)
    {
        auto peb_ldr_data = get_peb_ldr_data();
        auto node         = peb_ldr_data->InLoadOrderModuleList.Flink;
        while (node != &peb_ldr_data->InLoadOrderModuleList)
        {
            auto entry = std::bit_cast<::LDR_DATA_TABLE_ENTRY*>(node);
            if (entry->DllBase == cloaked_entry->DllBase)
            {
                unlink(entry->InLoadOrderLinks);
                unlink(entry->InMemoryOrderLinks);
                unlink(entry->InInitializationOrderLinks);
                unlink(entry->HashLinks);
                return;
            }
            node = node->Flink;
        }
    }
}
