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

#include "addon/scheduler.hpp"

#include "addon/error.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp"

#include <windows.h>

#include <lua.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <utility>
#include <vector>

#include <experimental/resumable>

bool windower::operator<=(wait_state const& lhs, wait_state const& rhs) noexcept
{
    static_assert(
        gsl::narrow_cast<std::make_signed_t<std::size_t>>(
            std::numeric_limits<std::size_t>::max()) == -1);

    return lhs.time <= rhs.time &&
           gsl::narrow_cast<std::make_signed_t<std::size_t>>(
               lhs.frame - rhs.frame) <= 0;
}

windower::wait_state
windower::min(wait_state const& a, wait_state const& b) noexcept
{
    using std::min;

    return {min(a.time, b.time), min(a.frame, b.frame)};
}

windower::task::task(task&& other) noexcept : m_coroutine{other.m_coroutine}
{
    other.m_coroutine = nullptr;
}

windower::task::task(
    std::experimental::coroutine_handle<promise_type> coroutine) noexcept :
    m_coroutine{coroutine}
{}

inline windower::task::~task()
{
    if (m_coroutine)
    {
        m_coroutine.destroy();
        m_coroutine = nullptr;
    }
}

windower::task& windower::task::operator=(task&& other) noexcept
{
    if (m_coroutine)
    {
        m_coroutine.destroy();
        m_coroutine = nullptr;
    }
    m_coroutine       = other.m_coroutine;
    other.m_coroutine = nullptr;
    return *this;
}

windower::wait_state& windower::task::wait_state() noexcept
{
    return m_coroutine.promise().m_wait_state;
}

windower::wait_state const& windower::task::wait_state() const noexcept
{
    return m_coroutine.promise().m_wait_state;
}

windower::wait_state windower::task::resume() const
{
    auto const handle = m_coroutine;
    if (handle && !handle.done())
    {
        handle.resume();
        if (auto exception = handle.promise().m_exception)
        {
            std::rethrow_exception(exception);
        }
        return handle.promise().m_wait_state;
    }
    return suspend;
}

bool windower::task::done() const noexcept
{
    return !m_coroutine || m_coroutine.done();
}

void const* windower::task::tag() const noexcept
{
    return m_coroutine.promise().m_tag;
}

void windower::task::tag(void const* tag) const noexcept
{
    m_coroutine.promise().m_tag = tag;
}

std::atomic<std::size_t> windower::scheduler::m_current_frame = 0;

void windower::scheduler::next_frame() noexcept { ++m_current_frame; }

std::size_t windower::scheduler::current_frame() noexcept
{
    return m_current_frame;
}

windower::scheduler::scheduler() noexcept {}

void windower::scheduler::schedule(windower::task&& task, void const* tag)
{
    task.tag(tag);
    m_scheduled_tasks.emplace_back(std::move(task));
}

windower::wait_state windower::scheduler::run_until_idle()
{
    if (m_scheduled_tasks.empty())
    {
        return suspend;
    }

    auto wait = suspend;
    auto idle = false;
    while (!idle)
    {
        idle = true;
        for (auto& scheduled_task : m_scheduled_tasks)
        {
            if (!scheduled_task.done() &&
                scheduled_task.wait_state() <=
                    wait_state{
                        std::chrono::steady_clock::now(), current_frame()})
            {
                auto tag = scheduled_task.tag();
                try
                {
                    wait = min(wait, scheduled_task.resume());
                }
                catch (...)
                {
                    if (!m_error_handler ||
                        !m_error_handler(std::current_exception(), tag))
                    {
                        throw;
                    }
                }
                idle = false;
            }
            else
            {
                wait = min(wait, scheduled_task.wait_state());
            }
        }
    }
    m_scheduled_tasks.erase(
        std::remove_if(
            m_scheduled_tasks.begin(), m_scheduled_tasks.end(),
            [](auto const& t) noexcept { return t.done(); }),
        m_scheduled_tasks.end());
    return wait;
}

void windower::scheduler::purge(void const* tag)
{
    m_scheduled_tasks.erase(
        std::remove_if(
            m_scheduled_tasks.begin(), m_scheduled_tasks.end(),
            [=](auto const& t) noexcept { return t.tag() == tag; }),
        m_scheduled_tasks.end());
}

void windower::scheduler::reset() noexcept { m_scheduled_tasks.clear(); }

void windower::scheduler::error_handler(
    std::function<bool(std::exception_ptr, void const*)> handler) noexcept
{
    m_error_handler = std::move(handler);
}

windower::task windower::scheduler::initialize()
{
    while (true)
    {
        m_scheduled_tasks.erase(
            std::partition(
                m_scheduled_tasks.begin(), m_scheduled_tasks.end(),
                [](auto const& t) noexcept { return !t.done(); }),
            m_scheduled_tasks.end());

        auto wait = suspend;
        wait_state const now{std::chrono::steady_clock::now(), current_frame()};
        for (auto& scheduled_task : m_scheduled_tasks)
        {
            if (scheduled_task.wait_state() <= now)
            {
                auto exception = std::exception_ptr{};
                try
                {
                    scheduled_task.resume();
                }
                catch (...)
                {
                    exception = std::current_exception();
                }

                if (exception)
                {
                    if (m_error_handler)
                    {
                        if (!m_error_handler(exception, scheduled_task.tag()))
                        {
                            co_return;
                        }
                    }
                    else
                    {
                        std::rethrow_exception(exception);
                    }
                }

                wait = resume;
                break;
            }
            else
            {
                wait = min(wait, scheduled_task.wait_state());
            }
        }

        co_yield wait;
    }
}
