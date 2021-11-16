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

static int freerender_pass(lua_State *L, ud_t *ud)
    {
    VkRenderPass render_pass = (VkRenderPass)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(render_pass, "render_pass");
    UD(device)->ddt->DestroyRenderPass(device, render_pass, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkRenderPass render_pass;
    VkRenderPassCreateInfo* info;
    VkRenderPassCreateInfo2* info2;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);

    if(device_ud->ddt->CreateRenderPass2)
        {
#define CLEANUP zfreeVkRenderPassCreateInfo2(L, info2, 1)
        info2 = zcheckVkRenderPassCreateInfo2(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        ec = device_ud->ddt->CreateRenderPass2(device, info2, allocator, &render_pass);
        CLEANUP;
        CheckError(L, ec);
#undef CLEANUP
        }
    else
        {
#define CLEANUP zfreeVkRenderPassCreateInfo(L, info, 1)
        info = zcheckVkRenderPassCreateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        ec = device_ud->ddt->CreateRenderPass(device, info, allocator, &render_pass);
        CLEANUP;
        CheckError(L, ec);
#undef CLEANUP
        }
    TRACE_CREATE(render_pass, "render_pass");
    ud = newuserdata_nondispatchable(L, render_pass, RENDER_PASS_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freerender_pass;
    ud->ddt = device_ud->ddt;
    return 1;
    }


static int GetRenderAreaGranularity(lua_State *L)
    {
    ud_t *ud;
    VkRenderPass render_pass = checkrender_pass(L, 1, &ud);
    VkDevice device = ud->device;
    VkExtent2D granularity;
    ud->ddt->GetRenderAreaGranularity(device, render_pass, &granularity);
    zpushVkExtent2D(L, &granularity);
    return 1;
    }

RAW_FUNC(render_pass)
TYPE_FUNC(render_pass)
INSTANCE_FUNC(render_pass)
DEVICE_FUNC(render_pass)
PARENT_FUNC(render_pass)
DELETE_FUNC(render_pass)
DESTROY_FUNC(render_pass)

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
        { "create_render_pass",  Create },
        { "destroy_render_pass",  Destroy },
        { "get_render_area_granularity", GetRenderAreaGranularity },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_render_pass(lua_State *L)
    {
    udata_define(L, RENDER_PASS_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

