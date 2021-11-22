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
    info.hinstance = (HINSTANCE)checklightuserdata(L, 3);
    info.hwnd = (HWND)checklightuserdata(L, 4);
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

/*- Headless ---------------------------------------------------------------*/

static int CreateHeadlessSurface(lua_State *L)
    {
    int err;
    VkResult ec;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkHeadlessSurfaceCreateInfoEXT* info;
    VkInstance instance = checkinstance(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckInstancePfn(L, ud, CreateHeadlessSurfaceEXT);
#define CLEANUP zfreeVkHeadlessSurfaceCreateInfoEXT(L, info, 1)
    info = zcheckVkHeadlessSurfaceCreateInfoEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->idt->CreateHeadlessSurfaceEXT(instance, info, allocator, &surface);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return newsurface(L, instance, surface, allocator);
    }

/*- Display ----------------------------------------------------------------*/

static int CreateDisplayPlaneSurface(lua_State *L)
    {
    int err;
    VkResult ec;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkDisplaySurfaceCreateInfoKHR* info;
    VkDisplayModeKHR display_mode = checkdisplay_mode(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckInstancePfn(L, ud, CreateDisplayPlaneSurfaceKHR);
#define CLEANUP zfreeVkDisplaySurfaceCreateInfoKHR(L, info, 1)
    info = zcheckVkDisplaySurfaceCreateInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->displayMode = display_mode;
    ec = ud->idt->CreateDisplayPlaneSurfaceKHR(ud->instance, info, allocator, &surface);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
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


static int PushSupportedSurfaceCounters(lua_State *L, VkPhysicalDevice physdev, ud_t *ud, VkSurfaceKHR surface)
/* Calls GetPhysicalDeviceSurfaceCapabilities2EXT() to retrieve the supportedSurfaceCounters
 * field, and sets the field in the table on the top of the stack.
 * (Note that all other capabilities retrieved by this function are the same as in
 * GetPhysicalDeviceSurfaceCapabilitiesKHR(), so we don't bother about them).
 */
    {
    VkResult ec;
    VkSurfaceCapabilities2EXT caps;
    if(!ud->idt->GetPhysicalDeviceSurfaceCapabilities2EXT) return 0;
    memset(&caps, 0, sizeof(caps));
    caps.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT;
    ec = ud->idt->GetPhysicalDeviceSurfaceCapabilities2EXT(physdev, surface, &caps);
    if(ec) return 0;
    pushflags(L, caps.supportedSurfaceCounters);
    lua_setfield(L, -2, "supported_surface_counters");
    return 0;
    }

static int GetPhysicalDeviceSurfaceCapabilities2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    VkPhysicalDeviceSurfaceInfo2KHR* info=NULL;
    VkSurfaceCapabilities2KHR* capabilities=NULL;
    VkSurfaceKHR surface = checksurface(L, 2, NULL);

#define CLEANUP do {                                    \
    zfreeVkSurfaceCapabilities2KHR(L, capabilities, 1); \
    zfreeVkPhysicalDeviceSurfaceInfo2KHR(L, info, 1);   \
} while(0)
    capabilities = znewchainVkSurfaceCapabilities2KHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    info = znewVkPhysicalDeviceSurfaceInfo2KHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    info->surface = surface;

    ec = ud->idt->GetPhysicalDeviceSurfaceCapabilities2KHR(physdev, info, capabilities);
    if(ec) { CLEANUP; CheckError(L, ec); }
    zpushVkSurfaceCapabilities2KHR(L, capabilities);
    PushSupportedSurfaceCounters(L, physdev, ud, info->surface);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceSurfaceCapabilities(lua_State *L)
    {
    int err;
    VkResult ec;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR* capabilities;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);

    if(ud->idt->GetPhysicalDeviceSurfaceCapabilities2KHR)
        return GetPhysicalDeviceSurfaceCapabilities2(L, physdev, ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfaceCapabilitiesKHR);

    surface = checksurface(L, 2, NULL);
#define CLEANUP zfreeVkSurfaceCapabilitiesKHR(L, capabilities, 1);
    capabilities = znewVkSurfaceCapabilitiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->idt->GetPhysicalDeviceSurfaceCapabilitiesKHR(physdev, surface, capabilities);
    CheckError(L, ec);
    zpushVkSurfaceCapabilitiesKHR(L, capabilities);
    PushSupportedSurfaceCounters(L, physdev, ud, surface);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceSurfaceFormats2(lua_State *L, VkPhysicalDevice physdev, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t i, count=0;
    VkSurfaceFormat2KHR* formats=NULL;
    VkPhysicalDeviceSurfaceInfo2KHR* info=NULL;
    VkSurfaceKHR surface = checksurface(L, 2, NULL);
#define CLEANUP do {                                        \
    zfreearrayVkSurfaceFormat2KHR(L, formats, count, 1);    \
    zfreeVkPhysicalDeviceSurfaceInfo2KHR(L, info, 1);       \
} while(0)
    info = znewVkPhysicalDeviceSurfaceInfo2KHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    info->surface = surface;
    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceSurfaceFormats2KHR(physdev, info, &count, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    if(count == 0) { CLEANUP; return 1; }
    formats = znewchainarrayVkSurfaceFormat2KHR(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->idt->GetPhysicalDeviceSurfaceFormats2KHR(physdev, info, &count, formats);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    for(i = 0; i < count; i++)
        {
        zpushVkSurfaceFormat2KHR(L, &formats[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }


static int GetPhysicalDeviceSurfaceFormats(lua_State *L)
    {
    int err;
    VkResult ec;
    uint32_t count, i;
    VkSurfaceFormatKHR* formats;
    VkSurfaceKHR surface;
    ud_t *ud;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);

    if(ud->idt->GetPhysicalDeviceSurfaceFormats2KHR)
        return GetPhysicalDeviceSurfaceFormats2(L, physdev, ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceSurfaceFormatsKHR);
    surface = checksurface(L, 2, NULL);

    lua_newtable(L);
    ec = ud->idt->GetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &count, NULL);
    CheckError(L, ec);
    if(count == 0) return 1;

#define CLEANUP zfreearrayVkSurfaceFormatKHR(L, formats, count, 1)
    formats = znewarrayVkSurfaceFormatKHR(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    
    ec = ud->idt->GetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &count, formats);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    for(i = 0; i < count; i++)
        {
        zpushVkSurfaceFormatKHR(L, &formats[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
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
    if(count == 0) return 1;

    modes = (VkPresentModeKHR*)Malloc(L, sizeof(VkPresentModeKHR)*count);
    
    ud->idt->GetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &count, modes);
    if(ec) { Free(L, modes); CheckError(L, ec); return 0; }

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


static int GetDeviceGroupSurfacePresentModes(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkDeviceGroupPresentModeFlagsKHR modes;
    VkDevice device = checkdevice(L, 1, &ud);
    VkSurfaceKHR surface = checksurface(L, 2, NULL);
    CheckDevicePfn(L, ud, GetDeviceGroupSurfacePresentModesKHR);
    ec = ud->ddt->GetDeviceGroupSurfacePresentModesKHR(device, surface, &modes);
    CheckError(L, ec);
    pushflags(L, modes);
    return 1;
    }

static int GetPhysicalDevicePresentRectangles(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    uint32_t i, count = 0;
    VkRect2D* rects = NULL;
    VkPhysicalDevice physdev = checkphysical_device(L, 1, &ud);
    VkSurfaceKHR surface = checksurface(L, 2, NULL);
    CheckDevicePfn(L, ud, GetPhysicalDevicePresentRectanglesKHR);
    ec = ud->ddt->GetPhysicalDevicePresentRectanglesKHR(physdev, surface, &count, NULL);
    CheckError(L, ec);
#define CLEANUP zfreearrayVkRect2D(L, rects, count, 1)
    rects = znewarrayVkRect2D(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->ddt->GetPhysicalDevicePresentRectanglesKHR(physdev, surface, &count, rects);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    lua_newtable(L);
    for(i = 0; i < count; i++)
        {
        zpushVkRect2D(L, &rects[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetDeviceGroupPresentCapabilities(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    VkDeviceGroupPresentCapabilitiesKHR* capabilities;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, GetDeviceGroupPresentCapabilitiesKHR);
#define CLEANUP zfreeVkDeviceGroupPresentCapabilitiesKHR(L, capabilities, 1)
    capabilities = znewchainVkDeviceGroupPresentCapabilitiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->ddt->GetDeviceGroupPresentCapabilitiesKHR(device, capabilities);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    zpushVkDeviceGroupPresentCapabilitiesKHR(L, capabilities);
    CLEANUP;
#undef CLEANUP
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
        { "get_physical_device_win32_presentation_support", GetPhysicalDeviceWin32PresentationSupport },
        { "create_xlib_surface", CreateXlibSurface },
        { "create_xcb_surface", CreateXcbSurface },
        { "create_wayland_surface", CreateWaylandSurface },
        { "create_android_surface", CreateAndroidSurface },
        { "create_win32_surface", CreateWin32Surface },
        { "create_headless_surface", CreateHeadlessSurface },
        { "create_display_plane_surface", CreateDisplayPlaneSurface },
        { "created_surface",  CreatedSurface },
        { "destroy_surface",  Destroy },
        { "get_device_group_surface_present_modes", GetDeviceGroupSurfacePresentModes  },
        { "get_physical_device_present_rectangles", GetPhysicalDevicePresentRectangles  },
        { "get_device_group_present_capabilities", GetDeviceGroupPresentCapabilities  },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_surface(lua_State *L)
    {
    udata_define(L, SURFACE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

