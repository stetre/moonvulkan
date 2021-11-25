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

typedef struct { ud_t *ud; } ud_info_t;

static int freedebug_report_callback(lua_State *L, ud_t *ud)
    {
    VkDebugReportCallbackEXT debug_report_callback = (VkDebugReportCallbackEXT)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkInstance instance = ud->instance;
    if(!freeuserdata(L, ud)) return 0; /* double call */
    TRACE_DELETE(debug_report_callback, "debug_report_callback");
    UD(instance)->idt->DestroyDebugReportCallbackEXT(instance, debug_report_callback, allocator);
    return 0;
    }

static VkBool32 Callback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType, uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
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
    pushflags(L, flags);
    pushdebugreportobjecttype(L, objectType);
    lua_pushinteger(L, object);
    lua_pushinteger(L, location);
    lua_pushinteger(L, messageCode);
    if(pLayerPrefix) lua_pushstring(L, pLayerPrefix);
    if(pMessage) lua_pushstring(L, pMessage);
    /* execute the callback */
    if(lua_pcall(L, 8, 0, 0) != LUA_OK)
        { lua_error(L); return 0; }
    lua_settop(L, top);
    return 0;
#undef L
#undef ud
    }

static int Create(lua_State *L)
    {
    ud_info_t *ud_info;
    ud_t *ud, *instance_ud;
    VkResult ec;
    VkDebugReportCallbackEXT debug_report_callback;
    VkDebugReportCallbackCreateInfoEXT info;
    int ref;
    VkInstance instance = checkinstance(L, 1, &instance_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 4);
    CheckInstancePfn(L, instance_ud, CreateDebugReportCallbackEXT);
    if(!lua_isfunction(L, 3)) return argerrorc(L, 3, ERR_TYPE);

    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    info.pNext = NULL;
    info.flags = checkflags(L, 2);
    info.pfnCallback = (PFN_vkDebugReportCallbackEXT)Callback;

    ud_info = (ud_info_t*)MallocNoErr(L, sizeof(ud_info_t));
    if(!ud_info) return errmemory(L);
    info.pUserData = ud_info;
    ec = instance_ud->idt->CreateDebugReportCallbackEXT(instance, &info, allocator, 
                            &debug_report_callback);
    if(ec)
        {
        Free(L, ud_info);
        CheckError(L, ec);
        return 0;
        }
    lua_pushvalue(L, 3);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    TRACE_CREATE(debug_report_callback, "debug_report_callback");
    ud = newuserdata_nondispatchable(L, debug_report_callback, DEBUG_REPORT_CALLBACK_MT);
    ud->ref1 = ref;
    ud->info = ud_info;
    ud_info->ud = ud;
    ud->ref1 = ref;
    ud->parent_ud = instance_ud;
    ud->instance = instance;
    ud->allocator = allocator;
    ud->destructor = freedebug_report_callback;
    ud->idt = instance_ud->idt;
    return 1;
    }

static int DebugReportMessage(lua_State *L)
    {
    ud_t *ud;
    VkInstance instance = checkinstance(L, 1, &ud);
    VkDebugReportFlagsEXT flags = checkflags(L, 2);
    VkDebugReportObjectTypeEXT objectType = checkdebugreportobjecttype(L, 3);
    uint64_t object = luaL_checkinteger(L, 4);
    size_t location = luaL_checkinteger(L, 5);
    int32_t messageCode = luaL_checkinteger(L, 6);
    const char* pLayerPrefix = luaL_checkstring(L, 7);
    const char* pMessage = luaL_checkstring(L, 8);
    CheckInstancePfn(L, ud, DebugReportMessageEXT);
    ud->idt->DebugReportMessageEXT(instance, flags, 
        objectType, object, location, messageCode, pLayerPrefix, pMessage);
    return 0;
    }


/*--- VK_EXT_debug_marker ------------------------------------------------------*/

static int DebugMarkerSetObjectTag(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    VkDebugMarkerObjectTagInfoEXT info;
    VkDevice device = checkdevice(L, 1, &ud);
    info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
    info.pNext = NULL;
    info.objectType = checkdebugreportobjecttype(L, 2);
    info.object = luaL_checkinteger(L, 3);
    info.tagName = luaL_checkinteger(L, 4);
    info.pTag = luaL_checklstring(L, 5, &info.tagSize);
    if(info.tagSize == 0)
        return argerrorc(L, 5, ERR_LENGTH);
    CheckDevicePfn(L, ud, DebugMarkerSetObjectTagEXT);
    ec = ud->ddt->DebugMarkerSetObjectTagEXT(device, &info);
    CheckError(L, ec);
    return 0;
    }

static int DebugMarkerSetObjectName(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    VkDebugMarkerObjectNameInfoEXT info;
    VkDevice device = checkdevice(L, 1, &ud);
    info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    info.pNext = NULL;
    info.objectType = checkdebugreportobjecttype(L, 2);
    info.object = luaL_checkinteger(L, 3);
    info.pObjectName = luaL_checkstring(L, 4);
    CheckDevicePfn(L, ud, DebugMarkerSetObjectNameEXT);
    ec = ud->ddt->DebugMarkerSetObjectNameEXT(device, &info);
    CheckError(L, ec);
    return 0;
    }


/*------------------------------------------------------------------------------*/

RAW_FUNC(debug_report_callback)
TYPE_FUNC(debug_report_callback)
INSTANCE_FUNC(debug_report_callback)
DEVICE_FUNC(debug_report_callback)
PARENT_FUNC(debug_report_callback)
DELETE_FUNC(debug_report_callback)
DESTROY_FUNC(debug_report_callback)


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
        { "create_debug_report_callback",  Create },
        { "destroy_debug_report_callback",  Destroy },
        { "debug_report_message", DebugReportMessage },
        { "debug_marker_set_object_tag", DebugMarkerSetObjectTag },
        { "debug_marker_set_object_name", DebugMarkerSetObjectName },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_debug_report_callback(lua_State *L)
    {
    udata_define(L, DEBUG_REPORT_CALLBACK_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

