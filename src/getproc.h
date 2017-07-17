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
//  F(DebugReportCallbackEXT);
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
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    F(CreateWaylandSurfaceKHR);
    F(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    F(CreateMirSurfaceKHR);
    F(GetPhysicalDeviceMirPresentationSupportKHR);
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
    F(GetPhysicalDeviceFeatures2KHR);
    F(GetPhysicalDeviceProperties2KHR);
    F(GetPhysicalDeviceFormatProperties2KHR);
    F(GetPhysicalDeviceImageFormatProperties2KHR);
    F(GetPhysicalDeviceQueueFamilyProperties2KHR);
    F(GetPhysicalDeviceMemoryProperties2KHR);
    F(GetPhysicalDeviceSparseImageFormatProperties2KHR);
    F(GetPhysicalDeviceExternalBufferPropertiesKHR);
    F(GetPhysicalDeviceExternalSemaphorePropertiesKHR);
    F(GetPhysicalDeviceExternalFencePropertiesKHR);
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
    F(TrimCommandPoolKHR);
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
    F(CreateDescriptorUpdateTemplateKHR);
    F(DestroyDescriptorUpdateTemplateKHR);
    F(UpdateDescriptorSetWithTemplateKHR);
    F(CmdPushDescriptorSetWithTemplateKHR);
    F(GetMemoryFdKHR);
    F(GetMemoryFdPropertiesKHR);
    F(ImportSemaphoreFdKHR);
    F(GetSemaphoreFdKHR);
    F(ImportFenceFdKHR);
    F(GetFenceFdKHR);
    F(GetImageMemoryRequirements2KHR);
    F(GetBufferMemoryRequirements2KHR);
    F(GetImageSparseMemoryRequirements2KHR);
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


