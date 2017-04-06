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
    VkSemaphoreCreateInfo info;

    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);

    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = NULL;
    info.flags = optflags(L, 2, 0);

    ec = device_ud->ddt->CreateSemaphore(device, &info, allocator, &semaphore);
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
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_semaphore(lua_State *L)
    {
    udata_define(L, SEMAPHORE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

