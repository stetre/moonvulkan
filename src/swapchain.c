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
    int err;
    ud_t *ud;
    VkResult ec;
    VkSwapchainKHR swapchain;
    VkSwapchainCreateInfoKHR *info;
    VkDevice device = checkdevice(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
#define CLEANUP zfreeVkSwapchainCreateInfoKHR(L, info, 1)
    info = zcheckVkSwapchainCreateInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    CheckDevicePfn(L, ud, CreateSwapchainKHR);
    ec = ud->ddt->CreateSwapchainKHR(device, info, allocator, &swapchain);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    return newswapchain(L, swapchain, device, allocator);
    }

static int CreateSharedSwapchains(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    uint32_t count, i;
    VkSwapchainKHR *swapchains;
    VkSwapchainCreateInfoKHR* infos;
    VkDevice device = checkdevice(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckDevicePfn(L, ud, CreateSharedSwapchainsKHR);
#define CLEANUP zfreearrayVkSwapchainCreateInfoKHR(L, infos, count, 1)
    infos = zcheckarrayVkSwapchainCreateInfoKHR(L, 2, &count, &err);
    swapchains = (VkSwapchainKHR*)MallocNoErr(L, count*sizeof(VkSwapchainKHR));
    if(!swapchains) { CLEANUP; return errmemory(L); }
    ec = ud->ddt->CreateSharedSwapchainsKHR(device, count, infos, allocator, swapchains);
    CLEANUP;
#undef CLEANUP
    if(ec) { Free(L, swapchains); CheckError(L, ec); }
    
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
    if(count == 0) return 1;
    
    images = (VkImage*)Malloc(L, sizeof(VkImage)*count);
    ec = ud->ddt->GetSwapchainImagesKHR(device, swapchain, &count, images);
    if(ec) { Free(L, images); CheckError(L, ec); }

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


static int AcquireNextImage1(lua_State *L, VkSwapchainKHR swapchain, ud_t *ud)
    {
    VkResult ec;
    uint32_t imageindex;
    VkDevice device = ud->device;
    uint64_t timeout = checktimeout(L, 2); /* = UINT64_MAX for 'blocking' */
    VkSemaphore semaphore = testsemaphore(L, 3, NULL);
    VkFence fence = testfence(L, 4, NULL);
    CheckDevicePfn(L, ud, AcquireNextImageKHR);
    ec = ud->ddt->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, &imageindex);
    switch(ec)
        {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR: lua_pushinteger(L, imageindex); break;
        //case VK_NOT_READY:
        //case VK_TIMEOUT:
        default: lua_pushnil(L); break;
        }
    pushresult(L, ec);
    return 2;
    }

static int AcquireNextImage2(lua_State *L, VkSwapchainKHR swapchain, ud_t *ud)
    {
    int err;
    VkResult ec;
    uint32_t imageindex;
    VkDevice device = ud->device;
#define CLEANUP zfreeVkAcquireNextImageInfoKHR(L, info, 1)
    VkAcquireNextImageInfoKHR* info = zcheckVkAcquireNextImageInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->swapchain = swapchain;
    ec = ud->ddt->AcquireNextImage2KHR(device, info, &imageindex);
    switch(ec)
        {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR: lua_pushinteger(L, imageindex); break;
        //case VK_NOT_READY:
        //case VK_TIMEOUT:
        default: lua_pushnil(L); break;
        }
    CLEANUP;
#undef CLEANUP
    pushresult(L, ec);
    return 2;
    }

static int AcquireNextImage(lua_State *L)
    {
    ud_t *ud;
    VkSwapchainKHR swapchain =  checkswapchain(L, 1, &ud);
    if((lua_type(L, 2) == LUA_TTABLE) && ud->ddt->AcquireNextImage2KHR)
        return AcquireNextImage2(L, swapchain, ud);
    return AcquireNextImage1(L, swapchain, ud);
    }

static int WaitForPresent(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkSwapchainKHR swapchain =  checkswapchain(L, 1, &ud);
    uint64_t presentId = luaL_checkinteger(L, 2);
    uint64_t timeout = luaL_checkinteger(L, 3);
    CheckDevicePfn(L, ud, WaitForPresentKHR);
    ec = ud->ddt->WaitForPresentKHR(ud->device, swapchain, presentId, timeout);
    pushresult(L, ec);
    return 1;
    }

static int QueuePresent(lua_State *L)
    {
    int err, per_swapchain_results;
    uint32_t i, n;
    VkResult ec;
    ud_t *ud;
    VkPresentInfoKHR  *info;
    VkQueue queue = checkqueue(L, 1, &ud);
    per_swapchain_results = optboolean(L, 3, 0);
    CheckDevicePfn(L, ud, QueuePresentKHR);
#define CLEANUP zfreeVkPresentInfoKHR(L, info, 1)
    info = zcheckVkPresentInfoKHR(L, 2, &err, per_swapchain_results);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->QueuePresentKHR(queue, info);
    pushresult(L, ec);
    n = 1;
    if(per_swapchain_results)
        {   
        lua_newtable(L);
        for(i = 0; i < info->swapchainCount; i++)
            {
            pushresult(L, info->pResults[i]);
            lua_rawseti(L, -2, i+1);
            }
        n = 2;
        }
    CLEANUP;
#undef CLEANUP
    return n;
    }

static int SetHdrMetadata(lua_State *L)
    {
    ud_t **ud;
    int err;
    uint32_t count;
    VkHdrMetadataEXT *metadata;
    VkSwapchainKHR *swapchains;
#define CLEANUP do { zfreeVkHdrMetadataEXT(L, metadata, 1); Free(L, swapchains); Free(L, ud); } while(0)
    swapchains = checkswapchainlist(L, 1, &count, &err, &ud);
    if(err) return argerrorc(L, 1, err);
    CheckDevicePfn(L, ud[0], SetHdrMetadataEXT);
    metadata = zcheckVkHdrMetadataEXT(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ud[0]->ddt->SetHdrMetadataEXT(ud[0]->device, count, swapchains, metadata);
    CLEANUP;
#undef CLEANUP
    return 0;
    }

static int GetSwapchainCounter(lua_State *L)
    {
    ud_t *ud;
    uint64_t val;
    VkSwapchainKHR swapchain = checkswapchain(L, 1, &ud);
    VkSurfaceCounterFlagBitsEXT counter = (VkSurfaceCounterFlagBitsEXT)optflags(L, 2, 0);
    CheckDevicePfn(L, ud, GetSwapchainCounterEXT);
    ud->ddt->GetSwapchainCounterEXT(ud->device, swapchain, counter, &val);
    lua_pushinteger(L, val);
    return 1;
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
        { "wait_for_present", WaitForPresent },
        { "queue_present", QueuePresent },
        { "get_swapchain_status", GetSwapchainStatus },
        { "set_hdr_metadata", SetHdrMetadata },
        { "get_swapchain_counter", GetSwapchainCounter },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_swapchain(lua_State *L)
    {
    udata_define(L, SWAPCHAIN_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

