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

static int freedevice(lua_State *L, ud_t *ud)
    {
    VkDevice device = (VkDevice)(uintptr_t)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    PFN_vkDeviceWaitIdle DeviceWaitIdle = NULL;
    PFN_vkDestroyDevice DestroyDevice = NULL;
    if(ud->ddt)
        {
        DeviceWaitIdle = ud->ddt->DeviceWaitIdle;
        DestroyDevice = ud->ddt->DestroyDevice;
        }
    freechildren(L, SAMPLER_YCBCR_CONVERSION_MT, ud);
    freechildren(L, VALIDATION_CACHE_MT, ud);
    freechildren(L, DESCRIPTOR_UPDATE_TEMPLATE_MT, ud);
    freechildren(L, SWAPCHAIN_MT, ud);
    freechildren(L, COMMAND_POOL_MT, ud);
    freechildren(L, PIPELINE_CACHE_MT, ud);
    freechildren(L, PIPELINE_MT, ud);
    freechildren(L, SAMPLER_MT, ud);
    freechildren(L, SEMAPHORE_MT, ud);
    freechildren(L, FENCE_MT, ud);
    freechildren(L, DEVICE_MEMORY_MT, ud);
    freechildren(L, BUFFER_MT, ud);
    freechildren(L, IMAGE_MT, ud);
    freechildren(L, EVENT_MT, ud);
    freechildren(L, SHADER_MODULE_MT, ud);
    freechildren(L, FRAMEBUFFER_MT, ud);
    freechildren(L, RENDER_PASS_MT, ud);
    freechildren(L, PIPELINE_LAYOUT_MT, ud);
    freechildren(L, DESCRIPTOR_SET_LAYOUT_MT, ud);
    freechildren(L, DESCRIPTOR_POOL_MT, ud);
    freechildren(L, QUERY_POOL_MT, ud);
    freechildren(L, QUEUE_MT, ud);
    if(ud->ddt) Free(L, ud->ddt);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(device, "device");
    if(DeviceWaitIdle) DeviceWaitIdle(device);
    if(DestroyDevice) DestroyDevice(device, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *physdev_ud;
    VkResult ec;
    VkDevice device;
    VkDeviceCreateInfo_CHAIN info;

    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &physdev_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);

    if(echeckdevicecreateinfo(L, 2, &info, physdev_ud)) return argerror(L, 2);

    ec = physdev_ud->idt->CreateDevice(physical_device, &info.p1, allocator, &device);
    if(ec)
        {
        freedevicecreateinfo(L, &info);
        CheckError(L, ec);
        return 0;
        }

    TRACE_CREATE(device, "device");
    ud = newuserdata_dispatchable(L, device, DEVICE_MT);
    ud->parent_ud = UD(physical_device)->parent_ud; /* instance ud */
    ud->instance = UD(physical_device)->instance;
    ud->allocator = allocator;
    ud->destructor = freedevice;
    ud->ddt = getproc_device(L, device, &info.p1);
    freedevicecreateinfo(L, &info);
    return 1;
    }


static int GetDeviceQueue(lua_State *L)
    {
    VkQueue queue;
    ud_t *ud;
    VkDeviceQueueInfo2_CHAIN info;
    VkDevice device = checkdevice(L, 1, &ud);
    if(lua_istable(L, 2))
        {
        if(echeckdevicequeueinfo2(L, 2, &info)) return argerror(L, 2);
        }
    else
        {
        memset(&info, 0, sizeof(info));
        info.p1.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        info.p1.pNext = NULL;
        info.p1.flags = 0;
        info.p1.queueFamilyIndex = luaL_checkinteger(L, 2);
        info.p1.queueIndex = luaL_checkinteger(L, 3);
        info.p1.flags = optflags(L, 4, 0);
        }
    if(ud->ddt->GetDeviceQueue2 && info.p1.flags != 0)
        ud->ddt->GetDeviceQueue2(device, &info.p1, &queue);
    else
        ud->ddt->GetDeviceQueue(device, info.p1.queueFamilyIndex, info.p1.queueIndex, &queue);
    
    freedevicequeueinfo2(L, &info);
    if(!queue) return luaL_error(L, "cannot retrieve queue");
    pushqueue(L, queue, device);
    return 1;
    }

static int DeviceWaitIdle(lua_State *L)
    {
    ud_t *ud;
    VkDevice device = checkdevice(L, 1, &ud);
    VkResult ec = ud->ddt->DeviceWaitIdle(device);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC_DISPATCHABLE(device)
TYPE_FUNC(device)
INSTANCE_FUNC(device)
DEVICE_FUNC(device)
PARENT_FUNC(device)
DELETE_FUNC(device)
DESTROY_FUNC(device)

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
        { "create_device",  Create },
        { "destroy_device",  Destroy },
        { "get_device_queue", GetDeviceQueue },
        { "device_wait_idle", DeviceWaitIdle },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_device(lua_State *L)
    {
    udata_define(L, DEVICE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

