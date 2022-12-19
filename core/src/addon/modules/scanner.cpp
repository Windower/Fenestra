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

#include "addon/modules/scanner.hpp"

#include "../../scanner.hpp"
#include "addon/lua.hpp"
#include "addon/modules/scanner.lua.hpp"

extern "C"
{
    static void* scan_native(
        char8_t const* module_string, std::size_t module_length,
        char8_t const* signature_string, std::size_t signature_length)
    {
        return windower::scan(
            {module_string, module_length},
            windower::signature{{signature_string, signature_length}});
    }
}

int windower::load_scanner_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_scanner_source, u8"core.scanner");

    lua::push(guard, &scan_native);
    lua::call(guard, 1);

    return guard.release();
}
