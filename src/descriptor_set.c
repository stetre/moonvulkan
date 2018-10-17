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

static int freedescriptor_set(lua_State *L, ud_t *ud)
    {
    VkResult ec;
    VkDescriptorSet descriptor_set = (VkDescriptorSet)ud->handle;
    VkDevice device = ud->device;
    VkDescriptorPool descriptor_pool = (VkDescriptorPool)ud->parent_ud->handle;
    int free_allowed = IsFreeDescriptorSetAllowed(ud->parent_ud);
    freeuserdata(L, ud);
    TRACE_DELETE(descriptor_set, "descriptor_set");
    if(!free_allowed)
        return 0;
    ec = UD(device)->ddt->FreeDescriptorSets(device, descriptor_pool, 1, &descriptor_set);
    CheckError(L, ec);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *descriptor_pool_ud;
    int err, isinfo;
    uint32_t count, i;
    VkResult ec;
    VkDescriptorSet *descriptor_set;
    VkDescriptorSetAllocateInfo* info;

    VkDescriptorPool descriptor_pool = checkdescriptor_pool(L, 1, &descriptor_pool_ud);
    VkDevice device = descriptor_pool_ud->device;

    /* since both versions of the function have a table as second argoment, we need a bit
     * of euristics to determine if the argument is a descriptorsetallocateinfo or a
     * list of descriptor_set_layout objects.
     */
    if(lua_istable(L, 2))
        { lua_rawgeti(L, 2, 1); isinfo = lua_isnoneornil(L, -1); lua_pop(L, 1); }
    else
        isinfo = 1; /* will fail later */

#define CLEANUP zfreeVkDescriptorSetAllocateInfo(L, info, 1)
    if(isinfo)
        {
        info = zcheckVkDescriptorSetAllocateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        info = znewVkDescriptorSetAllocateInfo(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->pSetLayouts = checkdescriptor_set_layoutlist(L, 2, &count, &err, NULL);
        info->descriptorSetCount = count;
        if(err) { CLEANUP; return argerrorc(L, 2, err); }
        }

    info->descriptorPool = descriptor_pool;

    descriptor_set = (VkDescriptorSet*)MallocNoErr(L, sizeof(VkDescriptorSet)*count);
    if(!descriptor_set) { CLEANUP; return errmemory(L); }

    ec = descriptor_pool_ud->ddt->AllocateDescriptorSets(device, info, descriptor_set);
    CLEANUP;
#undef CLEANUP
    if(ec)
        { Free(L, descriptor_set); CheckError(L, ec); return 0; }

    lua_newtable(L);
    for(i=0; i < count; i++)
        {
        TRACE_CREATE(descriptor_set[i], "descriptor_set");
        ud = newuserdata_nondispatchable(L, descriptor_set[i], DESCRIPTOR_SET_MT);
        ud->parent_ud = descriptor_pool_ud;
        ud->device = device;
        ud->instance = UD(device)->instance;
        ud->destructor = freedescriptor_set;
        lua_rawseti(L, -2, i+1);
        ud->ddt = descriptor_pool_ud->ddt;
        }

    Free(L, descriptor_set);
    return 1;
    }

static int FreeDescriptorSets(lua_State *L)
    {
    ud_t **ud;
    int err, free_allowed;
    VkResult ec;
    uint32_t i, count;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *descriptor_set = checkdescriptor_setlist(L, 1, &count, &err, &ud);
    if(err) return argerrorc(L, 1, err);
    free_allowed = IsFreeDescriptorSetAllowed(ud[1]->parent_ud);
#define CLEANUP do {  Free(L, descriptor_set); Free(L, ud); } while(0)
    /* check that they all are from the same pool */
    for(i = 1; i < count; i++)
        {
        if(ud[i]->parent_ud != ud[0]->parent_ud)
            { CLEANUP; return argerrorc(L, 1, ERR_POOL); }
        }

    descriptor_pool = (VkDescriptorPool)ud[0]->parent_ud->handle;
    for(i = 0; i < count; i++)
        {
        freeuserdata(L, ud[i]);
        TRACE_DELETE(descriptor_set[i], "descriptor_set");
        }

    if(!free_allowed)
        { CLEANUP; return argerrorc(L, 1, ERR_POOL); }

    ec = ud[0]->ddt->FreeDescriptorSets(ud[0]->device, descriptor_pool, count, descriptor_set);
    CLEANUP;
    CheckError(L, ec);
    return 0;
#undef CLEANUP
    }

RAW_FUNC(descriptor_set)
TYPE_FUNC(descriptor_set)
INSTANCE_FUNC(descriptor_set)
DEVICE_FUNC(descriptor_set)
PARENT_FUNC(descriptor_set)
DELETE_FUNC(descriptor_set)

static int UpdateDescriptorSets(lua_State *L)
    {
    ud_t *ud;
    int err;
    uint32_t wcount, ccount;
    VkWriteDescriptorSet* writes=NULL;
    VkCopyDescriptorSet* copies=NULL;
    VkDevice device = checkdevice(L, 1, &ud);
#define CLEANUP do {                                        \
    zfreearrayVkWriteDescriptorSet(L, writes, wcount, 1);   \
    zfreearrayVkCopyDescriptorSet(L, copies, ccount, 1);    \
} while(0)
    writes = zcheckarrayVkWriteDescriptorSet(L, 2, &wcount, &err);
    if(err < 0) { CLEANUP; return argerror(L, 2); }
    if(err==ERR_NOTPRESENT) lua_pop(L, 1);

    copies = zcheckarrayVkCopyDescriptorSet(L, 3, &ccount, &err);
    if(err < 0) { CLEANUP; return argerror(L, 3); }
    if(err==ERR_NOTPRESENT) lua_pop(L, 1);

    ud->ddt->UpdateDescriptorSets(device, wcount, writes, ccount, copies);

    CLEANUP;
    return 0;
#undef CLEANUP
    }

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
        { "allocate_descriptor_sets",  Create },
        { "free_descriptor_sets",  FreeDescriptorSets },
        { "update_descriptor_sets", UpdateDescriptorSets },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_descriptor_set(lua_State *L)
    {
    udata_define(L, DESCRIPTOR_SET_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

