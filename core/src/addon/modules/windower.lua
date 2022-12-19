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
    version,
    version_major,
    version_minor,
    version_build,
    build_tag,
    client_path,
    scripts_path,
    client_width,
    client_height,
    ui_width,
    ui_height,
    client_hwnd,
    settings_path,
    user_path,
    package_path,
    package_name = ...
-- LuaFormatter on

local windower = {
    version = version,
    version_major = version_major,
    version_minor = version_minor,
    version_build = version_build,
    build_tag = build_tag,
    client_path = client_path,
    scripts_path = scripts_path,
    settings_path = settings_path,
    user_path = user_path,
    package_path = package_path,
    package_name = package_name,
    settings = {
        client_size = {width = client_width, height = client_height},
        ui_size = {width = ui_width, height = ui_height}
    },
    client_hwnd = client_hwnd
}

return windower
