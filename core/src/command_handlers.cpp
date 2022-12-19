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

#include "command_handlers.hpp"

#include "command_manager.hpp"
#include "core.hpp"
#include "errors/command_error.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <limits>

namespace
{
constexpr auto unlimited = std::numeric_limits<std::size_t>::max();

void check_args(
    std::u8string_view command_name, std::vector<std::u8string> const& args,
    std::size_t min, std::size_t max)
{
    using namespace windower;

    auto const count = args.size();
    if (count < min)
    {
        std::u8string message;
        message.append(u8"Too few arguments; expected: ");
        message.append(to_u8string(min));
        message.append(u8", got: ");
        message.append(to_u8string(count));

        throw command_error{message, command_name};
    }
    else if (count > max)
    {
        std::u8string message;
        message.append(u8"Too many arguments; expected: ");
        message.append(to_u8string(min));
        message.append(u8", got: ");
        message.append(to_u8string(count));

        throw command_error{message, command_name};
    }
}

void check_args(
    std::u8string_view command_name, std::vector<std::u8string> const& args,
    std::size_t expected)
{
    check_args(command_name, args, expected, expected);
}

std::future<void> install_impl(std::vector<std::u8string> const& args)
{
    check_args(u8"/install", args, 1, unlimited);
    auto const& core = windower::core::instance();
    auto updated     = co_await core.package_manager->install(args);
    if (core.addon_manager)
    {
        core.addon_manager->reload(updated);
    }
}

std::future<void> update_impl(std::vector<std::u8string> const& args)
{
    check_args(u8"/update", args, 1, unlimited);
    auto const& core = windower::core::instance();
    auto updated     = co_await core.package_manager->update(args);
    if (core.addon_manager)
    {
        core.addon_manager->reload(updated);
    }
}

std::future<void> updateall_impl(std::vector<std::u8string> const& args)
{
    check_args(u8"/updateall", args, 0, 1);
    auto const& core = windower::core::instance();
    auto const force = !args.empty() && gsl::at(args, 0) == u8"force";
    auto updated     = co_await core.package_manager->update_all(force);
    if (core.addon_manager)
    {
        core.addon_manager->reload(updated);
    }
}
};

void windower::command_handlers::install(
    std::vector<std::u8string> const& args, windower::command_source)
{
    install_impl(args);
}

void windower::command_handlers::uninstall(
    std::vector<std::u8string> const& args, windower::command_source)
{
    check_args(u8"/uninstall", args, 1, unlimited);

    auto const& core = core::instance();
    auto packages    = core.package_manager->unload_order(args);
    std::vector<std::shared_ptr<windower::package const>> dependents;
    while (!packages.empty())
    {
        if (std::find(args.begin(), args.end(), packages.back()->name()) ==
            args.end())
        {
            dependents.push_back(packages.back());
        }
        packages.pop_back();
    }

    if (!dependents.empty())
    {
        std::u8string message = u8"Uninstall failed.\n The following ";
        message += dependents.size() == 1 ? u8"package depends on "
                                          : u8"packages depend on ";
        message += args.size() == 1 ? u8"this package:\n"
                                    : u8"one or more of these packages:\n";
        for (auto const& package : dependents)
        {
            message += u8"    ";
            message += package->name();
        }
        throw windower_error{message};
    }

    if (core.addon_manager)
    {
        core.addon_manager->unload(args);
    }

    core.package_manager->uninstall(args);
}

void windower::command_handlers::update(
    std::vector<std::u8string> const& args, windower::command_source)
{
    update_impl(args);
}

void windower::command_handlers::updateall(
    std::vector<std::u8string> const& args, windower::command_source)
{
    updateall_impl(args);
}

void windower::command_handlers::load(
    std::vector<std::u8string> const& args, windower::command_source)
{
    check_args(u8"/load", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->load(args);
    }
    else
    {
        throw command_error{u8"Addon manager is not initialized", u8"/load"};
    }
}

void windower::command_handlers::unload(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unload", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->unload(args);
    }
    else
    {
        throw command_error{u8"Addon manager is not initialized", u8"/unload"};
    }
}

void windower::command_handlers::reload(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reload", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->reload(args);
    }
    else
    {
        throw command_error{u8"Addon manager is not initialized", u8"/reload"};
    }
}

void windower::command_handlers::unloadall(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unloadall", args, 0);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->unload_all();
    }
    else
    {
        throw command_error{
            u8"Addon manager is not initialized", u8"/unloadall"};
    }
}

void windower::command_handlers::reloadall(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reloadall", args, 0);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->reload_all();
    }
    else
    {
        throw command_error{
            u8"Addon manager is not initialized", u8"/reloadall"};
    }
}

void windower::command_handlers::alias(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 6)
    {
        auto parsed = command_manager::get_arguments(args.at(0).substr(7), 1);
        check_args(u8"/alias", parsed, 2);
        std::u8string_view command = gsl::at(parsed, 1);
        auto index                 = std::size_t{};
        auto next_index            = index;
        while (is_whitespace(next_code_point(command, next_index)))
        {
            index = next_index;
        }
        command.remove_prefix(index);
        command_manager::instance().register_alias(gsl::at(parsed, 0), command);
    }
}

void windower::command_handlers::unalias(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unalias", args, 1);
    command_manager::instance().unregister_alias(gsl::at(args, 0));
}

void windower::command_handlers::bind(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 5)
    {
        core::instance().binding_manager.bind(gsl::at(args, 0).substr(6));
    }
}

void windower::command_handlers::unbind(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 7)
    {
        core::instance().binding_manager.unbind(gsl::at(args, 0).substr(8));
    }
}

void windower::command_handlers::listbinds(
    std::vector<std::u8string> const& args, command_source source)
{
    check_args(u8"/listbinds", args, 0);
    auto const& bindings = core::instance().binding_manager.get_binds();
    if (bindings.empty())
    {
        core::output(u8"core", u8"No key bindings registered.", source);
        return;
    }
    core::output(
        u8"core",
        u8"Total bindings: " + windower::to_u8string(bindings.size()));
    for (auto const& binding : bindings)
    {
        core::output(u8"core", binding.first + u8": " + binding.second, source);
    }
}

void windower::command_handlers::exec(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/exec", args, 1);
    core::instance().script_environment.execute(gsl::at(args, 0));
}

void windower::command_handlers::eval(
    std::vector<std::u8string> const& args, command_source)
{
    auto arg = std::u8string_view{args.at(0)};
    if (arg.length() > 5)
    {
        arg.remove_prefix(6);
        core::instance().script_environment.evaluate(arg);
    }
}

void windower::command_handlers::reset(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reset", args, 0);
    core::instance().script_environment.reset();
}

void windower::command_handlers::pkg(
    std::vector<std::u8string> const& args, command_source source)
{
    check_args(u8"/pkg", args, 1, unlimited);
    auto const& core = core::instance();
    if (gsl::at(args, 0) == u8"reload")
    {
        check_args(u8"/pkg", args, 1);
        core.package_manager->reset();
    }
    else if (gsl::at(args, 0) == u8"listsrc")
    {
        check_args(u8"/pkg", args, 1);
        core::output(u8"package manager", u8"listing sources...", source);
        for (auto const& s : core.package_manager->sources())
        {
            windower::core::output(u8"package manager", s, source);
        }
    }
    else if (gsl::at(args, 0) == u8"addsrc")
    {
        check_args(u8"/pkg", args, 2);
        core.package_manager->add_source(gsl::at(args, 1));
    }
    else if (gsl::at(args, 0) == u8"removesrc")
    {
        check_args(u8"/pkg", args, 2);
        core.package_manager->remove_source(gsl::at(args, 1));
    }
    else
    {
        std::u8string message;
        message.append(u8"Unrecognized package manager sub-command \"");
        message.append(gsl::at(args, 0));
        message.append(u8"\"");

        throw command_error{message, u8"/pkg"};
    }
}

void windower::command_handlers::nextwindow(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/nextwindow", args, 0);
    core::instance().ui.activate_next_window();
}

void windower::command_handlers::prevwindow(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/prevwindow", args, 0);
    core::instance().ui.activate_previous_window();
}
