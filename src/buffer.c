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

static int freebuffer(lua_State *L, ud_t *ud)
    {
    VkBuffer buffer = (VkBuffer)ud->handle;
    VkDevice device = ud->device;
    const VkAllocationCallbacks *allocator = ud->allocator;
    freechildren(L, BUFFER_VIEW_MT, ud);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(buffer, "buffer");
    UD(device)->ddt->DestroyBuffer(device, buffer, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkBuffer buffer;
    VkBufferCreateInfo info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    if(echeckbuffercreateinfo(L, 2, &info)) return argerror(L, 2);

    ec = device_ud->ddt->CreateBuffer(device, &info, allocator, &buffer);
    freebuffercreateinfo(L, &info);
    CheckError(L, ec);
    TRACE_CREATE(buffer, "buffer");
    ud = newuserdata_nondispatchable(L, buffer, BUFFER_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->destructor = freebuffer;
    ud->allocator = allocator;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int GetBufferMemoryRequirements2(lua_State *L, VkBuffer buffer, ud_t *ud) //@@DOC
    {
    VkMemoryRequirements2KHR req;
    VkBufferMemoryRequirementsInfo2KHR info;
    VkDevice device = ud->device;

    info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
    info.pNext = NULL;
    info.buffer = buffer;

    memset(&req, 0, sizeof(req));
    req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
    req.pNext = NULL; //@@ next in chain

    ud->ddt->GetBufferMemoryRequirements2KHR(device, &info, &req);
    return pushmemoryrequirements2(L, &req);
    }

static int GetBufferMemoryRequirements(lua_State *L)
    {
    ud_t *ud; 
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    VkDevice device = ud->device;
    VkMemoryRequirements req;

    if(ud->ddt->GetBufferMemoryRequirements2KHR)
        return GetBufferMemoryRequirements2(L, buffer, ud);

    ud->ddt->GetBufferMemoryRequirements(device, buffer, &req);
    return pushmemoryrequirements(L, &req);
    }

static int BindBufferMemory(lua_State *L)
    {
    VkResult ec;
    ud_t *ud; 
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    VkDevice device = ud->device;
    VkDeviceMemory memory = checkdevice_memory(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    ec = ud->ddt->BindBufferMemory(device, buffer, memory, offset);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC(buffer)
TYPE_FUNC(buffer)
INSTANCE_FUNC(buffer)
DEVICE_FUNC(buffer)
PARENT_FUNC(buffer)
DELETE_FUNC(buffer)
DESTROY_FUNC(buffer)

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
        { "create_buffer",  Create },
        { "destroy_buffer",  Destroy },
        { "get_buffer_memory_requirements", GetBufferMemoryRequirements },
        { "bind_buffer_memory", BindBufferMemory },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_buffer(lua_State *L)
    {
    udata_define(L, BUFFER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

