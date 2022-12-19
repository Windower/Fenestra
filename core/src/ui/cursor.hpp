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

#ifndef WINDOWER_UI_CURSOR_HPP
#define WINDOWER_UI_CURSOR_HPP

#include <filesystem>

namespace windower::ui
{

class context;

class cursor
{
public:
    cursor() noexcept = default;
    cursor(cursor const& other) noexcept;
    cursor(cursor&& other) noexcept;
    explicit cursor(std::filesystem::path const& path) noexcept;

    ~cursor() noexcept;

    cursor& operator=(cursor const& other) noexcept;
    cursor& operator=(cursor&& other) noexcept;
    explicit operator bool() const noexcept;

private:
    void* m_handle = nullptr;

    friend class context;
};

}

#endif
