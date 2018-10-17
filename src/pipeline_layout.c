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

static int freepipeline_layout(lua_State *L, ud_t *ud)
    {
    VkPipelineLayout pipeline_layout = (VkPipelineLayout)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(pipeline_layout, "pipeline_layout");
    UD(device)->ddt->DestroyPipelineLayout(device, pipeline_layout, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

#define CLEANUP zfreeVkPipelineLayoutCreateInfo(L, info, 1)
    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        info = zcheckVkPipelineLayoutCreateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        VkFlags flags = checkflags(L, 2);
        info = znewVkPipelineLayoutCreateInfo(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->flags = flags;
        info->pSetLayouts = checkdescriptor_set_layoutlist(L, 3, &info->setLayoutCount, &err, NULL);
        if(err<0) { CLEANUP; return argerror(L, 3); }
        info->pPushConstantRanges = zcheckarrayVkPushConstantRange(L, 4,
                                        &info->pushConstantRangeCount, &err);
        if(err<0) { CLEANUP; return argerror(L, 3); }
        else if(err == ERR_NOTPRESENT) lua_pop(L, 1);
        }
    ec = device_ud->ddt->CreatePipelineLayout(device, info, allocator, &pipeline_layout);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    TRACE_CREATE(pipeline_layout, "pipeline_layout");
    ud = newuserdata_nondispatchable(L, pipeline_layout, PIPELINE_LAYOUT_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freepipeline_layout;
    ud->ddt = device_ud->ddt;
    return 1;
    }

RAW_FUNC(pipeline_layout)
TYPE_FUNC(pipeline_layout)
INSTANCE_FUNC(pipeline_layout)
DEVICE_FUNC(pipeline_layout)
PARENT_FUNC(pipeline_layout)
DELETE_FUNC(pipeline_layout)
DESTROY_FUNC(pipeline_layout)

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
        { "create_pipeline_layout",  Create },
        { "destroy_pipeline_layout",  Destroy },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_pipeline_layout(lua_State *L)
    {
    udata_define(L, PIPELINE_LAYOUT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

