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

#ifndef WINDOWER_LIBRARY_HPP
#define WINDOWER_LIBRARY_HPP

#include "utility.hpp"

#include <gsl/gsl>

#include <filesystem>
#include <string_view>
#include <type_traits>

namespace windower
{

class library
{
public:
    library() noexcept = default;
    library(library const&);
    library(library&&) noexcept;
    library(std::filesystem::path const&);

    ~library();

    library& operator=(library const&);
    library& operator=(library&&) noexcept;
    explicit operator bool() const noexcept;

    template<typename T>
    operator T*() const noexcept
    {
        return static_cast<T*>(m_handle);
    }

    void (*get_function(u8zstring_view) const noexcept)();

    template<typename F>
    requires std::is_function_v<std::remove_pointer_t<F>>
        std::add_pointer_t<std::remove_pointer_t<F>>
        get_function(u8zstring_view name)
    const noexcept
    {
        GSL_SUPPRESS(type.1)
        {
            return reinterpret_cast<std::remove_pointer_t<F>*>(
                get_function(name));
        }
    }

private:
    void* m_handle = nullptr;
};

}

#endif
