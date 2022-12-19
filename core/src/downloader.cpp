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

#include "downloader.hpp"

#include "core.hpp"
#include "handle.hpp"
#include "unicode.hpp"
#include "version.hpp"

#include <windows.h>

#include <winhttp.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <experimental/coroutine>

namespace
{

std::array<wchar_t const*, 2> accept_types = {L"*/*", nullptr};

struct file_data
{
    file_data(windower::downloader::file const& file) : file{file} {}

    std::atomic<windower::downloader::status> status =
        windower::downloader::status::pending;
    std::atomic<std::uint16_t> http_status = 0;
    std::atomic<std::uint32_t> error       = 0;
    std::atomic<std::uint64_t> size        = 0;
    std::atomic<std::uint64_t> downloaded  = 0;
    windower::downloader::file file;
};

enum class scheme : std::uint16_t
{
    http,
    https
};

struct decoded_url
{
    scheme scheme      = scheme::http;
    std::uint16_t port = 0;
    std::wstring host;
    std::wstring request;
};

class internet_handle
{
public:
    internet_handle(::HINTERNET value) noexcept : m_value{value} {}
    internet_handle(internet_handle const&) = delete;
    internet_handle(internet_handle&&)      = delete;

    ~internet_handle() noexcept
    {
        if (m_value && !::WinHttpCloseHandle(m_value))
        {
            windower::fail_fast();
        }
    }

    internet_handle& operator=(internet_handle const&) = delete;
    internet_handle& operator=(internet_handle&&) = delete;

    explicit operator bool() noexcept { return m_value; }

    operator ::HINTERNET() noexcept { return m_value; }

private:
    ::HINTERNET m_value;
};

std::optional<decoded_url> decode_url(std::u8string_view utf8_url)
{
    using namespace windower;

    auto url  = to_wstring(utf8_url);
    auto size = std::min(url.find(L'#'), url.size());

    ::URL_COMPONENTS components  = {};
    components.dwStructSize      = sizeof components;
    components.dwSchemeLength    = 1;
    components.dwHostNameLength  = 1;
    components.dwUrlPathLength   = 1;
    components.dwExtraInfoLength = 1;
    if (::WinHttpCrackUrl(url.c_str(), size, 0, &components))
    {
        decoded_url result;
        switch (components.nScheme)
        {
        default: return std::nullopt;
        case INTERNET_SCHEME_HTTP: result.scheme = scheme::http; break;
        case INTERNET_SCHEME_HTTPS: result.scheme = scheme::https; break;
        }
        result.port = components.nPort;
        result.host =
            std::wstring{components.lpszHostName, components.dwHostNameLength};
        result.request = std::wstring{
            components.lpszUrlPath,
            size - (components.lpszUrlPath - url.data())};
        return result;
    }

    return std::nullopt;
}

void add_header(
    ::HINTERNET request, wchar_t const* header_name, ::SYSTEMTIME const& time)
{
    auto time_buffer =
        std::array<wchar_t, WINHTTP_TIME_FORMAT_BUFSIZE / sizeof(wchar_t)>{};
    ::WinHttpTimeFromSystemTime(&time, time_buffer.data());
    std::wstring header{header_name};
    header.append(1, L':');
    header.append(1, L' ');
    header.append(time_buffer.data());
    ::WinHttpAddRequestHeaders(
        request, header.c_str(), header.size(), WINHTTP_ADDREQ_FLAG_ADD);
}

void add_header(
    ::HINTERNET request, wchar_t const* header_name, ::FILETIME file_time)
{
    ::SYSTEMTIME system_time;
    ::FileTimeToSystemTime(&file_time, &system_time);
    add_header(request, header_name, system_time);
}

void add_header(
    ::HINTERNET request, wchar_t const* header_name,
    std::chrono::system_clock::time_point time)
{
    using filetime_duration =
        std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>;
    // Assumes std::chrono::system_clock uses Unix epoch.
    constexpr std::chrono::duration<std::int64_t> filetime_epoch{
        -11644473600LL};

    auto duration =
        std::chrono::duration_cast<filetime_duration>(time.time_since_epoch());
    auto const adjusted = duration - filetime_epoch;
    auto count          = adjusted.count();

    auto const file_time = ::FILETIME{
        .dwLowDateTime  = gsl::narrow_cast<::DWORD>(count & 0xFF),
        .dwHighDateTime = gsl::narrow_cast<::DWORD>(count >> 32 & 0xFF),
    };

    add_header(request, header_name, file_time);
}

void download_file(::HINTERNET session, file_data& data)
{
    using namespace windower;

    auto url = decode_url(data.file.url);
    if (!url)
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    internet_handle connection =
        ::WinHttpConnect(session, url->host.c_str(), url->port, 0);
    if (!connection)
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    internet_handle request = ::WinHttpOpenRequest(
        connection, L"GET", url->request.c_str(), nullptr, WINHTTP_NO_REFERER,
        accept_types.data(),
        url->scheme == scheme::https ? WINHTTP_FLAG_SECURE : 0);
    if (!request)
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    if (data.file.last_modified)
    {
        add_header(request, L"If-Modified-Since", *data.file.last_modified);
    }
    else if (
        handle file = ::CreateFileW(
            data.file.path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr))
    {
        ::FILETIME last_modified;
        if (::GetFileTime(file, nullptr, nullptr, &last_modified))
        {
            add_header(request, L"If-Modified-Since", last_modified);
        }
    }

    data.status = downloader::status::downloading;

    if (!::WinHttpSendRequest(
            request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA,
            0, 0, 0))
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    if (!::WinHttpReceiveResponse(request, nullptr))
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    std::uint32_t status_code  = 0;
    ::DWORD sizeof_status_code = sizeof status_code;
    if (!::WinHttpQueryHeaders(
            request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &sizeof_status_code,
            WINHTTP_NO_HEADER_INDEX))
    {
        data.status = downloader::status::error;
        data.error  = ::GetLastError();
        return;
    }

    data.http_status = gsl::narrow<std::uint16_t>(status_code);
    if (status_code == 304)
    {
        data.status = downloader::status::skipped;
        return;
    }

    if (status_code >= 400)
    {
        data.status = downloader::status::error;
        return;
    }

    auto size           = std::uint64_t{};
    ::DWORD sizeof_size = sizeof size;
    if (!::WinHttpQueryHeaders(
            request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER64,
            WINHTTP_HEADER_NAME_BY_INDEX, &size, &sizeof_size, 0))
    {
        size = 0;
    }
    data.size = size;

    {
        std::filesystem::create_directories(data.file.path.parent_path());
        handle file = ::CreateFileW(
            data.file.path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!file)
        {
            data.status = downloader::status::error;
            data.error  = ::GetLastError();
            return;
        }

        auto buffer     = std::array<std::uint8_t, 8192>{};
        auto bytes_read = ::DWORD{};
        do
        {
            if (!::WinHttpReadData(
                    request, buffer.data(), buffer.size(), &bytes_read))
            {
                data.status = downloader::status::error;
                data.error  = ::GetLastError();
                return;
            }

            auto bytes_written = ::DWORD{};
            if (!::WriteFile(
                    file, buffer.data(), bytes_read, &bytes_written, nullptr) ||
                bytes_written != bytes_read)
            {
                data.status = downloader::status::error;
                data.error  = ::GetLastError();
                return;
            }

            data.downloaded += bytes_read;
        }
        while (bytes_read != 0);

        auto time           = ::SYSTEMTIME{};
        ::DWORD sizeof_time = sizeof time;
        if (::WinHttpQueryHeaders(
                request,
                WINHTTP_QUERY_LAST_MODIFIED | WINHTTP_QUERY_FLAG_SYSTEMTIME,
                WINHTTP_HEADER_NAME_BY_INDEX, &time, &sizeof_time, 0))
        {
            ::FILETIME file_time;
            if (::SystemTimeToFileTime(&time, &file_time))
            {
                ::SetFileTime(file, nullptr, nullptr, &file_time);
            }
        }
    }

    data.status = downloader::status::complete;
}
}

struct windower::downloader::result::impl : file_data
{
    impl(windower::downloader::file const& file) : file_data{file} {}
};

windower::downloader::result::result(windower::downloader::file const& file) :
    m_impl{std::make_shared<impl>(file)}
{}

windower::downloader::file const&
windower::downloader::result::file() const noexcept
{
    return m_impl->file;
}

windower::downloader::status
windower::downloader::result::status() const noexcept
{
    return m_impl->status;
}

std::uint16_t windower::downloader::result::http_status() const noexcept
{
    return m_impl->http_status;
}

bool windower::downloader::result::is_http_error() const noexcept
{
    return m_impl->http_status >= 400;
}

std::error_code windower::downloader::result::error_code() const noexcept
{
    return {int(m_impl->error), std::system_category()};
}

windower::downloader::result::operator bool() const noexcept
{
    return m_impl->status >= status::complete;
}

struct windower::downloader::job::impl
{
    impl(std::vector<result>&& files) noexcept : files{std::move(files)} {}

    std::vector<result> const files;
    std::mutex mutex;
    std::atomic<bool> complete                      = false;
    std::experimental::coroutine_handle<> resumable = nullptr;
};

windower::downloader::job::awaiter::awaiter(job const& job) noexcept :
    m_job{job}
{}

bool windower::downloader::job::awaiter::await_ready() const noexcept
{
    return m_job.m_impl->complete;
}

void windower::downloader::job::awaiter::await_suspend(
    std::experimental::coroutine_handle<> resumable) const
{
    std::lock_guard<std::mutex> lock{m_job.m_impl->mutex};
    if (m_job.m_impl->complete)
    {
        resumable.resume();
    }
    else
    {
        m_job.m_impl->resumable = resumable;
    }
}

std::vector<windower::downloader::result>
windower::downloader::job::awaiter::await_resume() const
{
    return m_job.m_impl->files;
}

windower::downloader::job::job(std::vector<result>&& files) :
    m_impl{std::make_shared<impl>(std::move(files))}
{}

std::future<std::vector<windower::downloader::result>>
windower::downloader::job::future() const
{
    co_await *this;
    co_return m_impl->files;
}

windower::downloader::downloader() noexcept :
    m_running{true}, m_thread{&downloader::run, this}
{}

windower::downloader::~downloader()
{
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_running = false;
    }
    m_job_added.notify_one();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

windower::downloader::job
windower::downloader::download(std::vector<file> const& files)
{
    std::vector<result> temp;
    for (auto const& f : files)
    {
        temp.push_back(f);
    }

    job job{std::move(temp)};
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_jobs.push(job);
    }
    m_job_added.notify_one();
    return job;
}

windower::downloader::job windower::downloader::download(file f)
{
    return download(std::vector<file>{std::move(f)});
}

windower::downloader::job
windower::downloader::download(std::u8string url, std::u8string path)
{
    return download(std::vector<file>{file{std::move(url), std::move(path)}});
}

std::optional<windower::downloader::job> windower::downloader::get_next_job()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    while (m_jobs.empty() && m_running)
    {
        m_job_added.wait(lock);
    }

    if (m_running)
    {
        job result = m_jobs.front();
        m_jobs.pop();
        return result;
    }
    return std::nullopt;
}

void windower::downloader::run()
{
    while (m_running)
    {
        if (auto next_job = get_next_job())
        {
            std::u8string_view const u8user_agent =
                u8"Windower/" WINDOWER_VERSION_STRING;
            std::wstring user_agent{u8user_agent.begin(), u8user_agent.end()};
            internet_handle session = ::WinHttpOpen(
                user_agent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

            for (auto& file : next_job->m_impl->files)
            {
                download_file(session, *file.m_impl);
            }
            std::lock_guard<std::mutex> lock{next_job->m_impl->mutex};
            next_job->m_impl->complete = true;
            if (auto const resumable = next_job->m_impl->resumable)
            {
                resumable.resume();
            }
        }
    }
}
