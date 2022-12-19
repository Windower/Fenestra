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

#include "addon/errors/package_error.hpp"

windower::package_error::package_error(std::u8string_view error_code) :
    package_error{error_code, std::vector<std::u8string>{}}
{}

windower::package_error::package_error(
    std::u8string_view error_code, std::u8string_view package) :
    package_error{
        error_code, std::vector<std::u8string>{std::u8string{package}}}
{}

windower::package_error::package_error(
    std::u8string_view error_code, std::vector<std::u8string> packages) :
    windower_error{error_code},
    m_packages{
        std::make_shared<std::vector<std::u8string>>(std::move(packages))}
{}

std::vector<std::u8string> const&
windower::package_error::packages() const noexcept
{
    return *m_packages;
}
