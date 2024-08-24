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

#include "trampoline.hpp"

#include "handle.hpp"
#include "hooklib/x86.hpp"
#include "utility.hpp"

#include <windows.h>

#include <tlhelp32.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

class heap
{
public:
    static heap const& instance()
    {
        static heap const instance{HEAP_CREATE_ENABLE_EXECUTE};
        return instance;
    }

    operator ::HANDLE() const noexcept { return m_handle; }

private:
    ::HANDLE m_handle;

    heap(heap const&) = delete;
    heap(heap&&)      = delete;
    heap(::DWORD options) : m_handle{::HeapCreate(options, 0, 0)}
    {
        if (!m_handle)
        {
            windower::throw_system_error();
        }
    }

    ~heap() noexcept
    {
        if (m_handle && !::HeapDestroy(m_handle))
        {
            windower::fail_fast();
        }
    }

    heap& operator=(heap const&) = delete;
    heap& operator=(heap&&) = delete;
};

template<typename T>
class executable_allocator
{
public:
    using value_type = T;

    executable_allocator() = default;

    template<typename U>
    executable_allocator(executable_allocator<U> const&) noexcept
    {}

    T* allocate(std::size_t n)
    {
        auto ptr = ::HeapAlloc(heap::instance(), 0, n * sizeof(T));
        if (!ptr)
        {
            throw std::bad_alloc{};
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t)
    {
        if (!::HeapFree(heap::instance(), 0, ptr))
        {
            windower::throw_system_error();
        }
    }
};

template<typename T, typename U>
bool operator==(executable_allocator<T> const&, executable_allocator<U> const&)
{
    return true;
}

template<typename T, typename U>
bool operator!=(executable_allocator<T> const&, executable_allocator<U> const&)
{
    return false;
}

class patch_guard
{
public:
    patch_guard(
        void* target, std::size_t target_size, void* buffer,
        std::size_t buffer_size, bool thread_guard = true) :
        m_target{std::bit_cast<std::uint8_t*>(target)},
        m_buffer{std::bit_cast<std::uint8_t*>(buffer)},
        m_target_size{target_size}, m_buffer_size{buffer_size}
    {
        if (thread_guard)
        {
            auto threads = get_threads();
            for (auto& handle : threads)
            {
                m_threads.emplace_back(std::move(handle));
                auto& lock = m_threads.back();
                lock.advance_if_in_range(
                    m_target, std::next(m_target, m_target_size));
                lock.advance_if_in_range(
                    m_buffer, std::next(m_buffer, m_buffer_size));
            }
        }

        if (!::VirtualProtect(
                target, target_size, PAGE_EXECUTE_READWRITE, &m_original))
        {
            windower::throw_system_error();
        }
    }

    patch_guard(patch_guard const&) = delete;
    patch_guard(patch_guard&&)      = delete;

    ~patch_guard() noexcept
    {
        if (!::VirtualProtect(m_target, m_target_size, m_original, &m_original))
        {
            windower::fail_fast();
        }
    }

    patch_guard& operator=(patch_guard const&) = delete;
    patch_guard& operator=(patch_guard&&) = delete;

private:
    class thread_lock
    {
    public:
        thread_lock()                   = delete;
        thread_lock(thread_lock const&) = delete;
        thread_lock(thread_lock&&)      = default;

        thread_lock(windower::handle&& handle) : m_handle{std::move(handle)}
        {
            lock();
        }

        ~thread_lock() { unlock(); }

        thread_lock& operator=(thread_lock const&) = delete;
        thread_lock& operator=(thread_lock&&) = default;

        void advance_if_in_range(void* begin, void* end)
        {
            auto instruction_ptr = get_instruction_ptr();
            if (instruction_ptr >= begin && instruction_ptr < end)
            {
                if (thread_unlocker u{*this})
                {
                    do
                    {
                        ::Sleep(0);
                        instruction_ptr = get_instruction_ptr();
                    } while (instruction_ptr >= begin && instruction_ptr < end);
                }
                else
                {
                    auto const id = ::GetThreadId(m_handle);
                    if (id == std::numeric_limits<::DWORD>::max())
                    {
                        windower::throw_system_error();
                    }
                    throw std::runtime_error{
                        "thread [" + std::to_string(id) +
                        "] could not be unlocked."};
                }
            }
        }

    private:
        class thread_unlocker
        {
        public:
            thread_unlocker()                       = delete;
            thread_unlocker(thread_unlocker const&) = delete;
            thread_unlocker(thread_unlocker&&)      = default;

            explicit thread_unlocker(thread_lock& lock) :
                m_lock{lock}, m_unlocked{m_lock.unlock() == 0}
            {}

            ~thread_unlocker() { m_lock.lock(); }

            thread_unlocker& operator=(thread_unlocker const&) = delete;
            thread_unlocker& operator=(thread_unlocker&&) = default;

            explicit operator bool() const noexcept { return m_unlocked; }

        private:
            thread_lock& m_lock;
            bool m_unlocked;
        };

        windower::handle m_handle;

        ::DWORD lock() const
        {
            if (m_handle)
            {
                auto const result = ::SuspendThread(m_handle);
                if (result == std::numeric_limits<::DWORD>::max())
                {
                    auto const error = ::GetLastError();
                    if (error != ERROR_ACCESS_DENIED)
                    {
                        windower::throw_system_error();
                    }
                    return 0;
                }
                return result + 1;
            }
            return 0;
        }

        ::DWORD unlock() const
        {
            if (m_handle)
            {
                auto const result = ::ResumeThread(m_handle);
                if (result == std::numeric_limits<::DWORD>::max())
                {
                    auto const error = ::GetLastError();
                    if (error != ERROR_ACCESS_DENIED)
                    {
                        windower::throw_system_error();
                    }
                    return 0;
                }
                if (result == 0)
                {
                    return 0;
                }
                return result - 1;
            }
            return 0;
        }

        std::uint8_t* get_instruction_ptr() const
        {
            ::CONTEXT context    = {};
            context.ContextFlags = CONTEXT_CONTROL;
            if (!::GetThreadContext(m_handle, &context))
            {
                auto const error = ::GetLastError();
                if (error != ERROR_GEN_FAILURE && error != ERROR_ACCESS_DENIED)
                {
                    windower::throw_system_error(error);
                }
                return nullptr;
            }
            return std::bit_cast<std::uint8_t*>(context.Eip);
        }
    };

    static std::vector<windower::handle> get_threads()
    {
        std::vector<windower::handle> results;

        auto const process_id = ::GetCurrentProcessId();
        auto const thread_id  = ::GetCurrentThreadId();

        windower::handle snapshot =
            ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot)
        {
            THREADENTRY32 thread = {};
            thread.dwSize        = sizeof thread;
            if (::Thread32First(snapshot, &thread))
            {
                do
                {
                    if (thread.dwSize >=
                        FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                            sizeof thread.th32OwnerProcessID)
                    {
                        if (thread.th32OwnerProcessID == process_id &&
                            thread.th32ThreadID != thread_id)
                        {
                            windower::handle temp = ::OpenThread(
                                THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                                    THREAD_QUERY_INFORMATION,
                                false, thread.th32ThreadID);
                            if (temp)
                            {
                                results.emplace_back(std::move(temp));
                            }
                            else
                            {
                                auto const error = ::GetLastError();
                                if (error != ERROR_INVALID_PARAMETER)
                                {
                                    windower::throw_system_error();
                                }
                            }
                        }
                    }
                    thread.dwSize = sizeof thread;
                } while (::Thread32Next(snapshot, &thread));
            }

            auto const error = ::GetLastError();
            if (error != ERROR_NO_MORE_FILES)
            {
                windower::throw_system_error();
            }
        }

        return results;
    }

    std::uint8_t* m_target;
    std::uint8_t* m_buffer;
    std::size_t m_target_size;
    std::size_t m_buffer_size;
    ::DWORD m_original = 0;

    std::vector<thread_lock> m_threads;
};

}

class windower::hooklib::trampoline::block
{
public:
    block(block const&) = delete;
    block(block&&)      = delete;

    block(void (*target)(), void (*callback)(), void (*thunk)())
    {
        std::fill(m_raw.begin(), m_raw.end(), x86::trap_instruction);
        std::fill(m_thunk.begin(), m_thunk.end(), x86::trap_instruction);

        auto raw_target = std::bit_cast<std::uint8_t*>(target);
        m_hotpatched    = x86::is_hotpatchable(raw_target);
        if (m_hotpatched)
        {
            m_size = 2;
        }
        else
        {
            raw_target = x86::follow_jumps(raw_target);
            auto mark  = raw_target;
            do
            {
                mark   = x86::next_instruction(mark);
                m_size = std::distance(raw_target, mark);
            } while (m_size < sizeof(x86::jump));
        }

        std::copy(raw_target, std::next(raw_target, m_size), std::begin(m_raw));
        new (m_raw.data() + m_size) x86::jump(std::next(raw_target, m_size));

        if (thunk)
        {
            new (m_thunk.data()) x86::thiscall_thunk(
                std::bit_cast<std::uint8_t*>(callback),
                std::bit_cast<std::uint8_t*>(thunk));
        }

        m_target = raw_target;

        if (m_hotpatched)
        {
            auto const guard = patch_guard{
                std::prev(m_target, sizeof(x86::jump)),
                m_size + sizeof(x86::jump), this, sizeof(block), false};

            new (std::prev(m_target, sizeof(x86::jump))) x86::jump{
                thunk ? m_thunk.data()
                      : std::bit_cast<std::uint8_t*>(callback)};
            ::InterlockedExchange16(
                std::bit_cast<short*>(m_target),
                gsl::narrow_cast<short>(x86::hotpatch_jump_instruction));
        }
        else
        {
            auto const guard =
                patch_guard{m_target, m_size, this, sizeof(block)};

            new (m_target) x86::jump{
                thunk ? m_thunk.data()
                      : std::bit_cast<std::uint8_t*>(callback)};
            std::fill(
                std::next(m_target, sizeof(x86::jump)),
                std::next(m_target, m_size), x86::trap_instruction);
        }
    }

    block& operator=(block const&) = delete;
    block& operator=(block&&) = delete;

    void (*target() const noexcept)()
    {
        return std::bit_cast<void (*)()>(m_raw.data());
    }

private:
    std::array<unsigned char, x86::max_instruction_size + sizeof(x86::jump)>
        m_raw;
    std::array<unsigned char, sizeof(x86::thiscall_thunk)> m_thunk;
    std::size_t m_size     = 0;
    std::uint8_t* m_target = nullptr;
    bool m_hotpatched      = false;
};

windower::hooklib::trampoline::trampoline(
    void (*target)(), void (*callback)(), void (*thunk)()) :
    m_block{std::construct_at(
        executable_allocator<block>{}.allocate(1), target, callback, thunk)}
{}

windower::hooklib::trampoline::operator bool() const noexcept
{
    return bool(m_block);
}

void (*windower::hooklib::trampoline::target() const noexcept)()
{
    return m_block->target();
}
