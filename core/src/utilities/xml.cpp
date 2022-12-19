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

#include "utilities/xml.hpp"

#include "errors/xml_error.hpp"

#include <pugixml.hpp>

#include <cstddef>
#include <fstream>
#include <limits>

void windower::check(
    pugi::xml_parse_result const& result, std::ifstream& stream,
    std::filesystem::path const& path)
{
    if (result)
    {
        return;
    }

    std::u8string description;
    if (auto ptr = result.description())
    {
        auto const size = std::char_traits<char>::length(ptr);
        description.reserve(size);
        std::copy_n(ptr, size, std::back_inserter(description));
    }

    stream.seekg(0, std::ios::end);
    if (result.encoding != pugi::encoding_utf8 || result.offset < 0 ||
        stream.tellg() > std::numeric_limits<decltype(result.offset)>::max())
    {
        throw xml_error{description};
    }

    auto line     = std::size_t{};
    auto line_pos = std::size_t{};

    stream.seekg(0, std::ios::beg);
    {
        std::istreambuf_iterator<char> it{stream};
        std::istreambuf_iterator<char> const end{};
        decltype(result.offset) read_count = 0;
        for (; it != end && read_count != result.offset; ++it, ++read_count)
        {
            if (*it == u8'\n')
            {
                ++line;
                line_pos = read_count;
            }
        }
    }
    auto const column = result.offset - line_pos;

    stream.seekg(line_pos, std::ios::beg);
    std::u8string line_source;
    {
        std::istreambuf_iterator<char> it{stream};
        std::istreambuf_iterator<char> const end{};
        for (; it != end && *it != u8'\n'; ++it)
        {
            line_source.push_back(*it);
        }
    }

    throw xml_error{description, path, line_source, line, column};
}
