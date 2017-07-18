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

static int freefence(lua_State *L, ud_t *ud)
    {
    VkFence fence = (VkFence)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(fence, "fence");
    UD(device)->ddt->DestroyFence(device, fence, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkFence fence;
    VkFenceCreateInfo info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

    if(lua_istable(L, 2))
        {
        if(echeckfencecreateinfo(L, 2, &info)) return argerror(L, 2);
        allocator = optallocator(L, 3);
        }
    else
        {
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = NULL;
        info.flags = optflags(L, 2, 0);
        }

    ec = device_ud->ddt->CreateFence(device, &info, allocator, &fence);
    CheckError(L, ec);
    TRACE_CREATE(fence, "fence");
    ud = newuserdata_nondispatchable(L, fence, FENCE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freefence;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int ResetFences(lua_State *L)
    {
    ud_t **ud;
    int err;
    uint32_t count;
    VkResult ec;
    VkDevice device;
    VkFence *fences = checkfencelist(L, 1, &count, &err, &ud);
    if(err) return argerrorc(L, 1, err);

    device = ud[0]->device;
    ec = UD(device)->ddt->ResetFences(device, count, fences);
    Free(L, fences);
    Free(L, ud);
    CheckError(L, ec);
    return 0;
    }

static int GetFenceStatus(lua_State *L)
    {
    ud_t *ud;
    VkFence fence = checkfence(L, 1, &ud);
    VkDevice device = ud->device;
    VkResult ec = ud->ddt->GetFenceStatus(device, fence);
    switch(ec)
        {
        case VK_SUCCESS: lua_pushboolean(L, 1); return 1;
        case VK_NOT_READY: lua_pushboolean(L, 0); return 1;
        default:
            CheckError(L, ec);
        }
    return 0;
    }

static int WaitForFences(lua_State *L)
    {
    ud_t **ud;
    int err;
    uint32_t count;
    VkResult ec;
    VkBool32 waitall = checkboolean(L, 2);
    uint64_t timeout = checktimeout(L, 3);
    VkFence *fences = checkfencelist(L, 1, &count, &err, &ud);
    if(err) return argerrorc(L, 1, err);
    
    ec = ud[0]->ddt->WaitForFences(ud[0]->device, count, fences, waitall, timeout);
    Free(L, fences);
    Free(L, ud);

    switch(ec)
        {
        case VK_SUCCESS: lua_pushboolean(L, 1); return 1;
        case VK_TIMEOUT: lua_pushboolean(L, 0); return 1;
        default:
            CheckError(L, ec);
        }
    return 0;
    }

RAW_FUNC(fence)
TYPE_FUNC(fence)
INSTANCE_FUNC(fence)
DEVICE_FUNC(fence)
PARENT_FUNC(fence)
DELETE_FUNC(fence)
DESTROY_FUNC(fence)

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
        { "create_fence",  Create },
        { "destroy_fence",  Destroy },
        { "reset_fences", ResetFences },
        { "get_fence_status", GetFenceStatus },
        { "wait_for_fences", WaitForFences },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_fence(lua_State *L)
    {
    udata_define(L, FENCE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

