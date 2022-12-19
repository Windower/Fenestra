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

#ifndef WINDOWER_ADDON_PACKAGE_MANAGER_HPP
#define WINDOWER_ADDON_PACKAGE_MANAGER_HPP

#include "errors/windower_error.hpp"

#include <gsl/gsl>

#include <compare>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <vector>

namespace windower
{

struct package_version
{
    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t revision;
    std::uint32_t build;
    std::u8string tag;

    explicit package_version(
        std::uint32_t = 0, std::uint32_t = 0, std::uint32_t = 0,
        std::uint32_t = 0, std::u8string_view = u8"") noexcept;
    explicit package_version(std::u8string_view);
    explicit package_version(std::string_view);

    std::weak_ordering
    operator<=>(package_version const&) const noexcept = default;
};

std::u8string to_u8string(package_version const&);

enum class package_type
{
    library,
    addon,
    service,
};

class package;

class package_dependency
{
public:
    explicit package_dependency(std::u8string_view, bool = true);

    std::u8string const& name() const noexcept;
    bool required() const noexcept;

private:
    std::u8string m_name;
    bool m_required;
};

class package
{
public:
    explicit package(std::filesystem::path root_path, bool can_update);

    std::u8string const& name() const noexcept;
    package_version const& version() const noexcept;
    package_type type() const noexcept;
    std::vector<package_dependency> const& dependencies() const noexcept;
    std::filesystem::path const& path() const noexcept;

    std::ifstream resolve(std::filesystem::path const&) const;
    std::filesystem::path
    absolute_path(std::filesystem::path const& relative_path) const;

    bool can_update() const noexcept;

private:
    std::filesystem::path m_root_path;
    std::u8string m_name;
    package_version m_version;
    package_type m_type;
    std::vector<package_dependency> m_dependencies;
    bool m_can_update;
};

class package_manager
{
public:
    package_manager() noexcept;

    std::vector<std::shared_ptr<windower::package const>>
    installed_packages() const;
    std::vector<std::shared_ptr<windower::package const>>
        installed_packages(package_type) const;

    std::vector<std::shared_ptr<windower::package const>> load_order() const;
    std::vector<std::shared_ptr<windower::package const>>
    load_order(std::vector<std::u8string> const&) const;

    std::vector<std::shared_ptr<windower::package const>> unload_order() const;
    std::vector<std::shared_ptr<windower::package const>>
    unload_order(std::vector<std::u8string> const&) const;

    std::shared_ptr<windower::package const>
        get_package(std::u8string_view) const;

    void reset();

    std::vector<std::u8string> sources();
    std::future<void> add_source(std::u8string_view);
    void remove_source(std::u8string_view);

    std::future<std::vector<std::u8string>>
    install(std::vector<std::u8string> const&);
    std::future<std::vector<std::u8string>>
    update(std::vector<std::u8string> const&);
    std::future<std::vector<std::u8string>> update_all(bool = false);

    void uninstall(std::vector<std::u8string> const&);

private:
    enum class vertex_color
    {
        white,
        gray,
        black
    };

    struct vertex
    {
        std::shared_ptr<package> value;
        mutable vertex_color color = vertex_color::white;

        explicit vertex(package);
    };

    struct source
    {
        std::u8string url;
        std::u8string guid;
        bool built_in = false;
    };

    struct descriptor
    {
        std::u8string name;
        package_version version;
        std::vector<std::u8string> dependencies;

        std::u8string root_url;
        std::vector<std::filesystem::path> files;
    };

    mutable std::mutex m_mutex;
    std::vector<source> m_package_sources;
    std::vector<descriptor> m_available_packages;
    std::filesystem::path m_installed_package_directory;
    std::vector<std::filesystem::path> m_package_override_directories;
    std::map<std::u8string, vertex, std::less<>> m_installed_packages;

    void initialize_source_list();
    void initialize_package_override_directories();

    void save_source_list();

    std::future<void> update_sources(bool);
    std::future<void> update_source(source, bool);
    void reload_sources();

    std::vector<descriptor>
    out_of_date(std::vector<std::u8string> const&) const;
    std::future<std::vector<std::u8string>>
    install_or_update(std::vector<std::u8string>, bool);

    void populate_installed_packages();
    void populate_installed_packages(std::filesystem::path const&, bool);
    void load_source(std::filesystem::path const&, std::u8string const&);

    std::vector<std::shared_ptr<package const>>
    load_order_impl(std::vector<std::u8string> const&) const;
    std::vector<std::shared_ptr<package const>>
    unload_order_impl(std::vector<std::u8string> const&) const;

    void clear(vertex_color = vertex_color::white) const noexcept;
    void topological_sort(
        std::vector<std::shared_ptr<package const>>&, std::u8string const&,
        bool = true) const;
    void reverse_topological_sort(
        std::vector<std::shared_ptr<package const>>&,
        std::u8string const&) const;
    [[noreturn]] void throw_cycle_error(gsl::not_null<package const*>) const;
};

}

#endif
