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

#include "addon/package_manager.hpp"

#include "addon/errors/package_error.hpp"
#include "core.hpp"
#include "downloader.hpp"
#include "utilities/xml.hpp"
#include "utility.hpp"

#include <windows.h>

#include <objbase.h>

#include <gsl/gsl>
#include <pugixml.hpp>

#include <array>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <optional>
#include <ostream>
#include <regex>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace
{

bool check(windower::downloader::result const& result)
{
    using namespace windower;

    if (result)
    {
        return true;
    }

    if (result.is_http_error())
    {
        std::u8string message;
        message.append(u8"HTTP Error ");
        message.append(to_u8string(result.http_status()));
        message.append(1, u8'\n');
        message.append(result.file().url);
        core::error(u8"package manager", message);
    }
    else
    {
        std::u8string message;
        auto temp = result.error_code().message();
        std::copy(temp.begin(), temp.end(), std::back_inserter(message));
        message.append(1, u8'\n');
        message.append(result.file().path.u8string());
        core::error(u8"package manager", message);
    }
    return false;
}

std::optional<std::chrono::system_clock::time_point>
last_modified(std::filesystem::path const& path, bool force)
{
    if (!force)
    {
        try
        {
            auto const last_write_time = std::filesystem::last_write_time(path);
            return std::chrono::clock_cast<std::chrono::system_clock>(
                last_write_time);
        }
        catch (std::filesystem::filesystem_error const&)
        {}
    }
    return std::nullopt;
}

}

windower::package_version::package_version(
    std::uint32_t major, std::uint32_t minor, std::uint32_t revision,
    std::uint32_t build, std::u8string_view tag) noexcept :
    major{major},
    minor{minor}, revision{revision}, build{build}, tag{tag}
{}

windower::package_version::package_version(std::u8string_view value) :
    package_version(to_string_view(value))
{}

windower::package_version::package_version(std::string_view value) :
    minor{}, revision{}, build{}
{
    value.remove_prefix(parse(value, major));
    if (!value.empty() && value.front() != u8'.')
    {
        value.remove_prefix(parse(value, minor));
        if (!value.empty() && value.front() != u8'.')
        {
            value.remove_prefix(parse(value, revision));
            if (!value.empty() && value.front() != u8'.')
            {
                value.remove_prefix(parse(value, build));
            }
        }
    }

    while (!value.empty() && value.front() == u8' ')
    {
        value.remove_prefix(1);
    }

    tag = to_u8string(value);
}

std::u8string windower::to_u8string(package_version const& value)
{
    std::u8string result;
    result.append(to_u8string(value.major)).append(1, u8'.');
    result.append(to_u8string(value.minor)).append(1, u8'.');
    result.append(to_u8string(value.revision)).append(1, u8'.');
    result.append(to_u8string(value.build));
    if (!value.tag.empty())
    {
        result.append(1, u8' ').append(value.tag);
    }
    return result;
}

windower::package_dependency::package_dependency(
    std::u8string_view name, bool required) :
    m_name{name},
    m_required{required}
{}

std::u8string const& windower::package_dependency::name() const noexcept
{
    return m_name;
}

bool windower::package_dependency::required() const noexcept
{
    return m_required;
}

windower::package::package(std::filesystem::path root_path, bool can_update) :
    m_root_path{std::move(root_path)}, m_can_update{can_update}
{
    auto stream = resolve(u8"manifest.xml");
    pugi::xml_document doc;
    check(doc.load(stream), stream);

    auto const package = doc.child("package");
    if (!package)
    {
        throw package_error{u8"PKG:M1"};
    }

    m_name    = to_u8string(package.child_value("name"));
    m_version = package_version{package.child("version").text().as_string()};

    auto const type =
        std::string_view{package.child("type").text().as_string("addon")};
    if (type == "addon")
    {
        m_type = package_type::addon;
    }
    else if (type == "service")
    {
        m_type = package_type::service;
    }
    else if (type == "library")
    {
        m_type = package_type::library;
    }
    else
    {
        throw package_error{u8"PKG:M2"};
    }

    auto const dependencies = package.child("dependencies");
    for (auto const& dependency : dependencies.children("dependency"))
    {
        std::string_view const value = dependency.child_value();
        std::u8string name;
        name.assign(value.begin(), value.end());
        m_dependencies.emplace_back(
            name, !dependency.attribute("optional").as_bool());
    }
}

std::u8string const& windower::package::name() const noexcept { return m_name; }

windower::package_version const& windower::package::version() const noexcept
{
    return m_version;
}

windower::package_type windower::package::type() const noexcept
{
    return m_type;
}

std::vector<windower::package_dependency> const&
windower::package::dependencies() const noexcept
{
    return m_dependencies;
}

std::filesystem::path const& windower::package::path() const noexcept
{
    return m_root_path;
}

std::ifstream
windower::package::resolve(std::filesystem::path const& relative_path) const
{
    auto path = absolute_path(relative_path);
    if (auto stream = std::ifstream{path, std::ios::binary}; stream.is_open())
    {
        return stream;
    }
    throw package_error{u8"PKG:F2"};
}

std::filesystem::path windower::package::absolute_path(
    std::filesystem::path const& relative_path) const
{
    auto const type = std::filesystem::status(m_root_path).type();
    if (type == std::filesystem::file_type::directory)
    {
        return m_root_path / relative_path;
    }
    throw package_error{u8"PKG:F1"};
}

bool windower::package::can_update() const noexcept { return m_can_update; }

windower::package_manager::package_manager() noexcept
{
    m_package_sources.push_back(
        {u8"https://packages.windower.net",
         u8"D103E5FA-AD95-4407-A1E3-2D3E83404E63", true});
    m_package_sources.push_back(
        {u8"https://windower.github.io/Resources",
         u8"0403E868-52FE-4329-A6D5-5AC9745C57B7", true});
    initialize_source_list();
    m_installed_package_directory = settings_path() / u8"packages";
    initialize_package_override_directories();

    reset();
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::installed_packages() const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    std::vector<std::shared_ptr<package const>> result;
    for (auto const& p : m_installed_packages)
    {
        result.push_back(p.second.value);
    }
    return result;
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::installed_packages(package_type type) const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    std::vector<std::shared_ptr<package const>> result;
    for (auto const& p : m_installed_packages)
    {
        if (p.second.value->type() == type)
        {
            result.push_back(p.second.value);
        }
    }
    return result;
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order() const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    std::vector<std::u8string> names;
    for (auto const& p : m_installed_packages)
    {
        names.push_back(p.first);
    }
    return load_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order(
    std::vector<std::u8string> const& names) const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    return load_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order() const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    std::vector<std::u8string> names;
    for (auto const& p : m_installed_packages)
    {
        names.push_back(p.first);
    }
    return unload_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order(
    std::vector<std::u8string> const& names) const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    return unload_order_impl(names);
}

std::shared_ptr<windower::package const>
windower::package_manager::get_package(std::u8string_view name) const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    auto const it = m_installed_packages.find(name);
    if (it == m_installed_packages.end())
    {
        return nullptr;
    }
    return it->second.value;
}

void windower::package_manager::reset()
{
    std::lock_guard<std::mutex> lock{m_mutex};
    m_installed_packages.clear();
    populate_installed_packages();
}

std::vector<std::u8string> windower::package_manager::sources()
{
    std::vector<std::u8string> results;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        for (auto const& src : m_package_sources)
        {
            if (!src.built_in)
            {
                results.push_back(src.url);
            }
        }
    }
    return results;
}

std::future<void> windower::package_manager::add_source(std::u8string_view url)
{
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        auto it = std::find_if(
            m_package_sources.begin(), m_package_sources.end(),
            [&](auto const& s) { return s.url == url; });
        if (it != m_package_sources.end())
        {
            throw package_error{u8"PKG:S1"};
        }
    }

    auto const guid        = guid::generate();
    auto const guid_string = guid.string();
    source src{std::u8string{url}, guid_string, false};
    co_await update_source(src, true);
    m_package_sources.push_back(src);
    reload_sources();
    save_source_list();
}

void windower::package_manager::remove_source(std::u8string_view url)
{
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        auto it = std::remove_if(
            m_package_sources.begin(), m_package_sources.end(),
            [&](auto const& s) { return s.url == url; });
        auto const count = std::distance(it, m_package_sources.end());
        m_package_sources.erase(it, m_package_sources.end());
        if (count == 0)
        {
            throw package_error{u8"PKG:S2"};
        }
    }
    save_source_list();
    reload_sources();
}

std::future<std::vector<std::u8string>>
windower::package_manager::install(std::vector<std::u8string> const& names)
{
    std::vector<std::u8string> temp;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        std::copy_if(
            names.begin(), names.end(), std::back_inserter(temp),
            [=](auto const& n) {
                auto it = std::find_if(
                    m_installed_packages.begin(), m_installed_packages.end(),
                    [&](auto const& p) noexcept { return p.first == n; });
                return it == m_installed_packages.end();
            });
    }
    co_await update_sources(false);
    reload_sources();
    co_return co_await install_or_update(temp, false);
}

std::future<std::vector<std::u8string>>
windower::package_manager::update(std::vector<std::u8string> const& names)
{
    std::vector<std::u8string> temp;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        std::copy_if(
            names.begin(), names.end(), std::back_inserter(temp),
            [=](auto const& n) {
                auto it = std::find_if(
                    m_installed_packages.begin(), m_installed_packages.end(),
                    [&](auto const& p) noexcept { return p.first == n; });
                return it != m_installed_packages.end();
            });
    }
    co_await update_sources(false);
    reload_sources();
    co_return co_await install_or_update(temp, false);
}

std::future<std::vector<std::u8string>>
windower::package_manager::update_all(bool force)
{
    std::vector<std::u8string> names;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        for (auto const& p : m_installed_packages)
        {
            names.push_back(p.first);
        }
    }
    co_await update_sources(force);
    reload_sources();
    co_return co_await install_or_update(names, force);
}

void windower::package_manager::uninstall(
    std::vector<std::u8string> const& names)
{
    namespace fs = std::filesystem;

    std::lock_guard<std::mutex> lock{m_mutex};
    for (auto const& name : names)
    {
        auto const it = m_installed_packages.find(name);
        if (it != m_installed_packages.end())
        {
            fs::remove_all(it->second.value->path());
            m_installed_packages.erase(it);
        }
    }
}

windower::package_manager::vertex::vertex(package value) :
    value{std::make_unique<package>(std::move(value))}
{}

void windower::package_manager::initialize_source_list()
{
    auto xml_path = settings_path() / u8"updates" / u8"source_list";
    std::ifstream stream{xml_path, std::ios::binary};
    if (stream)
    {
        try
        {
            pugi::xml_document doc;
            check(doc.load(stream), stream, xml_path);
            auto const sources = doc.child("source_list");
            std::scoped_lock lock{m_mutex};
            for (auto const& s : sources.children("source"))
            {
                m_package_sources.push_back(source{
                    to_u8string(s.attribute("url").as_string()),
                    to_u8string(s.attribute("guid").as_string()), false});
            }
        }
        catch (windower_error const&)
        {
            core::error(u8"package manager");
        }
    }
}

void windower::package_manager::initialize_package_override_directories()
{
    namespace fs = std::filesystem;

    auto xml_path = settings_path() / u8"overrides.xml";
    std::ifstream stream{xml_path, std::ios::binary};
    if (stream)
    {
        try
        {
            pugi::xml_document doc;
            check(doc.load(stream), stream, xml_path);
            auto const root = doc.child("overrides");
            for (auto const& element : root.children("path"))
            {
                auto path = fs::path{element.child_value()};
                auto const expanded_size =
                    ::ExpandEnvironmentStringsW(path.c_str(), nullptr, 0);
                std::vector<wchar_t> buffer;
                buffer.resize(expanded_size);
                ::ExpandEnvironmentStringsW(
                    path.c_str(), buffer.data(), buffer.size());
                m_package_override_directories.emplace_back(
                    buffer.begin(), --buffer.end());
            }
        }
        catch (windower_error const&)
        {
            core::error(u8"package manager");
        }
    }
}

void windower::package_manager::save_source_list()
{
    pugi::xml_document doc;
    auto sources = doc.append_child("source_list");
    {
        std::scoped_lock lock{m_mutex};
        for (auto const& s : m_package_sources)
        {
            if (!s.built_in)
            {
                auto source = sources.append_child("source");
                source.append_attribute("url").set_value(
                    reinterpret_cast<char const*>(s.url.c_str()));
                source.append_attribute("guid").set_value(
                    reinterpret_cast<char const*>(s.guid.c_str()));
            }
        }
    }
    std::ofstream stream{
        settings_path() / u8"updates" / u8"source_list", std::ios::binary};
    doc.save(stream);
}

std::future<void> windower::package_manager::update_sources(bool force)
{
    namespace fs = std::filesystem;

    auto sources_root = settings_path() / u8"updates";
    auto staging_path = sources_root / u8"staging";
    try
    {
        fs::remove_all(staging_path);
        fs::create_directories(staging_path);
    }
    catch (fs::filesystem_error const&)
    {}

    std::vector<downloader::file> files;
    for (auto source : m_package_sources)
    {
        auto url  = source.url;
        auto path = staging_path / source.guid;
        if (url.empty() || url.back() != u8'/')
        {
            url.append(1, u8'/');
        }
        url.append(u8"packages.xml");
        files.push_back(
            {url, path, last_modified(sources_root / source.guid, force)});
    }

    auto results = co_await core::instance().downloader.download(files);

    std::lock_guard<std::mutex> lock{m_mutex};
    auto it1 = results.begin();
    auto it2 = m_package_sources.begin();
    for (; it1 != results.end() && it2 != m_package_sources.end(); ++it1, ++it2)
    {
        auto path = sources_root / fs::relative(it1->file().path, staging_path);
        if (::check(*it1))
        {
            if (it1->status() == downloader::status::complete)
            {
                try
                {
                    fs::create_directories(path.parent_path());
                    fs::rename(it1->file().path, path);
                }
                catch (fs::filesystem_error const&)
                {
                    core::error(u8"package manager");
                }
            }
        }
    }
}

std::future<void>
windower::package_manager::update_source(source source, bool force)
{
    namespace fs = std::filesystem;

    auto sources_root = settings_path() / u8"updates";
    auto staging_path = sources_root / u8"staging";
    try
    {
        fs::remove_all(staging_path);
        fs::create_directories(staging_path);
    }
    catch (fs::filesystem_error const&)
    {}

    std::vector<downloader::file> files;
    {
        auto url  = source.url;
        auto path = staging_path / source.guid;
        if (url.empty() || url.back() != u8'/')
        {
            url.append(1, u8'/');
        }
        url.append(u8"packages.xml");
        files.push_back(
            {url, path, last_modified(sources_root / source.guid, force)});
    }

    auto results = co_await core::instance().downloader.download(files);

    for (auto it = results.begin(); it != results.end(); ++it)
    {
        auto path = sources_root / fs::relative(it->file().path, staging_path);
        if (::check(*it))
        {
            if (it->status() == downloader::status::complete)
            {
                try
                {
                    fs::create_directories(path.parent_path());
                    fs::rename(it->file().path, path);
                }
                catch (fs::filesystem_error const&)
                {
                    core::error(u8"package manager");
                }
            }
        }
    }
}

void windower::package_manager::reload_sources()
{
    auto sources_root = settings_path() / u8"updates";

    std::lock_guard<std::mutex> lock{m_mutex};
    m_available_packages.clear();
    for (auto const& source : m_package_sources)
    {
        load_source(sources_root / source.guid, source.url);
    }
}

std::vector<windower::package_manager::descriptor>
windower::package_manager::out_of_date(
    std::vector<std::u8string> const& names) const
{
    std::vector<descriptor> results;

    auto unprocessed = std::vector<std::u8string>{names.rbegin(), names.rend()};
    while (!unprocessed.empty())
    {
        auto const name = std::move(unprocessed.back());
        unprocessed.pop_back();

        auto const installed = m_installed_packages.find(name);
        auto const available = std::lower_bound(
            m_available_packages.begin(), m_available_packages.end(), name,
            [](auto const& a, auto const& b) noexcept { return a.name < b; });
        if (available == m_available_packages.end() || available->name != name)
        {
            if (installed == m_installed_packages.end())
            {
                core::error(
                    u8"package manager",
                    u8"could not find requested package or dependency \"" +
                        name + u8"\"");
            }
            continue;
        }

        if (installed == m_installed_packages.end() ||
            installed->second.value->version() < available->version &&
                installed->second.value->can_update())
        {
            results.push_back(*available);
        }

        for (auto const& dependency : available->dependencies)
        {
            unprocessed.push_back(dependency);
        }
    }

    return results;
}

std::future<std::vector<std::u8string>>
windower::package_manager::install_or_update(
    std::vector<std::u8string> names, bool force)
{
    namespace fs = std::filesystem;

    std::vector<std::u8string> modified_packages;

    while (!names.empty())
    {
        std::vector<descriptor> packages;
        {
            std::lock_guard<std::mutex> lock{m_mutex};
            packages = out_of_date(names);
        }

        if (packages.empty())
        {
            co_return std::vector<std::u8string>{};
        }

        auto staging_path = settings_path() / u8"updates" / u8"temp";

        std::vector<downloader::job> jobs;
        for (auto const& p : packages)
        {
            auto root_url = p.root_url;
            if (root_url.empty() || root_url.back() != u8'/')
            {
                root_url.append(1, u8'/');
            }

            std::vector<downloader::file> files;
            for (auto const& file : p.files)
            {
                auto url  = root_url + file.generic_u8string();
                auto path = (staging_path / file).make_preferred();
                fs::create_directories(path.parent_path());
                auto const time =
                    last_modified(m_installed_package_directory / file, force);
                files.push_back({url, path, time});
            }

            jobs.push_back(core::instance().downloader.download(files));
        }

        auto it1 = jobs.begin();
        auto it2 = packages.begin();
        for (; it1 != jobs.end() && it2 != packages.end(); ++it1, ++it2)
        {
            auto results = co_await *it1;
            if (!results.empty())
            {
                if (!std::all_of(
                        results.begin(), results.end(),
                        [](auto const& r) noexcept { return bool(r); }))
                {
                    auto message = u8"a problem occurred while installing \"" +
                                   it2->name + u8"\"";
                    if (windower::core::instance().settings.verbose_logging)
                    {
                        for (auto r : results)
                        {
                            ::check(r);
                        }
                    }
                }
                else
                {
                    std::vector<fs::path> old_files;
                    {
                        auto path =
                            relative(results.front().file().path, staging_path);
                        if (path.has_parent_path())
                        {
                            auto root =
                                m_installed_package_directory / *path.begin();
                            if (fs::exists(root))
                            {
                                old_files.insert(
                                    old_files.end(),
                                    fs::recursive_directory_iterator{root},
                                    fs::recursive_directory_iterator{});
                            }
                        }
                    }

                    std::lock_guard<std::mutex> lock{m_mutex};
                    for (auto r : results)
                    {
                        auto path = m_installed_package_directory /
                                    relative(r.file().path, staging_path);
                        auto it =
                            std::find(old_files.begin(), old_files.end(), path);
                        if (it != old_files.end())
                        {
                            old_files.erase(it);
                        }

                        if (r.status() == downloader::status::complete)
                        {
                            try
                            {
                                fs::create_directories(path.parent_path());
                                fs::rename(r.file().path, path);
                            }
                            catch (fs::filesystem_error const&)
                            {
                                core::error(u8"package manager");
                            }
                        }
                    }

                    for (auto const& f : old_files)
                    {
                        if (!fs::is_directory(f))
                        {
                            fs::remove(f);
                        }
                    }

                    for (auto const& f : old_files)
                    {
                        if (fs::is_directory(f) && fs::is_empty(f))
                        {
                            fs::remove(f);
                        }
                    }
                }
            }
        }

        reset();

        names.clear();
        for (auto const& p : packages)
        {
            modified_packages.push_back(p.name);
            if (auto package = get_package(p.name))
            {
                for (auto const& d : package->dependencies())
                {
                    if (d.required() && !get_package(d.name()))
                    {
                        names.push_back(d.name());
                    }
                }
            }
        }
    }

    co_return modified_packages;
}

void windower::package_manager::populate_installed_packages()
{
    for (auto const& override_path : m_package_override_directories)
    {
        populate_installed_packages(override_path, false);
    }
    populate_installed_packages(m_installed_package_directory, true);
}

void windower::package_manager::populate_installed_packages(
    std::filesystem::path const& directory, bool can_update)
{
    namespace fs = std::filesystem;

    if (fs::exists(directory))
    {
        for (auto const& entry : fs::directory_iterator{directory})
        {
            try
            {
                if (fs::exists(entry.path() / u8"manifest.xml"))
                {
                    vertex v{package{entry.path(), can_update}};
                    m_installed_packages.try_emplace(
                        v.value->name(), std::move(v));
                }
            }
            catch (std::runtime_error const&)
            {
                core::error(u8"package manager");
            }
        }
    }
}

void windower::package_manager::load_source(
    std::filesystem::path const& path, std::u8string const& url)
{
    auto stream = std::ifstream{path, std::ios::binary};
    if (stream)
    {
        pugi::xml_document doc;
        check(doc.load(stream), stream, path);

        auto const packages = doc.child("packages");
        if (!packages)
        {
            core::error(u8"package manager", u8"invalid source manifest");
            return;
        }

        for (auto const& package : packages.children("package"))
        {
            auto name = to_u8string(package.child_value("name"));
            auto it   = std::lower_bound(
                  m_available_packages.begin(), m_available_packages.end(), name,
                  [](auto const& a, auto const& b) noexcept {
                    return a.name < b;
                });
            if (it == m_available_packages.end() || it->name != name)
            {
                auto& d   = *m_available_packages.emplace(it);
                d.name    = std::move(name);
                d.version = package_version{
                    package.child("version").text().as_string()};
                if (auto const dependencies = package.child("dependencies"))
                {
                    for (auto const& dependency :
                         dependencies.children("dependency"))
                    {
                        d.dependencies.push_back(
                            to_u8string(dependency.text().as_string()));
                    }
                }
                d.root_url = to_u8string(package.child("url").text().as_string(
                    reinterpret_cast<char const*>(url.c_str())));
                if (auto const files = package.child("files"))
                {
                    for (auto const& file : files.children("file"))
                    {
                        d.files.emplace_back(
                            to_u8string(file.text().as_string()));
                    }
                }
            }
        }
    }
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order_impl(
    std::vector<std::u8string> const& names) const
{
    std::vector<std::shared_ptr<package const>> results;

    if (!names.empty())
    {
        clear();
        for (auto const& name : names)
        {
            auto const it = m_installed_packages.find(name);
            if (it == m_installed_packages.end())
            {
                throw package_error{u8"PKG:P1", name};
            }
            topological_sort(results, name);
        }
    }

    return results;
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order_impl(
    std::vector<std::u8string> const& names) const
{
    std::vector<std::shared_ptr<package const>> results;

    if (!names.empty())
    {
        clear();
        for (auto const& name : names)
        {
            reverse_topological_sort(results, name);
        }
    }

    return results;
}

void windower::package_manager::clear(vertex_color color) const noexcept
{
    for (auto& p : m_installed_packages)
    {
        p.second.color = color;
    }
}

void windower::package_manager::topological_sort(
    std::vector<std::shared_ptr<package const>>& results,
    std::u8string const& name, bool required) const
{
    auto const it = m_installed_packages.find(name);
    if (it == m_installed_packages.end())
    {
        if (required)
        {
            throw package_error{u8"PKG:P2", name};
        }
    }
    else
    {
        auto const color = it->second.color;
        if (color == vertex_color::gray && required)
        {
            throw_cycle_error(it->second.value.get());
        }
        else if (color == vertex_color::white)
        {
            it->second.color = vertex_color::gray;
            for (auto const& d : it->second.value->dependencies())
            {
                topological_sort(results, d.name(), d.required());
            }
            it->second.color = vertex_color::black;
            results.push_back(it->second.value);
        }
    }
}

void windower::package_manager::reverse_topological_sort(
    std::vector<std::shared_ptr<package const>>& results,
    std::u8string const& name) const
{
    auto const package_it = m_installed_packages.find(name);
    if (package_it != m_installed_packages.end())
    {
        for (auto const& vertex : m_installed_packages)
        {
            auto const color = vertex.second.color;
            if (color == vertex_color::white)
            {
                auto& dependencies = vertex.second.value->dependencies();
                auto it            = std::find_if(
                               dependencies.begin(), dependencies.end(),
                               [&](auto const& d) noexcept { return d.name() == name; });
                if (it != dependencies.end())
                {
                    vertex.second.color = vertex_color::gray;
                    reverse_topological_sort(
                        results, vertex.second.value->name());
                    vertex.second.color = vertex_color::black;
                }
            }
        }
        results.push_back(package_it->second.value);
    }
}

void windower::package_manager::throw_cycle_error(
    gsl::not_null<package const*> start) const
{
    std::vector<std::u8string> packages;
    auto p = start;
    do
    {
        packages.push_back(p->name());
        for (auto const& d : p->dependencies())
        {
            auto const it = m_installed_packages.find(d.name());
            if (it != m_installed_packages.end() &&
                it->second.color == vertex_color::gray)
            {
                p = it->second.value.get();
                break;
            }
        }
    }
    while (p != start);
    throw package_error{u8"PKG:P3", std::move(packages)};
}
