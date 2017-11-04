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

static int EnumerateInstanceLayerProperties(lua_State *L)
    {
    uint32_t count, remaining, tot, i;
    VkResult ec;
    VkLayerProperties properties[32];
    int byname = optboolean(L, 1, 0);

    lua_newtable(L);
    ec = vk.EnumerateInstanceLayerProperties(&remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = vk.EnumerateInstanceLayerProperties(&count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
        
        if(byname)
            {
            for(i = 0; i < count; i++)
                {
                lua_pushstring(L, properties[i].layerName);
                pushlayerproperties(L, &properties[i]);
                lua_rawset(L, -3);
                }
            }
        else
            {
            for(i = 0; i < count; i++)
                {
                pushlayerproperties(L, &properties[i]);
                lua_rawseti(L, -2, ++tot);
                }
            }
        } while(remaining > 0);
    
    return 1;
    }

static int EnumerateInstanceExtensionProperties(lua_State *L)
    {
    uint32_t count, remaining, tot, i;
    VkResult ec;
    VkExtensionProperties properties[32];
    const char *layer_name = luaL_optstring(L, 1, NULL);
    int byname = optboolean(L, 2, 0);

    lua_newtable(L);
    ec = vk.EnumerateInstanceExtensionProperties(layer_name, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = vk.EnumerateInstanceExtensionProperties(layer_name, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
        
        if(byname)
            {
            for(i = 0; i < count; i++)
                {
                lua_pushstring(L, properties[i].extensionName);
                pushextensionproperties(L, &properties[i]);
                lua_rawset(L, -3);
                }
            }
        else
            {
            for(i = 0; i < count; i++)
                {
                pushextensionproperties(L, &properties[i]);
                lua_rawseti(L, -2, ++tot);
                }
            }

        } while(remaining > 0);
    
    return 1;
    }


static int EnumerateDeviceLayerProperties(lua_State *L) /* DEPRECATED */
    {
    uint32_t count, remaining, tot, i;
    VkResult ec;
    ud_t *ud;
    VkLayerProperties properties[32];
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    int byname = optboolean(L, 2, 0);

    lua_newtable(L);
    ec = ud->idt->EnumerateDeviceLayerProperties(physical_device, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->EnumerateDeviceLayerProperties(physical_device, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        if(byname)
            {
            for(i = 0; i < count; i++)
                {
                lua_pushstring(L, properties[i].layerName);
                pushlayerproperties(L, &properties[i]);
                lua_rawset(L, -3);
                }
            }
        else
            {
            for(i = 0; i < count; i++)
                {
                pushlayerproperties(L, &properties[i]);
                lua_rawseti(L, -2, ++tot);
                }
            }
    } while (remaining > 0);

    return 1;
    }

static int EnumerateDeviceExtensionProperties(lua_State *L)
    {
    uint32_t count, remaining, tot, i;
    VkResult ec;
    VkExtensionProperties properties[32];
    ud_t *ud;
    VkPhysicalDevice physical_device = checkphysical_device(L, 1, &ud);
    const char* layername = luaL_optstring(L, 2, NULL);
    int byname = optboolean(L, 3, 0);
    
    lua_newtable(L);
    ec = ud->idt->EnumerateDeviceExtensionProperties(physical_device, layername, &remaining, NULL);
    CheckError(L, ec);
    if(remaining==0) return 1;
    tot = 0;
    do {
        if(remaining > 32)
            { count = 32; remaining -= 32; }
        else
            { count = remaining; remaining = 0; }

        ec = ud->idt->EnumerateDeviceExtensionProperties(physical_device, layername, &count, properties);
        if(ec != VK_INCOMPLETE)
            CheckError(L, ec);
    
        if(byname)
            {
            for(i = 0; i < count; i++)
                {
                lua_pushstring(L, properties[i].extensionName);
                pushextensionproperties(L, &properties[i]);
                lua_rawset(L, -3);
                }
            }
        else
            {
            for(i = 0; i < count; i++)
                {
                pushextensionproperties(L, &properties[i]);
                lua_rawseti(L, -2, ++tot);
                }
            }

    } while (remaining > 0);

    return 1;
    }


static const struct luaL_Reg Functions[] = 
    {
        { "enumerate_instance_extension_properties", EnumerateInstanceExtensionProperties },
        { "enumerate_instance_layer_properties", EnumerateInstanceLayerProperties },
        { "enumerate_device_extension_properties", EnumerateDeviceExtensionProperties },
        { "enumerate_device_layer_properties", EnumerateDeviceLayerProperties },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_layers(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }


