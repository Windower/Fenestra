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

#ifndef WINDOWER_UI_STATIC_ANY_HPP
#define WINDOWER_UI_STATIC_ANY_HPP

#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace windower::ui
{

template<std::size_t S>
class static_any
{
public:
    static_any() noexcept = default;

    static_any(static_any const& other) noexcept :
        m_state_type_id{other.m_state_type_id},
        m_destructor{other.m_destructor}, m_copy{other.m_copy},
        m_move{other.m_move}
    {
        if (other.m_copy)
        {
            other.m_copy(other, *this);
        }
    }

    static_any(static_any&& other) noexcept :
        m_state_type_id{std::move(other.m_state_type_id)},
        m_destructor{std::move(other.m_destructor)},
        m_copy{std::move(other.m_copy)}, m_move{std::move(other.m_move)}
    {
        if (other.m_move)
        {
            other.m_move(other, *this);
        }
        other.clear();
    }

    template<typename T, typename... A>
    static_any(std::in_place_type_t<T>, A&&... args) noexcept
    {
        emplace<T>(std::forward<A>(args)...);
    }

    ~static_any() noexcept { clear(); }

    static_any& operator=(static_any const& other) noexcept
    {
        clear();
        if (other.m_copy)
        {
            other.m_copy(other, *this);
        }
        return *this;
    }

    static_any& operator=(static_any&& other) noexcept
    {
        clear();
        if (other.m_move)
        {
            other.m_move(other, *this);
        }
        other.clear();
        return *this;
    }

    template<typename T, typename... A>
    std::remove_cvref_t<T>& emplace(A&&... args) noexcept
    {
        using type = std::remove_cvref_t<T>;

        static_assert(std::is_nothrow_constructible_v<type, A...>);
        static_assert(std::is_nothrow_copy_constructible_v<type>);
        static_assert(std::is_nothrow_move_constructible_v<type>);
        static_assert(std::is_nothrow_destructible_v<type>);
        static_assert(sizeof(type) <= S);
        static_assert(alignof(type) <= alignof(std::max_align_t));

        clear();
        auto result  = new (&m_storage.data) type(std::forward<A>(args)...);
        m_destructor = [](static_any& ctx) noexcept {
            std::destroy_at(ctx.value<type>());
        };
        m_copy = [](static_any const& src, static_any& dst) noexcept {
            new (&dst.m_storage.data) type(*src.value<type>());
        };
        m_move = [](static_any& src, static_any& dst) noexcept {
            new (&dst.m_storage.data) type(std::move(*src.value<type>()));
        };
        m_state_type_id = type_id<type>();
        return *result;
    }

    void clear() noexcept
    {
        if (m_state_type_id)
        {
            m_destructor(*this);
            m_state_type_id = 0;
            m_destructor    = nullptr;
            m_move          = nullptr;
        }
    }

    template<typename T>
    std::remove_cvref_t<T>* value() noexcept
    {
        using type = std::remove_cvref_t<T>;

        if (m_state_type_id != type_id<type>())
        {
            return static_cast<type*>(nullptr);
        }
        void* ptr = static_cast<void*>(m_storage.data);
        return std::launder(static_cast<type*>(ptr));
    }

    template<typename T>
    std::remove_cvref_t<T> const* value() const noexcept
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;

        if (m_state_type_id != type_id<type>())
        {
            return static_cast<type const*>(nullptr);
        }
        void const* ptr = static_cast<void const*>(m_storage.data);
        return std::launder(static_cast<type const*>(ptr));
    }

    bool empty() const noexcept { return m_state_type_id == 0; }

private:
    union storage_type
    {
        alignas(std::max_align_t) std::byte data[S];

        storage_type() noexcept                    = default;
        storage_type(storage_type const&) noexcept = delete;
        storage_type(storage_type&&) noexcept      = delete;

        ~storage_type() noexcept {};

        storage_type& operator=(storage_type const&) noexcept = delete;
        storage_type& operator=(storage_type&&) noexcept = delete;
    };

    std::uintptr_t m_state_type_id                          = 0;
    storage_type m_storage                                  = {};
    void (*m_destructor)(static_any&) noexcept              = nullptr;
    void (*m_copy)(static_any const&, static_any&) noexcept = nullptr;
    void (*m_move)(static_any&, static_any&) noexcept       = nullptr;

    template<typename T>
    static std::uintptr_t type_id() noexcept
    {
        static constexpr char tag{0};
        return std::bit_cast<std::uintptr_t>(&tag);
    }
};

}

#endif
