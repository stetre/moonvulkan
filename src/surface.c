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

static int freesurface(lua_State *L, ud_t *ud)
    {
    VkSurfaceKHR surface = (VkSurfaceKHR)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkInstance instance = ud->instance;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(surface, "surface");
    UD(instance)->idt->DestroySurfaceKHR(instance, surface, allocator);
    return 0;
    }

static int newsurface(lua_State *L, VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *allocator)
    {
    ud_t *ud;
    TRACE_CREATE(surface, "surface");
    ud = newuserdata_nondispatchable(L, surface, SURFACE_MT);
    ud->parent_ud = UD(instance);
    ud->instance = instance;
    ud->destructor = freesurface;
    ud->allocator = allocator;
    ud->idt = UD(instance)->idt;
    return 1;
    }

static int CreatedSurface(lua_State *L) /* NONVK */
    {
    VkInstance instance = checkinstance(L, 1, NULL);
    VkSurfaceKHR surface = (VkSurfaceKHR)checkhandle(L, 2);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    return newsurface(L, instance, surface, allocator);
    }
 
/*- XLIB ---------------------------------------------------------------------------*/

static int CreateXlibSurface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XLIB_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkXlibSurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);
    info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.dpy = (Display*)checklightuserdata(L, 3);
    info.window = (Window)checklightuserdata(L, 4);
    CheckInstancePfn(L, ud, CreateXlibSurfaceKHR);
    ec = ud->idt->CreateXlibSurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

static int GetPhysicalDeviceXlibPresentationSupport(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XLIB_KHR
    ud_t *ud;
    VkBool32 rc;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    Display* dpy = (Display*)checklightuserdata(L, 3);
    VisualID visualid = luaL_checkinteger(L, 4);
    CheckInstancePfn(L, ud, GetPhysicalDeviceXlibPresentationSupportKHR);
    rc = ud->idt->GetPhysicalDeviceXlibPresentationSupportKHR(physdev, qfamily, dpy, visualid);
    lua_pushboolean(L, rc);
    return 1;
#else
    return notsupported(L);
#endif
    }


/*- XCB --------------------------------------------------------------------*/

static int CreateXcbSurface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XCB_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkXcbSurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);
    info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.connection = (xcb_connection_t*)checklightuserdata(L, 3);
    info.window = luaL_checkinteger(L, 4);
    CheckInstancePfn(L, ud, CreateXcbSurfaceKHR);
    ec = ud->idt->CreateXcbSurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

static int GetPhysicalDeviceXcbPresentationSupport(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_XCB_KHR
    ud_t *ud;
    VkBool32 rc;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    xcb_connection_t *connection = (xcb_connection_t*)checklightuserdata(L, 3);
    xcb_visualid_t visualid = luaL_checkinteger(L, 4);
    CheckInstancePfn(L, ud, GetPhysicalDeviceXcbPresentationSupportKHR);
    rc = ud->idt->GetPhysicalDeviceXcbPresentationSupportKHR(physdev, 
        qfamily, connection, visualid);
    lua_pushboolean(L, rc);
    return 1;
#else
    return notsupported(L);
#endif
    }


/*- Wayland ----------------------------------------------------------------*/

static int CreateWaylandSurface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkWaylandSurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);
    info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.display = (struct wl_display*)checklightuserdata(L, 3);
    info.surface = (struct wl_surface*)checklightuserdata(L, 4);
    CheckInstancePfn(L, ud, CreateWaylandSurfaceKHR);
    ec = ud->idt->CreateWaylandSurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

static int GetPhysicalDeviceWaylandPresentationSupport(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    ud_t *ud;
    VkBool32 rc;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    struct wl_display *display = (struct wl_display*)checklightuserdata(L, 3);
    CheckInstancePfn(L, ud, GetPhysicalDeviceWaylandPresentationSupportKHR);
    rc = ud->idt->GetPhysicalDeviceWaylandPresentationSupportKHR(physdev, qfamily, display);
    lua_pushboolean(L, rc);
    return 1;
#else
    return notsupported(L);
#endif
    }


/*- MIR --------------------------------------------------------------------*/

static int CreateMirSurface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_MIR_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkMirSurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);
    info.sType = VK_STRUCTURE_TYPE_MIR_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.connection = (MirConnection*)checklightuserdata(L, 3);
    info.mirSurface = (MirSurface*)checklightuserdata(L, 4);
    CheckInstancePfn(L, ud, CreateMirSurfaceKHR);
    ec = ud->idt->CreateMirSurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

static int GetPhysicalDeviceMirPresentationSupport(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_MIR_KHR
    ud_t *ud;
    VkBool32 rc;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    MirConnection *connection = (MirConnection*)checklightuserdata(L, 3);
    CheckInstancePfn(L, ud, GetPhysicalDeviceMirPresentationSupportKHR);
    rc = ud->idt->GetPhysicalDeviceMirPresentationSupportKHR(physdev, qfamily, connection);
    lua_pushboolean(L, rc);
    return 1;
#else
    return notsupported(L);
#endif
    }


/*- Android ----------------------------------------------------------------*/

static int CreateAndroidSurface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkAndroidSurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.window = (ANativeWindow*)checklightuserdata(L, 3);
    CheckInstancePfn(L, ud, CreateAndroidSurfaceKHR);
    ec = ud->idt->CreateAndroidSurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

/*- Win32 ------------------------------------------------------------------*/

static int CreateWin32Surface(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult ec;
    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR info;
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);
    info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; 
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.hinstance = (HINSTANCE *)checklightuserdata(L, 3);
    info.hwnd = (HWND*)checklightuserdata(L, 4);
    CheckInstancePfn(L, ud, CreateWin32SurfaceKHR);
    ec = ud->idt->CreateWin32SurfaceKHR(instance, &info, allocator, &surface);
    CheckError(L, ec);
    return newsurface(L, instance, surface, allocator);
#else
    return notsupported(L);
#endif
    }

static int GetPhysicalDeviceWin32PresentationSupport(lua_State *L)
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    ud_t *ud;
    VkBool32 rc;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    CheckInstancePfn(L, ud, GetPhysicalDeviceWin32PresentationSupportKHR);
    rc = ud->idt->GetPhysicalDeviceWin32PresentationSupportKHR(physdev, qfamily);
    lua_pushboolean(L, rc);
    return 1;
#else
    return notsupported(L);
#endif
    }

/*- Display ----------------------------------------------------------------*/

static int CreateDisplayPlaneSurface(lua_State *L)
    {
    VkResult ec;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkDisplayModeKHR display_mode = checkdisplay_mode(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    VkDisplaySurfaceCreateInfoKHR info;
    if(echeckdisplaysurfacecreateinfo(L, 2, &info)) return argerror(L, 2);
    info.displayMode = display_mode;
    CheckInstancePfn(L, ud, CreateDisplayPlaneSurfaceKHR);
    ec = ud->idt->CreateDisplayPlaneSurfaceKHR(ud->instance, &info, allocator, &surface);
    freedisplaysurfacecreateinfo(L, &info);
    CheckError(L, ec);
    return newsurface(L, ud->instance, surface, allocator);
    }


static int GetPhysicalDeviceSurfaceSupport(lua_State *L)
    {
    VkResult ec;
    VkBool32 supported;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    uint32_t qfamily = luaL_checkinteger(L, 2);
    VkSurfaceKHR surface = checksurface(L, 3, NULL);
    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfaceSupportKHR);
    ec = ud->idt->GetPhysicalDeviceSurfaceSupportKHR(physdev, qfamily, surface, &supported);
    CheckError(L, ec);
    lua_pushboolean(L, supported);
    return 1;
    }

static int GetPhysicalDeviceSurfaceCapabilities2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    VkResult ec;
    VkPhysicalDeviceSurfaceInfo2KHR info;
    VkSurfaceCapabilities2KHR *capabilities;

    capabilities = newsurfacecapabilities2(L);
    if(!capabilities) return errmemory(L);

    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    info.pNext = NULL;
    info.surface = checksurface(L, 2, NULL);

    ec = ud->idt->GetPhysicalDeviceSurfaceCapabilities2KHR(physdev, &info, capabilities);
    if(ec)
        {
        freesurfacecapabilities2(L, capabilities);
        CheckError(L, ec);
        return 0;
        }
    pushsurfacecapabilities2(L, capabilities);
    freesurfacecapabilities2(L, capabilities);
    return 1;
    }

static int GetPhysicalDeviceSurfaceCapabilities(lua_State *L)
    {
    VkResult ec;
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);

    if(ud->idt->GetPhysicalDeviceSurfaceCapabilities2KHR)
        return GetPhysicalDeviceSurfaceCapabilities2(L, physdev, ud);

    surface = checksurface(L, 2, NULL);
    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    ec = ud->idt->GetPhysicalDeviceSurfaceCapabilitiesKHR(physdev, surface, &capabilities);
    CheckError(L, ec);
    pushsurfacecapabilities(L, &capabilities);
    return 1;
    }

static int GetPhysicalDeviceSurfaceFormats2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    VkResult ec;
    uint32_t count, i;
    VkSurfaceFormat2KHR* formats;

    VkPhysicalDeviceSurfaceInfo2KHR info;

    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    info.pNext = NULL;
    info.surface = checksurface(L, 2, NULL);

    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceSurfaceFormats2KHR(physdev, &info, &count, NULL);
    CheckError(L, ec);
    if(count == 0)
        return 1;

    formats = newsurfaceformat2(L, count);

    ec = ud->idt->GetPhysicalDeviceSurfaceFormats2KHR(physdev, &info, &count, formats);
    if(ec)
        {
        freesurfaceformat2(L, formats, count);
        CheckError(L, ec);
        return 0;
        }
    for(i = 0; i < count; i++)
        {
        pushsurfaceformat2(L, &formats[i]);
        lua_rawseti(L, -2, i+1);
        }
    freesurfaceformat2(L, formats, count);
    return 1;
    }


static int GetPhysicalDeviceSurfaceFormats(lua_State *L)
    {
    VkResult ec;
    uint32_t count, i;
    VkSurfaceFormatKHR* formats;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);

    if(ud->idt->GetPhysicalDeviceSurfaceFormats2KHR)
        return GetPhysicalDeviceSurfaceFormats2(L, physdev, ud);

    surface = checksurface(L, 2, NULL);
    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfaceFormatsKHR);

    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &count, NULL);
    CheckError(L, ec);
    if(count == 0)
        return 1;

    formats = (VkSurfaceFormatKHR*)Malloc(L, sizeof(VkSurfaceFormatKHR)*count);
    
    ec = ud->idt->GetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &count, formats);
    if(ec)
        {
        Free(L, formats);
        CheckError(L, ec);
        return 0;
        }
    for(i = 0; i < count; i++)
        {
        pushsurfaceformat(L, &formats[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, formats);
    return 1;
    }


static int GetPhysicalDeviceSurfacePresentModes(lua_State *L)
    {
    VkResult ec;
    uint32_t count, i;
    VkPresentModeKHR* modes;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    VkSurfaceKHR surface = checksurface(L, 2, NULL);
    int byname = optboolean(L, 3, 0);

    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfacePresentModesKHR);
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &count, NULL);
    CheckError(L, ec);
    if(count == 0)
        return 1;

    modes = (VkPresentModeKHR*)Malloc(L, sizeof(VkPresentModeKHR)*count);
    
    ud->idt->GetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &count, modes);
    if(ec)
        {
        Free(L, modes);
        CheckError(L, ec);
        return 0;
        }

    if(byname)
        {
        for(i = 0; i < count; i++)
            {
            pushpresentmode(L, modes[i]);
            lua_pushboolean(L, 1);
            lua_rawset(L, -3);
            }
        }
    else
        {
        for(i = 0; i < count; i++)
            {
            pushpresentmode(L, modes[i]);
            lua_rawseti(L, -2, i+1);
            }
        }
    Free(L, modes);
    return 1;
    }


RAW_FUNC(surface)
TYPE_FUNC(surface)
INSTANCE_FUNC(surface)
DEVICE_FUNC(surface)
PARENT_FUNC(surface)
DELETE_FUNC(surface)
DESTROY_FUNC(surface)

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
        { "get_physical_device_surface_support", GetPhysicalDeviceSurfaceSupport },
        { "get_physical_device_surface_capabilities", GetPhysicalDeviceSurfaceCapabilities },
        { "get_physical_device_surface_formats", GetPhysicalDeviceSurfaceFormats },
        { "get_physical_device_surface_present_modes", GetPhysicalDeviceSurfacePresentModes },
        { "get_physical_device_xlib_presentation_support", GetPhysicalDeviceXlibPresentationSupport },
        { "get_physical_device_xcb_presentation_support", GetPhysicalDeviceXcbPresentationSupport },
        { "get_physical_device_wayland_presentation_support", GetPhysicalDeviceWaylandPresentationSupport },
        { "get_physical_device_mir_presentation_support", GetPhysicalDeviceMirPresentationSupport },
        { "get_physical_device_win32_presentation_support", GetPhysicalDeviceWin32PresentationSupport },
        { "create_xlib_surface", CreateXlibSurface },
        { "create_xcb_surface", CreateXcbSurface },
        { "create_wayland_surface", CreateWaylandSurface },
        { "create_mir_surface", CreateMirSurface },
        { "create_android_surface", CreateAndroidSurface },
        { "create_win32_surface", CreateWin32Surface },
        { "create_display_plane_surface", CreateDisplayPlaneSurface },
        { "created_surface",  CreatedSurface },
        { "destroy_surface",  Destroy },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_surface(lua_State *L)
    {
    udata_define(L, SURFACE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

