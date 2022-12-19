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

#include "ui/context.hpp"

#include "hooks/ffximain.hpp"
#include "hooks/user32.hpp"
#include "ui/bitmap.hpp"
#include "ui/command_buffer.hpp"
#include "ui/commands.hpp"
#include "ui/data_buffer.hpp"
#include "ui/dwrite_iids.hpp"
#include "ui/id.hpp"
#include "ui/layer.hpp"
#include "ui/mouse.hpp"
#include "ui/primitives.hpp"
#include "ui/text_layout_engine.hpp"
#include "ui/text_rasterizer.hpp"
#include "ui/texture_cache.hpp"
#include "ui/texture_loaders.hpp"
#include "ui/texture_token.hpp"
#include "ui/vector.hpp"
#include "ui/vertex.hpp"
#include "utility.hpp"

#include <windows.h>

#include <d3d8.h>
#include <windowsx.h>
#include <winrt/base.h>

#include <gsl/gsl>

#include <algorithm>
#include <cstddef>
#include <new>
#include <optional>
#include <span>

namespace windower::ui
{
namespace
{

constexpr float calculate_rhw(float depth) noexcept
{
    return 10.f - 10.f * depth;
}

template<typename T>
void primitive_command(
    std::span<vertex const> vertices, std::span<std::uint16_t const> indices,
    ::IDirect3DDevice8* d3d_device, data_buffer<vertex>& v_buffer,
    windower::ui::data_buffer<std::uint16_t>& i_buffer,
    windower::ui::command_buffer& commands, bool reflected) noexcept
{
    namespace range = std::ranges;

    auto v = v_buffer.allocate(d3d_device, commands, vertices.size());
    auto i = i_buffer.allocate(d3d_device, commands, indices.size());
    commands.emplace<T>(
        gsl::narrow_cast<std::uint16_t>(v.offset),
        gsl::narrow_cast<std::uint16_t>(v.data.size()),
        gsl::narrow_cast<std::uint16_t>(i.offset),
        gsl::narrow_cast<std::uint16_t>(i.data.size()));

    range::copy(vertices, v.data.begin());
    if (reflected)
    {
        range::transform(
            range::reverse_view{indices}, i.data.begin(),
            [offset = v.offset](auto idx) {
                return gsl::narrow_cast<std::uint16_t>(idx + offset);
            });
    }
    else
    {
        range::transform(
            indices, i.data.begin(), [offset = v.offset](auto idx) {
                return gsl::narrow_cast<std::uint16_t>(idx + offset);
            });
    }
}

}

context::context(
    ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
    dimension const& screen_size, dimension const& ui_size,
    dimension const& render_size) noexcept :
    m_hwnd{hwnd},
    m_d3d_device{d3d_device}
{
    if (FAILED(::CoCreateInstance(
            ::CLSID_WICImagingFactory, nullptr, ::CLSCTX_INPROC_SERVER,
            ::IID_IWICImagingFactory, m_wic_factory.put_void())))
    {
        // TODO: Handle Error
    }

    winrt::com_ptr<::IUnknown> unknown;
    ::DWriteCreateFactory(
        ::DWRITE_FACTORY_TYPE_SHARED, ::IID_IDWriteFactory, unknown.put());
    unknown.as(::IID_IDWriteFactory, m_dwrite_factory.put_void());

    m_d3d_device->CreateStateBlock(D3DSBT_ALL, &m_default_state);
    m_d3d_device->CreateStateBlock(D3DSBT_ALL, &m_previous_state);

    auto const ui_scale    = vector{screen_size} / vector{ui_size};
    auto const world_scale = vector{render_size} / vector{ui_size};

    descriptor(layer::layout) = {ui_scale, true, screen_size};
    descriptor(layer::screen) = {ui_scale, true, screen_size};
    descriptor(layer::world)  = {world_scale, false, render_size};

    auto const screen_bounds = rectangle{{}, vector{screen_size}};

    m_screen.m_layer = layer::screen;
    m_screen.m_depth = 0.f;
    m_screen.bounds(screen_bounds);
    m_screen.zoom_factor(1.f);

    m_layout_grid.m_layer = layer::layout;
    m_layout_grid.m_depth = 0.f;
    m_layout_grid.bounds(screen_bounds);
    m_layout_grid.zoom_factor(1.f);

    auto& screen_ctx  = m_window_stack.emplace_back();
    screen_ctx.window = &m_screen;
    screen_ctx.transform_stack.emplace_back();
    screen_ctx.clip_stack.push_back(screen_bounds);
    screen_ctx.enabled_stack.push_back(true);

    m_d3d_device->CaptureStateBlock(m_previous_state);

    auto const viewport = ::D3DVIEWPORT8{0, 0, 1, 1, 0, 1};
    m_d3d_device->SetViewport(&viewport);

    m_d3d_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    m_d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    m_d3d_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_LASTPIXEL, TRUE);
    m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    m_d3d_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 128);
    m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    m_d3d_device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_d3d_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_FOGCOLOR, 0);
    m_d3d_device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    m_d3d_device->SetRenderState(D3DRS_FOGSTART, std::bit_cast<::DWORD>(0.f));
    m_d3d_device->SetRenderState(D3DRS_FOGEND, std::bit_cast<::DWORD>(1.f));
    m_d3d_device->SetRenderState(D3DRS_FOGDENSITY, std::bit_cast<::DWORD>(1.f));
    m_d3d_device->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    m_d3d_device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    m_d3d_device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
    m_d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    m_d3d_device->SetRenderState(D3DRS_STENCILREF, 1);
    m_d3d_device->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
    m_d3d_device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
    m_d3d_device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    m_d3d_device->SetRenderState(D3DRS_WRAP0, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP1, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP2, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP3, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP4, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP5, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP6, 0);
    m_d3d_device->SetRenderState(D3DRS_WRAP7, 0);
    m_d3d_device->SetRenderState(D3DRS_CLIPPING, TRUE);
    m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_d3d_device->SetRenderState(D3DRS_AMBIENT, 0);
    m_d3d_device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
    m_d3d_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
    m_d3d_device->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
    m_d3d_device->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
    m_d3d_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    m_d3d_device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
    m_d3d_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    m_d3d_device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
    m_d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    m_d3d_device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    m_d3d_device->SetRenderState(D3DRS_POINTSIZE, std::bit_cast<::DWORD>(1.f));
    m_d3d_device->SetRenderState(
        D3DRS_POINTSIZE_MIN, std::bit_cast<::DWORD>(1.f));
    m_d3d_device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
    m_d3d_device->SetRenderState(
        D3DRS_POINTSCALE_A, std::bit_cast<::DWORD>(1.f));
    m_d3d_device->SetRenderState(
        D3DRS_POINTSCALE_B, std::bit_cast<::DWORD>(0.f));
    m_d3d_device->SetRenderState(
        D3DRS_POINTSCALE_C, std::bit_cast<::DWORD>(0.f));
    m_d3d_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    m_d3d_device->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
    m_d3d_device->SetRenderState(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
    m_d3d_device->SetRenderState(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);
    m_d3d_device->SetRenderState(
        D3DRS_POINTSIZE_MAX, std::bit_cast<::DWORD>(64.f));
    m_d3d_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    m_d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
    m_d3d_device->SetRenderState(
        D3DRS_TWEENFACTOR, std::bit_cast<::DWORD>(0.f));
    m_d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    for (auto i = 0u; i < 8; ++i)
    {
        m_d3d_device->SetTexture(0, nullptr);

        m_d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
        m_d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
        m_d3d_device->SetTextureStageState(
            i, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        m_d3d_device->SetTextureStageState(
            i, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        m_d3d_device->SetTextureStageState(i, D3DTSS_BORDERCOLOR, 0x00000000);
        m_d3d_device->SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_POINT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_POINT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_NONE);
        m_d3d_device->SetTextureStageState(
            i, D3DTSS_MIPMAPLODBIAS, std::bit_cast<::DWORD>(0.f));
        m_d3d_device->SetTextureStageState(i, D3DTSS_MAXMIPLEVEL, 0);
        m_d3d_device->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, 1);
        m_d3d_device->SetTextureStageState(
            i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_d3d_device->SetTextureStageState(
            i, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
        m_d3d_device->SetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
        m_d3d_device->SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_CURRENT);
    }

    m_d3d_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    m_d3d_device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_d3d_device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

    m_d3d_device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

    m_d3d_device->CaptureStateBlock(m_default_state);
    m_d3d_device->ApplyStateBlock(m_previous_state);

    m_texture_cache.initialize(*this);
    m_text_layout_engine.initialize(*this);
    m_text_rasterizer.initialize(*this);

    load_colors(u8":system");
    skin(u8":skin");

    auto const client = client_path();

    // Basic
    gsl::at(m_cursors, 0) = cursor{client / u8"mousenor.ani"};
    gsl::at(m_cursors, 1) = cursor{client / u8"mousehit.ani"};

    // Directions
    gsl::at(m_cursors, 2) = cursor{client / u8"arw_n.ani"};
    gsl::at(m_cursors, 3) = cursor{client / u8"arw_ne.ani"};
    gsl::at(m_cursors, 4) = cursor{client / u8"arw_e.ani"};
    gsl::at(m_cursors, 5) = cursor{client / u8"arw_se.ani"};
    gsl::at(m_cursors, 6) = cursor{client / u8"arw_s.ani"};
    gsl::at(m_cursors, 7) = cursor{client / u8"arw_sw.ani"};
    gsl::at(m_cursors, 8) = cursor{client / u8"arw_w.ani"};
    gsl::at(m_cursors, 9) = cursor{client / u8"arw_nw.ani"};

    // Directions (Alt)
    gsl::at(m_cursors, 10) = cursor{client / u8"oarw_n.ani"};
    gsl::at(m_cursors, 11) = cursor{client / u8"oarw_ne.ani"};
    gsl::at(m_cursors, 12) = cursor{client / u8"oarw_e.ani"};
    gsl::at(m_cursors, 13) = cursor{client / u8"oarw_se.ani"};
    gsl::at(m_cursors, 14) = cursor{client / u8"oarw_s.ani"};
    gsl::at(m_cursors, 15) = cursor{client / u8"oarw_sw.ani"};
    gsl::at(m_cursors, 16) = cursor{client / u8"oarw_w.ani"};
    gsl::at(m_cursors, 17) = cursor{client / u8"oarw_nw.ani"};

    m_client_shared = ffximain::menu<shared_window_config>(u8"conf5win");
    m_client_log1   = ffximain::menu<window_config>(u8"logwindo");
    m_client_log2   = ffximain::menu<window_config>(u8"logwin2 ");

    // m_layout_mode = true;
}

context::~context() noexcept
{
    m_d3d_device->DeleteStateBlock(m_default_state);
    m_d3d_device->DeleteStateBlock(m_previous_state);
}

dimension const& context::screen_size() const noexcept
{
    return descriptor(layer::screen).size;
}

std::u8string_view context::skin() const noexcept { return m_skin; }

void context::skin(std::u8string_view skin) noexcept
{
    m_skin = skin;
    load_colors(skin, 128);
}

color context::system_color(ui::system_color color) const noexcept
{
    auto const index = gsl::narrow_cast<std::uint8_t>(color);
    return gsl::at(m_colors, index);
}

bool context::enabled() const noexcept
{
    return m_window_stack.back().enabled_stack.back();
}

void context::enabled(bool enabled) noexcept
{
    m_window_stack.back().enabled_stack.back() = enabled;
}

void context::push_enabled(bool enabled) noexcept
{
    m_window_stack.back().enabled_stack.push_back(enabled);
}

void context::push_enabled() noexcept { push_enabled(enabled()); }

void context::pop_enabled() noexcept
{
    m_window_stack.back().enabled_stack.pop_back();
}

rectangle const& context::bounds() const noexcept { return m_bounds; }

void context::bounds(rectangle const& bounds) noexcept { m_bounds = bounds; }

window const* context::current_window() const noexcept
{
    auto const window = m_window_stack.back().window;
    return window == &m_screen ? nullptr : window;
}

window const* context::active_window() const noexcept
{
    return m_window_manager.active_window();
}

bool context::is_active(window const& wnd) const noexcept
{
    return active_window() == &wnd && interactable(wnd.layer()) &&
           wnd.interactable();
}

void context::activate_next_window() noexcept
{
    m_window_manager.activate_next(*this);
}

void context::activate_previous_window() noexcept
{
    m_window_manager.activate_previous(*this);
}

void context::use_id(id id) noexcept
{
    if (id == m_active_id)
    {
        m_active_id_used = true;
    }
}

bool context::hit_test(id id) const noexcept { return hit_test(id, m_bounds); }

bool context::hit_test(id id, rectangle const& bounds) const noexcept
{
    if (m_active_id != id && m_active_id != no_id)
    {
        return false;
    }

    auto const& wnd_ctx = m_window_stack.back();
    if (wnd_ctx.window != m_window_manager.hot_window() ||
        !interactable(wnd_ctx.window->layer()) ||
        !wnd_ctx.window->interactable())
    {
        return false;
    }

    auto const& mouse            = m_mouse.position();
    auto const transformed_mouse = to_widget(mouse);
    if (!transformed_mouse)
    {
        return false;
    }

    auto const origin = context::origin();
    auto const scale  = context::scale_factor();

    auto x0 = bounds.x0;
    auto y0 = bounds.y0;
    auto x1 = bounds.x1;
    auto y1 = bounds.y1;

    x0 = std::round(scale.x * x0) / scale.x;
    x1 = std::round(scale.x * x1) / scale.x;
    y0 = std::round(scale.y * y0) / scale.y;
    y1 = std::round(scale.y * y1) / scale.y;

    if (!is_inside(*transformed_mouse, {x0, y0, x1, y1}))
    {
        return false;
    }

    auto const& clip = m_window_stack.back().clip_stack.back();
    if (!is_inside(mouse, clip))
    {
        return false;
    }

    return true;
}

bool context::is_inactive() const noexcept { return m_active_id == no_id; }

bool context::is_active(id id) const noexcept { return m_active_id == id; }

bool context::activate(id id) noexcept
{
    if (m_active_id == no_id)
    {
        m_active_id_used = true;
        m_active_id      = id;
        return true;
    }
    return false;
}

bool context::deactivate(id id) noexcept
{
    if (m_active_id == id)
    {
        m_active_id = no_id;
        return true;
    }
    return false;
}

bool context::clear_active_state(id id) noexcept
{
    if (m_active_state_id == id)
    {
        m_active_state_id = no_id;
        m_active_state.clear();
        return true;
    }
    return false;
}

id context::focused_id() const noexcept
{
    return m_window_stack.back().window->focused_id();
}

bool context::is_blurred() const noexcept
{
    return m_window_stack.back().window->is_blurred();
}

bool context::is_focused(id id) const noexcept
{
    return m_window_stack.back().window->is_focused(id);
}

bool context::focus(id id) noexcept
{
    return m_window_stack.back().window->focus(id);
}

bool context::blur(id id) noexcept
{
    return m_window_stack.back().window->blur(id);
}

vector context::get_scroll(id id) noexcept { return get_scroll(id, bounds()); }

vector context::get_scroll(id id, rectangle const& bounds) noexcept
{
    if (hit_test(id, bounds))
    {
        m_scroll_requested_id = id;
        if (is_inactive() && m_scroll_id == id)
        {
            return m_mouse.scroll_offset();
        }
    }
    return {};
}

bool context::layout_mode() const noexcept { return m_layout_mode; }

void context::layout_mode(bool enabled) noexcept { m_layout_mode = enabled; }

vector context::origin() const noexcept
{
    auto const& wnd_ctx = m_window_stack.back();
    return wnd_ctx.window->origin() + wnd_ctx.offset_stack.back();
}

float context::zoom_factor() const noexcept
{
    return m_window_stack.back().window->zoom_factor();
}

vector context::scale_factor() const noexcept
{
    return scale_factor(m_window_stack.back().window->layer());
}

vector context::scale_factor(layer layer) const noexcept
{
    return descriptor(layer).scale_factor;
}

float context::scale_factor_uniform() const noexcept
{
    auto const scale = scale_factor();
    return std::max(scale.x, scale.y);
}

float context::scale_factor_uniform(layer layer) const noexcept
{
    auto const scale = scale_factor(layer);
    return std::max(scale.x, scale.y);
}

std::tuple<float, float> context::depth() const noexcept
{
    auto& wnd_ctx    = m_window_stack.back();
    auto const depth = wnd_ctx.window->depth();
    return {depth, calculate_rhw(depth)};
}

bool context::interactable() const noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    return wnd_ctx.window->interactable() &&
           interactable(wnd_ctx.window->layer());
}

bool context::interactable(layer layer) const noexcept
{
    return descriptor(layer).interactable;
}

transform context::current_transform() const noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    return wnd_ctx.transform_stack.back();
}

window&
context::begin_window(std::intptr_t id, layer layer, float depth) noexcept
{
    auto const* const parent =
        m_window_stack.size() > 1 ? m_window_stack.back().window : nullptr;
    auto& wnd_ctx            = m_window_stack.emplace_back();
    auto const clamped_depth = std::clamp(depth, 0.f, 1.f);

    auto const& layer_descriptor = descriptor(layer);
    auto const layer_size        = vector{layer_descriptor.size};
    auto const layer_bounds      = rectangle{{}, layer_size};

    wnd_ctx.window =
        m_window_manager.get(*this, id, layer, clamped_depth, parent);
    wnd_ctx.window->m_origin      = {};
    wnd_ctx.window->m_bounds      = layer_bounds;
    wnd_ctx.window->m_zoom_factor = 1;

    wnd_ctx.transform_stack.emplace_back();
    wnd_ctx.offset_stack.emplace_back();
    wnd_ctx.clip_stack.push_back(layer_bounds);
    wnd_ctx.enabled_stack.push_back(true);
    commands().emplace<set_clip_command>(layer_bounds);

    return *wnd_ctx.window;
}

void context::end_window() noexcept
{
    m_window_stack.pop_back();
    auto& wnd_ctx = m_window_stack.back();
    commands().emplace<set_clip_command>(wnd_ctx.clip_stack.back());
}

std::optional<vector> context::to_widget(vector const&) const noexcept
{
    auto const origin = context::origin();
    auto const zoom   = context::zoom_factor();
    auto const scale  = context::scale_factor();

    auto const origin_x = std::round(scale.x * origin.x);
    auto const origin_y = std::round(scale.y * origin.y);

    auto const transform = transform::translation(origin_x, origin_y) *
                           transform::scale(scale.x * zoom, scale.y * zoom) *
                           current_transform();

    auto inverse_transform = inverse(transform);
    if (!inverse_transform)
    {
        return std::nullopt;
    }
    return *inverse_transform * m_mouse.position();
}

void context::push_transform(ui::transform const& transform) noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    wnd_ctx.transform_stack.push_back(transform);
}

void context::pop_transform() noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    wnd_ctx.transform_stack.pop_back();
}

void context::push_clip(rectangle const& clip) noexcept
{
    auto const origin = context::origin();
    auto const zoom   = context::zoom_factor();
    auto const scale  = context::scale_factor();

    auto const origin_x = std::round(scale.x * origin.x);
    auto const origin_y = std::round(scale.y * origin.y);

    auto x0 = std::round(scale.x * zoom * clip.x0) + origin_x;
    auto x1 = std::round(scale.x * zoom * clip.x1) + origin_x;
    auto y0 = std::round(scale.y * zoom * clip.y0) + origin_y;
    auto y1 = std::round(scale.y * zoom * clip.y1) + origin_y;

    x0 = std::max(x0, 0.f);
    x1 = std::max(x1, x0);
    y0 = std::max(y0, 0.f);
    y1 = std::max(y1, y0);

    auto& wnd_ctx            = m_window_stack.back();
    auto const& current      = wnd_ctx.clip_stack.back();
    auto const adjusted_clip = intersection(current, {x0, y0, x1, y1});
    wnd_ctx.clip_stack.push_back(adjusted_clip);
    commands().emplace<set_clip_command>(adjusted_clip);
    wnd_ctx.fully_clipped = adjusted_clip.x1 <= adjusted_clip.x0 ||
                            adjusted_clip.y1 <= adjusted_clip.y0;
}

void context::pop_clip() noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    wnd_ctx.clip_stack.pop_back();
    auto const& clip = wnd_ctx.clip_stack.back();
    commands().emplace<set_clip_command>(clip);
    wnd_ctx.fully_clipped = clip.x1 <= clip.x0 || clip.y1 <= clip.y0;
}

void context::push_offset(vector const& offset) noexcept
{
    auto& wnd_ctx = m_window_stack.back();
    wnd_ctx.offset_stack.push_back(wnd_ctx.offset_stack.back() + offset);
}

void context::pop_offset() noexcept
{
    m_window_stack.back().offset_stack.pop_back();
}

void context::set_texture(texture_token texture) noexcept
{
    commands().emplace<set_texture_command>(texture.m_value);
}

void context::draw_triangle_list(
    std::initializer_list<vertex> vertices,
    std::initializer_list<std::uint16_t> indices, bool reflected) noexcept
{
    if (!m_window_stack.back().fully_clipped)
    {
        primitive_command<draw_triangle_list_command>(
            vertices, indices, m_d3d_device, m_vertex_buffer, m_index_buffer,
            commands(), reflected);
    }
}

void context::draw_triangle_list(
    std::span<vertex const> vertices, std::span<std::uint16_t const> indices,
    bool reflected) noexcept
{
    if (!m_window_stack.back().fully_clipped)
    {
        primitive_command<draw_triangle_list_command>(
            vertices, indices, m_d3d_device, m_vertex_buffer, m_index_buffer,
            commands(), reflected);
    }
}

void context::set_cursor(system_cursor cursor) noexcept { m_cursor = cursor; }

void context::process_mouse_message(::MSG const& msg) noexcept
{
    static constexpr auto get_wheel_delta = [](::WPARAM wParam) noexcept {
        static constexpr auto scale = 1.f / WHEEL_DELTA;
        return scale * GET_WHEEL_DELTA_WPARAM(wParam);
    };

    static constexpr auto get_mouse_position = [](::HWND hwnd) noexcept {
        auto point = ::POINT{};
        ::GetCursorPos(&point);
        ::ScreenToClient(hwnd, &point);
        auto const x = gsl::narrow_cast<float>(point.x);
        auto const y = gsl::narrow_cast<float>(point.y);
        return vector{x, y};
    };

    switch (msg.message)
    {
    default: break;
    case WM_MOUSEMOVE: m_mouse.move(get_mouse_position(m_hwnd)); break;
    case WM_LBUTTONDOWN: m_mouse.press(mouse_button::left); break;
    case WM_LBUTTONUP: m_mouse.release(mouse_button::left); break;
    case WM_LBUTTONDBLCLK: break;
    case WM_RBUTTONDOWN: m_mouse.press(mouse_button::right); break;
    case WM_RBUTTONUP: m_mouse.release(mouse_button::right); break;
    case WM_RBUTTONDBLCLK: break;
    case WM_MBUTTONDOWN: m_mouse.press(mouse_button::middle); break;
    case WM_MBUTTONUP: m_mouse.release(mouse_button::middle); break;
    case WM_MBUTTONDBLCLK: break;
    case WM_MOUSEWHEEL:
        m_mouse.scroll({0.f, -get_wheel_delta(msg.wParam)});
        break;
    case WM_XBUTTONDOWN:
        switch (HIWORD(msg.wParam))
        {
        default: break;
        case XBUTTON1: m_mouse.press(mouse_button::x1); break;
        case XBUTTON2: m_mouse.press(mouse_button::x2); break;
        }
        break;
    case WM_XBUTTONUP:
        switch (HIWORD(msg.wParam))
        {
        default: break;
        case XBUTTON1: m_mouse.release(mouse_button::x1); break;
        case XBUTTON2: m_mouse.release(mouse_button::x2); break;
        }
        break;
    case WM_XBUTTONDBLCLK:
        switch (HIWORD(msg.wParam))
        {
        default: break;
        case XBUTTON1: break;
        case XBUTTON2: break;
        }
        break;
    case WM_MOUSEHWHEEL:
        m_mouse.scroll({get_wheel_delta(msg.wParam), 0.f});
        break;
    }
}

std::optional<::LRESULT> context::process_message(::MSG const& message) noexcept
{
    if (message.hwnd == m_hwnd)
    {
        if (message.message >= WM_MOUSEFIRST && message.message <= WM_MOUSELAST)
        {
            process_mouse_message(message);
            if (m_window_manager.update_hot_window(*this) != nullptr ||
                m_active_id != no_id || m_layout_mode ||
                (m_window_manager.active_window() != nullptr &&
                 (message.message == WM_LBUTTONUP ||
                  message.message == WM_RBUTTONUP ||
                  message.message == WM_MBUTTONUP ||
                  message.message == WM_XBUTTONUP)))
            {
                if (message.message == WM_XBUTTONDOWN ||
                    message.message == WM_XBUTTONUP ||
                    message.message == WM_XBUTTONDBLCLK)
                {
                    return TRUE;
                }
                return 0;
            }
        }
        else
        {
            return m_window_manager.process_message(message);
        }
    }

    return std::nullopt;
}

void context::begin_frame() noexcept
{
    m_vertex_buffer.clear();
    m_index_buffer.clear();

    m_window_stack.clear();

    m_screen.commands().clear();
    m_layout_grid.commands().clear();
    m_window_manager.update(*this);

    auto const screen_bounds = rectangle{{}, vector{screen_size()}};

    auto& screen_ctx  = m_window_stack.emplace_back();
    screen_ctx.window = &m_screen;
    screen_ctx.transform_stack.emplace_back();
    screen_ctx.offset_stack.emplace_back();
    screen_ctx.clip_stack.push_back(screen_bounds);

    m_screen.commands().emplace<set_clip_command>(screen_bounds);

    if (m_last_active_id != m_active_id && m_active_id == no_id)
    {
        auto position = ::POINT{};
        ::GetCursorPos(&position);
        if (::WindowFromPoint(position) == m_hwnd)
        {
            auto const wParam = gsl::narrow_cast<::WPARAM>(
                (::GetKeyState(VK_LBUTTON) < 0 ? MK_LBUTTON : 0) |
                (::GetKeyState(MK_RBUTTON) < 0 ? MK_RBUTTON : 0) |
                (::GetKeyState(VK_SHIFT) < 0 ? MK_SHIFT : 0) |
                (::GetKeyState(VK_CONTROL) < 0 ? MK_CONTROL : 0) |
                (::GetKeyState(VK_MBUTTON) < 0 ? MK_MBUTTON : 0) |
                (::GetKeyState(VK_XBUTTON1) < 0 ? MK_XBUTTON1 : 0) |
                (::GetKeyState(VK_XBUTTON2) < 0 ? MK_XBUTTON2 : 0));

            ::ScreenToClient(m_hwnd, &position);
            auto const x      = gsl::narrow_cast<::WORD>(position.x);
            auto const y      = gsl::narrow_cast<::WORD>(position.y);
            auto const lParam = gsl::narrow_cast<::LPARAM>(x | (y << 16));

            ::PostMessageW(m_hwnd, WM_MOUSEMOVE, wParam, lParam);
        }
    }

    if (m_active_id != no_id)
    {
        if (m_active_id_used)
        {
            if (m_last_active_id == no_id)
            {
                ::SetCapture(m_hwnd);
            }
        }
        else
        {
            m_active_id       = no_id;
            m_active_state_id = no_id;
            m_active_state.clear();
            ::ReleaseCapture();
        }
    }
    m_last_active_id = m_active_id;
    m_active_id_used = false;

    m_scroll_id           = m_scroll_requested_id;
    m_scroll_requested_id = no_id;

    if (m_layout_mode)
    {
        draw_layout_grid();
    }

    if (m_window_manager.hot_window() ||
        (m_window_manager.active_window() &&
         (m_mouse.is_held(mouse_button::left) ||
          m_mouse.is_held(mouse_button::right) ||
          m_mouse.is_held(mouse_button::middle) ||
          m_mouse.is_held(mouse_button::x1) ||
          m_mouse.is_held(mouse_button::x2))) ||
        m_layout_mode)
    {
        m_cursor = system_cursor::normal;
    }
}

void context::end_frame() noexcept
{
    m_mouse.update();
    m_text_layout_engine.update();
    m_texture_cache.update();
    if (m_cursor)
    {
        auto const cursor_index  = gsl::narrow_cast<std::int8_t>(*m_cursor);
        auto const cursor_handle = gsl::at(m_cursors, cursor_index).m_handle;
        ::SetCursor(static_cast<::HCURSOR>(cursor_handle));
    }
    else
    {
        ::SetCursor(nullptr);
    }
    m_cursor = std::nullopt;
}

void context::render(ui::layer layer) noexcept
{
    namespace view = std::ranges::views;

    m_vertex_buffer.finalize();
    m_index_buffer.finalize();

    m_d3d_device->BeginScene();

    m_d3d_device->CaptureStateBlock(m_previous_state);
    m_d3d_device->ApplyStateBlock(m_default_state);

    if (m_screen.layer() == layer)
    {
        m_screen.commands().execute(m_d3d_device);
    }

    if (m_layout_grid.layer() == layer)
    {
        m_layout_grid.commands().execute(m_d3d_device);
    }

    for (auto& window : m_window_manager.z_order(layer) | view::reverse)
    {
        window->commands().execute(m_d3d_device);
    }

    m_d3d_device->ApplyStateBlock(m_previous_state);

    m_d3d_device->EndScene();
}

mouse& context::mouse() noexcept { return m_mouse; }

texture_cache& context::texture_cache() noexcept { return m_texture_cache; }

text_layout_engine& context::text_layout_engine() noexcept
{
    return m_text_layout_engine;
}

text_rasterizer& context::text_rasterizer() noexcept
{
    return m_text_rasterizer;
}

gsl::not_null<::IDirect3DDevice8*> context::d3d_device() noexcept
{
    return m_d3d_device;
}

gsl::not_null<::IWICImagingFactory*> context::wic_factory() noexcept
{
    return m_wic_factory.get();
}

gsl::not_null<::IDWriteFactory*> context::dwrite_factory() noexcept
{
    return m_dwrite_factory.get();
}

context::layer_descriptor& context::descriptor(layer layer) noexcept
{
    auto const layer_index = gsl::narrow_cast<std::size_t>(layer);
    return gsl::at(m_layers, layer_index);
}

context::layer_descriptor const& context::descriptor(layer layer) const noexcept
{
    auto const layer_index = gsl::narrow_cast<std::size_t>(layer);
    return gsl::at(m_layers, layer_index);
}

command_buffer& context::commands() noexcept
{
    return m_window_stack.back().window->commands();
}

void context::draw_layout_grid() noexcept
{
    static constexpr auto guide = nine_patch{{10, 126, 74, 190}, {20}, {18}};

    auto const layer = m_layout_grid.layer();
    auto const depth = m_layout_grid.depth();
    auto const rhw   = calculate_rhw(depth);

    auto const& texture = load_texture(*this, u8":layout-grid", 1);
    auto const& size    = descriptor(layer).size;
    auto const& scale   = descriptor(layer).scale_factor;

    auto const w = std::round(size.width / scale.x);
    auto const h = std::round(size.height / scale.y);

    auto const grid_w0 = std::round(w / 2);
    auto const grid_w1 = w - grid_w0;
    auto const grid_h0 = std::round(h / 2);
    auto const grid_h1 = h - grid_h0;

    auto const origin_x = grid_w0;
    auto const origin_y = grid_h0;

    auto const grid_x0 = (origin_x - grid_w0) * scale.x - .5f;
    auto const grid_x1 = origin_x * scale.x - .5f;
    auto const grid_x2 = (origin_x + grid_w1) * scale.x - .5f;
    auto const grid_y0 = (origin_y - grid_h0) * scale.y - .5f;
    auto const grid_y1 = origin_y * scale.y - .5f;
    auto const grid_y2 = (origin_y + grid_h1) * scale.y - .5f;

    auto const tx0 = grid_w0 / 64.f;
    auto const tx1 = grid_w1 / 64.f;
    auto const ty0 = grid_h0 / 64.f;
    auto const ty1 = grid_h1 / 64.f;

    auto& commands = m_layout_grid.commands();

    m_layout_grid.commands().emplace<set_clip_command>(
        rectangle{{}, vector{size}});

    commands.emplace<set_texture_wrap_command>(true);
    commands.emplace<set_texture_command>(texture.token.m_value);

    auto v = m_vertex_buffer.allocate(m_d3d_device, commands, 9);
    auto i = m_index_buffer.allocate(m_d3d_device, commands, 24);
    commands.emplace<draw_triangle_list_command>(
        gsl::narrow_cast<std::uint16_t>(v.offset),
        gsl::narrow_cast<std::uint16_t>(v.data.size()),
        gsl::narrow_cast<std::uint16_t>(i.offset),
        gsl::narrow_cast<std::uint16_t>(i.data.size()));

    gsl::at(v.data, v.offset + 0) = {grid_x0, grid_y0, depth, rhw, tx0, ty0};
    gsl::at(v.data, v.offset + 1) = {grid_x1, grid_y0, depth, rhw, 0.f, ty0};
    gsl::at(v.data, v.offset + 2) = {grid_x2, grid_y0, depth, rhw, tx1, ty0};
    gsl::at(v.data, v.offset + 3) = {grid_x0, grid_y1, depth, rhw, tx0, 0.f};
    gsl::at(v.data, v.offset + 4) = {grid_x1, grid_y1, depth, rhw, 0.f, 0.f};
    gsl::at(v.data, v.offset + 5) = {grid_x2, grid_y1, depth, rhw, tx1, 0.f};
    gsl::at(v.data, v.offset + 6) = {grid_x0, grid_y2, depth, rhw, tx0, ty1};
    gsl::at(v.data, v.offset + 7) = {grid_x1, grid_y2, depth, rhw, 0.f, ty1};
    gsl::at(v.data, v.offset + 8) = {grid_x2, grid_y2, depth, rhw, tx1, ty1};

    gsl::at(i.data, i.offset + 0)  = 0;
    gsl::at(i.data, i.offset + 1)  = 1;
    gsl::at(i.data, i.offset + 2)  = 3;
    gsl::at(i.data, i.offset + 3)  = 4;
    gsl::at(i.data, i.offset + 4)  = 3;
    gsl::at(i.data, i.offset + 5)  = 1;
    gsl::at(i.data, i.offset + 6)  = 1;
    gsl::at(i.data, i.offset + 7)  = 2;
    gsl::at(i.data, i.offset + 8)  = 4;
    gsl::at(i.data, i.offset + 9)  = 5;
    gsl::at(i.data, i.offset + 10) = 4;
    gsl::at(i.data, i.offset + 11) = 2;
    gsl::at(i.data, i.offset + 12) = 3;
    gsl::at(i.data, i.offset + 13) = 4;
    gsl::at(i.data, i.offset + 14) = 6;
    gsl::at(i.data, i.offset + 15) = 7;
    gsl::at(i.data, i.offset + 16) = 6;
    gsl::at(i.data, i.offset + 17) = 4;
    gsl::at(i.data, i.offset + 18) = 4;
    gsl::at(i.data, i.offset + 19) = 5;
    gsl::at(i.data, i.offset + 20) = 7;
    gsl::at(i.data, i.offset + 21) = 8;
    gsl::at(i.data, i.offset + 22) = 7;
    gsl::at(i.data, i.offset + 23) = 5;

    commands.emplace<set_texture_wrap_command>(false);

    auto& context  = m_window_stack.emplace_back();
    context.window = &m_layout_grid;
    context.transform_stack.push_back(transform::identity());
    context.offset_stack.emplace_back();
    context.clip_stack.emplace_back();

    primitive::set_texture(*this, u8":system");

    if (m_client_shared && m_client_log1 && m_client_log2)
    {
        auto const log1_w = m_client_log1->width + 14.f;
        auto const log1_h = m_client_log1->max_lines * 16 + 6.f;

        auto const log2_w = m_client_log2->width + 14.f;
        auto const log2_h = m_client_log2->max_lines * 16 + 6.f;

        constexpr auto x0 = 16.f;
        auto const y1     = h - 16.f;
        auto const x1     = x0 + log1_w;
        auto const y0     = y1 - log1_h;

        auto const x2 = m_client_shared->multi_window == 1 ? x0 : x1;
        auto const y3 = m_client_shared->multi_window == 1 ? y0 : y1;
        auto const x3 = x2 + log2_w;
        auto const y2 = y3 - log2_h;

        auto const x4 = m_client_shared->multi_window == 1 ? x2 : x1;
        auto const y5 = m_client_shared->multi_window == 1 ? y2 : x1;
        auto const x5 = x4 + 366;
        auto const y4 = y5 - 200;

        // Chat Log 1
        primitive::rectangle(*this, {x0, y0, x1, y1}, guide);

        // Chat Log 2
        if (m_client_shared->multi_window != 0)
        {
            primitive::rectangle(*this, {x2, y2, x3, y3}, guide);
        }

        // Menus & Tooltip
        primitive::rectangle(*this, {x4, y4, x5, y5}, guide);
    }

    // Party, Alliance & Target
    primitive::rectangle(*this, {w - 128, h - 150, w - 16, h - 16}, guide);
    primitive::rectangle(*this, {w - 128, h - 196, w - 16, h - 152}, guide);
    primitive::rectangle(*this, {w - 128, h - 298, w - 16, h - 198}, guide);
    primitive::rectangle(*this, {w - 128, h - 400, w - 16, h - 300}, guide);

    // Info
    primitive::rectangle(*this, {16, 16, w - 16, 46}, guide);

    // Status, Equipment & Tooltips
    primitive::rectangle(*this, {16, 48, 128, 238}, guide);
    primitive::rectangle(*this, {130, 240, 496, 440}, guide);
    primitive::rectangle(*this, {130, 48, 312, 238}, guide);
    primitive::rectangle(*this, {314, 48, 496, 238}, guide);
    primitive::rectangle(*this, {16, 240, 128, 344}, guide);

    // Menu
    primitive::rectangle(*this, {w - 128, 48, w - 16, 378}, guide);
    primitive::rectangle(*this, {w - 223, 48, w - 127, 378}, guide);

    // Macros
    if (w > 1138)
    {
        primitive::rectangle(*this, {498, 48, 1008, 132}, guide);
    }
    else if (w > 630)
    {
        primitive::rectangle(*this, {498, 48, w - 130, 132}, guide);
    }

    m_window_stack.pop_back();
}

void context::load_colors(
    std::u8string_view skin, std::size_t offset, std::size_t count) noexcept
{
    auto const skin_bitmap = bitmap::load(*this, skin);
    auto const& patch      = skin_bitmap.patch();

    auto const w = patch.texture_size.width;
    auto const h = patch.texture_size.height;

    auto const rows = gsl::narrow_cast<std::size_t>(h / 4);
    auto const cols = gsl::narrow_cast<std::size_t>(w / 4);
    auto const max  = std::min(rows * cols, count);

    for (auto i = 0u; i < max; ++i)
    {
        auto const x = (4 * (i / rows) + 2);
        auto const y = (4 * (i % rows) + 2);

        gsl::at(m_colors, (i + offset) % m_colors.size()) =
            skin_bitmap.sample(*this, {x / w, y / h});
    }

    for (auto i = max; i < count; ++i)
    {
        gsl::at(m_colors, (i + offset) % m_colors.size()) = colors::magenta;
    }
}

}
