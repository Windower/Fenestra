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

#ifndef WINDOWER_VERSION_HPP
#define WINDOWER_VERSION_HPP

#define WINDOWER_VERSION_MAJOR 5
#define WINDOWER_VERSION_MINOR 0
#define WINDOWER_VERSION_BUILD 0
#define WINDOWER_VERSION_REV 0

#define WINDOWER_COPYRIGHT_NAME "Windower Dev Team"

// The following values are automatically generated from the
// settings above. You probably don't need to edit these.

#ifndef WINDOWER_BUILD_TAG
#    ifdef WINDOWER_RELEASE_BUILD
#        define WINDOWER_BUILD_TAG ""
#    else
#        define WINDOWER_BUILD_TAG "Development Build"
#    endif
#endif

#ifndef RC_INVOKED
#    define WINDOWER_UTF_8_2(x) u8##x
#    define WINDOWER_UTF_8(x) WINDOWER_UTF_8_2(x)
#else
#    define WINDOWER_UTF_8(x)
#    define __has_include(x) defined(WINDOWER_AUTO_VERSION)
#endif

#if WINDOWER_AUTO_VERSION && __has_include("version.auto.hpp")
#    include "version.auto.hpp"
#endif

#ifndef RC_INVOKED
#    define WINDOWER_COPYRIGHT_SYMBOL u8"\u00A9"
#else
#    define WINDOWER_COPYRIGHT_SYMBOL "\xA9"
#endif

// clang-format off
#ifndef WINDOWER_STRINGIFY
#    define WINDOWER_STRINGIFY_2(s) WINDOWER_UTF_8(#s)
#    define WINDOWER_STRINGIFY(s) WINDOWER_STRINGIFY_2(s)
#endif

#define WINDOWER_COPYRIGHT_STRING                                              \
    WINDOWER_UTF_8("Copyright ")                                               \
    WINDOWER_COPYRIGHT_SYMBOL WINDOWER_UTF_8(" ")                              \
    WINDOWER_UTF_8(WINDOWER_COPYRIGHT_NAME)

#define WINDOWER_VERSION_STRING                                                \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_MAJOR) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_MINOR) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_BUILD) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_REV)

#define WINDOWER_VERSION_BUILD_STRING                                          \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_BUILD) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_REV)

#define WINDOWER_BUILD_TAG_STRING WINDOWER_UTF_8(WINDOWER_BUILD_TAG)
// clang-format on

#endif
