--[[
Copyright © Windower Dev Team

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"),to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

-- LuaFormatter off
local -- params
    instance,
    get_context_ptr,
    direct_to_screen_ptr,
    make_id_ptr,
    get_system_color_ptr,
    begin_window_ptr,
    end_window_ptr,
    begin_scroll_panel_ptr,
    end_scroll_panel_ptr,
    begin_scope_ptr,
    end_scope_ptr,
    set_enabled_ptr,
    set_bounds_ptr,
    button_ptr,
    check_ptr,
    color_picker_ptr,
    edit_ptr,
    image_button_ptr,
    label_ptr,
    link_ptr,
    progress_ptr,
    radio_ptr,
    slider_ptr,
    rectangle_ptr,
    rectangle_patch_ptr,
    rectangle_nine_patch_ptr = ...
-- LuaFormatter on

local bit = require('bit')
local ffi = require('ffi')
local math = require('math')
local table = require('table')

local ffi_string = ffi.string

local context_t = ffi.typeof('struct {}')

local vector_t = ffi.typeof([[struct {
    float x;
    float y;
}]])

local dimension_t = ffi.typeof([[struct {
    float width;
    float height;
}]])

local rectangle_t = ffi.typeof([[struct {
    float x0;
    float y0;
    float x1;
    float y1;
}]])

local thickness_t = ffi.typeof([[struct {
    float left;
    float top;
    float right;
    float bottom;
}]])

local patch_t = ffi.typeof([[struct {
    $ texture_size;
    $ bounds;
    $ overdraw;
}]], dimension_t, rectangle_t, thickness_t)

local nine_patch_t = ffi.typeof([[struct {
    $ texture_size;
    $ bounds;
    $ slice;
    $ overdraw;
}]], dimension_t, rectangle_t, thickness_t, thickness_t)

---@class layer_t
---@field screen number
---@field world number
local layer_t = ffi.new([[struct {
    static uint8_t const screen = 1;
    static uint8_t const world  = 2;
}]])
local layer_names = {[1] = 'screen', [2] = 'world'}

---@class window_style_t
---@field standard number
---@field tooltip number
---@field chromeless number
local window_style_t = ffi.new([[struct {
    static uint8_t const standard   = 0;
    static uint8_t const tooltip    = 1;
    static uint8_t const chromeless = 2;
}]])
local window_style_names = {
    [0] = 'standard',
    [1] = 'tooltip',
    [2] = 'chromeless'
}

---@class window_flags_t
---@field hidden number
---@field movable number
---@field resizable number
---@field closeable number
---@field layout_enabled number
---@field click_through number
local window_flags_t = ffi.new([[struct {
    static uint8_t const hidden         = 1 << 0;
    static uint8_t const movable        = 1 << 1;
    static uint8_t const resizable      = 1 << 2;
    static uint8_t const closeable      = 1 << 3;
    static uint8_t const layout_enabled = 1 << 4;
    static uint8_t const click_through  = 1 << 5;
}]])

local window_state_t = ffi.typeof([[struct {
    float depth;
    char const* title_data;
    size_t title_size;
    $ bounds;
    $ min_size;
    $ max_size;
    float zoom_factor;
    int32_t color;
    uint8_t layer;
    uint8_t style;
    uint8_t flags;
}]], rectangle_t, dimension_t, dimension_t)

---@class scroll_bar_visibility_t
---@field hidden number
---@field visible number
---@field automatic number
local scroll_bar_visibility_t = ffi.new([[struct {
    static uint8_t const hidden    = 0;
    static uint8_t const visible   = 1;
    static uint8_t const automatic = 2;
}]])
local scroll_bar_visibility_names = {
    [0] = 'hidden',
    [1] = 'visible',
    [2] = 'automatic'
}

local scroll_panel_state_t = ffi.typeof([[struct {
    $ canvas_size;
    uint8_t visibility_horizontal;
    uint8_t visibility_vertical;
    float line_height;
    $ offset;
}]], dimension_t, vector_t)

local button_state_t = ffi.typeof([[struct {
    bool hot;
    bool active;
    bool pressed;
    bool clicked;
    $ drag_offset;
    $ drag_position;
    uint64_t repeat_count;
    uint8_t button;
}]], vector_t, vector_t)

local image_button_descriptor_t = ffi.typeof([[struct {
    char const* image_data;
    size_t image_size;
    $ normal;
    $ hot;
    $ active;
    $ disabled;
    uint8_t cursor;
}]], patch_t, patch_t, patch_t, patch_t)

local progress_entry_t = ffi.typeof([[struct {
    float value;
    float max;
    int32_t color;
}]])
local progress_entry_array_t = ffi.typeof('$[?]', progress_entry_t)

local edit_state_t = ffi.typeof([[struct {
    char const* text_data;
    size_t text_size;
    bool text_changed;
}]])

---@class system_cursor_t
---@field normal number
---@field hot number
---@field north number
---@field north_east number
---@field east number
---@field south_east number
---@field south number
---@field south_west number
---@field west number
---@field north_west number
---@field north_alt number
---@field north_east_alt number
---@field east_alt number
---@field south_east_alt number
---@field south_alt number
---@field south_west_alt number
---@field west_alt number
---@field north_west_alt number
local system_cursor_t = ffi.new([[struct {
    static uint8_t const normal         = 0;
    static uint8_t const hot            = 1;
    static uint8_t const north          = 2;
    static uint8_t const north_east     = 3;
    static uint8_t const east           = 4;
    static uint8_t const south_east     = 5;
    static uint8_t const south          = 6;
    static uint8_t const south_west     = 7;
    static uint8_t const west           = 8;
    static uint8_t const north_west     = 9;
    static uint8_t const north_alt      = 10;
    static uint8_t const north_east_alt = 11;
    static uint8_t const east_alt       = 12;
    static uint8_t const south_east_alt = 13;
    static uint8_t const south_alt      = 14;
    static uint8_t const south_west_alt = 15;
    static uint8_t const west_alt       = 16;
    static uint8_t const north_west_alt = 17;
}]])
local system_cursor_names = {
    [0] = 'normal',
    [1] = 'hot',
    [2] = 'north',
    [3] = 'north_east',
    [4] = 'east',
    [5] = 'south_east',
    [6] = 'south',
    [7] = 'south_west',
    [8] = 'west',
    [9] = 'north_west',
    [10] = 'north_alt',
    [11] = 'north_east_alt',
    [12] = 'east_alt',
    [13] = 'south_east_alt',
    [14] = 'south_alt',
    [15] = 'south_west_alt',
    [16] = 'west_alt',
    [17] = 'north_west_alt'
}

local direction_t = ffi.new([[struct {
    static uint8_t const left_to_right = 0;
    static uint8_t const right_to_left = 1;
    static uint8_t const bottom_to_top = 2;
    static uint8_t const top_to_bottom = 3;
}]])
local direction_mames = {
    [0] = 'left_to_right',
    [1] = 'right_to_left',
    [2] = 'bottom_to_top',
    [3] = 'top_to_bottom'
}

-- LuaFormatter off
local get_context_t = ffi.typeof(
    '$*(*)()',
    context_t)
local direct_to_screen_t = ffi.typeof(
    'bool(*)($ const&)',
    context_t)
local make_id_t = ffi.typeof(
    'uint64_t(*)($ const&,void const*,uint32_t)',
    context_t)
local get_system_color_t = ffi.typeof(
    'int32_t(*)($ const&,uint8_t)',
    context_t)
local begin_window_t = ffi.typeof(
    'bool(*)($&,$&)',
    context_t, window_state_t)
local end_window_t = ffi.typeof(
    'void(*)($&)',
    context_t)
local begin_scroll_panel_t = ffi.typeof(
    'bool(*)($&,uint64_t,$&)',
    context_t, scroll_panel_state_t)
local end_scroll_panel_t = ffi.typeof(
    'void(*)($&)',
    context_t)
local begin_scope_t = ffi.typeof(
    'void(*)($&)',
    context_t)
local end_scope_t = ffi.typeof(
    'void(*)($&)',
    context_t)
local set_enabled_t = ffi.typeof(
    'void(*)($&,bool)',
    context_t)
local set_bounds_t = ffi.typeof(
    'void(*)($&,float,float,float,float)',
    context_t)

local button_t = ffi.typeof(
    '$(*)($&,uint64_t,char const*,size_t,bool)',
    button_state_t, context_t)
local check_t = ffi.typeof(
    '$(*)($&,uint64_t,char const*,size_t,int8_t)',
    button_state_t, context_t)
local color_picker_t = ffi.typeof(
    'int32_t(*)($&,uint64_t,int32_t,bool)',
    context_t)
local edit_t = ffi.typeof(
    'void(*)($&,uint64_t,$&,char const*,size_t)',
    context_t, edit_state_t)
local image_button_t = ffi.typeof(
    '$(*)($&,uint64_t,$ const&)',
    button_state_t, context_t, image_button_descriptor_t)
local label_t = ffi.typeof(
    'void(*)($&,char const*,size_t)',
    context_t)
local link_t = ffi.typeof(
    '$(*)($&,uint64_t,char const*,size_t)',
    button_state_t, context_t)
local progress_t = ffi.typeof(
    'void(*)($&,$ const*,size_t,uint8_t)',
    context_t, progress_entry_t)
local radio_t = ffi.typeof(
    '$(*)($&,uint64_t,char const*,size_t,bool)',
    button_state_t, context_t)
local slider_t = ffi.typeof(
    'float(*)($&,uint64_t,float,float,float,int32_t,uint8_t)',
    context_t)

local rectangle_flat_t = ffi.typeof(
    'void(*)($&,$ const&,int32_t)',
    context_t, rectangle_t)
local rectangle_patch_t = ffi.typeof(
    'float(*)($&,$ const&,void const*,size_t,$ const&,int32_t)',
    context_t, rectangle_t, patch_t)
local rectangle_nine_patch_t = ffi.typeof(
    'float(*)($&,$ const&,void const*,size_t,$ const&,int32_t)',
    context_t, rectangle_t, nine_patch_t)
-- LuaFormatter on

local get_context = get_context_t(get_context_ptr)
local direct_to_screen = direct_to_screen_t(direct_to_screen_ptr)
local make_id = make_id_t(make_id_ptr)
local get_system_color = get_system_color_t(get_system_color_ptr)
local begin_window = begin_window_t(begin_window_ptr)
local end_window = end_window_t(end_window_ptr)
local begin_scroll_panel = begin_scroll_panel_t(begin_scroll_panel_ptr)
local end_scroll_panel = end_scroll_panel_t(end_scroll_panel_ptr)
local begin_scope = begin_scope_t(begin_scope_ptr)
local end_scope = end_scope_t(end_scope_ptr)
local set_enabled = set_enabled_t(set_enabled_ptr)
local set_bounds = set_bounds_t(set_bounds_ptr)

local button = button_t(button_ptr)
local check = check_t(check_ptr)
local color_picker = color_picker_t(color_picker_ptr)
local edit = edit_t(edit_ptr)
local image_button = image_button_t(image_button_ptr)
local label = label_t(label_ptr)
local link = link_t(link_ptr)
local progress = progress_t(progress_ptr)
local radio = radio_t(radio_ptr)
local slider = slider_t(slider_ptr)

local rectangle_flat = rectangle_flat_t(rectangle_ptr)
local rectangle_patch = rectangle_patch_t(rectangle_patch_ptr)
local rectangle_nine_patch = rectangle_nine_patch_t(rectangle_nine_patch_ptr)

local ui = {}

ui.system_color = function(index)
    local context = get_context()
    return get_system_color(context, index)
end

-- ui.color
ui.color = {
    -- LuaFormatter off

    -- CSS Colors Level 4
    transparent            = 0x00000000,
    aliceblue              = 0xF0F8FF - 0x1000000,
    antiquewhite           = 0xFAEBD7 - 0x1000000,
    aqua                   = 0x00FFFF - 0x1000000,
    aquamarine             = 0x7FFFD4 - 0x1000000,
    azure                  = 0xF0FFFF - 0x1000000,
    beige                  = 0xF5F5DC - 0x1000000,
    bisque                 = 0xFFE4C4 - 0x1000000,
    black                  = 0x000000 - 0x1000000,
    blanchedalmond         = 0xFFEBCD - 0x1000000,
    blue                   = 0x0000FF - 0x1000000,
    blueviolet             = 0x8A2BE2 - 0x1000000,
    brown                  = 0xA52A2A - 0x1000000,
    burlywood              = 0xDEB887 - 0x1000000,
    cadetblue              = 0x5F9EA0 - 0x1000000,
    chartreuse             = 0x7FFF00 - 0x1000000,
    chocolate              = 0xD2691E - 0x1000000,
    coral                  = 0xFF7F50 - 0x1000000,
    cornflowerblue         = 0x6495ED - 0x1000000,
    cornsilk               = 0xFFF8DC - 0x1000000,
    crimson                = 0xDC143C - 0x1000000,
    cyan                   = 0x00FFFF - 0x1000000,
    darkblue               = 0x00008B - 0x1000000,
    darkcyan               = 0x008B8B - 0x1000000,
    darkgoldenrod          = 0xB8860B - 0x1000000,
    darkgoldenrodyellow    = 0xB8860B - 0x1000000,
    darkgray               = 0xA9A9A9 - 0x1000000,
    darkgrey               = 0xA9A9A9 - 0x1000000,
    darkgreen              = 0x006400 - 0x1000000,
    darkkhaki              = 0xBDB76B - 0x1000000,
    darkmagenta            = 0x8B008B - 0x1000000,
    darkolivegreen         = 0x556B2F - 0x1000000,
    darkorange             = 0xFF8C00 - 0x1000000,
    darkorchid             = 0x9932CC - 0x1000000,
    darkred                = 0x8B0000 - 0x1000000,
    darksalmon             = 0xE9967A - 0x1000000,
    darkseagreen           = 0x8FBC8F - 0x1000000,
    darkslateblue          = 0x483D8B - 0x1000000,
    darkslategray          = 0x2F4F4F - 0x1000000,
    darkslategrey          = 0x2F4F4F - 0x1000000,
    darkturquoise          = 0x00CED1 - 0x1000000,
    darkviolet             = 0x9400D3 - 0x1000000,
    deeppink               = 0xFF1493 - 0x1000000,
    deepskyblue            = 0x00BFFF - 0x1000000,
    dimgray                = 0x696969 - 0x1000000,
    dimgrey                = 0x696969 - 0x1000000,
    dodgerblue             = 0x1E90FF - 0x1000000,
    firebrick              = 0xB22222 - 0x1000000,
    floralwhite            = 0xFFFAF0 - 0x1000000,
    forestgreen            = 0x228B22 - 0x1000000,
    fuchsia                = 0xFF00FF - 0x1000000,
    gainsboro              = 0xDCDCDC - 0x1000000,
    ghostwhite             = 0xF8F8FF - 0x1000000,
    gold                   = 0xFFD700 - 0x1000000,
    goldenrod              = 0xDAA520 - 0x1000000,
    goldenrodyellow        = 0xDAA520 - 0x1000000,
    gray                   = 0x808080 - 0x1000000,
    grey                   = 0x808080 - 0x1000000,
    green                  = 0x008000 - 0x1000000,
    greenyellow            = 0xADFF2F - 0x1000000,
    honeydew               = 0xF0FFF0 - 0x1000000,
    hotpink                = 0xFF69B4 - 0x1000000,
    indianred              = 0xCD5C5C - 0x1000000,
    indigo                 = 0x4B0082 - 0x1000000,
    ivory                  = 0xFFFFF0 - 0x1000000,
    khaki                  = 0xF0E68C - 0x1000000,
    lavender               = 0xE6E6FA - 0x1000000,
    lavenderblush          = 0xFFF0F5 - 0x1000000,
    lawngreen              = 0x7CFC00 - 0x1000000,
    lemonchiffon           = 0xFFFACD - 0x1000000,
    lightblue              = 0xADD8E6 - 0x1000000,
    lightcoral             = 0xF08080 - 0x1000000,
    lightcyan              = 0xE0FFFF - 0x1000000,
    lightgoldenrod         = 0xFAFAD2 - 0x1000000,
    lightgoldenrodyellow   = 0xFAFAD2 - 0x1000000,
    lightgray              = 0xD3D3D3 - 0x1000000,
    lightgrey              = 0xD3D3D3 - 0x1000000,
    lightgreen             = 0x90EE90 - 0x1000000,
    lightpink              = 0xFFB6C1 - 0x1000000,
    lightsalmon            = 0xFFA07A - 0x1000000,
    lightseagreen          = 0x20B2AA - 0x1000000,
    lightskyblue           = 0x87CEFA - 0x1000000,
    lightslategray         = 0x778899 - 0x1000000,
    lightslategrey         = 0x778899 - 0x1000000,
    lightsteelblue         = 0xB0C4DE - 0x1000000,
    lightyellow            = 0xFFFFE0 - 0x1000000,
    lime                   = 0x00FF00 - 0x1000000,
    limegreen              = 0x32CD32 - 0x1000000,
    linen                  = 0xFAF0E6 - 0x1000000,
    magenta                = 0xFF00FF - 0x1000000,
    maroon                 = 0x800000 - 0x1000000,
    mediumaquamarine       = 0x66CDAA - 0x1000000,
    mediumblue             = 0x0000CD - 0x1000000,
    mediumorchid           = 0xBA55D3 - 0x1000000,
    mediumpurple           = 0x9370DB - 0x1000000,
    mediumseagreen         = 0x3CB371 - 0x1000000,
    mediumslateblue        = 0x7B68EE - 0x1000000,
    mediumspringgreen      = 0x00FA9A - 0x1000000,
    mediumturquoise        = 0x48D1CC - 0x1000000,
    mediumvioletred        = 0xC71585 - 0x1000000,
    midnightblue           = 0x191970 - 0x1000000,
    mintcream              = 0xF5FFFA - 0x1000000,
    mistyrose              = 0xFFE4E1 - 0x1000000,
    moccasin               = 0xFFE4B5 - 0x1000000,
    navajowhite            = 0xFFDEAD - 0x1000000,
    navy                   = 0x000080 - 0x1000000,
    oldlace                = 0xFDF5E6 - 0x1000000,
    olive                  = 0x808000 - 0x1000000,
    olivedrab              = 0x6B8E23 - 0x1000000,
    orange                 = 0xFFA500 - 0x1000000,
    orangered              = 0xFF4500 - 0x1000000,
    orchid                 = 0xDA70D6 - 0x1000000,
    palegoldenrod          = 0xEEE8AA - 0x1000000,
    palegoldenrodyellow    = 0xEEE8AA - 0x1000000,
    palegreen              = 0x98FB98 - 0x1000000,
    paleturquoise          = 0xAFEEEE - 0x1000000,
    palevioletred          = 0xDB7093 - 0x1000000,
    papayawhip             = 0xFFEFD5 - 0x1000000,
    peachpuff              = 0xFFDAB9 - 0x1000000,
    peru                   = 0xCD853F - 0x1000000,
    pink                   = 0xFFC0CB - 0x1000000,
    plum                   = 0xDDA0DD - 0x1000000,
    powderblue             = 0xB0E0E6 - 0x1000000,
    purple                 = 0x800080 - 0x1000000,
    rebeccapurple          = 0x663399 - 0x1000000,
    red                    = 0xFF0000 - 0x1000000,
    rosybrown              = 0xBC8F8F - 0x1000000,
    royalblue              = 0x4169E1 - 0x1000000,
    saddlebrown            = 0x8B4513 - 0x1000000,
    salmon                 = 0xFA8072 - 0x1000000,
    sandybrown             = 0xF4A460 - 0x1000000,
    seagreen               = 0x2E8B57 - 0x1000000,
    seashell               = 0xFFF5EE - 0x1000000,
    sienna                 = 0xA0522D - 0x1000000,
    silver                 = 0xC0C0C0 - 0x1000000,
    skyblue                = 0x87CEEB - 0x1000000,
    slateblue              = 0x6A5ACD - 0x1000000,
    slategray              = 0x708090 - 0x1000000,
    slategrey              = 0x708090 - 0x1000000,
    snow                   = 0xFFFAFA - 0x1000000,
    springgreen            = 0x00FF7F - 0x1000000,
    steelblue              = 0x4682B4 - 0x1000000,
    tan                    = 0xD2B48C - 0x1000000,
    teal                   = 0x008080 - 0x1000000,
    thistle                = 0xD8BFD8 - 0x1000000,
    tomato                 = 0xFF6347 - 0x1000000,
    turquoise              = 0x40E0D0 - 0x1000000,
    violet                 = 0xEE82EE - 0x1000000,
    wheat                  = 0xF5DEB3 - 0x1000000,
    white                  = 0xFFFFFF - 0x1000000,
    whitesmoke             = 0xF5F5F5 - 0x1000000,
    yellow                 = 0xFFFF00 - 0x1000000,
    yellowgreen            = 0x9ACD32 - 0x1000000,

    -- System
    system_transparent                         = ui.system_color(0),
    system_white                               = ui.system_color(1),
    system_black                               = ui.system_color(2),
    system_error                               = ui.system_color(3),
    system_layout_active_title                 = ui.system_color(4),
    system_layout_active_title_stroke          = ui.system_color(5),
    system_layout_active_hidden_title          = ui.system_color(6),
    system_layout_active_hidden_title_stroke   = ui.system_color(7),
    system_layout_inactive_title               = ui.system_color(8),
    system_layout_inactive_title_stroke        = ui.system_color(9),
    system_layout_inactive_hidden_title        = ui.system_color(10),
    system_layout_inactive_hidden_title_stroke = ui.system_color(11),
    system_color_picker_highlight              = ui.system_color(12),

    -- Skin
    skin_accent                = ui.system_color(128),
    skin_window_title          = ui.system_color(129),
    skin_window_title_inactive = ui.system_color(130),
    skin_label                 = ui.system_color(131),
    skin_label_disabled        = ui.system_color(132),
    skin_button                = ui.system_color(133),
    skin_button_disabled       = ui.system_color(134),
    skin_link                  = ui.system_color(135),
    skin_link_disabled         = ui.system_color(136),

    -- LuaFormatter on
}

do
    local bor = bit.bor
    local lshift = bit.lshift
    local band = bit.band
    ui.color.fade = function(color, a)
        return bor(lshift(band(a or 255, 0xFF), 24), band(color, 0xFFFFFF))
    end
end

do
    local rol = bit.rol
    local tohex = bit.tohex
    ui.color.tohex = function(color) return '#' .. tohex(rol(color, 8), -8) end
end

do
    local lshift = bit.lshift
    local bor = bit.bor
    local band = bit.band
    ui.color.rgb = function(r, g, b, a)
        return bor(lshift(band(a or 255, 0xFF), 24), lshift(band(r, 0xFF), 16),
                   lshift(band(g, 0xFF), 8), band(b, 0xFF))
    end
end

do
    local rshift = bit.rshift
    local band = bit.band
    ui.color.torgb = function(color)
        return band(rshift(color, 16), 0xFF), band(rshift(color, 8), 0xFF),
               band(color, 0xFF), band(rshift(color, 24), 0xFF)
    end
end

do
    local rgb = ui.color.rgb
    ui.color.hsv = function(h, s, v, a)
        v = v * 255
        local c = v * s
        local m = v - c
        local h_prime = h * 0.016666666666666666

        local r, g, b
        if h_prime < 1 then
            r, g, b = m + c, m + c * h_prime, m
        elseif h_prime < 2 then
            r, g, b = m + c * (2 - h_prime), m + c, m
        elseif h_prime < 3 then
            r, g, b = m, m + c, m + c * (h_prime - 2)
        elseif h_prime < 4 then
            r, g, b = m, m + c * (4 - h_prime), m + c
        elseif h_prime < 5 then
            r, g, b = m + c * (h_prime - 4), m, m + c
        else
            r, g, b = m + c, m, m + c * (6 - h_prime)
        end

        return rgb(r, g, b, a)
    end
end

do
    local torgb = ui.color.torgb
    local max = math.max
    local min = math.min
    ui.color.tohsv = function(color)
        local r, g, b = torgb(color)

        local value = max(r, g, b);
        local min = min(r, g, b);

        local chroma = value - min
        local saturation = value == 0 and 0 or chroma / value

        local hue = 0
        if chroma ~= 0 then
            if value == r then
                hue = 60 * (g - b) / chroma
            elseif value == g then
                hue = 60 * (b - r) / chroma + 120
            else
                hue = 60 * (r - g) / chroma + 240
            end
        end
        hue = hue < 0 and hue + 360 or hue

        return hue, saturation, value / 255
    end
end

local current_context = nil

-- ui.display
do
    local coroutine_schedule = coroutine.schedule
    local coroutine_create = coroutine.create
    local coroutine_resume = coroutine.resume
    local coroutine_yield = coroutine.yield
    local coroutine_status = coroutine.status
    local coroutine_sleep_frame = coroutine.sleep_frame

    ui.display = function(draw)
        coroutine_schedule(function()
            local wrapper = coroutine_create(function()
                while draw() ~= false do coroutine_yield() end
            end)

            while true do
                current_context = get_context()
                if current_context ~= nil then
                    local ok, message = coroutine_resume(wrapper)
                    current_context = nil
                    assert(ok, message)
                    if coroutine_status(wrapper) == 'dead' then
                        break
                    end
                end
                coroutine_sleep_frame()
            end
        end)
    end
end

local native_key = {}
local scope_key = {}

local set_id_scope
local get_id
do
    local bit_tobit = bit.tobit

    local screen = {[scope_key] = {ids = {}, next = 1}}

    local scope = screen
    local ids = rawget(scope, scope_key).ids
    local next = rawget(scope, scope_key).next

    set_id_scope = function(window)
        local previous_scope = scope
        rawget(previous_scope, scope_key).next = next
        scope = window
        local scope_data = rawget(scope, scope_key)
        ids = scope_data.ids
        next = scope_data.next
        return previous_scope
    end

    get_id = function(value, count)
        local result = ids[value]
        if not result then
            result = make_id(current_context, instance, next)
            next = bit_tobit(next + 1)
            ids[value] = result
        end
        return result
    end
end

local widgets = {}
local registered_layouts = {}
local calculate_bounds_key = {}

-- ui.register_layout
local register_layout_internal
do
    local table_unpack = table.unpack

    local layout_mt = {
        __index = widgets,
        __newindex = function() error('cannot modify a read-only object') end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    register_layout_internal = function(name, initialize, calculate_bounds)
        registered_layouts[name] = function(x, y, width, height, ...)
            local layout, draw = initialize({
                x = x,
                y = y,
                w = width,
                h = height,
                [calculate_bounds_key] = calculate_bounds
            }, ...)
            draw(setmetatable(layout, layout_mt))
        end
    end

    ui.register_layout = function(name, initialize, calculate_bounds)
        register_layout_internal(name, function(...)
            local args = {...}
            local arg_count = select('#', ...)
            initialize(table_unpack(args, 1, arg_count - 1))
            return select(1, ...), args[arg_count]
        end, calculate_bounds)
    end
end

-- ui.layout
do
    ui.layout = function(name, x, y, width, height, ...)
        local layout = registered_layouts[name]
        layout(x, y, width, height, ...)
    end
end

-- ui.window
do
    local rawget = rawget

    local get_flag
    local set_flag
    do
        local bit_band = bit.band
        local bit_bnot = bit.bnot
        local bit_bor = bit.bor

        get_flag = function(state, flag)
            return bit_band(state.flags, flag) == flag
        end

        set_flag = function(state, flag, value)
            -- LuaFormatter off
            state.flags = value and
                bit_bor(state.flags, flag) or
                bit_band(state.flags, bit_bnot(flag))
            -- LuaFormatter on
        end
    end

    ui.window = function(state, draw)
        local native_state = rawget(state, native_key)
        local w = native_state.bounds.x1 - native_state.bounds.x0
        local h = native_state.bounds.y1 - native_state.bounds.y0
        if begin_window(current_context, native_state) then
            local previous_scope = set_id_scope(state)
            if native_state.style == window_style_t.standard then
                ui.layout('default', 0, 0, w, h, function(layout)
                    layout:padding(10)
                    draw(layout)
                end)
            elseif native_state.style == window_style_t.tooltip then
                ui.layout('default', 0, 0, w, h, function(layout)
                    layout:padding(2)
                    draw(layout)
                end)
            else
                ui.layout('default', 0, 0, w, h, draw)
            end
            set_id_scope(previous_scope)
            end_window(current_context)
        end
        return not get_flag(state, window_flags_t.hidden)
    end

    local bit_bor = bit.bor
    local math_huge = math.huge
    local title_key = {}
    local window_state_mt = {
        __index = function(t, k)
            if k == 'title' then return rawget(t, title_key) end
            local state = rawget(t, native_key)
            if k == 'layer' then
                return layer_names[state.style]
            elseif k == 'style' then
                return window_style_names[state.layer]
            elseif k == 'position' then
                return {x = state.bounds.x0, y = state.bounds.y0}
            elseif k == 'size' then
                return {
                    width = state.bounds.x1 - state.bounds.x0,
                    height = state.bounds.y1 - state.bounds.y0
                }
            elseif k == 'visible' then
                return not get_flag(state, window_flags_t.hidden)
            elseif k == 'movable' then
                return get_flag(state, window_flags_t.movable)
            elseif k == 'resizable' then
                return get_flag(state, window_flags_t.resizable)
            elseif k == 'closeable' then
                return get_flag(state, window_flags_t.closeable)
            elseif k == 'layout_enabled' then
                return get_flag(state, window_flags_t.layout_enabled)
            elseif k == 'click_through' then
                return get_flag(state, window_flags_t.click_through)
            else
                return state[k]
            end
        end,
        __newindex = function(t, k, v)
            local state = rawget(t, native_key)
            if k == 'title' then
                rawset(t, title_key, v)
                state.title_data = v
                state.title_size = v == nil and 0 or #v
            elseif k == 'layer' then
                state.layer = layer_t[v]
            elseif k == 'style' then
                state.style = window_style_t[v]
            elseif k == 'position' then
                state.bounds.x1 = v.x - state.bounds.x0 + state.bounds.x1
                state.bounds.y1 = v.y - state.bounds.y0 + state.bounds.y1
                state.bounds.x0 = v.x
                state.bounds.y0 = v.y
            elseif k == 'x' then
                state.bounds.x1 = v - state.bounds.x0 + state.bounds.x1
                state.bounds.x0 = v
            elseif k == 'y' then
                state.bounds.y1 = v - state.bounds.y0 + state.bounds.y1
                state.bounds.y0 = v
            elseif k == 'size' then
                state.bounds.x1 = state.bounds.x0 + v.width
                state.bounds.y1 = state.bounds.y0 + v.height
            elseif k == 'width' then
                state.bounds.x1 = state.bounds.x0 + v
            elseif k == 'height' then
                state.bounds.y1 = state.bounds.y0 + v
            elseif k == 'visible' then
                set_flag(state, window_flags_t.hidden, not v)
            elseif k == 'movable' then
                set_flag(state, window_flags_t.movable, v)
            elseif k == 'resizable' then
                set_flag(state, window_flags_t.resizable, v)
            elseif k == 'closeable' then
                set_flag(state, window_flags_t.closeable, v)
            elseif k == 'layout_enabled' then
                set_flag(state, window_flags_t.layout_enabled, v)
            elseif k == 'click_through' then
                set_flag(state, window_flags_t.click_through, v)
            else
                state[k] = v
            end
        end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    ui.window_state = function()
        return setmetatable({
            [native_key] = window_state_t({
                depth = 0,
                title_data = nil,
                title_size = 0,
                bounds = {x0 = 100, y0 = 100, x1 = 300, y1 = 300},
                min_size = {width = 0, height = 0},
                max_size = {width = math_huge, height = math_huge},
                zoom_factor = 1,
                color = 0xFFFFFF,
                layer = layer_t.screen,
                style = window_style_t.standard,
                flags = bit_bor(window_flags_t.movable,
                                window_flags_t.closeable,
                                window_flags_t.layout_enabled)
            }),
            [scope_key] = {ids = {}, next = 1}
        }, window_state_mt)
    end
end

-- ui.screen
do
    ui.screen = function(draw)
        if direct_to_screen(current_context) then
            ui.layout('canvas', 0, 0, 1600, 900, draw)
        end
    end
end

ui.primitive = {}

-- primitive: rectangle
do
    local ui_color_white = ui.color.white
    local ffi_istype = ffi.istype

    local bounds = rectangle_t()

    local basic_patch = patch_t()
    basic_patch.texture_size.width = 1
    basic_patch.texture_size.height = 1
    basic_patch.bounds.x1 = 1
    basic_patch.bounds.y1 = 1

    ui.primitive.patch = function(tex_width, tex_height, x, y, width, height)
        local result = patch_t()
        result.texture_size.width = tex_width
        result.texture_size.height = tex_height
        result.bounds.x0 = x
        result.bounds.y0 = y
        result.bounds.x1 = x + width
        result.bounds.y1 = y + height
        return result
    end

    ui.primitive.nine_patch = function(tex_width, tex_height, x, y, width,
                                       height, left, top, right, bottom)
        local result = nine_patch_t()
        result.texture_size.width = tex_width
        result.texture_size.height = tex_height
        result.bounds.x0 = x
        result.bounds.y0 = y
        result.bounds.x1 = x + width
        result.bounds.y1 = y + height
        result.slice.left = left
        result.slice.top = top
        result.slice.right = right
        result.slice.bottom = bottom
        return result
    end

    local rectangle_impl = function(texture, patch, color)
        if color == nil then color = ui_color_white end

        if texture == nil or texture == '' then
            rectangle_flat(current_context, bounds, color)
        else
            local data = texture
            local size = 0
            if type(texture) == 'string' then size = #texture end

            if patch == nil then
                rectangle_patch(current_context, bounds, data, size,
                                basic_patch, color)
            elseif ffi_istype(patch, patch_t) then
                rectangle_patch(current_context, bounds, data, size, patch,
                                color)
            else
                rectangle_nine_patch(current_context, bounds, data, size, patch,
                                     color)
            end
        end
    end

    ui.primitive.rectangle = function(x, y, width, height, ...)
        bounds.x0 = x
        bounds.y0 = y
        bounds.x1 = x + width
        bounds.y1 = y + height
        local texture, patch, color = ...
        local args = select('#', ...)
        if args == 1 then
            patch = basic_patch
            local arg_type = type(texture)
            if arg_type == 'number' then
                color = texture
                texture = nil
            end
        elseif args == 2 then
            local arg_type = type(patch)
            if arg_type == 'number' then
                color = patch
                patch = basic_patch
            end
        end
        rectangle_impl(texture, patch, color)
    end
end

-- layout: default
do
    local math_max = math.max

    local state_key = {}

    local initialize
    do
        local padding = function(layout, ...)
            local l, t, r, b = ...
            local args = select('#', ...)
            if args < 2 then
                t, r, b = l, l, l
            elseif args < 3 then
                r, b = l, t
            elseif args < 4 then
                b = t
            end
            local state = rawget(layout, state_key)
            state.padding_l = l
            state.padding_t = t
            state.padding_r = r
            state.padding_b = b
            return layout
        end

        local same_line = function(layout)
            rawget(layout, state_key).same_line = true
            return layout
        end

        local space = function(layout, amount)
            if amount == nil then amount = 11 end
            rawget(layout, state_key).space = amount
            return layout
        end

        local indent = function(layout, amount)
            if amount == nil then amount = 16 end
            local state = rawget(layout, state_key)
            state.indent = state.indent + amount
            return layout
        end

        local unindent = function(layout, amount)
            if amount == nil then amount = 16 end
            local state = rawget(layout, state_key)
            state.indent = state.indent - amount
            return layout
        end

        local move = function(layout, x, y)
            local state = rawget(layout, state_key)

            -- not affected by padding:
            state.indent = x
            state.space = 0
            state.same_line = false

            -- affected by padding:
            state.line = y + state.padding_t
            state.cursor_x = x + state.padding_l
            state.cursor_y = y + state.padding_t

            return layout
        end

        local size = function(layout, width, height)
            local state = rawget(layout, state_key)
            state.next_w = width
            state.next_h = height
            return layout
        end

        local width = function(layout, width)
            local state = rawget(layout, state_key)
            state.next_w = width
            return layout
        end

        local height = function(layout, height)
            local state = rawget(layout, state_key)
            state.next_h = height
            return layout
        end

        local fill = function(layout)
            local state = rawget(layout, state_key)

            local x, y
            if state.same_line then
                x = state.cursor_x + state.space
            else
                state.line = state.cursor_y + state.space
                x = 0
            end
            y = state.line
            if x == 0 then x = state.padding_l + state.indent end
            if y == 0 then y = state.padding_t end

            state.next_w = layout.w - state.padding_r - x
            state.next_h = layout.h - state.padding_b - y

            return layout
        end

        initialize = function(layout, draw)
            layout[state_key] = {
                padding_l = 0,
                padding_t = 0,
                padding_r = 0,
                padding_b = 0,
                indent = 0,
                line = 0,
                cursor_x = 0,
                cursor_y = 0,
                space = 0,
                same_line = false
            }
            layout.padding = padding
            layout.same_line = same_line
            layout.space = space
            layout.indent = indent
            layout.unindent = unindent
            layout.move = move
            layout.size = size
            layout.width = width
            layout.height = height
            layout.fill = fill
            return layout, draw
        end
    end

    local calculate_bounds = function(layout, descriptor, ...)
        local state = rawget(layout, state_key)

        local x, y
        if state.same_line then
            x = state.cursor_x + state.space
        else
            state.line = state.cursor_y + state.space
            x = 0
        end
        y = state.line
        if x == 0 then x = state.padding_l + state.indent end
        if y == 0 then y = state.padding_t end

        local w = state.next_w
        local h = state.next_h
        if not w or not h then
            local def_w, def_h = descriptor.default_size(...)
            w = w or def_w
            h = h or def_h
        end
        if not w then w = layout.w - state.padding_r - x end
        if not h then h = layout.h - state.padding_b - y end

        state.cursor_x = x + w
        state.cursor_y = math_max(state.cursor_y, y + h)
        state.same_line = false
        state.space = 7
        state.next_w = nil
        state.next_h = nil

        return x + layout.x, y + layout.y, w, h
    end

    register_layout_internal('default', initialize, calculate_bounds)
end

-- layout: canvas
do
    local state_key = {}

    local initialize
    do
        local bounds = function(layout, x, y, width, height)
            local state = rawget(layout, state_key)
            state.x = x
            state.y = y
            state.w = width
            state.h = height
            return layout
        end

        initialize = function(layout, draw)
            layout[state_key] = {x = 0, y = 0, w = layout.w, h = layout.h}
            layout.bounds = bounds
            return layout, draw
        end
    end

    local calculate_bounds = function(layout, descriptor, ...)
        local state = rawget(layout, state_key)
        return layout.x + state.x, layout.y + state.y, state.w, state.h
    end

    register_layout_internal('canvas', initialize, calculate_bounds)
end

-- layout: stack
do
    local state_key = {}

    local initialize
    do
        local space = function(layout, amount)
            if amount == nil then amount = 11 end
            rawget(layout, state_key).space = amount
            return layout
        end

        local move = function(layout, position)
            local state = rawget(layout, state_key)
            state.cursor = position
            state.space = 0
            return layout
        end

        local size = function(layout, size)
            local state = rawget(layout, state_key)
            state.next_size = size
            return layout
        end

        local fill = function(layout)
            local state = rawget(layout, state_key)
            local direction = state.direction
            local layout_size
            if direction == 'top_to_bottom' or direction == 'bottom_to_top' then
                layout_size = layout.h
            elseif direction == 'left_to_right' or direction == 'right_to_left' then
                layout_size = layout.w
            end
            state.next_size = layout_size - state.cursor - state.space
            return layout
        end

        initialize = function(layout, ...)
            local direction, draw = ...
            if select('#', ...) < 2 then
                direction, draw = 'top_to_bottom', direction
            end
            layout[state_key] = {direction = direction, cursor = 0, space = 0}
            layout.space = space
            layout.move = move
            layout.size = size
            layout.fill = fill
            return layout, draw
        end
    end

    local vertical = function(layout, state, descriptor, ...)
        local x = 0
        local y = state.cursor + state.space
        local w = layout.w
        local h = state.next_size
        if not h then
            local _
            _, h = descriptor.default_size(...)
        end
        if not h then h = layout.h - y end
        state.cursor = y + h
        return x, y, w, h
    end

    local hotizontal = function(layout, state, descriptor, ...)
        local x = state.cursor + state.space
        local y = 0
        local w = state.next_size
        local h = layout.h
        if not w then w = descriptor.default_size(...) end
        if not w then w = layout.w - x end
        state.cursor = x + w
        return x, y, w, h
    end

    local calculate_bounds = function(layout, descriptor, ...)
        local state = layout[state_key]
        local direction = state.direction
        local x, y, w, h
        if direction == 'top_to_bottom' then
            x, y, w, h = vertical(layout, state, descriptor, ...)
        elseif direction == 'bottom_to_top' then
            x, y, w, h = vertical(layout, state, descriptor, ...)
            y = layout.h - y - h
        elseif direction == 'left_to_right' then
            x, y, w, h = hotizontal(layout, state, descriptor, ...)
        elseif direction == 'right_to_left' then
            x, y, w, h = hotizontal(layout, state, descriptor, ...)
            x = layout.w - x - w
        end
        state.space = 7
        state.next_size = nil
        return layout.x + x, layout.y + y, w, h
    end

    register_layout_internal('stack', initialize, calculate_bounds)
end

local read_only
do
    local read_only_mt = {
        __newindex = function() error('cannot modify a read-only object') end,
        __metatable = false
    }
    read_only = function(t) return setmetatable(t, read_only_mt) end
end

local prepare_widget = function(layout, descriptor, ...)
    local calculate_bounds = layout[calculate_bounds_key]
    local x, y, w, h = calculate_bounds(layout, descriptor, ...)
    set_bounds(current_context, x, y, x + w, y + h)
end

-- widget: layout
do
    local descriptor = read_only({
        name = 'layout',
        default_size = function() return nil, nil end
    })

    widgets.layout = function(layout, type, ...)
        local calculate_bounds = layout[calculate_bounds_key]
        local x, y, w, h = calculate_bounds(layout, descriptor, ...)
        ui.layout(type, x, y, w, h, ...)
    end
end

-- widget: scope
do
    widgets.scope = function(layout, draw)
        begin_scope(current_context)
        draw(layout)
        end_scope(current_context)
    end
end

-- widget: enabled
do
    widgets.enabled = function(layout, enabled)
        set_enabled(current_context, enabled)
    end
end

-- widget: button
do
    local descriptor = read_only({
        name = 'button',
        default_size = function() return 73, 21 end
    })

    widgets.button = function(layout, ...)
        local id, text, checked = ...
        if select('#', ...) < 3 then text, checked = id, text end
        id = get_id(id)
        checked = not not checked
        prepare_widget(layout, descriptor, text, checked)
        local state = button(current_context, id, text, #text, checked)
        return state.clicked and state.button == 0, state
    end
end

-- widget: check
do
    local descriptor = read_only({
        name = 'check',
        default_size = function() return nil, 12 end
    })

    widgets.check = function(layout, ...)
        local id, text, checked = ...
        if select('#', ...) < 3 then text, checked = id, text end
        id = get_id(id)
        if checked == nil then
            checked = -1
        elseif checked then
            checked = 1
        else
            checked = 0
        end
        prepare_widget(layout, descriptor, text, checked)
        local state = check(current_context, id, text, #text, checked)
        return state.clicked and state.button == 0, state
    end
end

-- widget: color_picker
do
    local descriptor = read_only({
        name = 'color_picker',
        default_size = function(_, alpha)
            if alpha then
                return 136, 152
            else
                return 136, 136
            end
        end
    })

    widgets.color_picker = function(layout, id, value, ...)
        id = get_id(id)
        local alpha = ...
        if select('#', ...) < 1 then alpha = true end
        alpha = not not alpha
        prepare_widget(layout, descriptor, value, alpha)
        return color_picker(current_context, id, value, alpha)
    end
end

-- widget: edit
do
    local descriptor = read_only({
        name = 'edit',
        default_size = function() return 160, 21 end
    })

    local text_key = {}
    local edit_state_mt = {
        __index = function(t, k)
            if k == 'text' then return rawget(t, text_key) end
            local state = rawget(t, native_key)
            if k == 'text_data' then
                return nil
            elseif k == 'text_size' then
                return nil
            elseif k == 'text_changed' then
                return nil
            else
                return state[k]
            end
        end,
        __newindex = function(t, k, v)
            local state = rawget(t, native_key)
            if k == 'text' then
                rawset(t, text_key, v or '')
                state.text_data = v
                state.text_size = v == nil and 0 or #v
            elseif k == 'text_data' then
            elseif k == 'text_size' then
            elseif k == 'text_changed' then
            else
                state[k] = v
            end
        end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    ui.edit_state = function()
        return setmetatable({
            [text_key] = '',
            [native_key] = edit_state_t({text_data = nil, text_size = 0})
        }, edit_state_mt)
    end

    widgets.edit = function(layout, state)
        prepare_widget(layout, descriptor)
        local id = get_id(state)
        local native = rawget(state, native_key)
        local text = rawget(state, text_key)
        local size = text and #text or 0
        edit(current_context, id, native, text, size)
        if native.text_changed then
            native.text_changed = false
            local text = ffi_string(native.text_data, native.text_size)
            rawset(state, text_key, text)
        end
    end
end

-- widget: image
do
    local descriptor = read_only({
        name = 'image',
        default_size = function() return nil, nil end
    })

    local rectangle = ui.primitive.rectangle

    widgets.image = function(layout, ...)
        local calculate_bounds = layout[calculate_bounds_key]
        local x, y, w, h = calculate_bounds(layout, descriptor)
        rectangle(x, y, w, h, ...)
    end
end

-- widget: image_button
do
    local rawget = rawget

    local image_key = {}
    local read_only_key = {}

    local descriptor = read_only({
        name = 'image_button',
        default_size = function(image_descriptor)
            local native = rawget(image_descriptor, native_key)
            local x0 = native.normal.bounds.x0
            local y0 = native.normal.bounds.y0
            local x1 = native.normal.bounds.x1
            local y1 = native.normal.bounds.y1
            local overdraw_l = native.normal.overdraw.left
            local overdraw_t = native.normal.overdraw.top
            local overdraw_r = native.normal.overdraw.right
            local overdraw_b = native.normal.overdraw.bottom
            return x1 - x0 - overdraw_l - overdraw_r,
                   y1 - y0 - overdraw_t - overdraw_b
        end
    })

    local image_button_descriptor_mt = {
        __index = function(t, k)
            local native = rawget(t, native_key)
            if k == 'size' then
                local x0 = native.normal.bounds.x0
                local y0 = native.normal.bounds.y0
                local x1 = native.normal.bounds.x1
                local y1 = native.normal.bounds.y1
                return {width = x1 - x0, height = y1 - y0}
            elseif k == 'width' then
                local x0 = native.normal.bounds.x0
                local x1 = native.normal.bounds.x1
                return x1 - x0
            elseif k == 'height' then
                local y0 = native.normal.bounds.y0
                local y1 = native.normal.bounds.y1
                return y1 - y0
            elseif k == 'normal' then
                local x = native.normal.bounds.x0
                local y = native.normal.bounds.y0
                return {x = x, y = y}
            elseif k == 'hot' then
                local x = native.hot.bounds.x0
                local y = native.hot.bounds.y0
                return {x = x, y = y}
            elseif k == 'active' then
                local x = native.active.bounds.x0
                local y = native.active.bounds.y0
                return {x = x, y = y}
            elseif k == 'disabled' then
                local x = native.disabled.bounds.x0
                local y = native.disabled.bounds.y0
                return {x = x, y = y}
            elseif k == 'overdraw' then
                local left = native.normal.overdraw.left
                local top = native.normal.overdraw.top
                local right = native.normal.overdraw.right
                local bottom = native.normal.overdraw.bottom
                return {left = left, top = top, right = right, bottom = bottom}
            elseif k == 'cursor' then
                return system_cursor_names[native.cursor]
            end
        end,
        __newindex = function(t, k, v)
            if rawget(t, read_only_key) then
                error('cannot modify a read-only object')
            end

            local native = rawget(t, native_key)
            if k == 'size' then
                native.normal.bounds.x1 = native.normal.bounds.x0 + v.width
                native.normal.bounds.y1 = native.normal.bounds.y0 + v.height
                native.hot.bounds.x1 = native.hot.bounds.x0 + v.width
                native.hot.bounds.y1 = native.hot.bounds.y0 + v.height
                native.active.bounds.x1 = native.active.bounds.x0 + v.width
                native.active.bounds.y1 = native.active.bounds.y0 + v.height
                native.disabled.bounds.x1 = native.disabled.bounds.x0 + v.width
                native.disabled.bounds.y1 = native.disabled.bounds.y0 + v.height
            elseif k == 'width' then
                native.normal.bounds.x1 = native.normal.bounds.x0 + v
                native.hot.bounds.x1 = native.hot.bounds.x0 + v
                native.active.bounds.x1 = native.active.bounds.x0 + v
                native.disabled.bounds.x1 = native.disabled.bounds.x0 + v
            elseif k == 'height' then
                native.normal.bounds.y1 = native.normal.bounds.y0 + v
                native.hot.bounds.y1 = native.hot.bounds.y0 + v
                native.active.bounds.y1 = native.active.bounds.y0 + v
                native.disabled.bounds.y1 = native.disabled.bounds.y0 + v
            elseif k == 'normal' then
                local x = native.normal.bounds.x0
                local y = native.normal.bounds.y0
                native.normal.bounds.x0 = v.x
                native.normal.bounds.y0 = v.y
                native.normal.bounds.x1 = v.x + native.normal.bounds.x1 - x
                native.normal.bounds.y1 = v.y + native.normal.bounds.y1 - y
            elseif k == 'hot' then
                local x = native.hot.bounds.x0
                local y = native.hot.bounds.y0
                native.hot.bounds.x0 = v.x
                native.hot.bounds.y0 = v.y
                native.hot.bounds.x1 = v.x + native.hot.bounds.x1 - x
                native.hot.bounds.y1 = v.y + native.hot.bounds.y1 - y
            elseif k == 'active' then
                local x = native.active.bounds.x0
                local y = native.active.bounds.y0
                native.active.bounds.x0 = v.x
                native.active.bounds.y0 = v.y
                native.active.bounds.x1 = v.x + native.active.bounds.x1 - x
                native.active.bounds.y1 = v.y + native.active.bounds.y1 - y
            elseif k == 'disabled' then
                local x = native.disabled.bounds.x0
                local y = native.disabled.bounds.y0
                native.disabled.bounds.x0 = v.x
                native.disabled.bounds.y0 = v.y
                native.disabled.bounds.x1 = v.x + native.disabled.bounds.x1 - x
                native.disabled.bounds.y1 = v.y + native.disabled.bounds.y1 - y
            elseif k == 'overdraw' then
                native.normal.overdraw.left = v.left
                native.normal.overdraw.top = v.top
                native.normal.overdraw.right = v.right
                native.normal.overdraw.bottom = v.bottom
            elseif k == 'cursor' then
                native.cursor = system_cursor_t[v]
            end
        end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    ui.image_button_descriptor = function(image, width, height)
        return setmetatable({
            [image_key] = image,
            [native_key] = image_button_descriptor_t({
                image_data = image,
                image_size = #image,
                normal = {
                    texture_size = {width = width, height = height},
                    bounds = {x1 = width, y1 = height}
                },
                hot = {
                    texture_size = {width = width, height = height},
                    bounds = {x1 = width, y1 = height}
                },
                active = {
                    texture_size = {width = width, height = height},
                    bounds = {x1 = width, y1 = height}
                },
                disabled = {
                    texture_size = {width = width, height = height},
                    bounds = {x1 = width, y1 = height}
                },
                cursor = 1
            })
        }, image_button_descriptor_mt)
    end

    widgets.image_button = function(layout, ...)
        local id, image_descriptor = ...
        if select('#', ...) < 2 then image_descriptor = id end
        id = get_id(id)
        prepare_widget(layout, descriptor, image_descriptor)
        local native = rawget(image_descriptor, native_key)
        local state = image_button(current_context, id, native)
        return state.clicked and state.button == 0, state
    end
end

-- widget: label
do
    local descriptor = read_only({
        name = 'label',
        default_size = function() return nil, 13 end
    })

    widgets.label = function(layout, text)
        prepare_widget(layout, descriptor, text)
        label(current_context, text, #text)
    end
end

-- widget: link
do
    local descriptor = read_only({
        name = 'link',
        default_size = function() return nil, 13 end
    })

    widgets.link = function(layout, ...)
        local id, text = ...
        if select('#', ...) < 2 then text = id end
        id = get_id(id)
        prepare_widget(layout, descriptor, text)
        local state = link(current_context, id, text, #text)
        return state.clicked and state.button == 0, state
    end
end

-- widget: progress
do
    local ui_color_skin_accent = ui.color.skin_accent

    local count_key = {}

    local descriptor = read_only({
        name = 'progress',
        default_size = function(direction)
            if direction == 'left_to_right' or direction == 'right_to_left' then
                return 160, 8
            elseif direction == 'bottom_to_top' or direction == 'top_to_bottom' then
                return 8, 160
            end
        end
    })

    local progress_entries_mt = {
        __index = function(t, k)
            if type(k) == 'number' then
                return rawget(t, native_key)[k - 1]
            end
            return rawget(t, native_key)[0][k]
        end,
        __newindex = function(t, k, v)
            if type(k) == 'number' then
                rawget(t, native_key)[k - 1] = v
            end
            rawget(t, native_key)[0][k] = v
        end,
        __len = function(t) return rawget(t, count_key) end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    ui.progress_entries = function(...)
        local count = ...
        if select('#', ...) == 0 then count = 1 end

        local native = progress_entry_array_t(count)
        local entries = setmetatable({
            [native_key] = native,
            [count_key] = count
        }, progress_entries_mt)

        for i = 0, count - 1 do
            native[i].max = 1
            native[i].color = ui_color_skin_accent
        end

        return entries
    end

    local progress_entry = progress_entry_t()
    widgets.progress = function(layout, ...)
        local entries
        local entries_count
        local direction
        local args = select('#', ...)
        if args == 1 then
            local arg = select(1, ...)
            local arg_type = type(arg)
            if arg_type == 'number' then
                progress_entry.value = arg
                progress_entry.max = 1
                progress_entry.color = ui_color_skin_accent
                entries = progress_entry
                entries_count = 1
            elseif arg_type == 'table' then
                entries = rawget(arg, native_key)
                entries_count = rawget(arg, count_key)
            end
            direction = 'left_to_right'
        elseif args == 2 then
            local arg = select(1, ...)
            local arg_type = type(arg)
            if arg_type == 'number' then
                progress_entry.value = arg
                progress_entry.max = select(2, ...)
                progress_entry.color = ui_color_skin_accent
                entries = progress_entry
                entries_count = 1
                direction = 'left_to_right'
            elseif arg_type == 'table' then
                entries = rawget(arg, native_key)
                entries_count = rawget(arg, count_key)
                direction = select(2, ...)
            end
        elseif args == 3 then
            local arg_type = type(select(3, ...))
            if arg_type == 'number' then
                progress_entry.value = select(1, ...)
                progress_entry.max = select(2, ...)
                progress_entry.color = select(3, ...)
                direction = 'left_to_right'
            elseif arg_type == 'string' then
                progress_entry.value = select(1, ...)
                progress_entry.max = select(2, ...)
                progress_entry.color = ui_color_skin_accent
                direction = direction_t[select(3, ...)]
            end
            entries = progress_entry
            entries_count = 1
        elseif args >= 4 then
            progress_entry.value = select(1, ...)
            progress_entry.max = select(2, ...)
            progress_entry.color = select(3, ...)
            entries = progress_entry
            entries_count = 1
            direction = direction_t[select(4, ...)]
        end
        prepare_widget(layout, descriptor, direction)
        direction = direction_t[direction]
        progress(current_context, entries, entries_count, direction)
    end
end

-- widget: radio
do
    local descriptor = read_only({
        name = 'radio',
        default_size = function() return nil, 12 end
    })

    widgets.radio = function(layout, ...)
        local id, text, checked = ...
        if select('#', ...) < 3 then text, checked = id, text end
        id = get_id(id)
        checked = not not checked
        prepare_widget(layout, descriptor, text, checked)
        local state = radio(current_context, id, text, #text, checked)
        return state.clicked and state.button == 0, state
    end
end

-- widget: scroll_panel
do
    local descriptor = read_only({
        name = 'scroll_panel',
        default_size = function() return nil, nil end
    })

    local scroll_panel_state_mt = {
        __index = function(t, k)
            local state = rawget(t, native_key)
            if k == 'offset' then
                return {x = state.offset.x, y = state.offset.y}
            elseif k == 'canvas_size' then
                return {
                    width = state.canvas_size.width,
                    height = state.canvas_size.height
                }
            elseif k == 'visibility_horizontal' then
                return scroll_bar_visibility_names[state.visibility_horizontal]
            elseif k == 'visibility_vertical' then
                return scroll_bar_visibility_names[state.visibility_vertical]
            else
                return state[k]
            end
        end,
        __newindex = function(t, k, v)
            local state = rawget(t, native_key)
            if k == 'offset' then
                state.offset.x = v.x
                state.offset.y = v.y
            elseif k == 'x' then
                state.offset.x = v.x
            elseif k == 'y' then
                state.offset.y = v.y
            elseif k == 'canvas_size' then
                state.canvas_size.width = v.width
                state.canvas_size.height = v.height
            elseif k == 'width' then
                state.canvas_size.width = v.width
            elseif k == 'height' then
                state.canvas_size.height = v.height
            elseif k == 'visibility_horizontal' then
                state.visibility_horizontal = scroll_bar_visibility_t[v]
            elseif k == 'visibility_vertical' then
                state.visibility_vertical = scroll_bar_visibility_t[v]
            else
                state[k] = v
            end
        end,
        __pairs = function(t) return function() return nil end, t, nil end,
        __metatable = false
    }

    ui.scroll_panel_state = function(width, height)
        return setmetatable({
            [native_key] = scroll_panel_state_t({
                canvas_size = {width = width, height = height},
                visibility_horizontal = 2,
                visibility_vertical = 2,
                line_height = 16,
                offset = {x = 0, y = 0}
            })
        }, scroll_panel_state_mt)
    end

    widgets.scroll_panel = function(layout, state, draw)
        prepare_widget(layout, descriptor)
        local id = get_id(state)
        state = rawget(state, native_key)
        begin_scroll_panel(current_context, id, state)
        local w = state.canvas_size.width
        local h = state.canvas_size.height
        ui.layout('default', 0, 0, w, h, draw)
        end_scroll_panel(current_context)
    end
end

-- widget: slider
do
    local ui_color_skin_accent = ui.color.skin_accent

    local descriptor = read_only({
        name = 'slider',
        default_size = function() return 160, 21 end
    })

    widgets.slider = function(layout, id, value, min, max, ...)
        local fill_color, direction
        local args = select('#', ...)
        if args == 0 then
            fill_color = ui_color_skin_accent
            direction = 'left_to_right'
        elseif args == 1 then
            local arg = select(1, ...)
            if type(arg) == 'number' then
                fill_color = arg
                direction = 'left_to_right'
            elseif type(arg) == 'string' then
                fill_color = ui_color_skin_accent
                direction = arg
            end
        else
            fill_color = select(1, ...)
            direction = select(2, ...)
        end
        id = get_id(id)
        prepare_widget(layout, descriptor, direction)
        direction = direction_t[direction]
        return slider(current_context, id, value, min, max, fill_color,
                      direction)
    end
end

return ui
