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

#ifndef WINDOWER_ADDON_SCHEDULER_HPP
#define WINDOWER_ADDON_SCHEDULER_HPP

#include "addon/lua.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include <experimental/resumable>

namespace windower
{

struct wait_state
{
    constexpr wait_state() = default;

    constexpr wait_state(
        std::chrono::steady_clock::time_point time, std::size_t frame) :
        time{time},
        frame{frame}
    {}

    std::chrono::steady_clock::time_point time =
        std::chrono::steady_clock::time_point::min();
    std::size_t frame = 0;
};

bool operator<=(wait_state const&, wait_state const&) noexcept;

wait_state min(wait_state const&, wait_state const&) noexcept;

constexpr wait_state resume{std::chrono::steady_clock::time_point::min(), 0};
constexpr wait_state suspend{
    std::chrono::steady_clock::time_point::max(),
    std::numeric_limits<std::size_t>::max()};

class task
{
public:
    class promise_type
    {
    public:
        auto get_return_object() noexcept
        {
            return task{
                std::experimental::coroutine_handle<promise_type>::from_promise(
                    *this)};
        }

        auto initial_suspend() noexcept
        {
            return std::experimental::suspend_always{};
        }

        auto final_suspend() noexcept
        {
            return std::experimental::suspend_always{};
        }

        void return_void() noexcept {}

        auto yield_value(wait_state const& wait) noexcept
        {
            m_wait_state = wait;
            return yield_result{true};
        }

        auto yield_value(std::tuple<bool, wait_state> const& wait) noexcept
        {
            m_wait_state = std::get<1>(wait);
            return yield_result{std::get<0>(wait)};
        }

        void unhandled_exception() noexcept
        {
            m_exception = std::current_exception();
        }

    private:
        class yield_result
        {
        public:
            yield_result(bool const condition) noexcept : m_condition{condition}
            {}

            bool await_ready() const noexcept { return !m_condition; }

            void
            await_suspend(std::experimental::coroutine_handle<>) const noexcept
            {}

            bool await_resume() const noexcept { return m_condition; }

        private:
            bool m_condition;
        };

        wait_state m_wait_state = windower::resume;
        std::exception_ptr m_exception;
        void const* m_tag = nullptr;

        friend class task;
    };

    task()            = default;
    task(task const&) = delete;
    task(task&&) noexcept;

    ~task();

    task& operator=(task const&) = delete;
    task& operator=(task&&) noexcept;

    windower::wait_state& wait_state() noexcept;
    windower::wait_state const& wait_state() const noexcept;
    windower::wait_state resume() const;
    bool done() const noexcept;

    void const* tag() const noexcept;
    void tag(void const*) const noexcept;

private:
    std::experimental::coroutine_handle<promise_type> m_coroutine;

    explicit task(std::experimental::coroutine_handle<promise_type>) noexcept;
};

class scheduler
{
public:
    static void next_frame() noexcept;
    static std::size_t current_frame() noexcept;

    scheduler() noexcept;

    scheduler(scheduler const&) = delete;
    scheduler(scheduler&&)      = delete;

    scheduler& operator=(scheduler const&) = delete;
    scheduler& operator=(scheduler&&)      = delete;

    void schedule(task&&, void const* = nullptr);

    wait_state run_until_idle();

    void purge(void const*);

    void reset() noexcept;

    void error_handler(
        std::function<bool(std::exception_ptr, void const*)>) noexcept;

private:
    static std::atomic<std::size_t> m_current_frame;

    std::vector<windower::task> m_scheduled_tasks;
    std::function<bool(std::exception_ptr, void const*)> m_error_handler;

    windower::task initialize();
};

template<typename C, typename D>
constexpr auto sleep_until(std::chrono::time_point<C, D> time_point)
{
    return wait_state{
        std::chrono::time_point_cast<std::chrono::steady_clock::duration>(
            time_point),
        0};
}

template<typename R, typename P>
auto sleep_for(std::chrono::duration<R, P> duration) noexcept
{
    return sleep_until(std::chrono::steady_clock::now() + duration);
}

inline auto sleep_frame(std::size_t frame) noexcept
{
    return wait_state{
        std::chrono::steady_clock::time_point::min(),
        scheduler::current_frame() + frame};
}

}

#endif
