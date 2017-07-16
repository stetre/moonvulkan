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

static int GetPhysicalDeviceProperties2
        (lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    VkPhysicalDeviceProperties2KHR *properties2 = newphysicaldeviceproperties2(L);
    if(!properties2) return errmemory(L);
    ud->idt->GetPhysicalDeviceProperties2KHR(physical_device, properties2);
    pushphysicaldeviceproperties2(L, properties2);
    freephysicaldeviceproperties2(L, properties2);
    return 1;
    }

static int GetPhysicalDeviceProperties(lua_State *L)
    {
    VkPhysicalDeviceProperties properties;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceProperties2KHR)
        return GetPhysicalDeviceProperties2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceProperties(physical_device, &properties);
    pushphysicaldeviceproperties(L, &properties);
    return 1;
    }


/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceFeatures2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    VkPhysicalDeviceFeatures2KHR *features2 = newphysicaldevicefeatures2(L);
    if(!features2) return errmemory(L);
    ud->idt->GetPhysicalDeviceFeatures2KHR(physical_device, features2);
    pushphysicaldevicefeatures2(L, features2);
    freephysicaldevicefeatures2(L, features2);
    return 1;
    }

static int GetPhysicalDeviceFeatures(lua_State *L)
    {
    VkPhysicalDeviceFeatures features;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceFeatures2KHR)
        return GetPhysicalDeviceFeatures2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceFeatures(physical_device, &features);
    pushphysicaldevicefeatures(L, &features);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceFormatProperties2
        (lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    VkFormatProperties2KHR *properties2;
    VkFormat format = checkformat(L, 2);

    properties2 = newformatproperties2(L);
    if(!properties2) return errmemory(L);

    ud->idt->GetPhysicalDeviceFormatProperties2KHR(physical_device, format, properties2);
    pushformatproperties2(L, properties2);
    freeformatproperties2(L, properties2);
    return 1;
    }

static int GetPhysicalDeviceFormatProperties(lua_State *L)
    {
    VkFormatProperties properties;
    VkFormat format;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceFormatProperties2KHR)
        return GetPhysicalDeviceFormatProperties2(L, physical_device, ud);
    format = checkformat(L, 2);
    ud->idt->GetPhysicalDeviceFormatProperties(physical_device, format, &properties);
    pushformatproperties(L, &properties);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceImageFormatProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    VkResult ec;
    VkPhysicalDeviceImageFormatInfo2KHR info;
    VkImageFormatProperties2KHR *properties2;
    if(echeckphysicaldeviceimageformatinfo2(L, 2, &info)) return argerror(L, 2);

    properties2 = newimageformatproperties2(L);
    if(!properties2) return errmemory(L);

    ec = ud->idt->GetPhysicalDeviceImageFormatProperties2KHR(physical_device, &info, properties2);
    if(ec)
        {
        freeimageformatproperties2(L, properties2);
        CheckError(L, ec);
        return 0;
        }
    pushimageformatproperties2(L, properties2);
    freeimageformatproperties2(L, properties2);
    return 1;
    }

static int GetPhysicalDeviceImageFormatProperties(lua_State *L)
    {
    VkResult ec;
    VkPhysicalDeviceImageFormatInfo2KHR info;
    VkImageFormatProperties properties;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceImageFormatProperties2KHR) 
        return GetPhysicalDeviceImageFormatProperties2(L, physical_device, ud);
    if(echeckphysicaldeviceimageformatinfo2(L, 2, &info)) return argerror(L, 2);
    ec = ud->idt->GetPhysicalDeviceImageFormatProperties(physical_device, 
            info.format, info.type, info.tiling, info.usage, info.flags, &properties);
    CheckError(L, ec);
    pushimageformatproperties(L, &properties);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceQueueFamilyProperties2
        (lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    uint32_t i, count = 0;
    VkQueueFamilyProperties2KHR *properties2;

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties2KHR(physical_device, &count, NULL);
    if(count == 0) return 1;
    properties2 = newqueuefamilyproperties2(L, count);
    if(!properties2) return errmemory(L);

    ud->idt->GetPhysicalDeviceQueueFamilyProperties2KHR(physical_device, &count, properties2);
    for(i=0; i<count; i++)
        {
        pushqueuefamilyproperties2(L, &(properties2[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    freequeuefamilyproperties2(L, properties2, count);
    return 1;
    }

static int GetPhysicalDeviceQueueFamilyProperties(lua_State *L)
    {
    uint32_t i, count = 0;
    VkQueueFamilyProperties *properties;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    
    if(ud->idt->GetPhysicalDeviceQueueFamilyProperties2KHR)
        return GetPhysicalDeviceQueueFamilyProperties2(L, physical_device, ud);

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
    if(count == 0) return 1;
    properties = (VkQueueFamilyProperties*)Malloc(L, sizeof(VkQueueFamilyProperties)*count);
    ud->idt->GetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties);
    for(i=0; i<count; i++)
        {
        pushqueuefamilyproperties(L, &(properties[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, properties);
    return 1;
    }

/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceMemoryProperties2(lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    VkPhysicalDeviceMemoryProperties2KHR *properties2;

    properties2 = newphysicaldevicememoryproperties2(L);
    if(!properties2) return errmemory(L);

    ud->idt->GetPhysicalDeviceMemoryProperties2KHR(physical_device, properties2);
    pushphysicaldevicememoryproperties2(L, properties2);
    freephysicaldevicememoryproperties2(L, properties2);
    return 1;
    }

static int GetPhysicalDeviceMemoryProperties(lua_State *L)
    {
    VkPhysicalDeviceMemoryProperties properties;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    if(ud->idt->GetPhysicalDeviceMemoryProperties2KHR)
        return GetPhysicalDeviceMemoryProperties2(L, physical_device, ud);
    ud->idt->GetPhysicalDeviceMemoryProperties(physical_device, &properties);
    pushphysicaldevicememoryproperties(L, &properties);
    return 1;
    }


/*-----------------------------------------------------------------------------*/
static int GetPhysicalDeviceSparseImageFormatProperties2
        (lua_State *L, VkPhysicalDevice physical_device, ud_t *ud)
    {
    uint32_t i, count = 0;
    VkPhysicalDeviceSparseImageFormatInfo2KHR info;
    VkSparseImageFormatProperties2KHR *properties2;

    if(echeckphysicaldevicesparseimageformatinfo2(L, 2, &info)) return argerror(L, 2);

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties2KHR(physical_device, &info, &count, NULL);
    if(count == 0) return 1;
    properties2 = newsparseimageformatproperties2(L, count);
    if(!properties2) return errmemory(L);

    ud->idt->GetPhysicalDeviceSparseImageFormatProperties2KHR(physical_device, &info, &count, properties2);
    for(i=0; i<count; i++)
        {
        pushsparseimageformatproperties2(L, &(properties2[i]));
        lua_rawseti(L, -2, i+1);
        }
    freesparseimageformatproperties2(L, properties2, count);
    return 1;
    }


static int GetPhysicalDeviceSparseImageFormatProperties(lua_State *L)
    {
    uint32_t i, count = 0;
    VkPhysicalDeviceSparseImageFormatInfo2KHR info;
    VkSparseImageFormatProperties *properties;
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    
    if(ud->idt->GetPhysicalDeviceSparseImageFormatProperties2KHR)
        return GetPhysicalDeviceSparseImageFormatProperties2(L, physical_device, ud);

    if(echeckphysicaldevicesparseimageformatinfo2(L, 2, &info)) return argerror(L, 2);

    lua_newtable(L);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties(physical_device, 
        info.format, info.type, info.samples, info.usage, info.tiling, &count, NULL);
    if(count == 0) return 1;
    properties = (VkSparseImageFormatProperties*)Malloc(L, sizeof(VkSparseImageFormatProperties)*count);
    ud->idt->GetPhysicalDeviceSparseImageFormatProperties(physical_device, 
        info.format, info.type, info.samples, info.usage, info.tiling, &count, properties);
    for(i=0; i<count; i++)
        {
        pushsparseimageformatproperties(L, &(properties[i]));
        lua_rawseti(L, -2, i+1);
        }
    Free(L, properties);
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
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_physical_device(lua_State *L)
    {
    udata_define(L, PHYSICAL_DEVICE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

