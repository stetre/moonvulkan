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


static void FreeInfo(lua_State *L, VkDescriptorSetLayoutCreateInfo *p)
    {
    if(p->pBindings)
        freedescriptorsetlayoutbindinglist(L, 
            (VkDescriptorSetLayoutBinding*)p->pBindings, p->bindingCount);
    }

static int Create(lua_State *L)
    {
    ud_t *ud;
    int err;
    VkResult ec;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSetLayoutCreateInfo info;

    VkDevice device = checkdevice(L, 1, NULL);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);

    memset(&info, 0, sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext = NULL;
    info.flags = checkflags(L, 2);

    info.pBindings = echeckdescriptorsetlayoutbindinglist(L, 3, &info.bindingCount, &err);
    if(err < 0)
        { FreeInfo(L, &info); return luaL_argerror(L, 3, lua_tostring(L, -1)); }
    if(err) lua_pop(L, 1);

    ec = UD(device)->ddt->CreateDescriptorSetLayout(device, &info, allocator, &descriptor_set_layout);
    FreeInfo(L, &info);
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
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_descriptor_set_layout(lua_State *L)
    {
    udata_define(L, DESCRIPTOR_SET_LAYOUT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

