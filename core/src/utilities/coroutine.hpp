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

#pragma once

#include <coroutine>
#include <future>
#include <type_traits>

template <typename R, typename T, typename... Args>
    requires(!std::is_void_v<R> && !std::is_reference_v<R>)
struct std::coroutine_traits<
    std::future<R>, T&, Args...>
{
    struct promise_type : std::promise<R>
    {
        std::future<R> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_value(R const& value) noexcept(
            std::is_nothrow_copy_constructible_v<R>)
        {
            this->set_value(value);
        }

        void return_value(R&& value) noexcept(
            std::is_nothrow_move_constructible_v<R>)
        {
            this->set_value(std::move(value));
        }

        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};

template <typename T, typename... Args>
struct std::coroutine_traits<
    std::future<void>, T&, Args...>
{
    struct promise_type : std::promise<void>
    {
        std::future<void> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_void() noexcept { this->set_value(); }

        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};

template<typename R>
auto operator co_await(std::future<R> future) noexcept
    requires(!std::is_reference_v<R>)
{
    struct awaiter : std::future<R>
    {
        bool await_ready() const noexcept
        {
            return this->wait_for(std::chrono::seconds::zero()) !=
                   std::future_status::timeout;
        }

        void await_suspend(std::coroutine_handle<> continuation) const
        {
            std::thread([this, continuation] {
                this->await_ready();
                continuation();
            }).detach();
        }

        R await_resume() { return this->get(); }
    };

    return awaiter{std::move(future)};
}
