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

#ifndef WINDOWER_CRASH_HANDLER_HPP
#define WINDOWER_CRASH_HANDLER_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace windower
{

enum class dump_type : std::int32_t
{
    basic = 0x00080000,
    extended = 0x00081965,
    full = 0x00021926,
};

class crash_handler
{
public:
    static void initialize();
    static crash_handler& instance();

    static std::filesystem::path
    write_dump(std::filesystem::path const& path, dump_type dump_type);

    crash_handler(crash_handler const&) = delete;
    crash_handler(crash_handler&&) = delete;

    crash_handler& operator=(crash_handler const&) = delete;
    crash_handler& operator=(crash_handler&&) = delete;

    std::filesystem::path const& dump_path() const noexcept;
    void dump_path(std::filesystem::path const& path);

    dump_type default_dump_type() const noexcept;
    void default_dump_type(dump_type dump_type) noexcept;

    std::filesystem::path write_dump() const;
    std::filesystem::path write_dump(dump_type dump_type) const;
    std::filesystem::path write_dump(std::filesystem::path const& path) const;

    void crash(void*) const;

private:
    std::filesystem::path m_path;
    std::filesystem::path m_full_path;
    std::wstring m_full_path_quoted;
    windower::dump_type m_dump_type;

    crash_handler(std::filesystem::path const&);

    ~crash_handler() = default;
};

}

#endif
