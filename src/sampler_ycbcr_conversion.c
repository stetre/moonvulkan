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

static int freesampler_ycbcr_conversion(lua_State *L, ud_t *ud)
    {
    VkSamplerYcbcrConversion sampler_ycbcr_conversion = (VkSamplerYcbcrConversion)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(sampler_ycbcr_conversion, "sampler_ycbcr_conversion");
    UD(device)->ddt->DestroySamplerYcbcrConversion(device, sampler_ycbcr_conversion, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    int err;
    ud_t *ud, *device_ud;
    VkResult ec;
    VkSamplerYcbcrConversion sampler_ycbcr_conversion;
    VkSamplerYcbcrConversionCreateInfoKHR* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = optallocator(L, 3);
    CheckDevicePfn(L, device_ud, CreateSamplerYcbcrConversion);
#define CLEANUP zfreeVkSamplerYcbcrConversionCreateInfoKHR(L, info, 1)
    info = zcheckVkSamplerYcbcrConversionCreateInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = device_ud->ddt->CreateSamplerYcbcrConversion(device,
                            info, allocator, &sampler_ycbcr_conversion);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    TRACE_CREATE(sampler_ycbcr_conversion, "sampler_ycbcr_conversion");
    ud = newuserdata_nondispatchable(L, sampler_ycbcr_conversion, SAMPLER_YCBCR_CONVERSION_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freesampler_ycbcr_conversion;
    ud->ddt = device_ud->ddt;
    return 1;
    }


RAW_FUNC(sampler_ycbcr_conversion)
TYPE_FUNC(sampler_ycbcr_conversion)
INSTANCE_FUNC(sampler_ycbcr_conversion)
DEVICE_FUNC(sampler_ycbcr_conversion)
PARENT_FUNC(sampler_ycbcr_conversion)
DELETE_FUNC(sampler_ycbcr_conversion)
DESTROY_FUNC(sampler_ycbcr_conversion)

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
        { "create_sampler_ycbcr_conversion",  Create },
        { "destroy_sampler_ycbcr_conversion",  Destroy },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_sampler_ycbcr_conversion(lua_State *L)
    {
    udata_define(L, SAMPLER_YCBCR_CONVERSION_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

