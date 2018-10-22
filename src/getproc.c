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

#if defined(LINUX)
#include <dlfcn.h>
static void *Handle = NULL;
#define LIBNAME "libvulkan.so"

#elif defined(MINGW)
#include "damnwindows.h"
#define LIBNAME "vulkan-1.dll"
#define LLIBNAME L"vulkan-1.dll"
#define LIBNAME1 "vulkan.dll"
#define LLIBNAME1 L"vulkan.dll"
static HMODULE Handle = NULL;

#else
#error "Cannot determine platform"
#endif


static PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr GetDeviceProcAddr;

#define FP(f) *(void**)(&(f))
/* Cast to silence compiler warnings without giving up the -Wpedantic flag.
 *("ISO C forbids conversion of function pointer to object pointer type")
 */

static int Init(lua_State *L)
    {
#if defined(LINUX)
    char *err;

    Handle = dlopen(LIBNAME, RTLD_LAZY | RTLD_LOCAL);
    if(!Handle)
        {
        err = dlerror();
        return luaL_error(L, err != NULL ? err : "cannot load " LIBNAME);
        }

    FP(GetInstanceProcAddr) = dlsym(Handle, "vkGetInstanceProcAddr");
    FP(GetDeviceProcAddr) = dlsym(Handle, "vkGetDeviceProcAddr");

#elif defined(MINGW)
    Handle = LoadLibraryW(LLIBNAME);
    if(!Handle)
        Handle = LoadLibraryW(LLIBNAME1);
    if(!Handle)
        return luaL_error(L, "cannot load " LIBNAME " or " LIBNAME1);

    GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(Handle, "vkGetInstanceProcAddr");
    GetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)GetProcAddress(Handle, "vkGetDeviceProcAddr");

#endif

    if(!GetInstanceProcAddr)
        return luaL_error(L, "cannot find vkGetInstanceProcAddr");
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
#define GET(fn) do {                                            \
    vk.fn = (PFN_vk##fn)GetInstanceProcAddr(NULL, "vk"#fn);     \
} while(0)
    GET(EnumerateInstanceVersion);
#undef GET

    return 0;
    }


/*----------------------------------------------------------------------------------*/

instance_dt_t * getproc_instance(lua_State *L, VkInstance instance, VkInstanceCreateInfo *createinfo)
    {
    instance_dt_t *dt = (instance_dt_t*)Malloc(L, sizeof(instance_dt_t));
    (void)createinfo;

    /* Core 1.0 functions ---------------------------- */
#define GET(fn) do {                                                \
    dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn);    \
    if(!dt->fn) { luaL_error(L, "cannot find vk"#fn); return dt; }\
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

    /* Extensions and core > 1.0 ----------------------- */
#define GET(fn) do { \
    dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn); \
    /* printf(""#fn" %p\n", (void*)(dt->fn)); */                 \
} while(0)
    /* promoted extensions may lack the KHR suffix, so try also without it */
#define GETKHR(fn) do { \
    dt->fn##KHR = (PFN_vk##fn##KHR)GetInstanceProcAddr(instance, "vk"#fn"KHR");             \
    if(!dt->fn##KHR) dt->fn##KHR = (PFN_vk##fn##KHR)GetInstanceProcAddr(instance, "vk"#fn); \
    /* printf(""#fn"KHR %p\n", (void*)(dt->fn##KHR)); */                                    \
} while(0)
    //VK_KHR_display
    GET(GetPhysicalDeviceDisplayPropertiesKHR);
    GET(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    GET(GetDisplayPlaneSupportedDisplaysKHR);
    GET(GetDisplayModePropertiesKHR);
    GET(CreateDisplayModeKHR);
    GET(GetDisplayPlaneCapabilitiesKHR);
    GET(CreateDisplayPlaneSurfaceKHR);
    //VK_KHR_external_fence_capabilities
    GETKHR(GetPhysicalDeviceExternalFenceProperties);
    //VK_KHR_external_memory_capabilities
    GETKHR(GetPhysicalDeviceExternalBufferProperties);
    //VK_KHR_external_semaphore_capabilities
    GETKHR(GetPhysicalDeviceExternalSemaphoreProperties);
    //VK_KHR_get_physical_device_properties2
    GETKHR(GetPhysicalDeviceFeatures2);
    GETKHR(GetPhysicalDeviceProperties2);
    GETKHR(GetPhysicalDeviceFormatProperties2);
    GETKHR(GetPhysicalDeviceImageFormatProperties2);
    GETKHR(GetPhysicalDeviceQueueFamilyProperties2);
    GETKHR(GetPhysicalDeviceMemoryProperties2);
    GETKHR(GetPhysicalDeviceSparseImageFormatProperties2);
    //VK_KHR_get_surface_capabilities2
    GET(GetPhysicalDeviceSurfaceCapabilities2KHR);
    GET(GetPhysicalDeviceSurfaceFormats2KHR);
    //VK_KHR_surface
    GET(DestroySurfaceKHR);
    GET(GetPhysicalDeviceSurfaceSupportKHR);
    GET(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET(GetPhysicalDeviceSurfaceFormatsKHR);
    GET(GetPhysicalDeviceSurfacePresentModesKHR);
    //VK_EXT_debug_report
    GET(CreateDebugReportCallbackEXT);
    GET(DestroyDebugReportCallbackEXT);
    GET(DebugReportMessageEXT);
    //VK_EXT_direct_mode_display
    GET(ReleaseDisplayEXT);
    //VK_EXT_display_surface_counter
    GET(GetPhysicalDeviceSurfaceCapabilities2EXT);
    //VK_EXT_swapchain_colorspace
    //VK_EXT_validation_flags
    //VK_EXT_debug_utils
    GET(SetDebugUtilsObjectNameEXT);
    GET(SetDebugUtilsObjectTagEXT);
    GET(QueueBeginDebugUtilsLabelEXT);
    GET(QueueEndDebugUtilsLabelEXT);
    GET(QueueInsertDebugUtilsLabelEXT);
    GET(CmdBeginDebugUtilsLabelEXT);
    GET(CmdEndDebugUtilsLabelEXT);
    GET(CmdInsertDebugUtilsLabelEXT);
    GET(CreateDebugUtilsMessengerEXT);
    GET(DestroyDebugUtilsMessengerEXT);
    GET(SubmitDebugUtilsMessageEXT);
    //VK_KHR_get_display_properties2
    GET(GetPhysicalDeviceDisplayProperties2KHR);
    GET(GetPhysicalDeviceDisplayPlaneProperties2KHR);
    GET(GetDisplayModeProperties2KHR);
    GET(GetDisplayPlaneCapabilities2KHR);
    //VK_KHR_device_group_creation
    GETKHR(EnumeratePhysicalDeviceGroups);
#ifdef VK_USE_PLATFORM_XCB_KHR
    //VK_KHR_xcb_surface
    GET(CreateXcbSurfaceKHR);
    GET(GetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    //VK_KHR_xlib_surface
    GET(CreateXlibSurfaceKHR);
    GET(GetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    //VK_EXT_acquire_xlib_display
    GET(AcquireXlibDisplayEXT);
    GET(GetRandROutputDisplayEXT);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    //VK_KHR_wayland_surface
    GET(CreateWaylandSurfaceKHR);
    GET(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    //VK_KHR_android_surface
    GET(CreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    //VK_KHR_win32_surface
    GET(CreateWin32SurfaceKHR);
    GET(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif
#undef GET
#undef GETKHR
    return dt;
    }

device_dt_t* getproc_device(lua_State *L, VkDevice device, VkDeviceCreateInfo *createinfo)
    {
    device_dt_t *dt = (device_dt_t*)Malloc(L, sizeof(device_dt_t));
    (void)createinfo;

    /* Core 1.0 functions ---------------------------- */
#define GET(fn) do {                                            \
    dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn);    \
    if(!dt->fn) { luaL_error(L, "cannot find vk"#fn); return dt; }  \
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

    /* Extensions and core > 1.0 ----------------------- */
#define GET(fn) do {                                            \
    dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn);    \
    /* printf(""#fn" %p\n", (void*)(dt->fn)); */                \
} while(0)
    /* promoted extensions may lack the KHR suffix, so try also without it */
#define GETKHR(fn) do { \
    dt->fn##KHR = (PFN_vk##fn##KHR)GetDeviceProcAddr(device, "vk"#fn"KHR");             \
    if(!dt->fn##KHR) dt->fn##KHR = (PFN_vk##fn##KHR)GetDeviceProcAddr(device, "vk"#fn); \
    /* printf(""#fn"KHR %p\n", (void*)(dt->fn##KHR)); */                                \
} while(0)
    GET(GetDeviceQueue2);
    //VK_KHR_8bit_storage
    //VK_KHR_16bit_storage
    //VK_KHR_descriptor_update_template
    GETKHR(CreateDescriptorUpdateTemplate);
    GETKHR(DestroyDescriptorUpdateTemplate);
    GETKHR(UpdateDescriptorSetWithTemplate);
    GETKHR(CmdPushDescriptorSetWithTemplate);
    //VK_KHR_dedicated_allocation
    //VK_KHR_display_swapchain
    GET(CreateSharedSwapchainsKHR);
    //VK_KHR_external_fence
    //VK_KHR_external_fence_fd
    GET(ImportFenceFdKHR);
    GET(GetFenceFdKHR);
    //VK_KHR_external_memory
    //VK_KHR_external_memory_fd
    GET(GetMemoryFdKHR);
    GET(GetMemoryFdPropertiesKHR);
    //VK_KHR_external_semaphore
    //VK_KHR_external_semaphore_fd
    GET(ImportSemaphoreFdKHR);
    GET(GetSemaphoreFdKHR);
    //VK_KHR_get_memory_requirements2
    GETKHR(GetImageMemoryRequirements2);
    GETKHR(GetBufferMemoryRequirements2);
    GETKHR(GetImageSparseMemoryRequirements2);
    //VK_KHR_incremental_present
    //VK_KHR_maintenance1
    GETKHR(TrimCommandPool);
    //VK_KHR_maintenance2
    //VK_KHR_push_descriptor
    GET(CmdPushDescriptorSetKHR);
    //VK_KHR_sampler_mirror_clamp_to_edge
    //VK_KHR_shader_draw_parameters
    //VK_KHR_shared_presentable_image
    GET(GetSwapchainStatusKHR);
    //VK_KHR_storage_buffer_storage_class
    //VK_KHR_swapchain
    GET(CreateSwapchainKHR);
    GET(DestroySwapchainKHR);
    GET(GetSwapchainImagesKHR);
    GET(AcquireNextImageKHR);
    GET(QueuePresentKHR);
    //VK_KHR_variable_pointers
    //VK_KHR_relaxed_block_layout
    //VK_EXT_blend_operation_advanced
    //VK_EXT_debug_marker
    GET(DebugMarkerSetObjectTagEXT);
    GET(DebugMarkerSetObjectNameEXT);
    GET(CmdDebugMarkerBeginEXT);
    GET(CmdDebugMarkerEndEXT);
    GET(CmdDebugMarkerInsertEXT);
    //VK_EXT_depth_range_unrestricted
    //VK_EXT_discard_rectangles
    GET(CmdSetDiscardRectangleEXT);
    //VK_EXT_display_control
    GET(DisplayPowerControlEXT);
    GET(RegisterDeviceEventEXT);
    GET(RegisterDisplayEventEXT);
    GET(GetSwapchainCounterEXT);
    //VK_EXT_hdr_metadata
    GET(SetHdrMetadataEXT);
    //VK_EXT_post_depth_coverage
    //VK_EXT_sampler_filter_minmax
    //VK_EXT_shader_subgroup_ballot
    //VK_EXT_shader_subgroup_vote
    //VK_EXT_shader_stencil_export
    //VK_EXT_shader_viewport_index_layer
    //VK_EXT_global_priority
    //VK_KHR_image_format_list
    //VK_KHR_bind_memory2
    GETKHR(BindBufferMemory2);
    GETKHR(BindImageMemory2);
    //VK_EXT_sample_locations
    GET(CmdSetSampleLocationsEXT);
    GET(GetPhysicalDeviceMultisamplePropertiesEXT);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    //VK_KHR_win32_keyed_mutex
    //VK_KHR_external_fence_win32
    GET(ImportFenceWin32HandleKHR);
    GET(GetFenceWin32HandleKHR);
    //VK_KHR_external_memory_win32
    GET(GetMemoryWin32HandleKHR);
    GET(GetMemoryWin32HandlePropertiesKHR);
    //VK_KHR_external_semaphore_win32
    GET(ImportSemaphoreWin32HandleKHR);
    GET(GetSemaphoreWin32HandleKHR);
#endif
    //VK_EXT_validation_cache
    GET(CreateValidationCacheEXT);
    GET(DestroyValidationCacheEXT);
    GET(MergeValidationCachesEXT);
    GET(GetValidationCacheDataEXT);
    //VK_KHR_sampler_ycbcr_conversion
    GETKHR(CreateSamplerYcbcrConversion);
    GETKHR(DestroySamplerYcbcrConversion);
    //VK_EXT_conditional_rendering
    GET(CmdBeginConditionalRenderingEXT);
    GET(CmdEndConditionalRenderingEXT);
    //VK_KHR_draw_indirect_count
    GET(CmdDrawIndirectCountKHR);
    GET(CmdDrawIndexedIndirectCountKHR);
    //VK_KHR_maintenance3
    GETKHR(GetDescriptorSetLayoutSupport);
    //VK_KHR_create_renderpass2 
    GET(CreateRenderPass2KHR);
    GET(CmdBeginRenderPass2KHR);
    GET(CmdNextSubpass2KHR);
    GET(CmdEndRenderPass2KHR);
    //VK_KHR_device_group
    GETKHR(GetDeviceGroupPeerMemoryFeatures);
    GETKHR(CmdSetDeviceMask);
    GETKHR(CmdDispatchBase);
    GETKHR(GetDeviceGroupPresentCapabilities);
    GETKHR(GetDeviceGroupSurfacePresentModes);
    GETKHR(GetPhysicalDevicePresentRectangles);
    GETKHR(AcquireNextImage2);
    //VK_EXT_external_memory_host
    GET(GetMemoryHostPointerPropertiesEXT);
#undef GET
#undef GETKHR
    return dt;
    }


void moonvulkan_atexit_getproc(void)
    {
#if defined(LINUX)
    if(Handle) dlclose(Handle);
#elif defined(MINGW)
    if(Handle) FreeLibrary(Handle);
#endif
    }

int moonvulkan_open_getproc(lua_State *L)
    {
    Init(L);
    return 0;
    }
