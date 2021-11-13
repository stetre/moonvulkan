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

static int freedescriptor_update_template(lua_State *L, ud_t *ud)
    {
    VkDescriptorUpdateTemplate du_template = (VkDescriptorUpdateTemplate)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(du_template, "descriptor_update_template");
    UD(device)->ddt->DestroyDescriptorUpdateTemplate(device, du_template, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkDescriptorUpdateTemplate du_template;
    VkDescriptorUpdateTemplateCreateInfoKHR* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckDevicePfn(L, device_ud, CreateDescriptorUpdateTemplate);
#define CLEANUP zfreeVkDescriptorUpdateTemplateCreateInfoKHR(L, info, 1)
    info = zcheckVkDescriptorUpdateTemplateCreateInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = device_ud->ddt->CreateDescriptorUpdateTemplate(device, info, allocator, &du_template);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    TRACE_CREATE(du_template, "descriptor_update_template");
    ud = newuserdata_nondispatchable(L, du_template, DESCRIPTOR_UPDATE_TEMPLATE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freedescriptor_update_template;
    ud->ddt = device_ud->ddt;
    return 1;
    }

/*------------------------------------------------------------------------------*
 | Functions                                                                    |
 *------------------------------------------------------------------------------*/

static int UpdateDescriptorSetWithTemplate(lua_State *L)
    {
    size_t len;
    ud_t *ud1, *ud2;
    VkDescriptorSet descriptor_set = checkdescriptor_set(L, 1, &ud1);
    VkDescriptorUpdateTemplate du_template = checkdescriptor_update_template(L, 2, &ud2);
    const void* data = luaL_checklstring(L, 3, &len);
    CheckDevicePfn(L, ud1, UpdateDescriptorSetWithTemplate);
    ud1->ddt->UpdateDescriptorSetWithTemplate(ud1->device, descriptor_set, du_template, data);
    return 0;
    }

/*------------------------------------------------------------------------------*
 | Methods                                                                      |
 *------------------------------------------------------------------------------*/

RAW_FUNC(descriptor_update_template)
TYPE_FUNC(descriptor_update_template)
INSTANCE_FUNC(descriptor_update_template)
DEVICE_FUNC(descriptor_update_template)
PARENT_FUNC(descriptor_update_template)
DELETE_FUNC(descriptor_update_template)
DESTROY_FUNC(descriptor_update_template)

/*------------------------------------------------------------------------------*
 | Registration                                                                 |
 *------------------------------------------------------------------------------*/

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
        { "create_descriptor_update_template",  Create },
        { "destroy_descriptor_update_template",  Destroy },
        { "update_descriptor_set_with_template", UpdateDescriptorSetWithTemplate },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_descriptor_update_template(lua_State *L)
    {
    udata_define(L, DESCRIPTOR_UPDATE_TEMPLATE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

