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
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

#define CLEANUP zfreeVkSemaphoreCreateInfo(L, info, 1)
    if(lua_istable(L, 2))
        {
        info = zcheckVkSemaphoreCreateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        allocator = optallocator(L, 3);
        }
    else
        {
        info = znewVkSemaphoreCreateInfo(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->flags = optflags(L, 2, 0);
        }

    ec = device_ud->ddt->CreateSemaphore(device, info, allocator, &semaphore);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
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
    int err;
    VkResult ec;
    ud_t *ud;
    VkImportSemaphoreFdInfoKHR* info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, ImportSemaphoreFdKHR);
#define CLEANUP zfreeVkImportSemaphoreFdInfoKHR(L, info, 1)
    info = zcheckVkImportSemaphoreFdInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->semaphore = semaphore;
    ec = ud->ddt->ImportSemaphoreFdKHR(ud->device, info);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }


static int GetSemaphoreFd(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    int fd;
    VkSemaphoreGetFdInfoKHR* info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, GetSemaphoreFdKHR);
#define CLEANUP zfreeVkSemaphoreGetFdInfoKHR(L, info, 1)
    info = zcheckVkSemaphoreGetFdInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->semaphore = semaphore;
    ec = ud->ddt->GetSemaphoreFdKHR(ud->device, info, &fd);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    lua_pushinteger(L, fd);
    return 1;
    }

static int GetSemaphoreCounterValue(lua_State *L)
    {
    VkResult ec;
    uint64_t value;
    ud_t *ud;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, GetSemaphoreCounterValue);
    ec = ud->ddt->GetSemaphoreCounterValue(ud->device, semaphore, &value);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int WaitSemaphores(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    VkSemaphoreWaitInfo* info;
    VkDevice device = checkdevice(L, 1, &ud);
    uint64_t timeout = luaL_checkinteger(L, 3);
    CheckDevicePfn(L, ud, WaitSemaphores);
#define CLEANUP zfreeVkSemaphoreWaitInfo(L, info, 1)
    info = zcheckVkSemaphoreWaitInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->WaitSemaphores(device, info, timeout);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    return 0;
    }

static int SignalSemaphore(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    VkSemaphoreSignalInfo* info;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, SignalSemaphore);
#define CLEANUP zfreeVkSemaphoreSignalInfo(L, info, 1)
    info = zcheckVkSemaphoreSignalInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->SignalSemaphore(device, info);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    return 0;
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR

static int ImportSemaphoreWin32Handle(lua_State *L) //@@DOC VK_KHR_external_semaphore_win32
    {
    int err;
    VkResult ec;
    ud_t *ud;
    VkImportSemaphoreWin32HandleInfoKHR* info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, ImportSemaphoreWin32HandleKHR);
#define CLEANUP zfreeVkImportSemaphoreWin32HandleInfoKHR(L, info, 1)
    info = zcheckVkImportSemaphoreWin32HandleInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->semaphore = semaphore;
    ec = ud->ddt->ImportSemaphoreWin32HandleKHR(ud->device, &info);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    return 0;
    }

static int GetSemaphoreWin32Handle(lua_State *L) //@@DOC VK_KHR_external_semaphore_win32
    {
    int err;
    VkResult ec;
    ud_t *ud;
    HANDLE handle;
    VkSemaphoreGetWin32HandleInfoKHR* info;
    VkSemaphore semaphore = checksemaphore(L, 1, &ud);
    CheckDevicePfn(L, ud, GetSemaphoreWin32HandleKHR);
#define CLEANUP zfreeVkSemaphoreGetWin32HandleInfoKHR(L, info, 1)
    info = zcheckVkSemaphoreGetWin32HandleInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->semaphore = semaphore;
    ec = ud->ddt->GetSemaphoreWin32HandleKHR(ud->device, info, &handle);
    CLEANUP;
#undef CLEANUP
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
        { "get_semaphore_counter_value", GetSemaphoreCounterValue },
        { "wait_semaphores", WaitSemaphores },
        { "signal_semaphore", SignalSemaphore },
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

