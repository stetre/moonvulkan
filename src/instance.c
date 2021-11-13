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

static int freeinstance(lua_State *L, ud_t *ud)
    {
    VkInstance instance = (VkInstance)(uintptr_t)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    PFN_vkDestroyInstance DestroyInstance = NULL;
    if(ud->idt) DestroyInstance = ud->idt->DestroyInstance;
    freechildren(L, DEBUG_UTILS_MESSENGER_MT, ud);
    freechildren(L, DEBUG_REPORT_CALLBACK_MT, ud);
    freechildren(L, DEVICE_MT, ud);
    freechildren(L, PHYSICAL_DEVICE_MT, ud);
    freechildren(L, SURFACE_MT, ud);
    if(ud->idt) Free(L, ud->idt);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(instance, "instance");
    if(DestroyInstance) DestroyInstance(instance, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkResult ec;
    VkInstance instance;
    VkInstanceCreateInfo* info;
    const VkAllocationCallbacks *allocator = optallocator(L, 2);
#define CLEANUP zfreeVkInstanceCreateInfo(L, info, 1)
    info = zcheckVkInstanceCreateInfo(L, 1, &err);
    if(err) { CLEANUP; return argerror(L, 1); }
    ec = vk.CreateInstance(info, allocator, &instance);
    if(ec)
        { CLEANUP; CheckError(L, ec); return 0; }
    TRACE_CREATE(instance, "instance");
    ud = newuserdata_dispatchable(L, instance, INSTANCE_MT);
    ud->instance = instance;
/*  ud->parent_ud = NULL; */
    ud->destructor = freeinstance;  
    ud->allocator = allocator; /* see you later allocator */
    ud->idt = getproc_instance(L, instance, info);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

#define N 32
static int EnumeratePhysicalDevices(lua_State *L)
    {
    uint32_t count, remaining, tot, i;
    VkResult ec;
    VkPhysicalDevice physical_device[N];
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);

    lua_newtable(L);
    ec = ud->idt->EnumeratePhysicalDevices(instance, &remaining, NULL);
    CheckError(L, ec);
    if(remaining == 0)
        return 1;

    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }
        ec = ud->idt->EnumeratePhysicalDevices(instance, &count, physical_device);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);

        for(i=0; i<count; i++)
            {
            pushphysical_device(L, physical_device[i], instance);
            lua_rawseti(L, -2, ++tot);
            }
    } while(remaining > 0);

    return 1;
    }

static int EnumeratePhysicalDeviceGroups(lua_State *L)
    {
    int err;
    ud_t *ud;
    uint32_t count, remaining, tot, i;
    VkResult ec;
    VkPhysicalDeviceGroupPropertiesKHR* props;
    VkInstance instance = checkinstance(L, 1, &ud);
    CheckInstancePfn(L, ud, EnumeratePhysicalDeviceGroups);
#define CLEANUP zfreearrayVkPhysicalDeviceGroupPropertiesKHR(L, props, N, 1)
    props = znewchainarrayVkPhysicalDeviceGroupPropertiesKHR(L, N, &err);
    if(err) { CLEANUP; return lua_error(L); }
    lua_newtable(L);
    ec = ud->idt->EnumeratePhysicalDeviceGroups(instance, &remaining, NULL);
    if(ec) { CLEANUP; CheckError(L, ec); }
    if(remaining == 0) { CLEANUP; return 1; }

    tot = 0;
    do {
        if(remaining > N)
            { count = N; remaining -= N; }
        else
            { count = remaining; remaining = 0; }
        ec = ud->idt->EnumeratePhysicalDeviceGroups(instance, &count, props);
        if(ec && ec != VK_INCOMPLETE) { CLEANUP; CheckError(L, ec); }

        for(i=0; i<count; i++)
            {
            zpushVkPhysicalDeviceGroupPropertiesKHR(L, &props[i], instance);
            lua_rawseti(L, -2, ++tot);
            }
    } while(remaining > 0);

    CLEANUP;
#undef CLEANUP
    return 1;
    }


RAW_FUNC_DISPATCHABLE(instance)
TYPE_FUNC(instance)
INSTANCE_FUNC(instance)
DEVICE_FUNC(instance)
PARENT_FUNC(instance)
DELETE_FUNC(instance)
DESTROY_FUNC(instance)

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
        { "create_instance", Create },
        { "destroy_instance", Destroy },
        { "enumerate_physical_devices", EnumeratePhysicalDevices },
        { "enumerate_physical_device_groups", EnumeratePhysicalDeviceGroups },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_instance(lua_State *L)
    {
    udata_define(L, INSTANCE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

