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

#define PHYSDEV(display_ud) (VkPhysicalDevice)(uintptr_t)((display_ud)->parent_ud->handle)

static int freedisplay(lua_State *L, ud_t *ud)
    {
    VkDisplayKHR display = (VkDisplayKHR)ud->handle;
    freechildren(L, DISPLAY_MODE_MT, ud);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(display, "display");
    return 0;
    }

static int pushdisplay(lua_State *L, VkDisplayKHR display, ud_t *parent_ud)
    {
    ud_t *ud;
    if(pushnondispatchable(L, (uint64_t)display, parent_ud, DISPLAY_MT)) return 1;
    TRACE_CREATE(display, "display");
    ud = newuserdata_nondispatchable(L, display, DISPLAY_MT);
    ud->parent_ud = parent_ud;
    ud->instance = parent_ud->instance;
    ud->destructor = freedisplay;
    ud->idt = parent_ud->idt;
    return 1;
    }

#define N 32

static int GetPhysicalDeviceDisplayProperties1(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayPropertiesKHR* properties; //[N];
#define CLEANUP zfreearrayVkDisplayPropertiesKHR(L, properties, N, 1)
    properties = znewarrayVkDisplayPropertiesKHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayPropertiesKHR(physdev, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayPropertiesKHR(physdev, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayPropertiesKHR(L, &properties[i]);
            pushdisplay(L, properties[i].display, ud);
            lua_setfield(L, -2, "display");
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceDisplayProperties2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayProperties2KHR* properties; //[N];
#define CLEANUP zfreearrayVkDisplayProperties2KHR(L, properties, N, 1)
    properties = znewchainarrayVkDisplayProperties2KHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayProperties2KHR(physdev, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayProperties2KHR(physdev, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayProperties2KHR(L, &properties[i]);
            pushdisplay(L, properties[i].displayProperties.display, ud);
            lua_setfield(L, -2, "display");
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceDisplayProperties(lua_State *L)
    {
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceDisplayProperties2KHR)
        return GetPhysicalDeviceDisplayProperties2(L, physdev, ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceDisplayPropertiesKHR);
    return GetPhysicalDeviceDisplayProperties1(L, physdev, ud);
    }

static int GetPhysicalDeviceDisplayPlaneProperties1(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayPlanePropertiesKHR* properties; //[N];
#define CLEANUP zfreearrayVkDisplayPlanePropertiesKHR(L, properties, N, 1)
    properties = znewarrayVkDisplayPlanePropertiesKHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayPlanePropertiesKHR(physdev, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayPlanePropertiesKHR(physdev, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayPlanePropertiesKHR(L, &properties[i]);
            if(properties[i].currentDisplay != VK_NULL_HANDLE)
                {
                pushdisplay(L, properties[i].currentDisplay, ud);
                lua_setfield(L, -2, "current_display");
                }
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    CLEANUP;
#undef CLEANUP
    return 1;
    }


static int GetPhysicalDeviceDisplayPlaneProperties2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayPlaneProperties2KHR* properties; //[N];
#define CLEANUP zfreearrayVkDisplayPlaneProperties2KHR(L, properties, N, 1)
    properties = znewchainarrayVkDisplayPlaneProperties2KHR(L, N, &err);
    if(err) { CLEANUP; lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayPlaneProperties2KHR(physdev, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining==0) { CLEANUP; return 1; }
    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayPlaneProperties2KHR(physdev, &count, properties);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }
    
        for(i = 0; i < count; i++)
            {
            zpushVkDisplayPlaneProperties2KHR(L, &properties[i]);
            if(properties[i].displayPlaneProperties.currentDisplay != VK_NULL_HANDLE)
                {
                pushdisplay(L, properties[i].displayPlaneProperties.currentDisplay, ud);
                lua_setfield(L, -2, "current_display");
                }
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceDisplayPlaneProperties(lua_State *L)
    {
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceDisplayPlaneProperties2KHR)
        return GetPhysicalDeviceDisplayPlaneProperties2(L, physdev, ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceDisplayPlanePropertiesKHR);
    return GetPhysicalDeviceDisplayPlaneProperties1(L, physdev, ud);
    }

static int GetDisplayPlaneSupportedDisplays(lua_State *L)
    {
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayKHR display[32];
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t planeIndex = luaL_checkinteger(L, 2);

    CheckInstancePfn(L, ud, GetDisplayPlaneSupportedDisplaysKHR);

    lua_newtable(L);
    ec = ud->idt->GetDisplayPlaneSupportedDisplaysKHR(physdev, planeIndex, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetDisplayPlaneSupportedDisplaysKHR(physdev, planeIndex, &count, display);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        for(i = 0; i < count; i++)
            {
            pushdisplay(L, display[i], ud);
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    return 1;
    }

static int ReleaseDisplay(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkDisplayKHR display = checkdisplay(L, 1, &ud);
    VkPhysicalDevice physdev = PHYSDEV(ud);
    CheckInstancePfn(L, ud, ReleaseDisplayEXT);
    ec = ud->idt->ReleaseDisplayEXT(physdev, display);
    CheckError(L, ec);
    return 0;
    }

static int DisplayPowerControl(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    VkDevice device = checkdevice(L, 1, &ud);
    VkDisplayKHR display = checkdisplay(L, 2, NULL);
    VkDisplayPowerInfoEXT* info;
    CheckDevicePfn(L, ud, DisplayPowerControlEXT);
#define CLEANUP zfreeVkDisplayPowerInfoEXT(L, info, 1)
    info = zcheckVkDisplayPowerInfoEXT(L, 3, &err);
    if(err) { CLEANUP; return argerror(L, 3); }
    ec = ud->ddt->DisplayPowerControlEXT(device, display, info);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int AcquireXlibDisplay(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    VkResult ec;
    ud_t *ud;
    Display *dpy = (Display*)checklightuserdata(L, 1);
    VkDisplayKHR display = checkdisplay(L, 2, &ud);
    VkPhysicalDevice physdev = PHYSDEV(ud);
    CheckInstancePfn(L, ud, AcquireXlibDisplayEXT);
    ec = ud->idt->AcquireXlibDisplayEXT(physdev, dpy, display);
    CheckError(L, ec);
    return 0;
#else
    return notsupported(L);
#endif
    }

static int GetRandROutputDisplay(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    VkResult ec;
    ud_t *ud;
    VkDisplayKHR display;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    Display *dpy = (Display*)checklightuserdata(L, 2);
    RROutput rrOutput = (RROutput)luaL_checkinteger(L, 1);
    CheckInstancePfn(L, ud, GetRandROutputDisplayEXT);
    ec = ud->idt->GetRandROutputDisplayEXT(physdev, dpy, rrOutput, &display);
    CheckError(L, ec);
    pushdisplay(L, display, ud);
    return 1;
#else
    return notsupported(L);
#endif
    }



RAW_FUNC(display)
TYPE_FUNC(display)
INSTANCE_FUNC(display)
DEVICE_FUNC(display)
PARENT_FUNC(display)
DELETE_FUNC(display)
DESTROY_FUNC(display)


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
        { "get_physical_device_display_properties", GetPhysicalDeviceDisplayProperties },
        { "get_physical_device_display_plane_properties", GetPhysicalDeviceDisplayPlaneProperties },
        { "get_display_plane_supported_displays", GetDisplayPlaneSupportedDisplays },
        { "destroy_display",  Destroy }, //@@
        { "release_display", ReleaseDisplay },
        { "display_power_control", DisplayPowerControl },
        { "acquire_xlib_display", AcquireXlibDisplay },
        { "get_randr_output_display", GetRandROutputDisplay },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_display(lua_State *L)
    {
    udata_define(L, DISPLAY_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

