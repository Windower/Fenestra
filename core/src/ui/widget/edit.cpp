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

#include "ui/widget/edit.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/mouse_button.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/scroll_panel.hpp"
#include "unicode.hpp"

namespace windower::ui::widget
{

namespace
{

class ime_context
{
public:
    ime_context(ime_context const&) = delete;
    ime_context(ime_context&&)      = delete;
    ime_context(gsl::not_null<::HWND> hwnd) noexcept :
        m_hwnd{hwnd}, m_himc{::ImmGetContext(hwnd)}
    {}

    ~ime_context()
    {
        if (m_himc)
        {
            ::ImmReleaseContext(m_hwnd, m_himc);
        }
    }

    ime_context& operator=(ime_context const&) = delete;
    ime_context& operator=(ime_context&&)      = delete;

    explicit operator bool() const noexcept { return m_himc != nullptr; }
    operator ::HIMC() const noexcept { return m_himc; }

private:
    ::HWND m_hwnd = nullptr;
    ::HIMC m_himc = nullptr;
};

class edit_focus_state
{
public:
    edit_focus_state() noexcept                        = default;
    edit_focus_state(edit_focus_state const&) noexcept = default;
    edit_focus_state(edit_focus_state&&) noexcept      = default;

    ~edit_focus_state() noexcept = default;

    edit_focus_state& operator=(edit_focus_state const&) noexcept = default;
    edit_focus_state& operator=(edit_focus_state&&) noexcept      = default;

    char8_t const* text_data = nullptr;
    std::size_t text_size    = 0;
    bool text_changed        = false;
    std::u8string text;
    windower::ui::rectangle bounds;
    windower::ui::text_layout layout;
    wchar_t high_surrogate          = 0;
    bool composition_string_changed = false;
    std::u8string composition_string;
    std::size_t composition_begin = 0;
    std::size_t composition_end   = 0;

    std::optional<::LRESULT> process_message(::MSG const& msg) noexcept
    {
        switch (msg.message)
        {
        default: return std::nullopt;
        case WM_KEYDOWN: return default_handler(msg);
        case WM_KEYUP: return default_handler(msg);
        case WM_CHAR: insert(gsl::narrow_cast<wchar_t>(msg.wParam)); return 0;
        case WM_IME_STARTCOMPOSITION: return ime_start_composition(msg);
        case WM_IME_ENDCOMPOSITION: return ime_end_composition(msg);
        case WM_IME_COMPOSITION: return ime_update_composition(msg);
        case WM_IME_SETCONTEXT: return ime_set_context(msg);
        case WM_IME_NOTIFY: return default_handler(msg);
        case WM_IME_CONTROL: return default_handler(msg);
        case WM_IME_COMPOSITIONFULL: return default_handler(msg);
        case WM_IME_SELECT: return default_handler(msg);
        case WM_IME_CHAR: return default_handler(msg);
        case WM_IME_REQUEST: return ime_request(msg);
        case WM_IME_KEYDOWN: return ime_key_down(msg);
        case WM_IME_KEYUP: return default_handler(msg);
        }
    }

    void insert(char32_t code_point) noexcept
    {
        switch (code_point)
        {
        case U'\b': {
            auto c = char8_t{};
            do
            {
                if (text.empty())
                {
                    return;
                }
                c = text.back();
                text.pop_back();
            }
            while (c >= 0x80 && c <= 0xBF);
            break;
        }
        default: append(text, code_point); break;
        }
        text_changed = true;
    }

    void insert(wchar_t code_unit) noexcept
    {
        if (code_unit >= 0xD800 && code_unit <= 0xDBFF)
        {
            high_surrogate = code_unit;
        }
        else
        {
            auto code_point = char32_t{};
            if (code_unit >= 0xDC00 && code_unit <= 0xDFFF)
            {
                auto code_units = std::array{high_surrogate, code_unit};
                auto index      = std ::size_t{};
                code_point      = next_code_point(
                    {code_units.data(), code_units.size()}, index);
            }
            else
            {
                code_point = code_unit;
            }
            insert(code_point);
        }
    }

    void insert(std::u8string_view) noexcept {}

    void update_composition_string(::HIMC himc, ::DWORD index) noexcept
    {
        auto const buffer_size =
            ::ImmGetCompositionStringW(himc, index, nullptr, 0);
        std::wstring buffer;
        buffer.resize((buffer_size + sizeof(::WCHAR) - 1) / sizeof(::WCHAR));
        ::ImmGetCompositionStringW(himc, index, buffer.data(), buffer_size);
        // composition_string.clear();
        // composition_string.reserve(buffer.size());
        composition_string = to_u8string(buffer);
    }

    ::LRESULT ime_start_composition(::MSG const&) noexcept { return 0; }

    ::LRESULT ime_end_composition(::MSG const& msg) noexcept
    {
        return ::DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    ::LRESULT ime_update_composition(::MSG const& msg) noexcept
    {
        auto context = ime_context{msg.hwnd};

        if (msg.lParam & GCS_RESULTSTR)
        {
            update_composition_string(context, GCS_RESULTSTR);
            insert(composition_string);
            composition_string.clear();
        }

        if (msg.lParam & GCS_COMPSTR)
        {
            update_composition_string(context, GCS_COMPSTR);
        }

        return 0;
    }

    ::LRESULT ime_set_context(::MSG const& msg) noexcept
    {
        auto const lParam = msg.lParam & ~ISC_SHOWUICOMPOSITIONWINDOW;
        return ::DefWindowProcW(msg.hwnd, msg.message, msg.wParam, lParam);
    }

    ::LRESULT ime_request(::MSG const& msg) noexcept
    {
        if (msg.wParam == IMR_RECONVERTSTRING) {}
        return ::DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    ::LRESULT ime_key_down(::MSG const& msg) noexcept
    {
        if (msg.wParam == VK_HANJA) {}
        return ::DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    ::LRESULT default_handler(::MSG const& msg) noexcept
    {
        return ::DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }
};

constexpr auto normal   = nine_patch{{107, 51, 121, 77}, {4}};
constexpr auto disabled = nine_patch{{123, 51, 137, 77}, {4}};

}

void edit_state::text(std::u8string_view text) noexcept
{
    text_data = text.data();
    text_size = text.size();
}

std::u8string_view edit_state::text() const noexcept
{
    return {text_data, text_size};
}

void basic_edit(context& ctx, id id, edit_state& state) noexcept
{
    ctx.use_id(id);

    auto const position = ctx.bounds().position();

    if (ctx.enabled())
    {
        auto const hot = ctx.hit_test(id);

        auto const& mouse = ctx.mouse();
        auto const mouse_position =
            ctx.to_widget(mouse.position()).value_or(vector{});

        if (mouse.was_pressed(mouse_button::left))
        {
            if (!ctx.is_focused(id) && hot)
            {
                auto const ptr = ctx.focus<edit_focus_state>(id);
                ptr->text      = state.text();
            }
        }

        auto const ptr = ctx.focus_state<edit_focus_state>(id);
        if (ptr)
        {
            /*if (ptr->text_data != state.text_data ||
                ptr->text_size != state.text_size)
            {
                ptr->text         = {state.text_data, state.text_size};
                ptr->text_data    = ptr->text.data();
                ptr->text_size    = ptr->text.size();
                ptr->text_changed = false;
            }
            else */
            if (ptr->text_changed)
            {
                ptr->text_data     = ptr->text.data();
                ptr->text_size     = ptr->text.size();
                ptr->text_changed  = false;
                state.text_data    = ptr->text_data;
                state.text_size    = ptr->text_size;
                state.text_changed = true;
            }

            ptr->layout = primitive::layout_text(
                ctx, {dimension::unbounded, dimension::unbounded},
                {state.text_data, state.text_size});

            auto const& metrics = ptr->layout.metric_bounds();

            auto const w = metrics.x1 + 8;
            auto const h = metrics.y1;

            scroll_panel_state scroll_state{};
            scroll_state.visibility_horizontal = scroll_bar_visibility::hidden;
            scroll_state.visibility_vertical   = scroll_bar_visibility::hidden;
            scroll_state.canvas_size.width     = w;
            scroll_state.canvas_size.height    = h;
            scroll_state.offset.x = std::max(w - ctx.bounds().width(), 0.f);
            begin_scroll_panel(ctx, id, scroll_state);

            primitive::text(
                ctx, {4, 0}, ptr->layout,
                {.flags = text_rasterization_flags::show_cursor});

            end_scroll_panel(ctx);
        }
        else
        {
            auto const layout = primitive::layout_text(
                ctx, {dimension::unbounded, dimension::unbounded},
                {state.text_data, state.text_size});

            auto const& metrics = layout.metric_bounds();

            auto const w = metrics.x1 + 8;
            auto const h = metrics.y1;

            scroll_panel_state scroll_state{};
            scroll_state.visibility_horizontal = scroll_bar_visibility::hidden;
            scroll_state.visibility_vertical   = scroll_bar_visibility::hidden;
            scroll_state.canvas_size.width     = w;
            scroll_state.canvas_size.height    = h;
            scroll_state.offset.x = std::max(w - ctx.bounds().width(), 0.f);
            begin_scroll_panel(ctx, id, scroll_state);

            primitive::text(
                ctx, {4, 0}, layout, {.flags = text_rasterization_flags::none});

            end_scroll_panel(ctx);
        }
    }
    else
    {
        ctx.blur(id);
    }
}

void edit(context& ctx, id id, edit_state& state) noexcept
{
    auto const enabled = ctx.enabled();

    primitive::set_texture(ctx, ctx.skin());
    if (!enabled)
    {
        primitive::rectangle(ctx, ctx.bounds(), disabled);
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), normal);
    }

    ctx.bounds(contract(ctx.bounds(), {1}));
    basic_edit(ctx, id, state);
}

}
