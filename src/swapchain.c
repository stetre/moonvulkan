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

static int freeswapchain(lua_State *L, ud_t *ud)
    {
    VkSwapchainKHR swapchain = (VkSwapchainKHR)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;

    freechildren(L, IMAGE_MT, ud);

    if(!freeuserdata(L, ud)) return 0; /* double call */
    TRACE_DELETE(swapchain, "swapchain");
    UD(device)->ddt->DestroySwapchainKHR(device, swapchain, allocator);
    return 0;
    }

static int newswapchain(lua_State *L, VkSwapchainKHR swapchain, VkDevice device, const VkAllocationCallbacks *allocator)
    {
    ud_t *ud, *device_ud;
    device_ud = UD(device);
    TRACE_CREATE(swapchain, "swapchain");
    ud = newuserdata_nondispatchable(L, swapchain, SWAPCHAIN_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freeswapchain;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int CreateSwapchain(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    VkSwapchainKHR swapchain;
    VkSwapchainCreateInfoKHR info;
    VkDevice device = checkdevice(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    if(echeckswapchaincreateinfo(L, 2, &info))
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    CheckDevicePfn(L, ud, CreateSwapchainKHR);
    ec = ud->ddt->CreateSwapchainKHR(device, &info, allocator, &swapchain);
    freeswapchaincreateinfo(L, &info);
    CheckError(L, ec);
    return newswapchain(L, swapchain, device, allocator);
    }

static int CreateSharedSwapchains(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    int err;
    uint32_t count, i;
    VkSwapchainKHR *swapchains;
    VkSwapchainCreateInfoKHR *infos;
    VkDevice device = checkdevice(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);

    CheckDevicePfn(L, ud, CreateSharedSwapchainsKHR);

    infos = echeckswapchaincreateinfolist(L, 2, &count, &err);
    if(err) return luaL_argerror(L, 2, lua_tostring(L, -1));

    swapchains = (VkSwapchainKHR*)MallocNoErr(L, count*sizeof(VkSwapchainKHR));
    if(!swapchains)
        {
        freeswapchaincreateinfolist(L, infos, count);
        return luaL_error(L, errstring(ERR_MEMORY));
        }

    ec = ud->ddt->CreateSharedSwapchainsKHR(device, count, infos, allocator, swapchains);
    freeswapchaincreateinfolist(L, infos, count);

    if(ec)
        {
        Free(L, swapchains);
        CheckError(L, ec);
        }
    
    lua_newtable(L);
    for(i = 0; i < count; i++)
        {
        newswapchain(L, swapchains[i], device, allocator);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, swapchains);
    return 0;
    }

static int GetSwapchainImages(lua_State *L)
    {
    VkResult ec; 
    uint32_t count, i;
    VkImage *images;
    ud_t *ud; 
    VkSwapchainKHR swapchain = checkswapchain(L, 1, &ud);
    VkDevice device = ud->device;
    CheckDevicePfn(L, ud, GetSwapchainImagesKHR);
    ec = ud->ddt->GetSwapchainImagesKHR(device, swapchain, &count, NULL);
    CheckError(L, ec);
    
    lua_newtable(L);
    if(count == 0)
        return 1;
    
    images = (VkImage*)Malloc(L, sizeof(VkImage)*count);
    ec = ud->ddt->GetSwapchainImagesKHR(device, swapchain, &count, images);
    if(ec)
        {
        Free(L, images);
        CheckError(L, ec);
        }

    for(i = 0; i < count; i++)
        {
        pushimage_swapchain(L, images[i], ud); 
        lua_rawseti(L, -2, i+1);
        }
    
    Free(L, images);
    return 1;
    }

static int GetSwapchainStatus(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkSwapchainKHR swapchain = checkswapchain(L, 1, &ud);
    VkDevice device = ud->device;
    CheckDevicePfn(L, ud, GetSwapchainStatusKHR);
    ec = ud->ddt->GetSwapchainStatusKHR(device, swapchain);

    pushresult(L, ec);
    return 1;
    }


static int AcquireNextImage(lua_State *L)
    {
    VkResult ec; 
    uint32_t imageindex;
    ud_t *ud;
    VkSwapchainKHR swapchain =  checkswapchain(L, 1, &ud);
    VkDevice device = ud->device;
    uint64_t timeout = checktimeout(L, 2); /* = UINT64_MAX for 'blocking' */
    VkSemaphore semaphore = testsemaphore(L, 3, NULL);
    VkFence fence = testfence(L, 4, NULL);
    CheckDevicePfn(L, ud, AcquireNextImageKHR);
    ec = ud->ddt->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, &imageindex);
    switch(ec)
        {
        case VK_SUCCESS: lua_pushinteger(L, imageindex); return 1;
        case VK_SUBOPTIMAL_KHR: lua_pushinteger(L, imageindex); lua_pushstring(L, "suboptimal"); return 2;
        case VK_TIMEOUT: lua_pushnil(L); lua_pushstring(L, "timeout"); return 2;
        case VK_NOT_READY: lua_pushnil(L); lua_pushstring(L, "not ready"); return 2;
        default:
            break;
        }
    CheckError(L, ec);
    return 0;
    }


static int QueuePresent(lua_State *L)
    {
    int err, per_swapchain_results;
    uint32_t count, i, n;
    VkResult ec;
    VkResult *results = NULL;
    ud_t *ud;
    VkPresentInfoKHR info;
    VkDisplayPresentInfoKHR display_info;
    int display_info_added = 0;
    VkPresentRegionsKHR regions;
    VkQueue queue = checkqueue(L, 1, &ud);
    int regions_added = 0;
    
    CheckDevicePfn(L, ud, QueuePresentKHR);

    err = echeckpresentinfo(L, 2, &info);
    if(err) return luaL_argerror(L, 2, lua_tostring(L, -1));

    per_swapchain_results = optboolean(L, 3, 0);

    /* optional VkDisplayPresentInfoKHR */
    err = echeckdisplaypresentinfo(L, 4, &display_info);
    if(err < 0)
        {
        freepresentinfo(L, &info); 
        return luaL_argerror(L, 4, lua_tostring(L, -1));
        }
    if(err == 0)
        {
        display_info_added = 1;
        info.pNext = &display_info;
        }
    else
        lua_pop(L, 1);

    /* optional VkPresentRegionsKHR */
    err = echeckpresentregions(L, 5, &regions);
    if(err < 0)
        {
        freepresentinfo(L, &info); 
        if(display_info_added) freedisplaypresentinfo(L, &display_info);
        return luaL_argerror(L, 5, lua_tostring(L, -1));
        }
    if(err == 0)
        {
        regions_added = 1;
        if(display_info_added)
            display_info.pNext = &regions;
        else
            info.pNext = &regions;
        }
    else
        lua_pop(L, 1);

    count = info.swapchainCount;
    if(per_swapchain_results)
        {
        info.pResults = (VkResult*)MallocNoErr(L, sizeof(VkResult)*count);
        if(!info.pResults)
            { 
            freepresentinfo(L, &info); 
            if(display_info_added) freedisplaypresentinfo(L, &display_info);
            if(regions_added) freepresentregions(L, &regions);
            return luaL_error(L, errstring(ERR_MEMORY));
            }

        }

    ec = ud->ddt->QueuePresentKHR(queue, &info);

    pushresult(L, ec);
    n = 1;

    if(per_swapchain_results)
        {   
        lua_newtable(L);
        for(i = 0; i < count; i++)
            {
            pushresult(L, results[i]);
            lua_rawseti(L, -2, i+1);
            }
        n++;
        }
    freepresentinfo(L, &info); 
    if(display_info_added) freedisplaypresentinfo(L, &display_info);
    if(regions_added) freepresentregions(L, &regions);
    return n;
    }

RAW_FUNC(swapchain)
TYPE_FUNC(swapchain)
INSTANCE_FUNC(swapchain)
DEVICE_FUNC(swapchain)
PARENT_FUNC(swapchain)
DELETE_FUNC(swapchain)
DESTROY_FUNC(swapchain)

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
        { "create_swapchain",  CreateSwapchain },
        { "create_shared_swapchains", CreateSharedSwapchains },
        { "destroy_swapchain",  Destroy },
        { "get_swapchain_images", GetSwapchainImages },
        { "acquire_next_image", AcquireNextImage },
        { "queue_present", QueuePresent },
        { "get_swapchain_status", GetSwapchainStatus },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_swapchain(lua_State *L)
    {
    udata_define(L, SWAPCHAIN_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

