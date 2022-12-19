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

#ifndef WINDOWER_COMMAND_HANDLERS_HPP
#define WINDOWER_COMMAND_HANDLERS_HPP

#include "command_manager.hpp"

#include <string>
#include <vector>

namespace windower::command_handlers
{

void install(std::vector<std::u8string> const&, windower::command_source);
void uninstall(std::vector<std::u8string> const&, windower::command_source);
void update(std::vector<std::u8string> const&, windower::command_source);
void updateall(std::vector<std::u8string> const&, windower::command_source);
void load(std::vector<std::u8string> const&, windower::command_source);
void unload(std::vector<std::u8string> const&, windower::command_source);
void reload(std::vector<std::u8string> const&, windower::command_source);
void unloadall(std::vector<std::u8string> const&, windower::command_source);
void reloadall(std::vector<std::u8string> const&, windower::command_source);
void alias(std::vector<std::u8string> const&, windower::command_source);
void unalias(std::vector<std::u8string> const&, windower::command_source);
void bind(std::vector<std::u8string> const&, windower::command_source);
void unbind(std::vector<std::u8string> const&, windower::command_source);
void listbinds(std::vector<std::u8string> const&, windower::command_source);
void exec(std::vector<std::u8string> const&, windower::command_source);
void eval(std::vector<std::u8string> const&, windower::command_source);
void reset(std::vector<std::u8string> const&, windower::command_source);
void pkg(std::vector<std::u8string> const&, windower::command_source);
void nextwindow(std::vector<std::u8string> const&, windower::command_source);
void prevwindow(std::vector<std::u8string> const&, windower::command_source);

};

#endif
