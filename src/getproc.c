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

global_dt_t vk; /* global dispatch table (non-instance and non-device functions) */

#ifdef LINUX
/*----------------------------------------------------------------------------------*
 | Linux                                                                            |
 *----------------------------------------------------------------------------------*/
typedef union { 
    /* To avoid "ISO C forbids conversion of function pointer to object pointer type" 
     * warnings without giving up the -Wpedantic flag, we store the dlsym return value 
     * into fp, and then use it as a function pointer by dereferencing one of the two
     * other union fields.
     */
    void * fp;
    PFN_vkGetInstanceProcAddr get_instance_proc_addr;
    PFN_vkGetDeviceProcAddr get_device_proc_addr;
} getproc_u;

static getproc_u gp_instance;
static getproc_u gp_device;
#define GetInstanceProcAddr gp_instance.get_instance_proc_addr 
#define GetDeviceProcAddr gp_device.get_device_proc_addr 
/* pfn = GetInstanceProcAddr(instance, name) 
 * pfn = GetDeviceProcAddr(device, name)
 */

#include <dlfcn.h>
static int Init(lua_State *L)
    {
    char *err;
    void *handle = dlopen("libvulkan.so", RTLD_LAZY | RTLD_LOCAL);
    if(!handle)
        {
        err = dlerror();
        return luaL_error(L, err != NULL ? err : "cannot load libvulkan.so");
        }

    gp_instance.fp = dlsym(handle, "vkGetInstanceProcAddr");
    if(!gp_instance.fp)
        return luaL_error(L, "cannot find vkGetInstanceProcAddr");

    gp_device.fp = dlsym(handle, "vkGetDeviceProcAddr");
    if(!gp_device.fp)
        return luaL_error(L, "cannot find vkGetDeviceProcAddr");

    /* Fill the global dispatch table */
#define GET(fn) do {                                            \
    vk.fn = (PFN_vk##fn)GetInstanceProcAddr(NULL, "vk"#fn);     \
    if(!vk.fn) return luaL_error(L, "cannot find vk"#fn);       \
} while(0)
    GET(EnumerateInstanceExtensionProperties);
    GET(EnumerateInstanceLayerProperties);
    GET(CreateInstance);
#undef GET

    return 0;
    }

#else
/*----------------------------------------------------------------------------------*
 | @@ Other platforms (MINGW, WIN32, ecc) 
 *----------------------------------------------------------------------------------*/
#define GetInstanceProcAddr(instance, name) NULL
#define GetDeviceProcAddr(device, name) NULL
static int Init(lua_State *L)
    {
    return luaL_error(L, "platform not supported");
    return 0;
    }

#endif

/*----------------------------------------------------------------------------------*/

instance_dt_t * getproc_instance(lua_State *L, VkInstance instance)
    {
    instance_dt_t *dt = (instance_dt_t*)Malloc(L, sizeof(instance_dt_t));
    /* Core functions ---------------------------- */
#define GET(fn) do {                                                \
    dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn);    \
    if(!dt->fn) luaL_error(L, "cannot find vk"#fn);                 \
} while(0)
    GET(DestroyInstance);
    GET(EnumeratePhysicalDevices);
    GET(EnumerateDeviceLayerProperties);
    GET(EnumerateDeviceExtensionProperties);
    GET(GetPhysicalDeviceFeatures);
    GET(GetPhysicalDeviceFormatProperties);
    GET(GetPhysicalDeviceImageFormatProperties);
    GET(GetPhysicalDeviceProperties);
    GET(GetPhysicalDeviceQueueFamilyProperties);
    GET(GetPhysicalDeviceMemoryProperties);
    GET(CreateDevice);
    GET(GetPhysicalDeviceSparseImageFormatProperties);
#undef GET
    /* Extensions -------------------------------- */
#define GET(fn) do { dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn); } while(0)
    GET(DestroySurfaceKHR);
    GET(GetPhysicalDeviceSurfaceSupportKHR);
    GET(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET(GetPhysicalDeviceSurfaceCapabilities2KHR);
    GET(GetPhysicalDeviceSurfaceFormatsKHR);
    GET(GetPhysicalDeviceSurfaceFormats2KHR);
    GET(GetPhysicalDeviceSurfacePresentModesKHR);
    GET(GetPhysicalDeviceDisplayPropertiesKHR);
    GET(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    GET(GetDisplayPlaneSupportedDisplaysKHR);
    GET(GetDisplayModePropertiesKHR);
    GET(CreateDisplayModeKHR);
    GET(GetDisplayPlaneCapabilitiesKHR);
    GET(CreateDisplayPlaneSurfaceKHR);
#ifdef VK_USE_PLATFORM_XCB_KHR
    GET(CreateXcbSurfaceKHR);
    GET(GetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GET(CreateXlibSurfaceKHR);
    GET(GetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    GET(CreateWaylandSurfaceKHR);
    GET(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    GET(CreateMirSurfaceKHR);
    GET(GetPhysicalDeviceMirPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    GET(CreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GET(CreateWin32SurfaceKHR);
    GET(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif
    GET(CreateDebugReportCallbackEXT);
    GET(DestroyDebugReportCallbackEXT);
    GET(DebugReportMessageEXT);
    GET(GetPhysicalDeviceFeatures2KHR);
    GET(GetPhysicalDeviceProperties2KHR);
    GET(GetPhysicalDeviceFormatProperties2KHR);
    GET(GetPhysicalDeviceImageFormatProperties2KHR);
    GET(GetPhysicalDeviceQueueFamilyProperties2KHR);
    GET(GetPhysicalDeviceMemoryProperties2KHR);
    GET(GetPhysicalDeviceSparseImageFormatProperties2KHR);
#undef GET
    return dt;
    }

device_dt_t* getproc_device(lua_State *L, VkDevice device)
    {
    device_dt_t *dt = (device_dt_t*)Malloc(L, sizeof(device_dt_t));

    /* Core functions ---------------------------- */
#define GET(fn) do {                                            \
    dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn);    \
    if(!dt->fn) luaL_error(L, "cannot find vk"#fn);             \
} while(0)
    GET(DestroyDevice);
    GET(GetDeviceQueue);
    GET(QueueSubmit);
    GET(QueueWaitIdle);
    GET(DeviceWaitIdle);
    GET(AllocateMemory);
    GET(FreeMemory);
    GET(MapMemory);
    GET(UnmapMemory);
    GET(FlushMappedMemoryRanges);
    GET(InvalidateMappedMemoryRanges);
    GET(GetDeviceMemoryCommitment);
    GET(BindBufferMemory);
    GET(BindImageMemory);
    GET(GetBufferMemoryRequirements);
    GET(GetImageMemoryRequirements);
    GET(GetImageSparseMemoryRequirements);
    GET(QueueBindSparse);
    GET(CreateFence);
    GET(DestroyFence);
    GET(ResetFences);
    GET(GetFenceStatus);
    GET(WaitForFences);
    GET(CreateSemaphore);
    GET(DestroySemaphore);
    GET(CreateEvent);
    GET(DestroyEvent);
    GET(GetEventStatus);
    GET(SetEvent);
    GET(ResetEvent);
    GET(CreateQueryPool);
    GET(DestroyQueryPool);
    GET(GetQueryPoolResults);
    GET(CreateBuffer);
    GET(DestroyBuffer);
    GET(CreateBufferView);
    GET(DestroyBufferView);
    GET(CreateImage);
    GET(DestroyImage);
    GET(GetImageSubresourceLayout);
    GET(CreateImageView);
    GET(DestroyImageView);
    GET(CreateShaderModule);
    GET(DestroyShaderModule);
    GET(CreatePipelineCache);
    GET(DestroyPipelineCache);
    GET(GetPipelineCacheData);
    GET(MergePipelineCaches);
    GET(CreateGraphicsPipelines);
    GET(CreateComputePipelines);
    GET(DestroyPipeline);
    GET(CreatePipelineLayout);
    GET(DestroyPipelineLayout);
    GET(CreateSampler);
    GET(DestroySampler);
    GET(CreateDescriptorSetLayout);
    GET(DestroyDescriptorSetLayout);
    GET(CreateDescriptorPool);
    GET(DestroyDescriptorPool);
    GET(ResetDescriptorPool);
    GET(AllocateDescriptorSets);
    GET(FreeDescriptorSets);
    GET(UpdateDescriptorSets);
    GET(CreateFramebuffer);
    GET(DestroyFramebuffer);
    GET(CreateRenderPass);
    GET(DestroyRenderPass);
    GET(GetRenderAreaGranularity);
    GET(CreateCommandPool);
    GET(DestroyCommandPool);
    GET(ResetCommandPool);
    GET(AllocateCommandBuffers);
    GET(FreeCommandBuffers);
    GET(BeginCommandBuffer);
    GET(EndCommandBuffer);
    GET(ResetCommandBuffer);
    GET(CmdBindPipeline);
    GET(CmdSetViewport);
    GET(CmdSetScissor);
    GET(CmdSetLineWidth);
    GET(CmdSetDepthBias);
    GET(CmdSetBlendConstants);
    GET(CmdSetDepthBounds);
    GET(CmdSetStencilCompareMask);
    GET(CmdSetStencilWriteMask);
    GET(CmdSetStencilReference);
    GET(CmdBindDescriptorSets);
    GET(CmdBindIndexBuffer);
    GET(CmdBindVertexBuffers);
    GET(CmdDraw);
    GET(CmdDrawIndexed);
    GET(CmdDrawIndirect);
    GET(CmdDrawIndexedIndirect);
    GET(CmdDispatch);
    GET(CmdDispatchIndirect);
    GET(CmdCopyBuffer);
    GET(CmdCopyImage);
    GET(CmdBlitImage);
    GET(CmdCopyBufferToImage);
    GET(CmdCopyImageToBuffer);
    GET(CmdUpdateBuffer);
    GET(CmdFillBuffer);
    GET(CmdClearColorImage);
    GET(CmdClearDepthStencilImage);
    GET(CmdClearAttachments);
    GET(CmdResolveImage);
    GET(CmdSetEvent);
    GET(CmdResetEvent);
    GET(CmdWaitEvents);
    GET(CmdPipelineBarrier);
    GET(CmdBeginQuery);
    GET(CmdEndQuery);
    GET(CmdResetQueryPool);
    GET(CmdWriteTimestamp);
    GET(CmdCopyQueryPoolResults);
    GET(CmdPushConstants);
    GET(CmdBeginRenderPass);
    GET(CmdNextSubpass);
    GET(CmdEndRenderPass);
    GET(CmdExecuteCommands);
#undef GET
    /* Extensions -------------------------------- */
#define GET(fn) do { dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn); } while(0)
    GET(CreateSwapchainKHR);
    GET(DestroySwapchainKHR);
    GET(GetSwapchainImagesKHR);
    GET(GetSwapchainStatusKHR);
    GET(AcquireNextImageKHR);
    GET(QueuePresentKHR);
    GET(CreateSharedSwapchainsKHR);
    GET(TrimCommandPoolKHR);
    GET(DebugReportCallbackEXT);
    GET(DebugMarkerSetObjectTagEXT);
    GET(DebugMarkerSetObjectNameEXT);
    GET(DisplayPowerControlEXT);
    GET(RegisterDeviceEventEXT);
    GET(RegisterDisplayEventEXT);
    GET(GetSwapchainCounterEXT);
    GET(CmdDebugMarkerBeginEXT);
    GET(CmdDebugMarkerEndEXT);
    GET(CmdDebugMarkerInsertEXT);
    GET(CmdPushDescriptorSetKHR);
    GET(CreateDescriptorUpdateTemplateKHR);
    GET(DestroyDescriptorUpdateTemplateKHR);
    GET(UpdateDescriptorSetWithTemplateKHR);
    GET(CmdPushDescriptorSetWithTemplateKHR);
#undef GET

    return dt;
    }


int moonvulkan_open_getproc(lua_State *L)
    {
    Init(L);
    return 0;
    }
