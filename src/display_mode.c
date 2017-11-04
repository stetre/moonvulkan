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
    ud_t *display_ud;
    VkResult ec;
    VkDisplayModeKHR display_mode;
    VkDisplayModeCreateInfoKHR info;

    VkDisplayKHR display = checkdisplay(L, 1, &display_ud);
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)display_ud->parent_ud->handle;
    const VkAllocationCallbacks *allocator = optallocator(L, 4);

    info.sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    if(echeckdisplaymodeparameters(L, 3, &info.parameters)) return argerror(L, 3);
    
    CheckInstancePfn(L, display_ud, CreateDisplayModeKHR);
    ec = display_ud->idt->CreateDisplayModeKHR(physical_device, display, &info, allocator, &display_mode);
    CheckError(L, ec);
    pushdisplaymode(L, display_mode, display_ud, allocator);
    return 1;
    }

static int GetDisplayModeProperties(lua_State *L)
    {
    VkResult ec;
    uint32_t count, remaining, tot, i;
    VkDisplayModePropertiesKHR properties[32];
    ud_t *ud;
    VkDisplayKHR display = checkdisplay(L, 1, &ud);
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->handle;

    CheckInstancePfn(L, ud, GetDisplayModePropertiesKHR);

    lua_newtable(L);
    ec = ud->idt->GetDisplayModePropertiesKHR(physical_device, display, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->GetDisplayModePropertiesKHR(physical_device, display, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        for(i = 0; i < count; i++)
            {
            pushdisplaymodeproperties(L, &properties[i]);
            pushdisplaymode(L, properties[i].displayMode, ud, NULL);
            lua_setfield(L, -2, "display_mode");
            lua_rawseti(L, -2, ++tot);
            }
        } while (remaining > 0);

    return 1;
    }

static int GetDisplayPlaneCapabilities(lua_State *L)
    {
    VkResult ec;
    VkDisplayPlaneCapabilitiesKHR capabilities;
    ud_t *ud;
    VkDisplayModeKHR display_mode = checkdisplay_mode(L, 1, &ud);
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->parent_ud->parent_ud->handle;
    uint32_t planeIndex = luaL_checkinteger(L, 2);

    CheckInstancePfn(L, ud, GetDisplayPlaneCapabilitiesKHR);
    ec = ud->idt->GetDisplayPlaneCapabilitiesKHR(physical_device, display_mode, planeIndex, &capabilities);
    CheckError(L, ec);
    pushdisplayplanecapabilities(L, &capabilities);
    return 1;
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

