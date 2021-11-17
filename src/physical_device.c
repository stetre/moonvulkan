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

static int freephysical_device(lua_State *L, ud_t *ud)
    {
    VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)ud->handle;
    freechildren(L, DISPLAY_MT, ud);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(physical_device, "physical_device");
    return 0;
    }

int pushphysical_device(lua_State *L, VkPhysicalDevice physical_device, VkInstance instance)
    { 
    ud_t *ud = UD(physical_device);
    if(!ud)
        {
        /* create the userdata associated with this object */
        ud = newuserdata_dispatchable(L, physical_device, PHYSICAL_DEVICE_MT);
        ud->instance = instance;
        ud->parent_ud = UD(instance);
        ud->destructor = freephysical_device;
        ud->idt = UD(instance)->idt;
        TRACE_CREATE(physical_device, "physical_device");
        return 1;
        }
    return pushuserdata(L, ud);
    }

/* Note: if the idt for the relevant physical_device contains valid pointers
 *       for the VK_KHR_get_physical_device_properties2 extension functions,
 *       then those functions will be used. Otherwise the first version functions
 *       will be used.
 */

/*-----------------------------------------------------------------------------*/

static int GetPhysicalDeviceProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    VkPhysicalDeviceProperties2* props;
#define CLEANUP zfreeVkPhysicalDeviceProperties2(L, props, 1)
    props = znewchainVkPhysicalDeviceProperties2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceProperties2(physical_device, props);
    zpushVkPhysicalDeviceProperties2(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceProperties(lua_State *L)
    {
    ud_t *ud;
    VkPhysicalDeviceProperties props;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceProperties2)
        return GetPhysicalDeviceProperties2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceProperties(physical_device, &props);
    zpushVkPhysicalDeviceProperties(L, &props);
    return 1;
    }


/*-----------------------------------------------------------------------------*/

static int GetPhysicalDeviceFeatures2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    VkPhysicalDeviceFeatures2* features;
#define CLEANUP zfreeVkPhysicalDeviceFeatures2(L, features, 1)
    features = znewchainVkPhysicalDeviceFeatures2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceFeatures2(physical_device, features);
    zpushVkPhysicalDeviceFeatures2(L, features);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceFeatures(lua_State *L)
    {
    ud_t *ud;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceFeatures2)
        return GetPhysicalDeviceFeatures2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceFeatures(physical_device, &features);
    zpushVkPhysicalDeviceFeatures(L, &features);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceFormatProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    VkFormatProperties2* props;
    VkFormat format = checkformat(L, 2);
#define CLEANUP zfreeVkFormatProperties2(L, props, 1)
    props = znewchainVkFormatProperties2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceFormatProperties2(physical_device, format, props);
    zpushVkFormatProperties2(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceFormatProperties(lua_State *L)
    {
    VkFormatProperties props;
    VkFormat format;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceFormatProperties2)
        return GetPhysicalDeviceFormatProperties2(L, physical_device, ud);
    format = checkformat(L, 2);
    ud->idt->GetPhysicalDeviceFormatProperties(physical_device, format, &props);
    zpushVkFormatProperties(L, &props);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceImageFormatProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    VkResult ec;
    VkPhysicalDeviceImageFormatInfo2* info = NULL;
    VkImageFormatProperties2* props = NULL;
#define CLEANUP do {            \
    zfreeVkPhysicalDeviceImageFormatInfo2(L, info, 1);           \
    zfreeVkImageFormatProperties2(L, props, 1);                  \
} while(0)
    info = zcheckVkPhysicalDeviceImageFormatInfo2(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    props = znewchainVkImageFormatProperties2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ec = ud->idt->GetPhysicalDeviceImageFormatProperties2(physical_device, info, props);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    zpushVkImageFormatProperties2(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceImageFormatProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkResult ec;
    VkPhysicalDeviceImageFormatInfo2* info;
    VkImageFormatProperties props;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceImageFormatProperties2)
        return GetPhysicalDeviceImageFormatProperties2(L, physical_device, ud);
#define CLEANUP zfreeVkPhysicalDeviceImageFormatInfo2(L, info, 1)
    info = zcheckVkPhysicalDeviceImageFormatInfo2(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->idt->GetPhysicalDeviceImageFormatProperties(physical_device, 
            info->format, info->type, info->tiling, info->usage, info->flags, &props);
    if(ec) { CLEANUP; CheckError(L, ec); return 0; }
    zpushVkImageFormatProperties(L, &props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceQueueFamilyProperties2
        (lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    uint32_t i, count = 0;
    VkQueueFamilyProperties2KHR* props;
    lua_newtable(L);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties2(physical_device, &count, NULL);
    if(count == 0) return 1;
#define CLEANUP zfreearrayVkQueueFamilyProperties2KHR(L, props, count, 1)
    props = znewchainarrayVkQueueFamilyProperties2KHR(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceQueueFamilyProperties2(physical_device, &count, props);
    for(i=0; i<count; i++)
        {
        zpushVkQueueFamilyProperties2KHR(L, &props[i], i);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceQueueFamilyProperties(lua_State *L)
    {
    uint32_t i, count = 0;
    VkQueueFamilyProperties *props;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    
    if(ud->idt->GetPhysicalDeviceQueueFamilyProperties2)
        return GetPhysicalDeviceQueueFamilyProperties2(L, physical_device, ud);

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
    if(count == 0) return 1;
    props = (VkQueueFamilyProperties*)Malloc(L, sizeof(VkQueueFamilyProperties)*count);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties(physical_device, &count, props);
    for(i=0; i<count; i++)
        {
        zpushVkQueueFamilyProperties(L, &(props[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, props);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceMemoryProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    VkPhysicalDeviceMemoryProperties2* props;
#define CLEANUP zfreeVkPhysicalDeviceMemoryProperties2(L, props, 1)
    props = znewchainVkPhysicalDeviceMemoryProperties2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceMemoryProperties2(physical_device, props);
    zpushVkPhysicalDeviceMemoryProperties2(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceMemoryProperties(lua_State *L)
    {
    VkPhysicalDeviceMemoryProperties props;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceMemoryProperties2)
        return GetPhysicalDeviceMemoryProperties2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceMemoryProperties(physical_device, &props);
    zpushVkPhysicalDeviceMemoryProperties(L, &props);
    return 1;
    }


/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceSparseImageFormatProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    int err;
    uint32_t i, count = 0;
    VkPhysicalDeviceSparseImageFormatInfo2* info = NULL;
    VkSparseImageFormatProperties2* props = NULL;

#define CLEANUP do {                                                \
    zfreeVkPhysicalDeviceSparseImageFormatInfo2(L, info, 1);        \
    zfreearrayVkSparseImageFormatProperties2(L, props, count, 1);   \
} while(0)
    info = zcheckVkPhysicalDeviceSparseImageFormatInfo2(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    lua_newtable(L);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties2(physical_device, info, &count, NULL);
    if(count == 0) { CLEANUP; return 1; }
    props = znewchainarrayVkSparseImageFormatProperties2(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties2(physical_device, info, &count, props);
    for(i=0; i<count; i++)
        {
        zpushVkSparseImageFormatProperties2(L, &props[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceSparseImageFormatProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    uint32_t i, count = 0;
    VkPhysicalDeviceSparseImageFormatInfo2* info;
    VkSparseImageFormatProperties* props;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceSparseImageFormatProperties2)
        return GetPhysicalDeviceSparseImageFormatProperties2(L, physical_device, ud);
#define CLEANUP zfreeVkPhysicalDeviceSparseImageFormatInfo2(L, info, 1)
    info = zcheckVkPhysicalDeviceSparseImageFormatInfo2(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties(physical_device, 
        info->format, info->type, info->samples, info->usage, info->tiling, &count, NULL);
    if(count == 0) { CLEANUP; return 1; }
    props = (VkSparseImageFormatProperties*)Malloc(L, sizeof(VkSparseImageFormatProperties)*count);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties(physical_device, 
        info->format, info->type, info->samples, info->usage, info->tiling, &count, props);
    for(i=0; i<count; i++)
        {
        zpushVkSparseImageFormatProperties(L, &props[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
    Free(L, props);
#undef CLEANUP
    return 1;
    }

/*-----------------------------------------------------------------------------*/

static int GetPhysicalDeviceExternalBufferProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkPhysicalDeviceExternalBufferInfoKHR* info = NULL;
    VkExternalBufferPropertiesKHR* props = NULL;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceExternalBufferProperties);
#define CLEANUP do {                                        \
    zfreeVkPhysicalDeviceExternalBufferInfoKHR(L, info, 1); \
    zfreeVkExternalBufferPropertiesKHR(L, props, 1);        \
} while(0)
    info = zcheckVkPhysicalDeviceExternalBufferInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    props = znewchainVkExternalBufferPropertiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceExternalBufferProperties(physical_device, info, props);
    zpushVkExternalBufferPropertiesKHR(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetPhysicalDeviceExternalFenceProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkPhysicalDeviceExternalFenceInfoKHR* info = NULL;
    VkExternalFencePropertiesKHR* props = NULL;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceExternalFenceProperties);
#define CLEANUP do {                                        \
    zfreeVkPhysicalDeviceExternalFenceInfoKHR(L, info, 1);  \
    zfreeVkExternalFencePropertiesKHR(L, props, 1);     \
} while(0)
    info = zcheckVkPhysicalDeviceExternalFenceInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    props = znewchainVkExternalFencePropertiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceExternalFenceProperties(physical_device, info, props);
    zpushVkExternalFencePropertiesKHR(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }


static int GetPhysicalDeviceExternalSemaphoreProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkPhysicalDeviceExternalSemaphoreInfo* info = NULL;
    VkExternalSemaphorePropertiesKHR* props = NULL;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceExternalSemaphoreProperties);
#define CLEANUP do {                                        \
    zfreeVkPhysicalDeviceExternalSemaphoreInfo(L, info, 1);  \
    zfreeVkExternalSemaphorePropertiesKHR(L, props, 1);     \
} while(0)
    info = zcheckVkPhysicalDeviceExternalSemaphoreInfo(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    props = znewchainVkExternalSemaphorePropertiesKHR(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceExternalSemaphoreProperties(physical_device, info, props);
    zpushVkExternalSemaphorePropertiesKHR(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

/*-----------------------------------------------------------------------------*/

static int GetPhysicalDeviceMultisampleProperties(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkMultisamplePropertiesEXT* props;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    VkSampleCountFlagBits samples = checkflags(L, 2);
    CheckInstancePfn(L, ud, GetPhysicalDeviceMultisamplePropertiesEXT);
#define CLEANUP zfreeVkMultisamplePropertiesEXT(L, props, 1)
    props = znewchainVkMultisamplePropertiesEXT(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->idt->GetPhysicalDeviceMultisamplePropertiesEXT(physical_device, samples, props);
    zpushVkMultisamplePropertiesEXT(L, props);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

/*-----------------------------------------------------------------------------*/

static int GetPhysicalDeviceCalibrateableTimeDomains(lua_State *L)
    {
    ud_t *ud;
    VkResult ec;
    uint32_t i, count = 0;
    VkTimeDomainEXT* domains;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    CheckInstancePfn(L, ud, GetPhysicalDeviceCalibrateableTimeDomainsEXT);

    ec = ud->idt->GetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, NULL);
    CheckError(L, ec);

    lua_newtable(L);
    if(count==0) return 1;

    domains = (VkTimeDomainEXT*)Malloc(L, sizeof(VkTimeDomainEXT)*count);
    ec = ud->idt->GetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, domains);
    if(ec)
        { Free(L, domains); CheckError(L, ec); }
    for(i=0; i<count; i++)
        {
        pushtimedomain(L, domains[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, domains);
    return 1;
    }

/*-----------------------------------------------------------------------------*/

RAW_FUNC_DISPATCHABLE(physical_device)
TYPE_FUNC(physical_device)
INSTANCE_FUNC(physical_device)
DEVICE_FUNC(physical_device)
PARENT_FUNC(physical_device)
DELETE_FUNC(physical_device)

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
        { "get_physical_device_properties", GetPhysicalDeviceProperties },
        { "get_physical_device_features", GetPhysicalDeviceFeatures },
        { "get_physical_device_format_properties", GetPhysicalDeviceFormatProperties },
        { "get_physical_device_image_format_properties", GetPhysicalDeviceImageFormatProperties },
        { "get_physical_device_queue_family_properties", GetPhysicalDeviceQueueFamilyProperties },
        { "get_physical_device_memory_properties", GetPhysicalDeviceMemoryProperties },
        { "get_physical_device_sparse_image_format_properties", GetPhysicalDeviceSparseImageFormatProperties },
        { "get_physical_device_external_buffer_properties", GetPhysicalDeviceExternalBufferProperties },
        { "get_physical_device_external_fence_properties", GetPhysicalDeviceExternalFenceProperties },
        { "get_physical_device_external_semaphore_properties", GetPhysicalDeviceExternalSemaphoreProperties },
        { "get_physical_device_multisample_properties", GetPhysicalDeviceMultisampleProperties },
        { "get_physical_device_calibrateable_time_domains", GetPhysicalDeviceCalibrateableTimeDomains },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_physical_device(lua_State *L)
    {
    udata_define(L, PHYSICAL_DEVICE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

