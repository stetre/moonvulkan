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

static int freequery_pool(lua_State *L, ud_t *ud)
    {
    VkQueryPool query_pool = (VkQueryPool)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(query_pool, "query_pool");
    UD(device)->ddt->DestroyQueryPool(device, query_pool, allocator);
    return 0;
    }


static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
#define CLEANUP zfreeVkQueryPoolCreateInfo(L, info, 1)
    info = zcheckVkQueryPoolCreateInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = device_ud->ddt->CreateQueryPool(device, info, allocator, &query_pool);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    TRACE_CREATE(query_pool, "query_pool");
    ud = newuserdata_nondispatchable(L, query_pool, QUERY_POOL_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freequery_pool;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int GetQueryPoolResults(lua_State *L)
    {
// @@ TODO: specialization for performance counters results (see VK_KHR_performance_query).
    VkResult ec;
    ud_t * ud;
    VkQueryPool query_pool  = checkquery_pool(L, 1, &ud);
    VkDevice device = ud->device;
    uint32_t first = luaL_checkinteger(L, 2);
    uint32_t count = luaL_checkinteger(L, 3);
    size_t size = luaL_checkinteger(L, 4);
    VkDeviceSize stride = luaL_checkinteger(L, 5);
    VkQueryResultFlags flags = checkflags(L, 6);

    char* data = (char*)Malloc(L, size);

    ec = ud->ddt->GetQueryPoolResults(device, query_pool, first, count, size, data, stride, flags);
    if(ec)
        {
        Free(L, data);
        CheckError(L, ec);
        return 0;
        }
    lua_pushlstring(L, data, size);
    Free(L, data);
    return 1;
    }

static int ResetQueryPool(lua_State *L)
    {
    ud_t *ud;
    VkQueryPool query_pool  = checkquery_pool(L, 1, &ud);
    VkDevice device = ud->device;
    uint32_t first = luaL_checkinteger(L, 2);
    uint32_t count = luaL_checkinteger(L, 3);
    CheckDevicePfn(L, ud, ResetQueryPool);
    ud->ddt->ResetQueryPool(device, query_pool, first, count);
    return 0;
    }

RAW_FUNC(query_pool)
TYPE_FUNC(query_pool)
INSTANCE_FUNC(query_pool)
DEVICE_FUNC(query_pool)
PARENT_FUNC(query_pool)
DELETE_FUNC(query_pool)
DESTROY_FUNC(query_pool)

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
        { "create_query_pool",  Create },
        { "destroy_query_pool",  Destroy },
        { "get_query_pool_results", GetQueryPoolResults },
        { "reset_query_pool", ResetQueryPool },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_query_pool(lua_State *L)
    {
    udata_define(L, QUERY_POOL_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

