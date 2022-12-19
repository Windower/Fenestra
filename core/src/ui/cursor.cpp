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

#include "ui/cursor.hpp"

#include "utility.hpp"

#include <windows.h>

#include <utility>

namespace windower::ui
{

cursor::cursor(cursor const& other) noexcept :
    m_handle{::CopyImage(
        static_cast<::HANDLE>(other.m_handle), IMAGE_CURSOR, 0, 0, 0)}
{}

cursor::cursor(cursor&& other) noexcept : m_handle{std::move(other.m_handle)}
{
    other.m_handle = nullptr;
}

cursor::cursor(std::filesystem::path const& path) noexcept :
    m_handle{::LoadCursorFromFileW(path.c_str())}
{}

cursor::~cursor() noexcept
{
    if (*this && !::DestroyCursor(static_cast<::HCURSOR>(m_handle)) &&
        ::GetLastError() != ERROR_SUCCESS)
    {
        fail_fast();
    }
    m_handle = nullptr;
}

cursor& cursor::operator=(cursor const& other) noexcept
{
    using std::swap;

    if (this != &other)
    {
        auto temp = other;
        swap(*this, temp);
    }

    return *this;
}

cursor& cursor::operator=(cursor&& other) noexcept
{
    cursor::~cursor();
    m_handle       = nullptr;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

cursor::operator bool() const noexcept
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

}
