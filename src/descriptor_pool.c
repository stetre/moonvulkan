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

static int freedescriptor_pool(lua_State *L, ud_t *ud)
    {
    VkDescriptorPool descriptor_pool = (VkDescriptorPool)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    freechildren(L, DESCRIPTOR_SET_MT, ud);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(descriptor_pool, "descriptor_pool");
    UD(device)->ddt->DestroyDescriptorPool(device, descriptor_pool, allocator);
    return 0;
    }


static void FreeInfo(lua_State *L, VkDescriptorPoolCreateInfo *p)
    {
    if(p->pPoolSizes)
        Free(L, (void*)p->pPoolSizes);
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkDescriptorPool descriptor_pool;
    VkDescriptorPoolCreateInfo info;
    int err;
    uint32_t count;

    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 5);

    memset(&info, 0, sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = NULL;

    info.flags = checkflags(L, 2);
    info.maxSets = luaL_checkinteger(L, 3);
    info.pPoolSizes = echeckdescriptorpoolsizelist(L, 4, &count, &err);
    info.poolSizeCount = count;
    if(err)
        { FreeInfo(L, &info); return argerror(L, 4); }

    ec = device_ud->ddt->CreateDescriptorPool(device, &info, allocator, &descriptor_pool);
    FreeInfo(L, &info);
    CheckError(L, ec);
    TRACE_CREATE(descriptor_pool, "descriptor_pool");
    ud = newuserdata_nondispatchable(L, descriptor_pool, DESCRIPTOR_POOL_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freedescriptor_pool;
    ud->ddt = device_ud->ddt;
    /* Rfr. Vulkan Spec '13.2.3. Allocation of Descriptor Sets': */
    if(info.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        MarkFreeDescriptorSetAllowed(ud); 
    return 1;
    }


static int ResetDescriptorPool(lua_State *L)
    {
    ud_t *ud;
    VkDescriptorPool descriptor_pool = checkdescriptor_pool(L, 1, &ud);
    VkDevice device = ud->device;
    VkDescriptorPoolResetFlags flags = optflags(L, 2, 0);
    VkResult ec = ud->ddt->ResetDescriptorPool(device, descriptor_pool, flags);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC(descriptor_pool)
TYPE_FUNC(descriptor_pool)
INSTANCE_FUNC(descriptor_pool)
DEVICE_FUNC(descriptor_pool)
PARENT_FUNC(descriptor_pool)
DELETE_FUNC(descriptor_pool)
DESTROY_FUNC(descriptor_pool)

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
        { "create_descriptor_pool",  Create },
        { "destroy_descriptor_pool",  Destroy },
        { "reset_descriptor_pool", ResetDescriptorPool },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_descriptor_pool(lua_State *L)
    {
    udata_define(L, DESCRIPTOR_POOL_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

