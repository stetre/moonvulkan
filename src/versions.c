/* The MIT License (MIT)
 *
 * Copyright (c) 2017 Stefano Trettel
 *
 * Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"
    
static int HomeTown(lua_State *L)
    {
    lua_pushstring(L, "VkLlanfairPwllgwyngyllGogeryChwyrnDrobwllLlanTysilioGogoGoch");
    return 1;
    }

static int AddVersions(lua_State *L)
    {
    int i = 0;
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, "MoonVulkan "MOONVULKAN_VERSION);
    lua_settable(L, -3);

    lua_pushstring(L, "HEADER_VERSION");
    lua_pushinteger(L, VK_HEADER_VERSION);
    lua_settable(L, -3);

    /* supported versions --------------------------- */
    lua_pushstring(L, "API_VERSIONS");
    lua_newtable(L);
#define ADD(ver) do {           \
    lua_pushstring(L, ""#ver);  \
    lua_rawseti(L, -2, ++i);    \
    lua_pushstring(L, ""#ver);  \
    lua_pushinteger(L, VK_##ver);\
    lua_settable(L, -5);        \
} while(0)

#ifdef VK_API_VERSION_1_0
    ADD(API_VERSION_1_0);
#endif
#ifdef VK_API_VERSION_1_1
    ADD(API_VERSION_1_1);
#endif
#ifdef VK_API_VERSION_1_2
    ADD(API_VERSION_1_2);
#endif
/* @@ Add future versions like so:
#ifdef VK_API_VERSION_2_0
    ADD(API_VERSION_2_0);
#endif
...
*/
    lua_settable(L, -3);
#undef ADD
    return 0;
    }

static int Platforms(lua_State *L) //@@DOC
    {
    int i = 0;
    (void)i; /* prevent 'not used' warnings if no VK_USE_PLATFORM_XXX are defined */
    lua_newtable(L);
    lua_newtable(L);
#define ADD(platform) do {                          \
    lua_pushboolean(L, 1); lua_setfield(L, -2, platform);   \
    lua_pushstring(L, platform); lua_rawseti(L, -3, ++i);   \
} while(0)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    ADD("xlib");
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    ADD("xlib_xrandr");
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    ADD("xcb");
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    ADD("wayland");
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    ADD("android");
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ADD("win32");
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    ADD("fuchsia");
#endif
#ifdef VK_USE_PLATFORM_IOS_MVK
    ADD("ios_mvk");
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    ADD("macos_mvk");
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    ADD("metal");
#endif
#ifdef VK_USE_PLATFORM_VI_NN
    ADD("vi");
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    ADD("directfb");
#endif
#ifdef VK_USE_PLATFORM_GGP
    ADD("ggp");
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    ADD("screen_qnx");
#endif
#undef ADD
    return 2;
    }

static int VersionNumbers(lua_State *L)
/* major, minor, patch = version_numbers(ver) */
    {
    uint32_t ver = luaL_checkinteger(L, 1);
    lua_pushinteger(L, VK_API_VERSION_MAJOR(ver));
    lua_pushinteger(L, VK_API_VERSION_MINOR(ver));
    lua_pushinteger(L, VK_API_VERSION_PATCH(ver));
    lua_pushinteger(L, VK_API_VERSION_VARIANT(ver));
    return 4;
    }

static int VersionString(lua_State *L)
/* "major.minor.patch" = version_string(ver) */
    {
    uint32_t ver = luaL_checkinteger(L, 1);
    lua_pushfstring(L, "%d.%d.%d", 
        VK_VERSION_MAJOR(ver), VK_VERSION_MINOR(ver), VK_VERSION_PATCH(ver));
    return 1;
    }

static int MakeVersion(lua_State *L)
/* ver = make_version(major, minor, patch, [variant=0]) */
    {
    uint32_t major = luaL_checkinteger(L, 1);
    uint32_t minor = luaL_checkinteger(L, 2);
    uint32_t patch = luaL_checkinteger(L, 3);
    uint32_t variant = luaL_optinteger(L, 4, 0);
    uint32_t version = VK_MAKE_API_VERSION(variant, major, minor, patch);
    lua_pushinteger(L, version);
    return 1;
    }

static int EnumerateInstanceVersion(lua_State *L)
    {
    VkResult ec;
    uint32_t version = VK_API_VERSION_1_0;
    if(vk.EnumerateInstanceVersion)
        {
        ec = vk.EnumerateInstanceVersion(&version);
        CheckError(L, ec);
        }
    lua_pushinteger(L, version);
    return 1;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "hometown", HomeTown },
        { "version_numbers", VersionNumbers },
        { "version_string", VersionString },
        { "make_version", MakeVersion },
        { "platforms", Platforms },
        { "enumerate_instance_version", EnumerateInstanceVersion },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_versions(lua_State *L)
    {
    AddVersions(L);
    luaL_setfuncs(L, Functions, 0);
    }


