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

#include "ui/widget/image_button.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{

button_state image_button(
    context& ctx, id id, image_button_descriptor const& descriptor) noexcept
{
    auto const state = basic_button(ctx, id);

    primitive::set_texture(ctx, descriptor.image());
    if (!ctx.enabled())
    {
        primitive::rectangle(ctx, ctx.bounds(), descriptor.disabled());
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(descriptor.cursor());
        if (state.hot)
        {
            primitive::rectangle(ctx, ctx.bounds(), descriptor.active());
        }
        else
        {
            primitive::rectangle(ctx, ctx.bounds(), descriptor.hot());
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(descriptor.cursor());
        primitive::rectangle(ctx, ctx.bounds(), descriptor.hot());
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), descriptor.normal());
    }

    return state;
}

}
