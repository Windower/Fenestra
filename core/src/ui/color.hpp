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

#ifndef WINDOWER_UI_COLOR_HPP
#define WINDOWER_UI_COLOR_HPP

#include <d3d8.h>

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>

namespace windower::ui
{

struct color
{
    std::uint8_t b = 0;
    std::uint8_t g = 0;
    std::uint8_t r = 0;
    std::uint8_t a = 255;

    constexpr color() noexcept = default;

    constexpr color(
        std::uint8_t r, std::uint8_t g, std::uint8_t b,
        std::uint8_t a = 255) noexcept :
        b{b},
        g{g}, r{r}, a{a}
    {}

    explicit constexpr color(std::uint32_t argb) noexcept :
        b{(argb >> 0) & 0xFF}, g{(argb >> 8) & 0xFF}, r{(argb >> 16) & 0xFF},
        a{(argb >> 24) & 0xFF}
    {}

    explicit constexpr color(std::int32_t argb) noexcept :
        color{std::bit_cast<std::uint32_t>(argb)}
    {}

    explicit constexpr operator std::uint32_t() const noexcept
    {
        return b | g << 8 | r << 16 | a << 24;
    }

    explicit constexpr operator std::int32_t() const noexcept
    {
        return std::bit_cast<std::int32_t>(std::uint32_t{*this});
    }

    constexpr operator ::D3DCOLORVALUE() const noexcept
    {
        return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    }

    constexpr float hue() const noexcept
    {
        auto const value = std::max(std::max(r, g), b);
        auto const min   = std::min(std::min(r, g), b);

        auto const chroma = value - min;
        auto hue          = 0.f;

        if (chroma != 0)
        {
            if (value == r)
            {
                hue = 60.f * (g - b) / chroma;
            }
            else if (value == g)
            {
                hue = 60.f * (b - r) / chroma + 120.f;
            }
            else
            {
                hue = 60.f * (r - g) / chroma + 240.f;
            }
        }

        return (hue < 0.f) ? hue + 360.f : hue;
    }

    constexpr float chroma() const noexcept
    {
        return (std::max(std::max(r, g), b) - std::min(std::min(r, g), b)) /
               255.f;
    }

    constexpr float value() const noexcept
    {
        return std::max(std::max(r, g), b) / 255.f;
    }

    constexpr float saturation() const noexcept
    {
        auto const value = std::max(std::max(r, g), b);
        if (value == 0)
        {
            return 0.f;
        }
        return 1.f - float(std::min(std::min(r, g), b)) / value;
    }
};

constexpr bool operator==(color lhs, color rhs) noexcept
{
    return lhs.b == rhs.b && lhs.g == rhs.g && lhs.r == rhs.r && lhs.a == rhs.a;
}

constexpr bool operator!=(color lhs, color rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr color to_associated_alpha(color c) noexcept
{
    auto const a = c.a / 255.f;

    auto const temp_r = std::clamp(std::round(c.r * a), 0.f, 255.f);
    auto const temp_g = std::clamp(std::round(c.g * a), 0.f, 255.f);
    auto const temp_b = std::clamp(std::round(c.b * a), 0.f, 255.f);

    auto const uint8_r = gsl::narrow_cast<std::uint8_t>(temp_r);
    auto const uint8_g = gsl::narrow_cast<std::uint8_t>(temp_g);
    auto const uint8_b = gsl::narrow_cast<std::uint8_t>(temp_b);

    return {uint8_r, uint8_g, uint8_b, c.a};
}

constexpr color to_straight_alpha(color c) noexcept
{
    auto const inv_a = 255.f / c.a;

    auto const temp_r = std::clamp(std::round(c.r * inv_a), 0.f, 255.f);
    auto const temp_g = std::clamp(std::round(c.g * inv_a), 0.f, 255.f);
    auto const temp_b = std::clamp(std::round(c.b * inv_a), 0.f, 255.f);

    auto const uint8_r = gsl::narrow_cast<std::uint8_t>(temp_r);
    auto const uint8_g = gsl::narrow_cast<std::uint8_t>(temp_g);
    auto const uint8_b = gsl::narrow_cast<std::uint8_t>(temp_b);

    return {uint8_r, uint8_g, uint8_b, c.a};
}

constexpr color fade(color c, std::uint8_t a, bool associated = false) noexcept
{
    if (associated)
    {
        auto const scale = gsl::narrow_cast<float>(a) / c.a;

        auto const temp_r = std::clamp(std::round(c.r * scale), 0.f, 255.f);
        auto const temp_g = std::clamp(std::round(c.g * scale), 0.f, 255.f);
        auto const temp_b = std::clamp(std::round(c.b * scale), 0.f, 255.f);

        auto const uint8_r = gsl::narrow_cast<std::uint8_t>(temp_r);
        auto const uint8_g = gsl::narrow_cast<std::uint8_t>(temp_g);
        auto const uint8_b = gsl::narrow_cast<std::uint8_t>(temp_b);

        return {uint8_r, uint8_g, uint8_b, a};
    }

    return color{c.r, c.g, c.b, a};
}

constexpr color
hcv(float h, float c, float v, std::uint8_t a = 255,
    bool associated = false) noexcept
{
    v = v * 255.f;
    c *= 255.f;
    auto const m       = v - c;
    auto const h_prime = h * 0.016666666666666666f;

    auto r = 0.f;
    auto g = 0.f;
    auto b = 0.f;
    if (h_prime < 1.f)
    {
        r = m + c;
        g = m + c * h_prime;
        b = m;
    }
    else if (h_prime < 2.f)
    {
        r = m + c * (2.f - h_prime);
        g = m + c;
        b = m;
    }
    else if (h_prime < 3.f)
    {
        r = m;
        g = m + c;
        b = m + c * (h_prime - 2.f);
    }
    else if (h_prime < 4.f)
    {
        r = m;
        g = m + c * (4.f - h_prime);
        b = m + c;
    }
    else if (h_prime < 5.f)
    {
        r = m + c * (h_prime - 4.f);
        g = m;
        b = m + c;
    }
    else
    {
        r = m + c;
        g = m;
        b = m + c * (6.f - h_prime);
    }

    r = std::clamp(std::round(r), 0.f, 255.f);
    g = std::clamp(std::round(g), 0.f, 255.f);
    b = std::clamp(std::round(b), 0.f, 255.f);

    auto const uint8_r = gsl::narrow_cast<std::uint8_t>(r);
    auto const uint8_g = gsl::narrow_cast<std::uint8_t>(g);
    auto const uint8_b = gsl::narrow_cast<std::uint8_t>(b);

    if (associated)
    {
        return to_associated_alpha({uint8_r, uint8_g, uint8_b, a});
    }

    return {uint8_r, uint8_g, uint8_b, a};
}

constexpr color
hsv(float h, float s, float v, std::uint8_t a = 255,
    bool associated = false) noexcept
{
    return hcv(h, s * v, v, a, associated);
}

constexpr color modulate(color a, color b) noexcept
{
    auto const temp_r = std::clamp(std::round(a.r * b.r / 255.f), 0.f, 255.f);
    auto const temp_g = std::clamp(std::round(a.g * b.g / 255.f), 0.f, 255.f);
    auto const temp_b = std::clamp(std::round(a.b * b.b / 255.f), 0.f, 255.f);
    auto const temp_a = std::clamp(std::round(a.a * b.a / 255.f), 0.f, 255.f);

    auto const uint8_r = gsl::narrow_cast<std::uint8_t>(temp_r);
    auto const uint8_g = gsl::narrow_cast<std::uint8_t>(temp_g);
    auto const uint8_b = gsl::narrow_cast<std::uint8_t>(temp_b);
    auto const uint8_a = gsl::narrow_cast<std::uint8_t>(temp_a);

    return {uint8_r, uint8_g, uint8_b, uint8_a};
}

constexpr color
lerp(color a, color b, float t, bool associated = false) noexcept
{
    auto a_r = gsl::narrow_cast<float>(a.r);
    auto a_g = gsl::narrow_cast<float>(a.g);
    auto a_b = gsl::narrow_cast<float>(a.b);
    auto a_a = gsl::narrow_cast<float>(a.a);

    auto b_r = gsl::narrow_cast<float>(b.r);
    auto b_g = gsl::narrow_cast<float>(b.g);
    auto b_b = gsl::narrow_cast<float>(b.b);
    auto b_a = gsl::narrow_cast<float>(b.a);

    if (!associated)
    {
        auto const a_scale = a_a / 255.f;
        a_r *= a_scale;
        a_g *= a_scale;
        a_b *= a_scale;

        auto const b_scale = b_a / 255.f;
        b_r *= b_scale;
        b_g *= b_scale;
        b_b *= b_scale;
    }

    auto temp_r = std::lerp(a_r, b_r, t);
    auto temp_g = std::lerp(a_g, b_g, t);
    auto temp_b = std::lerp(a_b, b_b, t);
    auto temp_a = std::lerp(a_a, b_a, t);

    if (!associated)
    {
        auto const scale = 255.f / temp_a;
        temp_r *= scale;
        temp_g *= scale;
        temp_b *= scale;
    }

    temp_r = std::clamp(std::round(temp_r), 0.f, 255.f);
    temp_g = std::clamp(std::round(temp_g), 0.f, 255.f);
    temp_b = std::clamp(std::round(temp_b), 0.f, 255.f);
    temp_a = std::clamp(std::round(temp_a), 0.f, 255.f);

    auto const uint8_r = gsl::narrow_cast<std::uint8_t>(temp_r);
    auto const uint8_g = gsl::narrow_cast<std::uint8_t>(temp_g);
    auto const uint8_b = gsl::narrow_cast<std::uint8_t>(temp_b);
    auto const uint8_a = gsl::narrow_cast<std::uint8_t>(temp_a);

    return {uint8_r, uint8_g, uint8_b, uint8_a};
}

namespace colors
{

// "CSS Color Level 4" colors
constexpr color transparent          = {0, 0, 0, 0};
constexpr color aliceblue            = {240, 248, 255};
constexpr color antiquewhite         = {250, 235, 215};
constexpr color aqua                 = {0, 255, 255};
constexpr color aquamarine           = {127, 255, 212};
constexpr color azure                = {240, 255, 255};
constexpr color beige                = {245, 245, 220};
constexpr color bisque               = {255, 228, 196};
constexpr color black                = {0, 0, 0};
constexpr color blanchedalmond       = {255, 235, 205};
constexpr color blue                 = {0, 0, 255};
constexpr color blueviolet           = {138, 43, 226};
constexpr color brown                = {165, 42, 42};
constexpr color burlywood            = {222, 184, 135};
constexpr color cadetblue            = {95, 158, 160};
constexpr color chartreuse           = {127, 255, 0};
constexpr color chocolate            = {210, 105, 30};
constexpr color coral                = {255, 127, 80};
constexpr color cornflowerblue       = {100, 149, 237};
constexpr color cornsilk             = {255, 248, 220};
constexpr color crimson              = {220, 20, 60};
constexpr color cyan                 = {0, 255, 255};
constexpr color darkblue             = {0, 0, 139};
constexpr color darkcyan             = {0, 139, 139};
constexpr color darkgoldenrod        = {184, 134, 11};
constexpr color darkgray             = {169, 169, 169};
constexpr color darkgreen            = {0, 100, 0};
constexpr color darkgrey             = {169, 169, 169};
constexpr color darkkhaki            = {189, 183, 107};
constexpr color darkmagenta          = {139, 0, 139};
constexpr color darkolivegreen       = {85, 107, 47};
constexpr color darkorange           = {255, 140, 0};
constexpr color darkorchid           = {153, 50, 204};
constexpr color darkred              = {139, 0, 0};
constexpr color darksalmon           = {233, 150, 122};
constexpr color darkseagreen         = {143, 188, 143};
constexpr color darkslateblue        = {72, 61, 139};
constexpr color darkslategray        = {47, 79, 79};
constexpr color darkslategrey        = {47, 79, 79};
constexpr color darkturquoise        = {0, 206, 209};
constexpr color darkviolet           = {148, 0, 211};
constexpr color deeppink             = {255, 20, 147};
constexpr color deepskyblue          = {0, 191, 255};
constexpr color dimgray              = {105, 105, 105};
constexpr color dimgrey              = {105, 105, 105};
constexpr color dodgerblue           = {30, 144, 255};
constexpr color firebrick            = {178, 34, 34};
constexpr color floralwhite          = {255, 250, 240};
constexpr color forestgreen          = {34, 139, 34};
constexpr color fuchsia              = {255, 0, 255};
constexpr color gainsboro            = {220, 220, 220};
constexpr color ghostwhite           = {248, 248, 255};
constexpr color gold                 = {255, 215, 0};
constexpr color goldenrod            = {218, 165, 32};
constexpr color gray                 = {128, 128, 128};
constexpr color green                = {0, 128, 0};
constexpr color greenyellow          = {173, 255, 47};
constexpr color grey                 = {128, 128, 128};
constexpr color honeydew             = {240, 255, 240};
constexpr color hotpink              = {255, 105, 180};
constexpr color indianred            = {205, 92, 92};
constexpr color indigo               = {75, 0, 130};
constexpr color ivory                = {255, 255, 240};
constexpr color khaki                = {240, 230, 140};
constexpr color lavender             = {230, 230, 250};
constexpr color lavenderblush        = {255, 240, 245};
constexpr color lawngreen            = {124, 252, 0};
constexpr color lemonchiffon         = {255, 250, 205};
constexpr color lightblue            = {173, 216, 230};
constexpr color lightcoral           = {240, 128, 128};
constexpr color lightcyan            = {224, 255, 255};
constexpr color lightgoldenrodyellow = {250, 250, 210};
constexpr color lightgray            = {211, 211, 211};
constexpr color lightgreen           = {144, 238, 144};
constexpr color lightgrey            = {211, 211, 211};
constexpr color lightpink            = {255, 182, 193};
constexpr color lightsalmon          = {255, 160, 122};
constexpr color lightseagreen        = {32, 178, 170};
constexpr color lightskyblue         = {135, 206, 250};
constexpr color lightslategray       = {119, 136, 153};
constexpr color lightslategrey       = {119, 136, 153};
constexpr color lightsteelblue       = {176, 196, 222};
constexpr color lightyellow          = {255, 255, 224};
constexpr color lime                 = {0, 255, 0};
constexpr color limegreen            = {50, 205, 50};
constexpr color linen                = {250, 240, 230};
constexpr color magenta              = {255, 0, 255};
constexpr color maroon               = {128, 0, 0};
constexpr color mediumaquamarine     = {102, 205, 170};
constexpr color mediumblue           = {0, 0, 205};
constexpr color mediumorchid         = {186, 85, 211};
constexpr color mediumpurple         = {147, 112, 219};
constexpr color mediumseagreen       = {60, 179, 113};
constexpr color mediumslateblue      = {123, 104, 238};
constexpr color mediumspringgreen    = {0, 250, 154};
constexpr color mediumturquoise      = {72, 209, 204};
constexpr color mediumvioletred      = {199, 21, 133};
constexpr color midnightblue         = {25, 25, 112};
constexpr color mintcream            = {245, 255, 250};
constexpr color mistyrose            = {255, 228, 225};
constexpr color moccasin             = {255, 228, 181};
constexpr color navajowhite          = {255, 222, 173};
constexpr color navy                 = {0, 0, 128};
constexpr color oldlace              = {253, 245, 230};
constexpr color olive                = {128, 128, 0};
constexpr color olivedrab            = {107, 142, 35};
constexpr color orange               = {255, 165, 0};
constexpr color orangered            = {255, 69, 0};
constexpr color orchid               = {218, 112, 214};
constexpr color palegoldenrod        = {238, 232, 170};
constexpr color palegreen            = {152, 251, 152};
constexpr color paleturquoise        = {175, 238, 238};
constexpr color palevioletred        = {219, 112, 147};
constexpr color papayawhip           = {255, 239, 213};
constexpr color peachpuff            = {255, 218, 185};
constexpr color peru                 = {205, 133, 63};
constexpr color pink                 = {255, 192, 203};
constexpr color plum                 = {221, 160, 221};
constexpr color powderblue           = {176, 224, 230};
constexpr color purple               = {128, 0, 128};
constexpr color rebeccapurple        = {102, 51, 153};
constexpr color red                  = {255, 0, 0};
constexpr color rosybrown            = {188, 143, 143};
constexpr color royalblue            = {65, 105, 225};
constexpr color saddlebrown          = {139, 69, 19};
constexpr color salmon               = {250, 128, 114};
constexpr color sandybrown           = {244, 164, 96};
constexpr color seagreen             = {46, 139, 87};
constexpr color seashell             = {255, 245, 238};
constexpr color sienna               = {160, 82, 45};
constexpr color silver               = {192, 192, 192};
constexpr color skyblue              = {135, 206, 235};
constexpr color slateblue            = {106, 90, 205};
constexpr color slategray            = {112, 128, 144};
constexpr color slategrey            = {112, 128, 144};
constexpr color snow                 = {255, 250, 250};
constexpr color springgreen          = {0, 255, 127};
constexpr color steelblue            = {70, 130, 180};
constexpr color tan                  = {210, 180, 140};
constexpr color teal                 = {0, 128, 128};
constexpr color thistle              = {216, 191, 216};
constexpr color tomato               = {255, 99, 71};
constexpr color turquoise            = {64, 224, 208};
constexpr color violet               = {238, 130, 238};
constexpr color wheat                = {245, 222, 179};
constexpr color white                = {255, 255, 255};
constexpr color whitesmoke           = {245, 245, 245};
constexpr color yellow               = {255, 255, 0};
constexpr color yellowgreen          = {154, 205, 50};

}

}

#endif
