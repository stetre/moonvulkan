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

static int freeimage(lua_State *L, ud_t *ud)
    {
    VkImage image = (VkImage)ud->handle;
    VkDevice device = ud->device;
    const VkAllocationCallbacks *allocator = ud->allocator;
    int borrowed = IsBorrowed(ud);

    /* delete all contained objects */
    freechildren(L, IMAGE_VIEW_MT, ud);

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(image, "image");
    if(!borrowed)
        UD(device)->ddt->DestroyImage(device, image, allocator);
    return 0;
    }

static int newimage(lua_State *L, VkImage image, ud_t *parent_ud, VkDevice device, const VkAllocationCallbacks *allocator, int borrowed)
    {
    ud_t *ud;
    TRACE_CREATE(image, "image");
    ud = newuserdata_nondispatchable(L, image, IMAGE_MT);
    ud->parent_ud = parent_ud;
    ud->device = device;
    ud->instance = UD(device)->instance;
    ud->allocator = allocator;
    ud->destructor = freeimage;
    ud->ddt = UD(device)->ddt;
    if(borrowed) MarkBorrowed(ud); 
    /* e.g. images got from a swapchain are borrowed, since they are owned by 
     * Vulkan and must not be destroyed by the application. */
    return 1;
    }

int pushimage_swapchain(lua_State *L, VkImage image, ud_t *swapchain_ud)
    {
    return newimage(L, image, swapchain_ud, swapchain_ud->device, NULL, 1);
    /* Note: since images ar nondispatchable objects, they are not guaranteed to be
     * unique, so we may have 1+ ud associated with the same image if this function 
     * is called more than once (which should not be the case).
     * Apart from memory consumption, this should not be a problem because we do not 
     * destroy borrowed images.
     */
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkResult ec;
    VkImage image;
    VkImageCreateInfo* info;

    VkDevice device = checkdevice(L, 1, &ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
#define CLEANUP zfreeVkImageCreateInfo(L, info, 1)
    info = zcheckVkImageCreateInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }

    ec = ud->ddt->CreateImage(device, info, allocator, &image);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return newimage(L, image, UD(device), device, allocator, 0);
    }

static int GetImageSubresourceLayout(lua_State *L)
    {
    int err;
    VkImageSubresource* subresource;
    VkSubresourceLayout layout;
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;
#define CLEANUP zfreeVkImageSubresource(L, subresource, 1)
    subresource = zcheckVkImageSubresource(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ud->ddt->GetImageSubresourceLayout(device, image, subresource, &layout);
    zpushVkSubresourceLayout(L, &layout);
    CLEANUP;
#undef CLEANUP
    return 1;
    }


RAW_FUNC(image)
TYPE_FUNC(image)
INSTANCE_FUNC(image)
DEVICE_FUNC(image)
PARENT_FUNC(image)
DELETE_FUNC(image)
DESTROY_FUNC(image)

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
        { "create_image",  Create },
        { "destroy_image",  Destroy },
        { "get_image_subresource_layout", GetImageSubresourceLayout },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_image(lua_State *L)
    {
    udata_define(L, IMAGE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

