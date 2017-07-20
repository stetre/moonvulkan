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

static int GetPhysicalDeviceDisplayProperties(lua_State *L)
    {
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayPropertiesKHR properties[32];
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);

    CheckInstancePfn(L, ud, GetPhysicalDeviceDisplayPropertiesKHR);

    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        for(i = 0; i < count; i++)
            {
            pushdisplayproperties(L, &properties[i]);
            pushdisplay(L, properties[i].display, ud);
            lua_setfield(L, -2, "display");
            lua_rawseti(L, -2, ++tot);
            }
        } while (ec==VK_INCOMPLETE);

    return 1;
    }


static int GetPhysicalDeviceDisplayPlaneProperties(lua_State *L)
    {
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayPlanePropertiesKHR properties[32];
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);

    CheckInstancePfn(L, ud, GetPhysicalDeviceDisplayPlanePropertiesKHR);

    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        for(i = 0; i < count; i++)
            {
            pushdisplayplaneproperties(L, &properties[i]);
            if(properties[i].currentDisplay != VK_NULL_HANDLE)
                {
                pushdisplay(L, properties[i].currentDisplay, ud);
                lua_setfield(L, -2, "current_display");
                }
            lua_rawseti(L, -2, ++tot);
            }
        } while (ec==VK_INCOMPLETE);

    return 1;
    }


static int GetDisplayPlaneSupportedDisplays(lua_State *L)
    {
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayKHR display[32];
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    uint32_t planeIndex = luaL_checkinteger(L, 2);

    CheckInstancePfn(L, ud, GetDisplayPlaneSupportedDisplaysKHR);

    lua_newtable(L);
    ec = ud->idt->GetDisplayPlaneSupportedDisplaysKHR(physical_device, planeIndex, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetDisplayPlaneSupportedDisplaysKHR(physical_device, planeIndex, &count, display);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        for(i = 0; i < count; i++)
            {
            pushdisplay(L, display[i], ud);
            lua_rawseti(L, -2, ++tot);
            }
        } while (ec==VK_INCOMPLETE);

    return 1;
    }

#if 0
#define VK_EXT_direct_mode_display 1
//typedef VkResult (VKAPI_PTR *PFN_vkReleaseDisplayEXT)(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
        { "release_display", ReleaseDisplay },
static int ReleaseDisplay(lua_State *L)//@@TODO?
    {
	VkResult ec;
	VkPhysicalDevice physdev;
	VkDisplayKHR display;
    CheckInstancePfn(L, ud, ReleaseDisplayEXT);
	ec = ud->idt->ReleaseDisplayEXT(physdev, display);
	CheckError(L, ec);
    return 0;
    }
#endif

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
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_display(lua_State *L)
    {
    udata_define(L, DISPLAY_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

