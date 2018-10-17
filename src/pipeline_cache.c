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

static int freepipeline_cache(lua_State *L, ud_t *ud)
    {
    VkPipelineCache pipeline_cache = (VkPipelineCache)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(pipeline_cache, "pipeline_cache");
    UD(device)->ddt->DestroyPipelineCache(device, pipeline_cache, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkPipelineCache pipeline_cache;
    VkPipelineCacheCreateInfo* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;
#define CLEANUP zfreeVkPipelineCacheCreateInfo(L, info, 1)
    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        info = zcheckVkPipelineCacheCreateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        lua_getfield(L, 2, "initial_data");
        info->pInitialData = (void*)luaL_optlstring(L, -1, NULL, &info->initialDataSize);
        }
    else
        {
        info = znewVkPipelineCacheCreateInfo(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->flags = optflags(L, 2, 0);
        info->pInitialData = (void*)luaL_optlstring(L, 3, NULL, &info->initialDataSize);
        }
    ec = device_ud->ddt->CreatePipelineCache(device, info, allocator, &pipeline_cache);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    TRACE_CREATE(pipeline_cache, "pipeline_cache");
    ud = newuserdata_nondispatchable(L, pipeline_cache, PIPELINE_CACHE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freepipeline_cache;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int GetPipelineCacheData(lua_State *L)
    {
    VkResult ec;
    size_t size;
    char *data;
    ud_t *ud;
    VkPipelineCache pipeline_cache = checkpipeline_cache(L, 1, &ud);
    VkDevice device = ud->device;
    /* first, get the size */
    ec = ud->ddt->GetPipelineCacheData(device, pipeline_cache, &size, NULL);
    CheckError(L, ec);
    if(size == 0)
        { lua_pushstring(L, ""); return 1; }
    data = (char*)Malloc(L, size);
    ec = ud->ddt->GetPipelineCacheData(device, pipeline_cache, &size, data);
    if(ec) { Free(L, data); CheckError(L, ec); return 0; }
    lua_pushlstring(L, data, size);
    Free(L, data);
    return 1;
    }

static int MergePipelineCaches(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkPipelineCache destination = checkpipeline_cache(L, 1, &ud);
    VkDevice device = ud->device;
    VkPipelineCache *source = checkpipeline_cachelist(L, 2, &count, &err, NULL);
    if(err) return argerrorc(L, 2, err);
    ec = ud->ddt->MergePipelineCaches(device, destination, count, source);
    Free(L, source);
    CheckError(L, ec);
    return 0;
# undef first
    }


RAW_FUNC(pipeline_cache)
TYPE_FUNC(pipeline_cache)
INSTANCE_FUNC(pipeline_cache)
DEVICE_FUNC(pipeline_cache)
PARENT_FUNC(pipeline_cache)
DELETE_FUNC(pipeline_cache)
DESTROY_FUNC(pipeline_cache)

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
        { "create_pipeline_cache",  Create },
        { "destroy_pipeline_cache",  Destroy },
        { "get_pipeline_cache_data", GetPipelineCacheData },
        { "merge_pipeline_caches", MergePipelineCaches },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_pipeline_cache(lua_State *L)
    {
    udata_define(L, PIPELINE_CACHE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

