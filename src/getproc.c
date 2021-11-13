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
#define PGET(fn) do { /* promoted extensions: try also with the suffixes */         \
    dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn);                    \
    if(!dt->fn) dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn"KHR");   \
    if(!dt->fn) dt->fn = (PFN_vk##fn)GetInstanceProcAddr(instance, "vk"#fn"EXT");   \
    /* printf(""#fn" %p\n", (void*)(dt->fn)); */                                    \
} while(0)
    GET(GetPhysicalDeviceDisplayPropertiesKHR);
    GET(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    GET(GetDisplayPlaneSupportedDisplaysKHR);
    GET(GetDisplayModePropertiesKHR);
    GET(CreateDisplayModeKHR);
    GET(GetDisplayPlaneCapabilitiesKHR);
    GET(CreateDisplayPlaneSurfaceKHR);
    PGET(GetPhysicalDeviceExternalFenceProperties);
    PGET(GetPhysicalDeviceExternalBufferProperties);
    PGET(GetPhysicalDeviceExternalSemaphoreProperties);
    PGET(GetPhysicalDeviceFeatures2);
    PGET(GetPhysicalDeviceProperties2);
    PGET(GetPhysicalDeviceFormatProperties2);
    PGET(GetPhysicalDeviceImageFormatProperties2);
    PGET(GetPhysicalDeviceQueueFamilyProperties2);
    PGET(GetPhysicalDeviceMemoryProperties2);
    PGET(GetPhysicalDeviceSparseImageFormatProperties2);
    GET(GetPhysicalDeviceSurfaceCapabilities2KHR);
    GET(GetPhysicalDeviceSurfaceFormats2KHR);
    GET(DestroySurfaceKHR);
    GET(GetPhysicalDeviceSurfaceSupportKHR);
    GET(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET(GetPhysicalDeviceSurfaceFormatsKHR);
    GET(GetPhysicalDeviceSurfacePresentModesKHR);
    GET(CreateDebugReportCallbackEXT);
    GET(DestroyDebugReportCallbackEXT);
    GET(DebugReportMessageEXT);
    GET(ReleaseDisplayEXT);
    GET(GetPhysicalDeviceSurfaceCapabilities2EXT);
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
    GET(GetPhysicalDeviceDisplayProperties2KHR);
    GET(GetPhysicalDeviceDisplayPlaneProperties2KHR);
    GET(GetDisplayModeProperties2KHR);
    GET(GetDisplayPlaneCapabilities2KHR);
    PGET(EnumeratePhysicalDeviceGroups);
    GET(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET(GetPhysicalDeviceMultisamplePropertiesEXT);
    GET(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
    GET(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR);
    GET(GetPhysicalDeviceFragmentShadingRatesKHR);
    GET(GetPhysicalDeviceToolPropertiesEXT);
    GET(AcquireDrmDisplayEXT);
    GET(GetDrmDisplayEXT);
#ifdef VK_USE_PLATFORM_XCB_KHR
    GET(CreateXcbSurfaceKHR);
    GET(GetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GET(CreateXlibSurfaceKHR);
    GET(GetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    GET(AcquireXlibDisplayEXT);
    GET(GetRandROutputDisplayEXT);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    GET(CreateWaylandSurfaceKHR);
    GET(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    GET(CreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GET(CreateWin32SurfaceKHR);
    GET(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif
#undef GET
#undef PGET
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
#define PGET(fn) do { /* promoted extensions: try also with the suffixes */     \
    dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn);                    \
    if(!dt->fn) dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn"KHR");   \
    if(!dt->fn) dt->fn = (PFN_vk##fn)GetDeviceProcAddr(device, "vk"#fn"EXT");   \
    /* printf(""#fn" %p\n", (void*)(dt->fn)); */                                \
} while(0)
    GET(GetDeviceQueue2);
    PGET(CreateDescriptorUpdateTemplate);
    PGET(DestroyDescriptorUpdateTemplate);
    PGET(UpdateDescriptorSetWithTemplate);
    GET(CmdPushDescriptorSetWithTemplateKHR);
    GET(CreateSharedSwapchainsKHR);
    GET(ImportFenceFdKHR);
    GET(GetFenceFdKHR);
    GET(GetMemoryFdKHR);
    GET(GetMemoryFdPropertiesKHR);
    GET(ImportSemaphoreFdKHR);
    GET(GetSemaphoreFdKHR);
    PGET(GetImageMemoryRequirements2);
    PGET(GetBufferMemoryRequirements2);
    PGET(GetImageSparseMemoryRequirements2);
    PGET(TrimCommandPool);
    GET(CmdPushDescriptorSetKHR);
    GET(GetSwapchainStatusKHR);
    GET(CreateSwapchainKHR);
    GET(DestroySwapchainKHR);
    GET(GetSwapchainImagesKHR);
    GET(AcquireNextImageKHR);
    GET(QueuePresentKHR);
    GET(DebugMarkerSetObjectTagEXT);
    GET(DebugMarkerSetObjectNameEXT);
    GET(CmdDebugMarkerBeginEXT);
    GET(CmdDebugMarkerEndEXT);
    GET(CmdDebugMarkerInsertEXT);
    GET(CmdSetDiscardRectangleEXT);
    GET(DisplayPowerControlEXT);
    GET(RegisterDeviceEventEXT);
    GET(RegisterDisplayEventEXT);
    GET(GetSwapchainCounterEXT);
    GET(SetHdrMetadataEXT);
    PGET(BindBufferMemory2);
    PGET(BindImageMemory2);
    GET(CmdSetSampleLocationsEXT);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GET(ImportFenceWin32HandleKHR);
    GET(GetFenceWin32HandleKHR);
    GET(GetMemoryWin32HandleKHR);
    GET(GetMemoryWin32HandlePropertiesKHR);
    GET(ImportSemaphoreWin32HandleKHR);
    GET(GetSemaphoreWin32HandleKHR);
#endif
    GET(CreateValidationCacheEXT);
    GET(DestroyValidationCacheEXT);
    GET(MergeValidationCachesEXT);
    GET(GetValidationCacheDataEXT);
    PGET(CreateSamplerYcbcrConversion);
    PGET(DestroySamplerYcbcrConversion);
    GET(CmdBeginConditionalRenderingEXT);
    GET(CmdEndConditionalRenderingEXT);
    PGET(CmdDrawIndirectCount);
    PGET(CmdDrawIndexedIndirectCount);
    PGET(GetDescriptorSetLayoutSupport);
    PGET(CreateRenderPass2);
    PGET(CmdBeginRenderPass2);
    PGET(CmdNextSubpass2);
    PGET(CmdEndRenderPass2);
    PGET(GetDeviceGroupPeerMemoryFeatures);
    PGET(CmdSetDeviceMask);
    PGET(CmdDispatchBase);
    GET(GetDeviceGroupPresentCapabilitiesKHR);
    GET(GetDeviceGroupSurfacePresentModesKHR);
    GET(GetPhysicalDevicePresentRectanglesKHR);
    GET(AcquireNextImage2KHR);
    GET(GetMemoryHostPointerPropertiesEXT);
    GET(CmdBindTransformFeedbackBuffersEXT);
    GET(CmdBeginTransformFeedbackEXT);
    GET(CmdEndTransformFeedbackEXT);
    GET(CmdBeginQueryIndexedEXT);
    GET(CmdEndQueryIndexedEXT);
    GET(CmdDrawIndirectByteCountEXT);
    GET(GetCalibratedTimestampsEXT);
    PGET(GetSemaphoreCounterValue);
    PGET(WaitSemaphores);
    PGET(SignalSemaphore);
    PGET(GetBufferDeviceAddress);
    PGET(GetBufferOpaqueCaptureAddress);
    PGET(GetDeviceMemoryOpaqueCaptureAddress);
    PGET(ResetQueryPool);
    GET(AcquireProfilingLockKHR);
    GET(ReleaseProfilingLockKHR);
    GET(CmdSetFragmentShadingRateKHR);
    GET(WaitForPresentKHR);
    GET(CreateDeferredOperationKHR);
    GET(DestroyDeferredOperationKHR);
    GET(GetDeferredOperationMaxConcurrencyKHR);
    GET(GetDeferredOperationResultKHR);
    GET(DeferredOperationJoinKHR);
    GET(GetPipelineExecutablePropertiesKHR);
    GET(GetPipelineExecutableStatisticsKHR);
    GET(GetPipelineExecutableInternalRepresentationsKHR);
    GET(CmdSetEvent2KHR);
    GET(CmdResetEvent2KHR);
    GET(CmdWaitEvents2KHR);
    GET(CmdPipelineBarrier2KHR);
    GET(CmdWriteTimestamp2KHR);
    GET(QueueSubmit2KHR);
    GET(CmdCopyBuffer2KHR);
    GET(CmdCopyImage2KHR);
    GET(CmdCopyBufferToImage2KHR);
    GET(CmdCopyImageToBuffer2KHR);
    GET(CmdBlitImage2KHR);
    GET(CmdResolveImage2KHR);
    GET(GetRayTracingShaderGroupHandlesKHR);
    GET(CreateAccelerationStructureKHR);
    GET(DestroyAccelerationStructureKHR);
    GET(CmdBuildAccelerationStructuresKHR);
    GET(CmdBuildAccelerationStructuresIndirectKHR);
    GET(BuildAccelerationStructuresKHR);
    GET(CopyAccelerationStructureKHR);
    GET(CopyAccelerationStructureToMemoryKHR);
    GET(CopyMemoryToAccelerationStructureKHR);
    GET(WriteAccelerationStructuresPropertiesKHR);
    GET(CmdCopyAccelerationStructureKHR);
    GET(CmdCopyAccelerationStructureToMemoryKHR);
    GET(CmdCopyMemoryToAccelerationStructureKHR);
    GET(GetAccelerationStructureDeviceAddressKHR);
    GET(CmdWriteAccelerationStructuresPropertiesKHR);
    GET(GetDeviceAccelerationStructureCompatibilityKHR);
    GET(GetAccelerationStructureBuildSizesKHR);
    GET(CmdTraceRaysKHR);
    GET(CreateRayTracingPipelinesKHR);
    GET(GetRayTracingCaptureReplayShaderGroupHandlesKHR);
    GET(CmdTraceRaysIndirectKHR);
    GET(GetRayTracingShaderGroupStackSizeKHR);
    GET(CmdSetRayTracingPipelineStackSizeKHR);
    GET(CmdSetLineStippleEXT);
    GET(CmdSetCullModeEXT);
    GET(CmdSetFrontFaceEXT);
    GET(CmdSetPrimitiveTopologyEXT);
    GET(CmdSetViewportWithCountEXT);
    GET(CmdSetScissorWithCountEXT);
    GET(CmdBindVertexBuffers2EXT);
    GET(CmdSetDepthTestEnableEXT);
    GET(CmdSetDepthWriteEnableEXT);
    GET(CmdSetDepthCompareOpEXT);
    GET(CmdSetDepthBoundsTestEnableEXT);
    GET(CmdSetStencilTestEnableEXT);
    GET(CmdSetStencilOpEXT);
    GET(CreatePrivateDataSlotEXT);
    GET(DestroyPrivateDataSlotEXT);
    GET(SetPrivateDataEXT);
    GET(GetPrivateDataEXT);
    GET(CmdSetVertexInputEXT);
    GET(CmdSetPatchControlPointsEXT);
    GET(CmdSetRasterizerDiscardEnableEXT);
    GET(CmdSetDepthBiasEnableEXT);
    GET(CmdSetLogicOpEXT);
    GET(CmdSetPrimitiveRestartEnableEXT);
    GET(CmdSetColorWriteEnableEXT);
    GET(CmdDrawMultiEXT);
    GET(CmdDrawMultiIndexedEXT);

#undef GET
#undef PGET
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
