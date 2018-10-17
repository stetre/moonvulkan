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

static int freepipeline(lua_State *L, ud_t *ud)
    {
    VkPipeline pipeline = (VkPipeline)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    PFN_vkDestroyPipeline DestroyPipeline = ud->ddt->DestroyPipeline;
    VkDevice device = ud->device;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(pipeline, "pipeline");
    DestroyPipeline(device, pipeline, allocator);
    return 0;
    }

static int newpipeline(lua_State *L, VkPipeline  pipeline, VkDevice device, const VkAllocationCallbacks *allocator)
    {
    ud_t *ud;
    TRACE_CREATE(pipeline, "pipeline");
    ud = newuserdata_nondispatchable(L, pipeline, PIPELINE_MT);
    ud->parent_ud = UD(device);
    ud->device = device;
    ud->instance = UD(device)->instance;
    ud->allocator = allocator;
    ud->destructor = freepipeline;
    ud->ddt = UD(device)->ddt;
    return 1;
    }


static int CreateGraphics(lua_State *L)
    {
    int err;
    uint32_t count, i;
    VkResult ec;
    VkPipeline *pipeline;

    VkDevice device = checkdevice(L, 1, NULL);
    VkPipelineCache cache = testpipeline_cache(L, 2, NULL);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
#define CLEANUP zfreearrayVkGraphicsPipelineCreateInfo(L, info, count, 1)
    VkGraphicsPipelineCreateInfo *info = zcheckarrayVkGraphicsPipelineCreateInfo(L, 3, &count, &err);
    if(err) { CLEANUP; return argerror(L, 3); }

    pipeline = (VkPipeline*)MallocNoErr(L, count*sizeof(VkPipeline));
    if(!pipeline) { CLEANUP; return errmemory(L); }

    ec = UD(device)->ddt->CreateGraphicsPipelines(device, cache, count, info, allocator, pipeline);
    CLEANUP;
#undef CLEANUP
    if(ec)
        {
        Free(L, pipeline);
        CheckError(L, ec);
        }
    lua_newtable(L);
    for(i = 0; i < count; i++)
        {
        newpipeline(L, pipeline[i], device, allocator);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, pipeline);
    return 1;
    }

static int CreateCompute(lua_State *L)
    {
    int err;
    uint32_t count, i;
    VkResult ec;
    VkPipeline *pipeline;

    VkDevice device = checkdevice(L, 1, NULL);
    VkPipelineCache cache = testpipeline_cache(L, 2, NULL);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
#define CLEANUP zfreearrayVkComputePipelineCreateInfo(L, info, count, 1)
    VkComputePipelineCreateInfo *info = zcheckarrayVkComputePipelineCreateInfo(L, 3, &count, &err);
    if(err) { CLEANUP; return argerror(L, 3); }

    pipeline = (VkPipeline*)MallocNoErr(L, count*sizeof(VkPipeline));
    if(!pipeline) { CLEANUP; return errmemory(L); }

    ec = UD(device)->ddt->CreateComputePipelines(device, cache, count, info, allocator, pipeline);
    CLEANUP;
#undef CLEANUP

    if(ec)
        {
        Free(L, pipeline);
        CheckError(L, ec);
        }
    lua_newtable(L);
    for(i = 0; i < count; i++)
        {
        newpipeline(L, pipeline[i], device, allocator);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, pipeline);
    return 1;
    }

RAW_FUNC(pipeline)
TYPE_FUNC(pipeline)
INSTANCE_FUNC(pipeline)
DEVICE_FUNC(pipeline)
PARENT_FUNC(pipeline)
DELETE_FUNC(pipeline)
DESTROY_FUNC(pipeline)

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
        { "create_graphics_pipelines",  CreateGraphics },
        { "create_compute_pipelines",  CreateCompute },
        { "destroy_pipeline",  Destroy },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_pipeline(lua_State *L)
    {
    udata_define(L, PIPELINE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

