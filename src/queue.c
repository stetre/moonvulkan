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

static int freequeue(lua_State *L, ud_t *ud)
    {
    VkQueue queue = (VkQueue)(uintptr_t)ud->handle;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(queue, "queue");
    return 0;
    }

int pushqueue(lua_State *L, VkQueue queue, VkDevice device)
    { 
    ud_t *ud = UD(queue);
    if(!ud)
        {
        /* create the userdata associated with this object */
        ud = newuserdata_dispatchable(L, queue, QUEUE_MT);
        ud->parent_ud = UD(device);
        ud->instance = UD(device)->instance;
        ud->device = device;
        ud->destructor = freequeue;
        ud->ddt = UD(device)->ddt;
        TRACE_CREATE(queue, "queue");
        return 1;
        }
    return pushuserdata(L, ud);
    }

static int QueueSubmit(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkSubmitInfo* submits;
    VkQueue queue = checkqueue(L, 1, &ud);
    VkFence fence = testfence(L, 3, NULL);
#define CLEANUP zfreearrayVkSubmitInfo(L, submits, count, 1)
    submits = zcheckarrayVkSubmitInfo(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->QueueSubmit(queue, count, submits, fence);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int QueueSubmit2(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkSubmitInfo2KHR* submits;
    VkQueue queue = checkqueue(L, 1, &ud);
    VkFence fence = testfence(L, 3, NULL);
    CheckDevicePfn(L, ud, QueueSubmit2KHR);
#define CLEANUP zfreearrayVkSubmitInfo2KHR(L, submits, count, 1)
    submits = zcheckarrayVkSubmitInfo2KHR(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->QueueSubmit2KHR(queue, count, submits, fence);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int QueueBindSparse(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkBindSparseInfo* binds;
    VkQueue queue = checkqueue(L, 1, &ud);
    VkFence fence = testfence(L, 3, NULL);
#define CLEANUP zfreearrayVkBindSparseInfo(L, binds, count, 1)
    binds = zcheckarrayVkBindSparseInfo(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->QueueBindSparse(queue, count, binds, fence);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int QueueWaitIdle(lua_State *L)
    {
    ud_t *ud;
    VkQueue queue = checkqueue(L, 1, &ud);
    VkResult ec = ud->ddt->QueueWaitIdle(queue);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC_DISPATCHABLE(queue)
TYPE_FUNC(queue)
INSTANCE_FUNC(queue)
DEVICE_FUNC(queue)
PARENT_FUNC(queue)
DELETE_FUNC(queue)

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
        { "queue_wait_idle", QueueWaitIdle },
        { "queue_submit", QueueSubmit },
        { "queue_submit2", QueueSubmit2 },
        { "queue_bind_sparse", QueueBindSparse },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_queue(lua_State *L)
    {
    udata_define(L, QUEUE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

