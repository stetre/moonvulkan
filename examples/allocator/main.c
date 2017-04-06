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

#define _ISOC11_SOURCE /* see man aligned_alloc(3) */
#include <stdlib.h>
#include <lua.h>
#include "lualib.h"
#include "lauxlib.h"
#define VK_NO_PROTOTYPES 1
#include "vulkan/vulkan.h" /* local include */

static VkAllocationCallbacks Allocator; /* our custom allocator */
static int TraceEnabled = 0; /* =1: trace allocator calls, =0: don't */


/* Allocator callbacks ------------------------------------------------------*/

static VKAPI_ATTR void* VKAPI_CALL Reallocation
    (void *ud, void *original, size_t size, size_t alignment, VkSystemAllocationScope scope) 
    {
    void *mem = realloc(original, size);
    if(TraceEnabled)
        printf("ALLOCATOR: Reallocation(%p, %p, %lu, %lu, %u) -> %p\n", 
            ud, original, size, alignment, scope, mem);
    return mem;
    }

static VKAPI_ATTR void* VKAPI_CALL Allocation
    (void *ud, size_t size, size_t alignment, VkSystemAllocationScope scope) 
    {
    void *mem = aligned_alloc(alignment, size);
    if(TraceEnabled)
        printf("ALLOCATOR: Allocation(%p, %lu, %lu, %u) -> %p\n", ud, size, alignment, scope, mem);
    return mem;
    }

static VKAPI_ATTR void VKAPI_CALL Free(void *ud, void *mem) 
    {
    if(TraceEnabled)
        printf("ALLOCATOR: Free(%p, %p)\n", ud, mem);
    free(mem);
    }

static VKAPI_ATTR void VKAPI_CALL InternalAllocation
    (void* ud, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    if(TraceEnabled)
        printf("ALLOCATOR: InternalAllocation(%p, %lu, %u, %u)\n", ud, size, type, scope);
    }

static VKAPI_ATTR void VKAPI_CALL InternalFree
    (void* ud, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    if(TraceEnabled)
        printf("ALLOCATOR: InternalFree(%p, %lu, %u, %u)\n", ud, size, type, scope);
    }

/* Module functions ---------------------------------------------------------*/

static int Get(lua_State *L)
/* allocatorLUD = allocator.get() 
 * Returns the a pointer to allocator as a lightuserdata.
 * This can then be passed to MoonVulkan bindings to vkCreate/Allocate*** functions.
 */
    {
    lua_pushlightuserdata(L, &Allocator);
    return 1;
    }

static int Trace(lua_State *L)
/* allocator.trace(boolean) 
 * enable/disable tracing of the allocator's function calls
 */
    {
    if(lua_type(L, 1) != LUA_TBOOLEAN)
        return luaL_argerror(L, 1, "boolean expected");
    TraceEnabled = lua_toboolean(L, 1);
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "get", Get },
        { "trace", Trace },
        { NULL, NULL } /* sentinel */
    };

int luaopen_allocator(lua_State *L)
/* Lua calls this function to load the module */
    {
    /* Init the allocator: */
    Allocator.pUserData = (void*)0xaabbccdd; /* just to see that it is passed to the callbacks */
    Allocator.pfnAllocation = Allocation;
    Allocator.pfnFree = Free;
    Allocator.pfnReallocation = Reallocation;
    Allocator.pfnInternalAllocation = InternalAllocation;
    Allocator.pfnInternalFree = InternalFree;

    /* Create and return the module table: */
    lua_newtable(L); 
    luaL_setfuncs(L, Functions, 0);

    return 1;
    }

