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

static int freedisplay_mode(lua_State *L, ud_t *ud)
    {
    VkDisplayModeKHR display_mode = (VkDisplayModeKHR)ud->handle;
    //const VkAllocationCallbacks *allocator = ud->allocator;
    //VkPhysicalDevice physical_device = (VkPhysicalDevice)ud->parent_ud->parent_ud->handle;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(display_mode, "display_mode");
    //@@UD(device)->ddt->DestroyDisplayModeKHR(device, display_mode, allocator);
    return 0;
    }

static int pushdisplaymode(lua_State *L, VkDisplayModeKHR display_mode, ud_t *parent_ud, const VkAllocationCallbacks *allocator)
    {
    ud_t *ud;
    if(pushnondispatchable(L, (uint64_t)display_mode, parent_ud, DISPLAY_MODE_MT)) return 1;
    TRACE_CREATE(display_mode, "display_mode");
    ud = newuserdata_nondispatchable(L, display_mode, DISPLAY_MODE_MT);
    ud->parent_ud = parent_ud;
    ud->instance = parent_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freedisplay_mode;
    ud->idt = parent_ud->idt;
    return 1;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *display_ud;
    VkResult ec;
    VkDisplayModeKHR display_mode;
    VkDisplayModeCreateInfoKHR* info = NULL;
    VkDisplayKHR display = checkdisplay(L, 1, &display_ud);
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)display_ud->parent_ud->handle;
    const VkAllocationCallbacks *allocator = NULL;
    CheckInstancePfn(L, display_ud, CreateDisplayModeKHR);

#define CLEANUP zfreeVkDisplayModeCreateInfoKHR(L, info, 1)
    if(lua_istable(L, 2))
        {
        info = zcheckVkDisplayModeCreateInfoKHR(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        allocator = optallocator(L, 3);
        }
    else
        {
#define CLEANUP1 zfreeVkDisplayModeParametersKHR(L, parameters, 1)
        VkDisplayModeCreateFlagsKHR flags = checkflags(L, 2);
        VkDisplayModeParametersKHR* parameters = zcheckVkDisplayModeParametersKHR(L, 3, &err);
        if(err) { CLEANUP1; return argerror(L, 3); }
        info = znewVkDisplayModeCreateInfoKHR(L, &err);
        if(err) { CLEANUP; CLEANUP1; return lua_error(L); }
#undef CLEANUP1
        info->flags = flags;
        memcpy(&info->parameters, parameters, sizeof(VkDisplayModeParametersKHR));
        Free(L, parameters); /* no zfree here */
        allocator = optallocator(L, 4);
        }
    ec = display_ud->idt->CreateDisplayModeKHR(physical_device, display, info, allocator, &display_mode);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    pushdisplaymode(L, display_mode, display_ud, allocator);
    return 1;
    }

#define N 32

static int GetDisplayModeProperties1(lua_State *L, VkDisplayKHR display, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayModePropertiesKHR* properties; //[N];
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->handle;
    CheckInstancePfn(L, ud, GetDisplayModePropertiesKHR);
#define CLEANUP zfreearrayVkDisplayModePropertiesKHR(L, properties, N, 1)
    properties = znewarrayVkDisplayModePropertiesKHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetDisplayModePropertiesKHR(physical_device, display, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetDisplayModePropertiesKHR(physical_device, display, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayModePropertiesKHR(L, &properties[i]);
            pushdisplaymode(L, properties[i].displayMode, ud, NULL);
            lua_setfield(L, -2, "display_mode");
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetDisplayModeProperties2(lua_State *L, VkDisplayKHR display, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayModeProperties2KHR* properties; //[N];
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->handle;
    CheckInstancePfn(L, ud, GetDisplayModeProperties2KHR);
#define CLEANUP zfreearrayVkDisplayModeProperties2KHR(L, properties, N, 1)
    properties = znewarrayVkDisplayModeProperties2KHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetDisplayModeProperties2KHR(physical_device, display, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetDisplayModeProperties2KHR(physical_device, display, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayModeProperties2KHR(L, &properties[i]);
            pushdisplaymode(L, properties[i].displayModeProperties.displayMode, ud, NULL);
            lua_setfield(L, -2, "display_mode");
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    CLEANUP;
#undef CLEANUP
    return 1;
    }


static int GetDisplayModeProperties(lua_State *L)
    {
    ud_t *ud;
    VkDisplayKHR display = checkdisplay(L, 1, &ud);
    if(ud->idt->GetDisplayModeProperties2KHR)
        return GetDisplayModeProperties2(L, display, ud);
    CheckInstancePfn(L, ud, GetDisplayModePropertiesKHR);
    return GetDisplayModeProperties1(L, display, ud);
    }

static int GetDisplayPlaneCapabilities1(lua_State *L, VkDisplayModeKHR display_mode, ud_t *ud)
    {
    int err;
    VkResult ec;
    VkDisplayPlaneCapabilitiesKHR* capabilities;
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->parent_ud->handle;
    uint32_t planeIndex = luaL_checkinteger(L, 2);
#define CLEANUP zfreeVkDisplayPlaneCapabilitiesKHR(L, capabilities, 1)
    capabilities = znewVkDisplayPlaneCapabilitiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->idt->GetDisplayPlaneCapabilitiesKHR(physical_device, display_mode, planeIndex, capabilities);
    if(ec) { CLEANUP; CheckError(L, ec); }
    zpushVkDisplayPlaneCapabilitiesKHR(L, capabilities);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetDisplayPlaneCapabilities2(lua_State *L, VkDisplayModeKHR display_mode, ud_t *ud)
    {
    int err;
    VkResult ec;
    VkDisplayPlaneInfo2KHR* info = NULL;
    VkDisplayPlaneCapabilities2KHR* capabilities = NULL;
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->parent_ud->handle;
#define CLEANUP do {                                            \
    zfreeVkDisplayPlaneInfo2KHR(L, info, 1);                    \
    zfreeVkDisplayPlaneCapabilities2KHR(L, capabilities, 1);    \
} while(0)
    if(lua_istable(L, 2))
        {
        info = zcheckVkDisplayPlaneInfo2KHR(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        uint32_t index = luaL_checkinteger(L, 2);
        info = znewVkDisplayPlaneInfo2KHR(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->planeIndex = index;
        }
    info->mode = display_mode;
    capabilities = znewchainVkDisplayPlaneCapabilities2KHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->idt->GetDisplayPlaneCapabilities2KHR(physical_device, info, capabilities);
    if(ec) { CLEANUP; CheckError(L, ec); }
    zpushVkDisplayPlaneCapabilities2KHR(L, capabilities);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetDisplayPlaneCapabilities(lua_State *L)
    {
    ud_t *ud;
    VkDisplayModeKHR display_mode = checkdisplay_mode(L, 1, &ud);
    if(ud->idt->GetDisplayPlaneCapabilitiesKHR && lua_type(L, 2) == LUA_TTABLE)
        return GetDisplayPlaneCapabilities2(L, display_mode, ud);
    CheckInstancePfn(L, ud, GetDisplayPlaneCapabilitiesKHR);
    return GetDisplayPlaneCapabilities1(L, display_mode, ud);
    }


RAW_FUNC(display_mode)
TYPE_FUNC(display_mode)
INSTANCE_FUNC(display_mode)
DEVICE_FUNC(display_mode)
PARENT_FUNC(display_mode)
DELETE_FUNC(display_mode)
DESTROY_FUNC(display_mode)


static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "instance", Instance },
        { "device", Device },
        { "parent", Parent },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "create_display_mode",  Create },
        { "get_display_mode_properties", GetDisplayModeProperties },
        { "get_display_plane_capabilities", GetDisplayPlaneCapabilities },
        { "destroy_display_mode",  Destroy }, //@@
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_display_mode(lua_State *L)
    {
    udata_define(L, DISPLAY_MODE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

