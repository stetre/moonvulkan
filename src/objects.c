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

ud_t *newuserdata(lua_State *L, uint64_t handle, const char *mt, int dispatchable)
    {
    ud_t *ud;
    /* For dispatchable objects we use handle as search key.
     * For non-dispatchable objects we cannot use handle as search key, because it
     * is not guaranteed to be unique, so we use ud instead.
     */
    if(dispatchable)
        ud = (ud_t*)udata_new(L, sizeof(ud_t), (uint64_t)handle, mt);
    else 
        ud = (ud_t*)udata_new(L, sizeof(ud_t), 0, mt);
    memset(ud, 0, sizeof(ud_t));
    ud->handle = handle;
    MarkValid(ud);
    ud->ref1 = ud->ref2 = ud->ref3 = ud->ref4 = LUA_NOREF;
    if(dispatchable) MarkDispatchable(ud);
    return ud;
    }

int freeuserdata(lua_State *L, ud_t *ud)
    {
    /* The 'Valid' mark prevents double calls when an object is explicitly destroyed, 
     * and subsequently deleted also by the GC (the ud sticks around until the GC
     * collects it, so we mark it as invalid when the object is explicitly destroyed
     * by the script, or implicitly destroyed because child of a destroyed object). */
    if(!IsValid(ud)) return 0;
    CancelValid(ud);
    Unreference(L, ud->ref1);
    Unreference(L, ud->ref2);
    Unreference(L, ud->ref3);
    Unreference(L, ud->ref4);
    if(ud->info) 
        Free(L, ud->info);
    if(IsDispatchable(ud))
        udata_free(L, ud->handle);
    else
        udata_free(L, (uint64_t)(uintptr_t)ud);
    return 1;
    }


static int freeifchild(lua_State *L, const void *mem, const char *mt, const void *parent_ud)
/* callback for udata_scan */
    {
    ud_t *ud = (ud_t*)mem;
    (void)mt;
    if(IsValid(ud) && (ud->parent_ud == parent_ud))
        ud->destructor(L, ud);
    return 0;
    }

int freechildren(lua_State *L,  const char *mt, ud_t *parent_ud)
/* calls the self destructor for all 'mt' objects that are children of the given parent_ud */
    {
    return udata_scan(L, mt, parent_ud, freeifchild);
    }



int pushuserdata(lua_State *L, ud_t *ud)
    {
    if(!IsValid(ud)) return unexpected(L);
    if(IsDispatchable(ud))
        return udata_push(L, ud->handle);
    return udata_push(L, (uint64_t)(uintptr_t)ud);
    }


typedef struct {
    ud_t *parent_ud;
    uint64_t handle;    
} searchinfo_t;

static int searchnondispatchable(lua_State *L, const void *mem, const char *mt, const void *info)
/* callback for udata_scan */
    {
    ud_t *ud = (ud_t*)mem;
    searchinfo_t *searchinfo = (searchinfo_t *)info;
    (void)mt;
    if( IsValid(ud) && (ud->handle == searchinfo->handle) && (ud->parent_ud == searchinfo->parent_ud))
        {
        udata_push(L, (uint64_t)(uintptr_t)ud); 
        return 1;
        }
    return 0;
    }

int pushnondispatchable(lua_State *L, uint64_t handle, ud_t *parent_ud, const char *mt)
/* Search for an 'mt' nondispatchable object with the given handle and parent_ud and push it.
 * Returns 1 if the object is found and pushed, 0 otherwise.
 */
    {
    searchinfo_t info;
    info.parent_ud = parent_ud;
    info.handle = handle;
    if(udata_scan(L, mt, &info, searchnondispatchable))
        return 1;
    return 0;
    }


ud_t *userdata(void *handle) /* dispatchable objects only */
    {
    ud_t *ud = (ud_t*)udata_mem((uint64_t)(uintptr_t)handle);
    if(ud && IsValid(ud)) return ud;
    return NULL;
    }

uint64_t testxxx(lua_State *L, int arg, ud_t **udp, const char *mt)
    {
    ud_t *ud = (ud_t*)udata_test(L, arg, mt);
    if(ud && IsValid(ud)) { if(udp) *udp=ud; return ud->handle; }
    if(udp) *udp = NULL;
    return 0;
    }

uint64_t checkxxx(lua_State *L, int arg, ud_t **udp, const char *mt)
    {
    ud_t *ud = (ud_t*)udata_test(L, arg, mt);
    if(ud && IsValid(ud)) 
        { if(udp) *udp = ud; return ud->handle; }
    lua_pushfstring(L, "not a %s", mt);
    return argerror(L, arg);
    }

int pushxxx(lua_State *L, uint64_t handle)  /* dispatchable objects only */
    { return udata_push(L, handle); }



/* VkXxx* checkxxxlist(lua_State *L, int arg, uint32_t *count, int *err)
 *
 * Checks if the variable at arg on the Lua stack is a list of VkXxx objects.
 * On success, returns an array of VkXxx handles and sets its length in *count.
 * The array s Malloc'd and must be released by the caller using Free(L, ...).
 * On error, sets *err to ERR_XXX, *count to 0, and returns NULL. 
 */

void** checkxxxlist_dispatchable(lua_State *L, int arg, uint32_t *count, int *err, const char *mt)
    {
    void** list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (void**)MallocNoErr(L, sizeof(void*) * (*count));

    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        list[i] = (void*)(uintptr_t)testxxx(L, -1, NULL, mt);
        if(!list[i])
            { Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        lua_pop(L, 1);
        }
    return list;
    }


uint64_t* checkxxxlist_nondispatchable(lua_State *L, int arg, uint32_t *count, int *err, ud_t ***ud, const char *mt)
/* if ud != NULL, at return it will point to the array of ud_t* for the elements
 * i.e. ud[i] = the ud for element i  (also this array must be Free'd by the caller).
 */
    {
    uint64_t* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(ud) *ud = NULL;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (uint64_t*)MallocNoErr(L, sizeof(uint64_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }
    if(ud)
        {
        *ud = (ud_t**)MallocNoErr(L, sizeof(ud_t*) *(*count));
        if(!*ud)
            { Free(L, list); *count = 0; *err = ERR_MEMORY; return NULL; }
        }
    
    if(ud)
        {
        for(i=0; i<*count; i++)
            {
            lua_rawgeti(L, arg, i+1);
            (void)testxxx(L, -1, ud[i], mt);
            if(!*ud[i])
                {  Free(L, list); Free(L, *ud); *count = 0; *err = ERR_TYPE; return NULL; }
            list[i] = (*ud[i])->handle;
            lua_pop(L, 1);
            }
        }
    else
        {
        for(i=0; i<*count; i++)
            {
            lua_rawgeti(L, arg, i+1);
            list[i] = (uint64_t)testxxx(L, -1, NULL, mt);
            if(!list[i])
                { Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
            lua_pop(L, 1);
            }
        }
    return list;
    }

