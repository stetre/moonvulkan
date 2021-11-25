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

typedef struct { ud_t *ud; } ud_info_t;

static int freedebug_utils_messenger(lua_State *L, ud_t *ud)
    {
    VkDebugUtilsMessengerEXT debug_utils_messenger = (VkDebugUtilsMessengerEXT)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkInstance instance = ud->instance;
    if(!freeuserdata(L, ud)) return 0; /* double call */
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
#define ud ((ud_info_t*)pUserData)->ud
#define L moonvulkan_L
    int top = lua_gettop(L);
    /* retrieve and push the callback */
    if(lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref1) != LUA_TFUNCTION)
        return unexpected(L);
    /* push args */
    pushinstance(L, ud->instance);
    pushflags(L, messageSeverity);
    pushflags(L, messageType);
    if(pCallbackData) zpushVkDebugUtilsMessengerCallbackDataEXT(L, pCallbackData);
    /* execute the callback */
    if(lua_pcall(L, 4, 0, 0) != LUA_OK)
        { lua_error(L); return 0; }
    lua_settop(L, top);
    return 0;
#undef L
#undef ud
    }

static int Create(lua_State *L)
    {
    int err, ref;
    ud_t *ud, *instance_ud;
    ud_info_t *ud_info;
    VkResult ec;
    VkDebugUtilsMessengerEXT debug_utils_messenger;
    VkDebugUtilsMessengerCreateInfoEXT* info;
    VkInstance instance = checkinstance(L, 1, &instance_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
    CheckInstancePfn(L, instance_ud, CreateDebugUtilsMessengerEXT);
    if(!lua_isfunction(L, 3)) return argerrorc(L, 3, ERR_TYPE);
#define CLEANUP zfreeVkDebugUtilsMessengerCreateInfoEXT(L, info, 1)
    info = zcheckVkDebugUtilsMessengerCreateInfoEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->pfnUserCallback = /*(PFN_vkDebugUtilsMessengerEXT)*/Callback;
    ud_info = (ud_info_t*)MallocNoErr(L, sizeof(ud_info_t));
    if(!ud_info) { CLEANUP; return errmemory(L); }
    info->pUserData = ud_info;
    ec = instance_ud->idt->CreateDebugUtilsMessengerEXT(instance,info,allocator,&debug_utils_messenger);
    CLEANUP;
#undef CLEANUP
    if(ec)
        {
        Free(L, ud_info);
        CheckError(L, ec);
        return 0;
        }
    lua_pushvalue(L, 3);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    TRACE_CREATE(debug_utils_messenger, "debug_utils_messenger");
    ud = newuserdata_nondispatchable(L, debug_utils_messenger, DEBUG_UTILS_MESSENGER_MT);
    ud->ref1 = ref;
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
    int err;
    ud_t *ud;
    VkResult ec;
    VkDebugUtilsObjectNameInfoEXT* info;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckInstancePfn(L, ud, SetDebugUtilsObjectNameEXT);
#define CLEANUP zfreeVkDebugUtilsObjectNameInfoEXT(L, info, 1)
    info = zcheckVkDebugUtilsObjectNameInfoEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->idt->SetDebugUtilsObjectNameEXT(device, info);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int SetDebugUtilsObjectTag(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkResult ec;
    VkDebugUtilsObjectTagInfoEXT* info;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckInstancePfn(L, ud, SetDebugUtilsObjectTagEXT);
#define CLEANUP zfreeVkDebugUtilsObjectTagInfoEXT(L, info, 1)
    info = zcheckVkDebugUtilsObjectTagInfoEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->idt->SetDebugUtilsObjectTagEXT(device, info);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int SubmitDebugUtilsMessage(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkDebugUtilsMessageSeverityFlagBitsEXT severity;
    VkDebugUtilsMessageTypeFlagsEXT types;
    VkDebugUtilsMessengerCallbackDataEXT* cbdata;
    VkInstance instance = checkinstance(L, 1, &ud);
    CheckInstancePfn(L, ud, SubmitDebugUtilsMessageEXT);
    severity = checkflags(L, 2);
    types = checkflags(L, 3);
#define CLEANUP zfreeVkDebugUtilsMessengerCallbackDataEXT(L, cbdata, 1)
    cbdata = zcheckVkDebugUtilsMessengerCallbackDataEXT(L, 4, &err);
    if(err) { CLEANUP; return argerror(L, 4); }
    ud->idt->SubmitDebugUtilsMessageEXT(instance, severity, types, cbdata);
    CLEANUP;
#undef CLEANUP
    return 0;
    }

static int QueueBeginDebugUtilsLabel(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkDebugUtilsLabelEXT* label;
    VkQueue queue = checkqueue(L, 1, &ud);
    CheckInstancePfn(L, ud, QueueBeginDebugUtilsLabelEXT);
#define CLEANUP zfreeVkDebugUtilsLabelEXT(L, label, 1)
    label = zcheckVkDebugUtilsLabelEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ud->idt->QueueBeginDebugUtilsLabelEXT(queue, label);
    CLEANUP;
#undef CLEANUP
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
    int err;
    ud_t *ud;
    VkDebugUtilsLabelEXT* label;
    VkQueue queue = checkqueue(L, 1, &ud);
    CheckInstancePfn(L, ud, QueueInsertDebugUtilsLabelEXT);
#define CLEANUP zfreeVkDebugUtilsLabelEXT(L, label, 1)
    label = zcheckVkDebugUtilsLabelEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ud->idt->QueueInsertDebugUtilsLabelEXT(queue, label);
    CLEANUP;
#undef CLEANUP
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

