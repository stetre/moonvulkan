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

static int freedescriptor_set_layout(lua_State *L, ud_t *ud)
    {
    VkDescriptorSetLayout descriptor_set_layout = (VkDescriptorSetLayout)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(descriptor_set_layout, "descriptor_set_layout");
    UD(device)->ddt->DestroyDescriptorSetLayout(device, descriptor_set_layout, allocator);
    return 0;
    }


static int Create(lua_State *L)
    {
    ud_t *ud;
    int err;
    VkResult ec;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSetLayoutCreateInfo *info;
    VkDevice device = checkdevice(L, 1, NULL);
    const VkAllocationCallbacks *allocator = NULL;

#define CLEANUP zfreeVkDescriptorSetLayoutCreateInfo(L, info, 1)
    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        info = zcheckVkDescriptorSetLayoutCreateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        info = znewVkDescriptorSetLayoutCreateInfo(L, &err);
        info->flags = optflags(L, 2, 0);
        info->pBindings = zcheckarrayVkDescriptorSetLayoutBinding(L, 3, &info->bindingCount, &err);
        if(err < 0) { CLEANUP; return argerror(L, 3); }
        if(err == ERR_NOTPRESENT) lua_pop(L, 1);
        }

    ec = UD(device)->ddt->CreateDescriptorSetLayout(device, info, allocator, &descriptor_set_layout);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    TRACE_CREATE(descriptor_set_layout, "descriptor_set_layout");
    ud = newuserdata_nondispatchable(L, descriptor_set_layout, DESCRIPTOR_SET_LAYOUT_MT);
    ud->parent_ud = UD(device);
    ud->device = device;
    ud->instance = UD(device)->instance;
    ud->allocator = allocator;
    ud->destructor = freedescriptor_set_layout;
    ud->ddt = UD(device)->ddt;
    return 1;
    }

static int GetDescriptorSetLayoutSupport(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkDescriptorSetLayoutCreateInfo *info=NULL;
    VkDescriptorSetLayoutSupport *support=NULL;
    VkDevice device = checkdevice(L, 1, &ud);
#define CLEANUP do { zfreeVkDescriptorSetLayoutCreateInfo(L, info, 1); \
                    zfreeVkDescriptorSetLayoutSupport(L, support, 1); } while(0);
    CheckDevicePfn(L, ud, GetDescriptorSetLayoutSupport);
    info = zcheckVkDescriptorSetLayoutCreateInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    support = znewchainVkDescriptorSetLayoutSupport(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetDescriptorSetLayoutSupport(device, info, support);
    zpushVkDescriptorSetLayoutSupport(L, support);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

RAW_FUNC(descriptor_set_layout)
TYPE_FUNC(descriptor_set_layout)
INSTANCE_FUNC(descriptor_set_layout)
DEVICE_FUNC(descriptor_set_layout)
PARENT_FUNC(descriptor_set_layout)
DELETE_FUNC(descriptor_set_layout)
DESTROY_FUNC(descriptor_set_layout)

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
        { "create_descriptor_set_layout",  Create },
        { "destroy_descriptor_set_layout",  Destroy },
        { "get_descriptor_set_layout_support", GetDescriptorSetLayoutSupport },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_descriptor_set_layout(lua_State *L)
    {
    udata_define(L, DESCRIPTOR_SET_LAYOUT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

