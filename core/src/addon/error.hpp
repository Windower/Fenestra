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

#ifndef WINDOWER_ADDON_ERROR_HPP
#define WINDOWER_ADDON_ERROR_HPP
#pragma once

#include "addon/lua.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace windower::lua
{

class source_descriptor
{
public:
    std::u8string type;
    std::u8string value;
    std::size_t line = 0;
};

class variable_descriptor
{
public:
    std::u8string name;
    std::u8string type;
    std::u8string value;
};

class stack_frame
{
public:
    std::u8string name;
    std::u8string type;
    source_descriptor source;
    std::vector<variable_descriptor> locals;
    std::vector<variable_descriptor> upvalues;
};

class error : public std::runtime_error
{
public:
    error(state);
    error(std::string const&, state);
    error(char const*, state);
    error(std::string const&);
    error(char const*);

    bool has_stack_trace() const noexcept;
    std::vector<stack_frame> const& stack_trace() const noexcept;

private:
    std::shared_ptr<std::vector<stack_frame> const> m_stack_trace;
};

}

#endif
