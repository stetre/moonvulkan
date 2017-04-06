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

static int freeevent(lua_State *L, ud_t *ud)
    {
    VkEvent event = (VkEvent)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(event, "event");
    UD(device)->ddt->DestroyEvent(device, event, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkEvent event;
    VkEventCreateInfo info;

    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 2);
    info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    info.pNext = NULL;
    info.flags = optflags(L, 2, 0);

    ec = device_ud->ddt->CreateEvent(device, &info, allocator, &event);
    CheckError(L, ec);
    TRACE_CREATE(event, "event");
    ud = newuserdata_nondispatchable(L, event, EVENT_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freeevent;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int GetEventStatus(lua_State *L)
    {
    ud_t *ud;
    VkEvent event = checkevent(L, 1, &ud);
    VkDevice device = ud->device;
    VkResult ec = ud->ddt->GetEventStatus(device, event);
    switch(ec)
        {
        case VK_EVENT_SET: lua_pushboolean(L, 1); return 1;
        case VK_EVENT_RESET: lua_pushboolean(L, 0); return 1;
        default:
            CheckError(L, ec);
        }
    return 0;
    }

static int SetEvent(lua_State *L)
    {
    ud_t *ud;
    VkEvent event = checkevent(L, 1, &ud);
    VkDevice device = ud->device;
    VkResult ec = ud->ddt->SetEvent(device, event);
    CheckError(L, ec);
    return 0;
    }

static int ResetEvent(lua_State *L)
    {
    ud_t *ud;
    VkEvent event = checkevent(L, 1, &ud);
    VkDevice device = ud->device;
    VkResult ec = ud->ddt->ResetEvent(device, event);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC(event)
TYPE_FUNC(event)
INSTANCE_FUNC(event)
DEVICE_FUNC(event)
PARENT_FUNC(event)
DELETE_FUNC(event)
DESTROY_FUNC(event)

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
        { "create_event",  Create },
        { "destroy_event",  Destroy },
        { "get_event_status", GetEventStatus },
        { "set_event", SetEvent },
        { "reset_event", ResetEvent },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_event(lua_State *L)
    {
    udata_define(L, EVENT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

