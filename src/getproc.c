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

static PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr GetDeviceProcAddr;

#define FP(f) *(void**)(&(f))
/* Cast to silent compiler warnings without giving up the -Wpedantic flag.
 *("ISO C forbids conversion of function pointer to object pointer type")
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

    FP(GetInstanceProcAddr) = dlsym(handle, "vkGetInstanceProcAddr");
    if(!GetInstanceProcAddr)
        return luaL_error(L, "cannot find vkGetInstanceProcAddr");

    FP(GetDeviceProcAddr) = dlsym(handle, "vkGetDeviceProcAddr");
    if(!GetDeviceProcAddr)
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

instance_dt_t * getproc_instance(lua_State *L, VkInstance instance, VkInstanceCreateInfo *createinfo)
    {
    uint32_t i;
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

    /* EXTENSIONS -------------------------------- */
#define GET(fn) do { dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn); } while(0)

    for(i = 0; i < createinfo->enabledExtensionCount; i++)
        {
#define IF(extensionname) if(strncmp(extensionname, createinfo->ppEnabledExtensionNames[i], 256)==0)
        IF("VK_KHR_display")
            {
            GET(GetPhysicalDeviceDisplayPropertiesKHR);
            GET(GetPhysicalDeviceDisplayPlanePropertiesKHR);
            GET(GetDisplayPlaneSupportedDisplaysKHR);
            GET(GetDisplayModePropertiesKHR);
            GET(CreateDisplayModeKHR);
            GET(GetDisplayPlaneCapabilitiesKHR);
            GET(CreateDisplayPlaneSurfaceKHR);
            continue;
            }
        IF("VK_KHR_external_fence_capabilities")
            {
            GET(GetPhysicalDeviceExternalFencePropertiesKHR);
            continue;
            }
        IF("VK_KHR_external_memory_capabilities")
            {
            GET(GetPhysicalDeviceExternalBufferPropertiesKHR);
            continue;
            }
        IF("VK_KHR_external_semaphore_capabilities")
            {
            GET(GetPhysicalDeviceExternalSemaphorePropertiesKHR);
            continue;
            }
        IF("VK_KHR_get_physical_device_properties2")
            {
            GET(GetPhysicalDeviceFeatures2KHR);
            GET(GetPhysicalDeviceProperties2KHR);
            GET(GetPhysicalDeviceFormatProperties2KHR);
            GET(GetPhysicalDeviceImageFormatProperties2KHR);
            GET(GetPhysicalDeviceQueueFamilyProperties2KHR);
            GET(GetPhysicalDeviceMemoryProperties2KHR);
            GET(GetPhysicalDeviceSparseImageFormatProperties2KHR);
            continue;
            }
        IF("VK_KHR_get_surface_capabilities2")
            {
            GET(GetPhysicalDeviceSurfaceCapabilities2KHR);
            GET(GetPhysicalDeviceSurfaceFormats2KHR);
            continue;
            }
        IF("VK_KHR_surface")
            {
            GET(DestroySurfaceKHR);
            GET(GetPhysicalDeviceSurfaceSupportKHR);
            GET(GetPhysicalDeviceSurfaceCapabilitiesKHR);
            GET(GetPhysicalDeviceSurfaceFormatsKHR);
            GET(GetPhysicalDeviceSurfacePresentModesKHR);
            continue;
            }
        IF("VK_EXT_acquire_xlib_display")
            {
            continue;
            }
        IF("VK_EXT_debug_report")
            {
            //  GET(DebugReportCallbackEXT);
            GET(CreateDebugReportCallbackEXT);
            GET(DestroyDebugReportCallbackEXT);
            GET(DebugReportMessageEXT);
            continue;
            }
        IF("VK_EXT_direct_mode_display")
            {
            GET(ReleaseDisplayEXT);
            continue;
            }
        IF("VK_EXT_display_surface_counter")
            {
            GET(GetPhysicalDeviceSurfaceCapabilities2EXT);
            continue;
            }
        IF("VK_EXT_swapchain_colorspace")
            {
            continue;
            }
        IF("VK_EXT_validation_flags")
            {
            continue;
            }
#ifdef VK_USE_PLATFORM_XCB_KHR
        IF("VK_KHR_xcb_surface")
            {
            GET(CreateXcbSurfaceKHR);
            GET(GetPhysicalDeviceXcbPresentationSupportKHR);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        IF("VK_KHR_xlib_surface")
            {
            GET(CreateXlibSurfaceKHR);
            GET(GetPhysicalDeviceXlibPresentationSupportKHR);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
        IF("VK_EXT_acquire_xlib_display")
            {
            GET(AcquireXlibDisplayEXT);
            GET(GetRandROutputDisplayEXT);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        IF("VK_KHR_wayland_surface")
            {
            GET(CreateWaylandSurfaceKHR);
            GET(GetPhysicalDeviceWaylandPresentationSupportKHR);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
        IF("VK_KHR_mir_surface")
            {
            GET(CreateMirSurfaceKHR);
            GET(GetPhysicalDeviceMirPresentationSupportKHR);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        IF("VK_KHR_android_surface")
            {
            GET(CreateAndroidSurfaceKHR);
            continue;
            }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        IF("VK_KHR_win32_surface")
            {
            GET(CreateWin32SurfaceKHR);
            GET(GetPhysicalDeviceWin32PresentationSupportKHR);
            continue;
            }
#endif
#undef IF
        }
#undef GET
    return dt;
    }

device_dt_t* getproc_device(lua_State *L, VkDevice device, VkDeviceCreateInfo *createinfo)
    {
    uint32_t i;
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

    /* EXTENSIONS -------------------------------- */
#define GET(fn) do {                                            \
    dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn);    \
    /*printf(""#fn" %p\n", (void*)(dt->fn)); */                 \
} while(0)

    for(i = 0; i < createinfo->enabledExtensionCount; i++)
        {
#define IF(extensionname) if(strncmp(extensionname, createinfo->ppEnabledExtensionNames[i], 256)==0)
        IF("VK_KHR_16_bit_storage")
            {
            continue;
            }
        IF("VK_KHR_descriptor_update_template")
            {
            GET(CreateDescriptorUpdateTemplateKHR);
            GET(DestroyDescriptorUpdateTemplateKHR);
            GET(UpdateDescriptorSetWithTemplateKHR);
            GET(CmdPushDescriptorSetWithTemplateKHR);
            continue;
            }
        IF("VK_KHR_dedicated_allocation")
            {
            continue;
            }
        IF("VK_KHR_display_swapchain")
            {
            GET(CreateSharedSwapchainsKHR);
            continue;
            }

        IF("VK_KHR_external_fence")
            {
            continue;
            }
        IF("VK_KHR_external_fence_fd")
            {
            GET(ImportFenceFdKHR);
            GET(GetFenceFdKHR);
            continue;
            }
        IF("VK_KHR_external_memory")
            {
            continue;
            }
        IF("VK_KHR_external_memory_fd")
            {
            GET(GetMemoryFdKHR);
            GET(GetMemoryFdPropertiesKHR);
            continue;
            }
        IF("VK_KHR_external_semaphore")
            {
            continue;
            }
        IF("VK_KHR_external_semaphore_fd")
            {
            GET(ImportSemaphoreFdKHR);
            GET(GetSemaphoreFdKHR);
            continue;
            }
        IF("VK_KHR_get_memory_requirements2")
            {
            GET(GetImageMemoryRequirements2KHR);
            GET(GetBufferMemoryRequirements2KHR);
            GET(GetImageSparseMemoryRequirements2KHR);
            continue;
            }
        IF("VK_KHR_incremental_present")
            {
            continue;
            }
        IF("VK_KHR_maintenance1")
            {
            GET(TrimCommandPoolKHR);
            continue;
            }
        IF("VK_KHR_push_descriptor")
            {
            GET(CmdPushDescriptorSetKHR);
            continue;
            }
        IF("VK_KHR_sampler_mirror_clamp_to_edge")
            {
            continue;
            }
        IF("VK_KHR_shader_draw_parameters")
            {
            continue;
            }
        IF("VK_KHR_shader_presentable_image")
            {
            GET(GetSwapchainStatusKHR);
            continue;
            }
        IF("VK_KHR_storage_buffer_storage_class")
            {
            continue;
            }
        IF("VK_KHR_swapchain")
            {
            GET(CreateSwapchainKHR);
            GET(DestroySwapchainKHR);
            GET(GetSwapchainImagesKHR);
            GET(AcquireNextImageKHR);
            GET(QueuePresentKHR);
            continue;
            }
        IF("VK_KHR_variable_pointers")
            {
            continue;
            }
        IF("VK_KHR_relaxed_block_layout")
            {
            continue;
            }
        IF("VK_EXT_blend_operation_advanced")
            {
            continue;
            }
        IF("VK_EXT_debug_marker")
            {
            GET(DebugMarkerSetObjectTagEXT);
            GET(DebugMarkerSetObjectNameEXT);
            GET(CmdDebugMarkerBeginEXT);
            GET(CmdDebugMarkerEndEXT);
            GET(CmdDebugMarkerInsertEXT);
            continue;
            }
        IF("VK_EXT_depth_range_unrestricted")
            {
            continue;
            }
        IF("VK_EXT_discard_rectangles")
            {
            GET(CmdSetDiscardRectangleEXT);
            continue;
            }
        IF("VK_EXT_display_control")
            {
            GET(DisplayPowerControlEXT);
            GET(RegisterDeviceEventEXT);
            GET(RegisterDisplayEventEXT);
            GET(GetSwapchainCounterEXT);
            continue;
            }
        IF("VK_EXT_hdr_metadata")
            {
            GET(SetHdrMetadataEXT);
            continue;
            }
        IF("VK_EXT_post_depth_coverage")
            {
            continue;
            }
        IF("VK_EXT_sampler_filter_minmax")
            {
            continue;
            }
        IF("VK_EXT_shader_subgroup_ballot")
            {
            continue;
            }
        IF("VK_EXT_shader_subgroup_vote")
            {
            continue;
            }
        IF("VK_EXT_")
            {
            continue;
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        IF("VK_KHR_win32_keyed_mutex")
            {
            continue;
            }
        IF("VK_KHR_external_fence_win32")
            {
            GET(ImportFenceWin32HandleKHR);
            GET(GetFenceWin32HandleKHR);
            continue;
            }
        IF("VK_KHR_external_memory_win32")
            {
            GET(GetMemoryWin32HandleKHR);
            GET(GetMemoryWin32HandlePropertiesKHR);
            continue;
            }
        IF("VK_KHR_external_semaphore_win32")
            {
            GET(ImportSemaphoreWin32HandleKHR);
            GET(GetSemaphoreWin32HandleKHR);
            continue;
            }
#endif
#undef IF
        }
#undef GET
    return dt;
    }


int moonvulkan_open_getproc(lua_State *L)
    {
    Init(L);
    return 0;
    }
