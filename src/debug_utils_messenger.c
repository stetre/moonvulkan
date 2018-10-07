/* The MIT License (MIT)
 *
 * Copyright (c) 2018 Stefano Trettel
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

typedef struct {
    lua_State *lua_state;
    int ref;
    ud_t *ud; /* ud for debug_utils_messenger */
} ud_info_t;

static int freedebug_utils_messenger(lua_State *L, ud_t *ud)
    {
    VkDebugUtilsMessengerEXT debug_utils_messenger = (VkDebugUtilsMessengerEXT)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkInstance instance = ud->instance;

    if(!freeuserdata(L, ud))
        return 0; /* double call */

    luaL_unref(L, LUA_REGISTRYINDEX, ((ud_info_t*)ud->info)->ref);
    TRACE_DELETE(debug_utils_messenger, "debug_utils_messenger");
    UD(instance)->idt->DestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger, allocator);
    return 0;
    }

static VkBool32 Callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData)
    {
#define info ((ud_info_t*)pUserData)
#define L info->lua_state
    int top = lua_gettop(L);
    /* retrieve and push the callback */
    if(lua_rawgeti(L, LUA_REGISTRYINDEX, info->ref) != LUA_TFUNCTION)
        return unexpected(L);

    /* push args */
    pushinstance(L, info->ud->instance);
    pushflags(L, messageSeverity);
    pushflags(L, messageType);
    if(pCallbackData) pushdebugutilsmessengercallbackdata(L, pCallbackData);
    
    /* execute the callback */
    if(lua_pcall(L, 4, 0, 0) != LUA_OK)
        { lua_error(L); return 0; }

    lua_settop(L, top);
    return 0;
#undef L
#undef info
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *instance_ud;
    ud_info_t *ud_info;
    VkResult ec;
    VkDebugUtilsMessengerEXT debug_utils_messenger;
    VkDebugUtilsMessengerCreateInfo_CHAIN info;
    int ref;

    VkInstance instance = checkinstance(L, 1, &instance_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
    CheckInstancePfn(L, instance_ud, CreateDebugUtilsMessengerEXT);

    if(echeckdebugutilsmessengercreateinfo(L, 2, &info)) return argerror(L, 2);
    info.p1.pfnUserCallback = /*(PFN_vkDebugUtilsMessengerEXT)*/Callback;

    /* get the Lua function, arg 3 and anchor it in the Lua registry */
    if(!lua_isfunction(L, 3))
        {
        freedebugutilsmessengercreateinfo(L, &info);
        return argerrorc(L, 3, ERR_TYPE);
        }
    lua_pushvalue(L, 3);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ud_info = (ud_info_t*)MallocNoErr(L, sizeof(ud_info_t));
    if(!ud_info)
        { 
        freedebugutilsmessengercreateinfo(L, &info);
        luaL_unref(L, LUA_REGISTRYINDEX, ref); 
        return errmemory(L);
        }
    ud_info->lua_state = L;
    ud_info->ref = ref;
/*  ud_info->ud later */
    info.p1.pUserData = ud_info;

    ec = instance_ud->idt->CreateDebugUtilsMessengerEXT(instance, &info.p1, allocator, 
                            &debug_utils_messenger);
    freedebugutilsmessengercreateinfo(L, &info);

    if(ec)
        {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        Free(L, ud_info);
        CheckError(L, ec);
        return 0;
        }
    TRACE_CREATE(debug_utils_messenger, "debug_utils_messenger");
    ud = newuserdata_nondispatchable(L, debug_utils_messenger, DEBUG_UTILS_MESSENGER_MT);
    ud->info = ud_info;
    ud_info->ud = ud;
    ud->parent_ud = instance_ud;
    ud->instance = instance;
    ud->allocator = allocator;
    ud->destructor = freedebug_utils_messenger;
    ud->idt = instance_ud->idt;
    return 1;
    }

static int SetDebugUtilsObjectName(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    VkDebugUtilsObjectNameInfoEXT info;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckInstancePfn(L, ud, SetDebugUtilsObjectNameEXT);
    if(echeckdebugutilsobjectnameinfo(L, 2, &info)) return argerror(L, 2);
    ec = ud->idt->SetDebugUtilsObjectNameEXT(device, &info);
    freedebugutilsobjectnameinfo(L, &info);
    CheckError(L, ec);
    return 0;
    }

static int SetDebugUtilsObjectTag(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    VkDebugUtilsObjectTagInfoEXT info;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckInstancePfn(L, ud, SetDebugUtilsObjectTagEXT);
    if(echeckdebugutilsobjecttaginfo(L, 2, &info)) return argerror(L, 2);
    ec = ud->idt->SetDebugUtilsObjectTagEXT(device, &info);
    freedebugutilsobjecttaginfo(L, &info);
    CheckError(L, ec);
    return 0;
    }

static int SubmitDebugUtilsMessage(lua_State *L)
    {
    ud_t *ud;
    VkDebugUtilsMessageSeverityFlagBitsEXT severity;
    VkDebugUtilsMessageTypeFlagsEXT types;
    VkDebugUtilsMessengerCallbackDataEXT cbdata;
    VkInstance instance = checkinstance(L, 1, &ud);
    CheckInstancePfn(L, ud, SubmitDebugUtilsMessageEXT);
    severity = checkflags(L, 2);
    types = checkflags(L, 3);
    if(echeckdebugutilsmessengercallbackdata(L, 4, &cbdata)) return argerror(L, 4);
    ud->idt->SubmitDebugUtilsMessageEXT(instance, severity, types, &cbdata);
    freedebugutilsmessengercallbackdata(L, &cbdata);
    return 0;
    }

static int QueueBeginDebugUtilsLabel(lua_State *L)
    {
    ud_t *ud;
    VkDebugUtilsLabelEXT label;
    VkQueue queue = checkqueue(L, 1, &ud);
    CheckInstancePfn(L, ud, QueueBeginDebugUtilsLabelEXT);
    if(echeckdebugutilslabel(L, 2, &label)) return argerror(L, 2);
    ud->idt->QueueBeginDebugUtilsLabelEXT(queue, &label);
    freedebugutilslabel(L, &label);
    return 0;
    }

static int QueueEndDebugUtilsLabel(lua_State *L)
    {
    ud_t *ud;
    VkQueue queue = checkqueue(L, 1, &ud);
    CheckInstancePfn(L, ud, QueueEndDebugUtilsLabelEXT);
    ud->idt->QueueEndDebugUtilsLabelEXT(queue);
    return 0;
    }

static int QueueInsertDebugUtilsLabel(lua_State *L)
    {
    ud_t *ud;
    VkDebugUtilsLabelEXT label;
    VkQueue queue = checkqueue(L, 1, &ud);
    CheckInstancePfn(L, ud, QueueInsertDebugUtilsLabelEXT);
    if(echeckdebugutilslabel(L, 2, &label)) return argerror(L, 2);
    ud->idt->QueueInsertDebugUtilsLabelEXT(queue, &label);
    freedebugutilslabel(L, &label);
    return 0;
    }

/*------------------------------------------------------------------------------*/

RAW_FUNC(debug_utils_messenger)
TYPE_FUNC(debug_utils_messenger)
INSTANCE_FUNC(debug_utils_messenger)
DEVICE_FUNC(debug_utils_messenger)
PARENT_FUNC(debug_utils_messenger)
DELETE_FUNC(debug_utils_messenger)
DESTROY_FUNC(debug_utils_messenger)

static const struct luaL_Reg Methods[] = 
    {
        { "raw",  Raw },
        { "type",  Type },
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
        { "create_debug_utils_messenger",  Create },
        { "destroy_debug_utils_messenger",  Destroy },
        { "queue_begin_debug_utils_label", QueueBeginDebugUtilsLabel },
        { "queue_end_debug_utils_label", QueueEndDebugUtilsLabel },
        { "queue_begin_insert_utils_label", QueueInsertDebugUtilsLabel },
        { "set_debug_utils_object_name", SetDebugUtilsObjectName },
        { "set_debug_utils_object_tag", SetDebugUtilsObjectTag },
        { "submit_debug_utils_message", SubmitDebugUtilsMessage },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_debug_utils_messenger(lua_State *L)
    {
    udata_define(L, DEBUG_UTILS_MESSENGER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

