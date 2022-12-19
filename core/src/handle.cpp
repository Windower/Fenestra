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

#include "handle.hpp"

#include "utility.hpp"

#include <windows.h>

#include <system_error>
#include <utility>

windower::handle::handle() noexcept : m_handle{INVALID_HANDLE_VALUE} {}

windower::handle::handle(handle const& other) : m_handle{INVALID_HANDLE_VALUE}
{
    if (other)
    {
        auto process = ::GetCurrentProcess();

        auto handle = ::HANDLE{};
        if (!::DuplicateHandle(
                process, other.m_handle, process, &handle, 0, false,
                DUPLICATE_SAME_ACCESS))
        {
            throw std::system_error{
                std::error_code(::GetLastError(), std::system_category())};
        }

        m_handle = handle;
    }
}

windower::handle::handle(handle&& other) noexcept : m_handle{other.m_handle}
{
    other.m_handle = INVALID_HANDLE_VALUE;
}

windower::handle::handle(void* handle) noexcept : m_handle{handle} {}

windower::handle::~handle()
{
    if (*this && !::CloseHandle(m_handle))
    {
        fail_fast();
    }
}

windower::handle& windower::handle::operator=(handle const& other) noexcept
{
    using std::swap;

    if (this != &other)
    {
        auto temp = other;
        swap(*this, temp);
    }

    return *this;
}

windower::handle& windower::handle::operator=(handle&& other) noexcept
{
    handle::~handle();
    m_handle       = nullptr;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

windower::handle::operator bool() const noexcept
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

windower::handle::operator void*() const noexcept { return m_handle; }
