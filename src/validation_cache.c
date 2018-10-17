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


static int freevalidation_cache(lua_State *L, ud_t *ud)
    {
    VkValidationCacheEXT validation_cache = (VkValidationCacheEXT)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(validation_cache, "validation_cache");
    UD(device)->ddt->DestroyValidationCacheEXT(device, validation_cache, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkValidationCacheEXT validation_cache;
    VkValidationCacheCreateInfoEXT* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckDevicePfn(L, device_ud, CreateValidationCacheEXT);
#define CLEANUP zfreeVkValidationCacheCreateInfoEXT(L, info, 1)
    info = zcheckVkValidationCacheCreateInfoEXT(L, 2, &err);
    if(err) { CLEANUP;  return argerror(L, 2); }
    lua_getfield(L, 2, "initial_data");
    info->pInitialData = (void*)luaL_optlstring(L, -1, NULL, &info->initialDataSize);
    ec = device_ud->ddt->CreateValidationCacheEXT(device, info, allocator, &validation_cache);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    TRACE_CREATE(validation_cache, "validation_cache");
    ud = newuserdata_nondispatchable(L, validation_cache, VALIDATION_CACHE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freevalidation_cache;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int GetValidationCacheData(lua_State *L)
    {
    VkResult ec;
    size_t size;
    char *data;
    ud_t *ud;
    VkValidationCacheEXT validation_cache = checkvalidation_cache(L, 1, &ud);
    VkDevice device = ud->device;
    
    CheckDevicePfn(L, ud, GetValidationCacheDataEXT);

    /* first, get the size */
    ec = ud->ddt->GetValidationCacheDataEXT(device, validation_cache, &size, NULL);
    CheckError(L, ec);

    if(size == 0)
        {
        lua_pushstring(L, "");
        return 1;
        }
    
    data = (char*)Malloc(L, size);
    ec = ud->ddt->GetValidationCacheDataEXT(device, validation_cache, &size, data);
    if(ec)
        {
        Free(L, data);
        CheckError(L, ec); /* raises an error */
        return 0;
        }

    lua_pushlstring(L, data, size);
    Free(L, data);
    return 1;
    }

static int MergeValidationCaches(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkValidationCacheEXT *source;
    VkValidationCacheEXT destination = checkvalidation_cache(L, 1, &ud);
    VkDevice device = ud->device;

    CheckDevicePfn(L, ud, MergeValidationCachesEXT);

    source = checkvalidation_cachelist(L, 2, &count, &err, NULL);
    if(err) return argerrorc(L, 2, err);

    ec = ud->ddt->MergeValidationCachesEXT(device, destination, count, source);
    Free(L, source);
    CheckError(L, ec);
    return 0;
# undef first
    }

RAW_FUNC(validation_cache)
TYPE_FUNC(validation_cache)
INSTANCE_FUNC(validation_cache)
DEVICE_FUNC(validation_cache)
PARENT_FUNC(validation_cache)
DELETE_FUNC(validation_cache)
DESTROY_FUNC(validation_cache)

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
        { "create_validation_cache",  Create },
        { "destroy_validation_cache",  Destroy },
        { "merge_validation_caches", MergeValidationCaches },
        { "get_validation_cache_data", GetValidationCacheData },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_validation_cache(lua_State *L)
    {
    udata_define(L, VALIDATION_CACHE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

