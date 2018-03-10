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

static int freesemaphore(lua_State *L, ud_t *ud)
    {
    VkSemaphore semaphore = (VkSemaphore)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(semaphore, "semaphore");
    UD(device)->ddt->DestroySemaphore(device, semaphore, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo_CHAIN info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

    if(lua_istable(L, 2))
        {
        if(echecksemaphorecreateinfo(L, 2, &info)) return argerror(L, 2);
        allocator = optallocator(L, 3);
        }
    else
        {
        info.p1.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.p1.pNext = NULL;
        info.p1.flags = optflags(L, 2, 0);
        }

    ec = device_ud->ddt->CreateSemaphore(device, &info.p1, allocator, &semaphore);
    CheckError(L, ec);

    TRACE_CREATE(semaphore, "semaphore");
    ud = newuserdata_nondispatchable(L, semaphore, SEMAPHORE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freesemaphore;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int ImportSemaphoreFd(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkImportSemaphoreFdInfoKHR info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, ImportSemaphoreFdKHR);
    if(echeckimportsemaphorefdinfo(L, 2, &info)) return argerror(L, 2);
    info.semaphore = semaphore;
    ec = ud->ddt->ImportSemaphoreFdKHR(ud->device, &info);
    CheckError(L, ec);
    return 0;
    }


static int GetSemaphoreFd(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    int fd;
    VkSemaphoreGetFdInfoKHR info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, GetSemaphoreFdKHR);
    if(echecksemaphoregetfdinfo(L, 2, &info)) return argerror(L, 2);
    info.semaphore = semaphore;
    ec = ud->ddt->GetSemaphoreFdKHR(ud->device, &info, &fd);
    CheckError(L, ec);
    lua_pushinteger(L, fd);
    return 1;
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR

static int ImportSemaphoreWin32Handle(lua_State *L) //@@DOC VK_KHR_external_semaphore_win32
    {
    VkResult ec;
    ud_t *ud;
    VkImportSemaphoreWin32HandleInfo info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, ImportSemaphoreWin32Handle);
    if(echeckimportsemaphorewin32handleinfo(L, 2, &info)) return argerror(L, 2);
    info.semaphore = semaphore;
    ec = ud->ddt->ImportSemaphoreWin32Handle(ud->device, &info);
    CheckError(L, ec);
    return 0;
    }

static int GetSemaphoreWin32Handle(lua_State *L) //@@DOC VK_KHR_external_semaphore_win32
    {
    VkResult ec;
    ud_t *ud;
    HANDLE handle;
    VkSemaphoreGetWin32HandleInfoKHR info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, GetSemaphoreWin32HandleKHR);
    if(echecksemaphoregetwin32handleinfo(L, 2, &info)) return argerror(L, 2);
    info.semaphore = semaphore;
    ec = ud->ddt->GetSemaphoreWin32HandleKHR(ud->device, &info, &handle);
    CheckError(L, ec);
    lua_pushlightuserdata(L, handle);
    return 1;
    }

#endif /* VK_USE_PLATFORM_WIN32_KHR */


RAW_FUNC(semaphore)
TYPE_FUNC(semaphore)
INSTANCE_FUNC(semaphore)
DEVICE_FUNC(semaphore)
PARENT_FUNC(semaphore)
DELETE_FUNC(semaphore)
DESTROY_FUNC(semaphore)

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
        { "create_semaphore",  Create },
        { "destroy_semaphore",  Destroy },
        { "import_semaphore_fd", ImportSemaphoreFd },
        { "get_semaphore_fd", GetSemaphoreFd },
#ifdef VK_USE_PLATFORM_WIN32_KHR
        { "import_semaphore_win32_handle", ImportSemaphoreWin32Handle },
        { "get_semaphore_win32_handle", GetSemaphoreWin32Handle },
#endif /* VK_USE_PLATFORM_WIN32_KHR */
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_semaphore(lua_State *L)
    {
    udata_define(L, SEMAPHORE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

