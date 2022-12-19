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

#ifndef WINDOWER_CLOAK_HPP
#define WINDOWER_CLOAK_HPP

namespace windower
{

class cloak_guard;

void pin_and_cloak() noexcept;
[[nodiscard]] cloak_guard uncloak() noexcept;

class cloak_guard final
{
public:
    cloak_guard(cloak_guard const&) = delete;
    cloak_guard(cloak_guard&&)      = default;

    ~cloak_guard();

    cloak_guard& operator=(cloak_guard const&) = delete;
    cloak_guard& operator=(cloak_guard&&)      = delete;

private:
    cloak_guard() = default;

    friend cloak_guard uncloak() noexcept;
};

}

#endif
