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

#ifndef getprocDEFINED
#define getprocDEFINED

#define TestInstancePfn(L, ud, pfn) ((ud)->idt->pfn!=NULL)
#define TestDevicePfn(L, ud, pfn) ((ud)->ddt->pfn!=NULL)

#define CheckInstancePfn(L, ud, pfn) \
    do { if((ud)->idt->pfn==NULL) return luaL_error((L), "vk"#pfn" address not loaded"); } while(0)
#define CheckDevicePfn(L, ud, pfn) \
    do { if((ud)->ddt->pfn==NULL) return luaL_error((L), "vk"#pfn" address not loaded"); } while(0)

/* Dispatch tables */

#define F(x) PFN_vk##x x


/* Global functions */
#define global_dt_t moonvulkan_global_dt_t
typedef struct {
    F(EnumerateInstanceExtensionProperties);
    F(EnumerateInstanceLayerProperties);
    F(CreateInstance);
    F(EnumerateInstanceVersion);
} global_dt_t;


/* Instance functions */
#define instance_dt_t moonvulkan_instance_dt_t
typedef struct {
    F(DestroyInstance);
    F(EnumeratePhysicalDevices);
    F(EnumerateDeviceLayerProperties);
    F(EnumerateDeviceExtensionProperties);
    F(GetPhysicalDeviceFeatures);
    F(GetPhysicalDeviceFormatProperties);
    F(GetPhysicalDeviceImageFormatProperties);
    F(GetPhysicalDeviceProperties);
    F(GetPhysicalDeviceQueueFamilyProperties);
    F(GetPhysicalDeviceMemoryProperties);
    F(CreateDevice);
    F(GetPhysicalDeviceSparseImageFormatProperties);
    F(DestroySurfaceKHR);
    F(GetPhysicalDeviceSurfaceSupportKHR);
    F(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    F(GetPhysicalDeviceSurfaceCapabilities2KHR);
    F(GetPhysicalDeviceSurfaceFormatsKHR);
    F(GetPhysicalDeviceSurfaceFormats2KHR);
    F(GetPhysicalDeviceSurfacePresentModesKHR);
    F(GetPhysicalDeviceDisplayPropertiesKHR);
    F(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    F(GetDisplayPlaneSupportedDisplaysKHR);
    F(GetDisplayModePropertiesKHR);
    F(CreateDisplayModeKHR);
    F(GetDisplayPlaneCapabilitiesKHR);
    F(CreateDisplayPlaneSurfaceKHR);
#ifdef VK_USE_PLATFORM_XCB_KHR
    F(CreateXcbSurfaceKHR);
    F(GetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    F(CreateXlibSurfaceKHR);
    F(GetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    F(AcquireXlibDisplayEXT);
    F(GetRandROutputDisplayEXT);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    F(CreateWaylandSurfaceKHR);
    F(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    F(CreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    F(CreateWin32SurfaceKHR);
    F(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif
    F(CreateDebugReportCallbackEXT);
    F(DestroyDebugReportCallbackEXT);
    F(DebugReportMessageEXT);
    F(GetPhysicalDeviceFeatures2);
    F(GetPhysicalDeviceProperties2);
    F(GetPhysicalDeviceFormatProperties2);
    F(GetPhysicalDeviceImageFormatProperties2);
    F(GetPhysicalDeviceQueueFamilyProperties2);
    F(GetPhysicalDeviceMemoryProperties2);
    F(GetPhysicalDeviceSparseImageFormatProperties2);
    F(GetPhysicalDeviceExternalBufferProperties);
    F(GetPhysicalDeviceExternalSemaphoreProperties);
    F(GetPhysicalDeviceExternalFenceProperties);
    F(ReleaseDisplayEXT);
    F(GetPhysicalDeviceSurfaceCapabilities2EXT);
    F(SetDebugUtilsObjectNameEXT);
    F(SetDebugUtilsObjectTagEXT);
    F(QueueBeginDebugUtilsLabelEXT);
    F(QueueEndDebugUtilsLabelEXT);
    F(QueueInsertDebugUtilsLabelEXT);
    F(CmdBeginDebugUtilsLabelEXT);
    F(CmdEndDebugUtilsLabelEXT);
    F(CmdInsertDebugUtilsLabelEXT);
    F(CreateDebugUtilsMessengerEXT);
    F(DestroyDebugUtilsMessengerEXT);
    F(SubmitDebugUtilsMessageEXT);
    F(GetPhysicalDeviceDisplayProperties2KHR);
    F(GetPhysicalDeviceDisplayPlaneProperties2KHR);
    F(GetDisplayModeProperties2KHR);
    F(GetDisplayPlaneCapabilities2KHR);
    F(EnumeratePhysicalDeviceGroups);
    F(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    F(GetPhysicalDeviceMultisamplePropertiesEXT);
    F(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
    F(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR);
    F(GetPhysicalDeviceFragmentShadingRatesKHR);
    F(GetPhysicalDeviceToolPropertiesEXT);
    F(AcquireDrmDisplayEXT);
    F(GetDrmDisplayEXT);
    F(CreateHeadlessSurfaceEXT);
} instance_dt_t;


/* Device functions */
#define device_dt_t moonvulkan_device_dt_t
typedef struct {
    F(DestroyDevice);
    F(GetDeviceQueue);
    F(QueueSubmit);
    F(QueueWaitIdle);
    F(DeviceWaitIdle);
    F(AllocateMemory);
    F(FreeMemory);
    F(MapMemory);
    F(UnmapMemory);
    F(FlushMappedMemoryRanges);
    F(InvalidateMappedMemoryRanges);
    F(GetDeviceMemoryCommitment);
    F(BindBufferMemory);
    F(BindImageMemory);
    F(GetBufferMemoryRequirements);
    F(GetImageMemoryRequirements);
    F(GetImageSparseMemoryRequirements);
    F(QueueBindSparse);
    F(CreateFence);
    F(DestroyFence);
    F(ResetFences);
    F(GetFenceStatus);
    F(WaitForFences);
    F(CreateSemaphore);
    F(DestroySemaphore);
    F(CreateEvent);
    F(DestroyEvent);
    F(GetEventStatus);
    F(SetEvent);
    F(ResetEvent);
    F(CreateQueryPool);
    F(DestroyQueryPool);
    F(GetQueryPoolResults);
    F(CreateBuffer);
    F(DestroyBuffer);
    F(CreateBufferView);
    F(DestroyBufferView);
    F(CreateImage);
    F(DestroyImage);
    F(GetImageSubresourceLayout);
    F(CreateImageView);
    F(DestroyImageView);
    F(CreateShaderModule);
    F(DestroyShaderModule);
    F(CreatePipelineCache);
    F(DestroyPipelineCache);
    F(GetPipelineCacheData);
    F(MergePipelineCaches);
    F(CreateGraphicsPipelines);
    F(CreateComputePipelines);
    F(DestroyPipeline);
    F(CreatePipelineLayout);
    F(DestroyPipelineLayout);
    F(CreateSampler);
    F(DestroySampler);
    F(CreateDescriptorSetLayout);
    F(DestroyDescriptorSetLayout);
    F(CreateDescriptorPool);
    F(DestroyDescriptorPool);
    F(ResetDescriptorPool);
    F(AllocateDescriptorSets);
    F(FreeDescriptorSets);
    F(UpdateDescriptorSets);
    F(CreateFramebuffer);
    F(DestroyFramebuffer);
    F(CreateRenderPass);
    F(DestroyRenderPass);
    F(GetRenderAreaGranularity);
    F(CreateCommandPool);
    F(DestroyCommandPool);
    F(ResetCommandPool);
    F(AllocateCommandBuffers);
    F(FreeCommandBuffers);
    F(CreateSwapchainKHR);
    F(DestroySwapchainKHR);
    F(GetSwapchainImagesKHR);
    F(GetSwapchainStatusKHR);
    F(AcquireNextImageKHR);
    F(QueuePresentKHR);
    F(CreateSharedSwapchainsKHR);
    F(TrimCommandPool);
    F(DebugMarkerSetObjectTagEXT);
    F(DebugMarkerSetObjectNameEXT);
    F(DisplayPowerControlEXT);
    F(RegisterDeviceEventEXT);
    F(RegisterDisplayEventEXT);
    F(GetSwapchainCounterEXT);
    F(BeginCommandBuffer);
    F(EndCommandBuffer);
    F(ResetCommandBuffer);
    F(CmdBindPipeline);
    F(CmdSetViewport);
    F(CmdSetScissor);
    F(CmdSetLineWidth);
    F(CmdSetDepthBias);
    F(CmdSetBlendConstants);
    F(CmdSetDepthBounds);
    F(CmdSetStencilCompareMask);
    F(CmdSetStencilWriteMask);
    F(CmdSetStencilReference);
    F(CmdBindDescriptorSets);
    F(CmdBindIndexBuffer);
    F(CmdBindVertexBuffers);
    F(CmdDraw);
    F(CmdDrawIndexed);
    F(CmdDrawIndirect);
    F(CmdDrawIndexedIndirect);
    F(CmdDispatch);
    F(CmdDispatchIndirect);
    F(CmdCopyBuffer);
    F(CmdCopyImage);
    F(CmdBlitImage);
    F(CmdCopyBufferToImage);
    F(CmdCopyImageToBuffer);
    F(CmdUpdateBuffer);
    F(CmdFillBuffer);
    F(CmdClearColorImage);
    F(CmdClearDepthStencilImage);
    F(CmdClearAttachments);
    F(CmdResolveImage);
    F(CmdSetEvent);
    F(CmdResetEvent);
    F(CmdWaitEvents);
    F(CmdPipelineBarrier);
    F(CmdBeginQuery);
    F(CmdEndQuery);
    F(CmdResetQueryPool);
    F(CmdWriteTimestamp);
    F(CmdCopyQueryPoolResults);
    F(CmdPushConstants);
    F(CmdBeginRenderPass);
    F(CmdNextSubpass);
    F(CmdEndRenderPass);
    F(CmdExecuteCommands);
    F(CmdDebugMarkerBeginEXT);
    F(CmdDebugMarkerEndEXT);
    F(CmdDebugMarkerInsertEXT);
    F(CmdPushDescriptorSetKHR);
    F(GetDeviceQueue2);
    F(CreateDescriptorUpdateTemplate);
    F(DestroyDescriptorUpdateTemplate);
    F(UpdateDescriptorSetWithTemplate);
    F(CmdPushDescriptorSetWithTemplateKHR);
    F(GetMemoryFdKHR);
    F(GetMemoryFdPropertiesKHR);
    F(ImportSemaphoreFdKHR);
    F(GetSemaphoreFdKHR);
    F(ImportFenceFdKHR);
    F(GetFenceFdKHR);
    F(GetImageMemoryRequirements2);
    F(GetBufferMemoryRequirements2);
    F(GetImageSparseMemoryRequirements2);
    F(CmdSetDiscardRectangleEXT);
    F(SetHdrMetadataEXT);
    F(BindBufferMemory2);
    F(BindImageMemory2);
    F(CmdSetSampleLocationsEXT);
    F(CreateValidationCacheEXT);
    F(DestroyValidationCacheEXT);
    F(MergeValidationCachesEXT);
    F(GetValidationCacheDataEXT);
    F(CreateSamplerYcbcrConversion);
    F(DestroySamplerYcbcrConversion);
    F(CmdBeginConditionalRenderingEXT);
    F(CmdEndConditionalRenderingEXT);
    F(GetDescriptorSetLayoutSupport);
    F(GetDeviceGroupPeerMemoryFeatures);
    F(CmdSetDeviceMask);
    F(CmdDispatchBase);
    F(GetDeviceGroupPresentCapabilitiesKHR);
    F(GetDeviceGroupSurfacePresentModesKHR);
    F(GetPhysicalDevicePresentRectanglesKHR);
    F(AcquireNextImage2KHR);
    F(GetMemoryHostPointerPropertiesEXT);
    F(CmdBindTransformFeedbackBuffersEXT);
    F(CmdBeginTransformFeedbackEXT);
    F(CmdEndTransformFeedbackEXT);
    F(CmdBeginQueryIndexedEXT);
    F(CmdEndQueryIndexedEXT);
    F(CmdDrawIndirectByteCountEXT);
    F(GetCalibratedTimestampsEXT);
    F(CmdDrawIndirectCount);
    F(CmdDrawIndexedIndirectCount);
    F(CreateRenderPass2);
    F(CmdBeginRenderPass2);
    F(CmdNextSubpass2);
    F(CmdEndRenderPass2);
    F(GetSemaphoreCounterValue);
    F(WaitSemaphores);
    F(SignalSemaphore);
    F(GetBufferDeviceAddress);
    F(GetBufferOpaqueCaptureAddress);
    F(GetDeviceMemoryOpaqueCaptureAddress);
    F(ResetQueryPool);
    F(AcquireProfilingLockKHR);
    F(ReleaseProfilingLockKHR);
    F(CmdSetFragmentShadingRateKHR);
    F(WaitForPresentKHR);
    F(CreateDeferredOperationKHR);
    F(DestroyDeferredOperationKHR);
    F(GetDeferredOperationMaxConcurrencyKHR);
    F(GetDeferredOperationResultKHR);
    F(DeferredOperationJoinKHR);
    F(GetPipelineExecutablePropertiesKHR);
    F(GetPipelineExecutableStatisticsKHR);
    F(GetPipelineExecutableInternalRepresentationsKHR);
    F(CmdSetEvent2KHR);
    F(CmdResetEvent2KHR);
    F(CmdWaitEvents2KHR);
    F(CmdPipelineBarrier2KHR);
    F(CmdWriteTimestamp2KHR);
    F(QueueSubmit2KHR);
    F(CmdCopyBuffer2KHR);
    F(CmdCopyImage2KHR);
    F(CmdCopyBufferToImage2KHR);
    F(CmdCopyImageToBuffer2KHR);
    F(CmdBlitImage2KHR);
    F(CmdResolveImage2KHR);
    F(GetRayTracingShaderGroupHandlesKHR);
    F(CreateAccelerationStructureKHR);
    F(DestroyAccelerationStructureKHR);
    F(CmdBuildAccelerationStructuresKHR);
    F(CmdBuildAccelerationStructuresIndirectKHR);
    F(BuildAccelerationStructuresKHR);
    F(CopyAccelerationStructureKHR);
    F(CopyAccelerationStructureToMemoryKHR);
    F(CopyMemoryToAccelerationStructureKHR);
    F(WriteAccelerationStructuresPropertiesKHR);
    F(CmdCopyAccelerationStructureKHR);
    F(CmdCopyAccelerationStructureToMemoryKHR);
    F(CmdCopyMemoryToAccelerationStructureKHR);
    F(GetAccelerationStructureDeviceAddressKHR);
    F(CmdWriteAccelerationStructuresPropertiesKHR);
    F(GetDeviceAccelerationStructureCompatibilityKHR);
    F(GetAccelerationStructureBuildSizesKHR);
    F(CmdTraceRaysKHR);
    F(CreateRayTracingPipelinesKHR);
    F(GetRayTracingCaptureReplayShaderGroupHandlesKHR);
    F(CmdTraceRaysIndirectKHR);
    F(GetRayTracingShaderGroupStackSizeKHR);
    F(CmdSetRayTracingPipelineStackSizeKHR);
    F(CmdSetLineStippleEXT);
    F(CmdSetCullModeEXT);
    F(CmdSetFrontFaceEXT);
    F(CmdSetPrimitiveTopologyEXT);
    F(CmdSetViewportWithCountEXT);
    F(CmdSetScissorWithCountEXT);
    F(CmdBindVertexBuffers2EXT);
    F(CmdSetDepthTestEnableEXT);
    F(CmdSetDepthWriteEnableEXT);
    F(CmdSetDepthCompareOpEXT);
    F(CmdSetDepthBoundsTestEnableEXT);
    F(CmdSetStencilTestEnableEXT);
    F(CmdSetStencilOpEXT);
    F(CreatePrivateDataSlotEXT);
    F(DestroyPrivateDataSlotEXT);
    F(SetPrivateDataEXT);
    F(GetPrivateDataEXT);
    F(CmdSetVertexInputEXT);
    F(CmdSetPatchControlPointsEXT);
    F(CmdSetRasterizerDiscardEnableEXT);
    F(CmdSetDepthBiasEnableEXT);
    F(CmdSetLogicOpEXT);
    F(CmdSetPrimitiveRestartEnableEXT);
    F(CmdSetColorWriteEnableEXT);
    F(CmdDrawMultiEXT);
    F(CmdDrawMultiIndexedEXT);
    F(CmdBeginRenderingKHR);
    F(CmdEndRenderingKHR);
    F(GetDeviceBufferMemoryRequirementsKHR);
    F(GetDeviceImageMemoryRequirementsKHR);
    F(GetDeviceImageSparseMemoryRequirementsKHR);
    F(SetDeviceMemoryPriorityEXT);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    F(GetMemoryWin32HandleKHR);
    F(GetMemoryWin32HandlePropertiesKHR);
    F(ImportSemaphoreWin32HandleKHR);
    F(GetSemaphoreWin32HandleKHR);
    F(ImportFenceWin32HandleKHR);
    F(GetFenceWin32HandleKHR);
#endif
} device_dt_t;

#undef F

#define vk moonvulkan_vk
extern global_dt_t vk;

#define getproc_instance moonvulkan_getproc_instance
instance_dt_t * getproc_instance(lua_State *L, VkInstance instance, VkInstanceCreateInfo *createinfo);
#define getproc_device moonvulkan_getproc_device
device_dt_t* getproc_device(lua_State *L, VkDevice device, VkDeviceCreateInfo *createinfo);

#endif /* getprocDEFINED */
