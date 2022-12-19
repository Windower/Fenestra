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

#ifndef WINDOWER_SETTINGS_HPP
#define WINDOWER_SETTINGS_HPP

#include "enums.hpp"
#include "geometry.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <type_traits>
#include <vector>

namespace windower
{

class settings
{
public:
    bool debug          = false;
    bool developer_mode = false;

    std::u8string display_device_name;

    window_type window_type = window_type::window;

    rectangle display_bounds = rectangle{0, 0, 1280, 720};
    rectangle window_bounds  = rectangle{0, 0, 1280, 720};
    dimension render_size    = dimension{1280, 720};
    dimension ui_size        = dimension{1280, 720};

    bool hardware_mouse = true;

    unsigned int max_sounds        = 32;
    bool play_sound_when_unfocused = false;

    unsigned int mipmapping                 = false;
    bool bump_mapping                       = false;
    bool map_compression                    = false;
    texture_compression texture_compression = texture_compression::uncompressed;
    environment_animation environment_animation = environment_animation::smooth;
    font_type font_type                         = font_type::uncompressed;

    float gamma = 2.2f;

    bool driver_stability = false;
    bool play_intro       = true;

    bool verbose_logging = true;

    std::filesystem::path settings_path;
    std::filesystem::path user_path;
    std::filesystem::path temp_path;

    std::u8string command_line_args;

    void load();
};

}

#endif
