#!/usr/bin/env lua
-- The MIT License (MIT)
--
-- Copyright (c) 2018 Stefano Trettel
--
-- Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

-------------------------------------------------------------------------------
--
-- Generates the contents of zcheck.h and prints them to stdout.
--
-- Usage:   $ ./zcheck.lua > zcheck.h
--          and then manually correct any non-standard declaration.
--
-- Add any new struct to the TYPED or the UNTYPED list below
-- and add any additional declaration in the HEADER section
--

local HEADER = [[
#ifndef zcheckDEFINED
#define zcheckDEFINED

/****** DO NOT EDIT THIS FILE (automatically generated by zcheck.lua) ******/

Corrections for non-standard declarations: @@
VkDeviceCreateInfo* zcheckVkDeviceCreateInfo(lua_State *L, int arg, int *err, ud_t *ud); //non-standard
VkPresentInfoKHR* zcheckVkPresentInfoKHR(lua_State *L, int arg, int *err, int results); //non-standard
int zpushVkQueueFamilyProperties(lua_State *L, const VkQueueFamilyProperties *p, uint32_t index); //non-standard
int zpushVkQueueFamilyProperties2KHR(lua_State *L, const VkQueueFamilyProperties2KHR *p, uint32_t index); //non-standard
int zpushVkPhysicalDeviceGroupPropertiesKHR(lua_State *L, const VkPhysicalDeviceGroupPropertiesKHR *p, VkInstance instance); // non-standard

#define znew moonvulkan_znew
void* znew(lua_State *L, VkStructureType sType /* or -1 */, size_t sz, int *err);
#define znewarray moonvulkan_znewarray
void* znewarray(lua_State *L, VkStructureType sType /* or -1 */, size_t sz, uint32_t count, int *err);

/* Untyped structs only: */
#define zfree_untyped moonvulkan_zfree_untyped
void zfree_untyped(lua_State *L, const void *p, int base, void (*clearfunc)(lua_State *L, const void *p));
#define zfreearray_untyped moonvulkan_zfreearray_untyped
void zfreearray_untyped(lua_State *L, const void *p, size_t sz, uint32_t count, int base, void (*clearfunc)(lua_State *L, const void *p));

/* Typed structs only: */
#define zfree moonvulkan_zfree
void zfree(lua_State *L, const void *p, int base);
#define zfreearray moonvulkan_zfreearray
void zfreearray(lua_State *L, const void *p, size_t sz, uint32_t count, int base);
]]

local TRAILER = [[
#endif /* zcheckDEFINED */
]]

-- List of untyped structs:
local UNTYPED = { -- set the second element to true if the type has a zclear function
-- { "Xxx", has_clear_func },
   { "VkViewport" },
   { "VkOffset2D" },
   { "VkOffset3D" },
   { "VkExtent2D" },
   { "VkExtent3D" },
   { "VkRect2D" },
   { "VkXYColorEXT" },
   { "VkComponentMapping" },
   { "VkSampleLocationEXT" },
   { "VkImageSubresourceRange" },
   { "VkImageSubresourceRange" },
   { "VkExtensionProperties" },
   { "VkLayerProperties" },
   { "VkClearColorValue" },
   { "VkClearValue" },
   { "VkClearAttachment" },
   { "VkClearRect" },
   { "VkImageSubresourceLayers" },
   { "VkImageCopy" },
   { "VkImageBlit" },
   { "VkBufferImageCopy" },
   { "VkImageResolve" },
   { "VkBufferCopy" },
   { "VkSparseMemoryBind" },
   { "VkImageSubresource" },
   { "VkSparseImageMemoryBind" },
   { "VkSparseBufferMemoryBindInfo", true },
   { "VkSparseImageOpaqueMemoryBindInfo", true },
   { "VkSparseImageMemoryBindInfo", true },
   { "VkSubresourceLayout" },
   { "VkSpecializationMapEntry" },
   { "VkSpecializationInfo", true },
   { "VkVertexInputBindingDescription" },
   { "VkVertexInputAttributeDescription" },
   { "VkStencilOpState" },
   { "VkPipelineColorBlendAttachmentState" },
   { "VkRectLayerKHR" },
   { "VkPresentRegionKHR", true },
   { "VkDescriptorImageInfo" },
   { "VkDescriptorBufferInfo" },
   { "VkDisplayModeParametersKHR" },
   { "VkDisplayModePropertiesKHR" },
   { "VkDisplayPlaneCapabilitiesKHR" },
   { "VkDescriptorUpdateTemplateEntry" },
   { "VkAttachmentDescription" },
   { "VkSubpassDependency" },
   { "VkAttachmentReference" },
   { "VkDescriptorPoolSize" },
   { "VkPushConstantRange" },
   { "VkSubpassDescription", true },
   { "VkPhysicalDeviceFeatures" },
   { "VkPhysicalDeviceLimits" },
   { "VkPhysicalDeviceSparseProperties" },
   { "VkPhysicalDeviceProperties" },
   { "VkInputAttachmentAspectReference" },
   { "VkAttachmentSampleLocationsEXT", true },
   { "VkSubpassSampleLocationsEXT", true },
   { "VkFormatProperties" },
   { "VkImageFormatProperties" },
   { "VkSparseImageFormatProperties" },
   { "VkExternalMemoryProperties" },
   { "VkPhysicalDeviceMemoryProperties" },
   { "VkMemoryRequirements" },
   { "VkSparseImageMemoryRequirements" },
   { "VkSurfaceCapabilitiesKHR" }, 
   { "VkSurfaceFormatKHR" },
   { "VkQueueFamilyProperties" },
   { "VkDisplayPropertiesKHR" },
   { "VkDisplayPlanePropertiesKHR" },
   { "VkDescriptorSetLayoutBinding", true },
   { "VkVertexInputBindingDivisorDescriptionEXT" },
   { "VkDrmFormatModifierPropertiesEXT" },
   { "VkPipelineCreationFeedbackEXT" },
   { "VkAccelerationStructureBuildRangeInfoKHR" },
   { "VkStridedDeviceAddressRegionKHR" },
   { "VkMultiDrawInfoEXT" },
   { "VkMultiDrawIndexedInfoEXT" },
   { "VkPerformanceCounterResultKHR" },
   { "VkPipelineExecutableStatisticValueKHR" },
   { "VkDeviceOrHostAddressKHR" },
   { "VkAccelerationStructureGeometryDataKHR" },
}

local UNTYPED_WIN32 = {
}

-- List of typed structs:
local TYPED = {
-- { "Xxx", "XXX" },
   { "VkInstanceCreateInfo", "INSTANCE_CREATE_INFO" },
   { "VkApplicationInfo", "APPLICATION_INFO" },
   { "VkValidationFlagsEXT", "VALIDATION_FLAGS_EXT" },
   { "VkDeviceCreateInfo", "DEVICE_CREATE_INFO" },
   { "VkDeviceQueueCreateInfo", "DEVICE_QUEUE_CREATE_INFO" },
   { "VkCommandPoolCreateInfo", "COMMAND_POOL_CREATE_INFO" },
   { "VkCommandBufferAllocateInfo", "COMMAND_BUFFER_ALLOCATE_INFO" },
   { "VkCommandBufferInheritanceInfo", "COMMAND_BUFFER_INHERITANCE_INFO" },
   { "VkCommandBufferInheritanceConditionalRenderingInfoEXT", "COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT" },
   { "VkExternalMemoryBufferCreateInfo", "EXTERNAL_MEMORY_BUFFER_CREATE_INFO" },
   { "VkDeviceQueueGlobalPriorityCreateInfoEXT", "DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT" },
   { "VkCommandBufferBeginInfo", "COMMAND_BUFFER_BEGIN_INFO" },
   { "VkBufferCreateInfo", "BUFFER_CREATE_INFO" },
   { "VkBufferViewCreateInfo", "BUFFER_VIEW_CREATE_INFO" },
   { "VkPhysicalDeviceFeatures2", "PHYSICAL_DEVICE_FEATURES_2" },
   { "VkPhysicalDeviceProperties2", "PHYSICAL_DEVICE_PROPERTIES_2" },
   { "VkFormatProperties2", "FORMAT_PROPERTIES_2" },
   { "VkImageFormatProperties2" , "IMAGE_FORMAT_PROPERTIES_2" },
   { "VkSparseImageFormatProperties2", "SPARSE_IMAGE_FORMAT_PROPERTIES_2" },
   { "VkPhysicalDeviceMemoryProperties2", "PHYSICAL_DEVICE_MEMORY_PROPERTIES_2"},
   { "VkMemoryRequirements2", "MEMORY_REQUIREMENTS_2" },
   { "VkSparseImageMemoryRequirements2", "SPARSE_IMAGE_MEMORY_REQUIREMENTS_2" },
   { "VkSurfaceCapabilities2KHR", "SURFACE_CAPABILITIES_2_KHR" }, 
   { "VkSurfaceFormat2KHR", "SURFACE_FORMAT_2_KHR" },
   { "VkQueueFamilyProperties2KHR", "QUEUE_FAMILY_PROPERTIES_2_KHR" },
   { "VkImageFormatListCreateInfo", "IMAGE_FORMAT_LIST_CREATE_INFO" },
   { "VkExternalMemoryImageCreateInfoKHR", "EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR" },
   { "VkImageCreateInfo", "IMAGE_CREATE_INFO" },
   { "VkImageViewUsageCreateInfoKHR", "IMAGE_VIEW_USAGE_CREATE_INFO_KHR" },
   { "VkImageViewCreateInfo", "IMAGE_VIEW_CREATE_INFO" },
   { "VkDescriptorPoolCreateInfo", "DESCRIPTOR_POOL_CREATE_INFO" },
   { "VkDescriptorSetAllocateInfo", "DESCRIPTOR_SET_ALLOCATE_INFO" },
   { "VkDescriptorSetLayoutCreateInfo", "DESCRIPTOR_SET_LAYOUT_CREATE_INFO" },
   { "VkPipelineLayoutCreateInfo", "PIPELINE_LAYOUT_CREATE_INFO" },
   { "VkQueryPoolCreateInfo", "QUERY_POOL_CREATE_INFO" },
   { "VkRenderPassInputAttachmentAspectCreateInfoKHR", "RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO_KHR" },
   { "VkRenderPassCreateInfo", "RENDER_PASS_CREATE_INFO" },
   { "VkFramebufferCreateInfo", "FRAMEBUFFER_CREATE_INFO" },
   { "VkShaderModuleValidationCacheCreateInfoEXT", "SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT" },
   { "VkShaderModuleCreateInfo", "SHADER_MODULE_CREATE_INFO" },
   { "VkSwapchainCounterCreateInfoEXT", "SWAPCHAIN_COUNTER_CREATE_INFO_EXT" },
   { "VkSwapchainCreateInfoKHR", "SWAPCHAIN_CREATE_INFO_KHR" },
   { "VkPipelineCacheCreateInfo", "PIPELINE_CACHE_CREATE_INFO" },
   { "VkValidationCacheCreateInfoEXT", "VALIDATION_CACHE_CREATE_INFO_EXT" },
   { "VkSamplerReductionModeCreateInfo", "SAMPLER_REDUCTION_MODE_CREATE_INFO" },
   { "VkSamplerYcbcrConversionInfoKHR", "SAMPLER_YCBCR_CONVERSION_INFO_KHR" },
   { "VkSamplerCreateInfo", "SAMPLER_CREATE_INFO" },
   { "VkSamplerYcbcrConversionCreateInfoKHR", "SAMPLER_YCBCR_CONVERSION_CREATE_INFO_KHR" },
   { "VkEventCreateInfo", "EVENT_CREATE_INFO" },
   { "VkExportFenceCreateInfoKHR", "EXPORT_FENCE_CREATE_INFO_KHR" },
   { "VkFenceCreateInfo", "FENCE_CREATE_INFO" },
   { "VkExportSemaphoreCreateInfoKHR", "EXPORT_SEMAPHORE_CREATE_INFO_KHR" },
   { "VkSemaphoreCreateInfo", "SEMAPHORE_CREATE_INFO" },
   { "VkDisplaySurfaceCreateInfoKHR", "DISPLAY_SURFACE_CREATE_INFO_KHR" },
   { "VkDescriptorUpdateTemplateCreateInfoKHR", "DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_KHR" },
   { "VkDebugUtilsMessengerCreateInfoEXT", "DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT" },
   { "VkMemoryBarrier", "MEMORY_BARRIER" },
   { "VkBufferMemoryBarrier", "BUFFER_MEMORY_BARRIER" },
   { "VkImageMemoryBarrier", "IMAGE_MEMORY_BARRIER" },
   { "VkBufferMemoryRequirementsInfo2KHR", "BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR" },
   { "VkImagePlaneMemoryRequirementsInfoKHR", "IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO_KHR" },
   { "VkImageMemoryRequirementsInfo2KHR", "IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR" },
   { "VkImageSparseMemoryRequirementsInfo2KHR", "IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2_KHR" },
   { "VkDeviceQueueInfo2", "DEVICE_QUEUE_INFO_2" },
   { "VkMemoryAllocateInfo", "MEMORY_ALLOCATE_INFO" },
   { "VkMemoryDedicatedAllocateInfoKHR", "MEMORY_DEDICATED_ALLOCATE_INFO_KHR" },
   { "VkExportMemoryAllocateInfoKHR", "EXPORT_MEMORY_ALLOCATE_INFO_KHR" },
   { "VkImportMemoryFdInfoKHR", "IMPORT_MEMORY_FD_INFO_KHR" },
   { "VkMappedMemoryRange", "MAPPED_MEMORY_RANGE" },
   { "VkHdrMetadataEXT", "HDR_METADATA_EXT" },
   { "VkSubmitInfo", "SUBMIT_INFO" },
   { "VkPresentRegionsKHR", "PRESENT_REGIONS_KHR" },
   { "VkDisplayPresentInfoKHR", "DISPLAY_PRESENT_INFO_KHR" },
   { "VkPresentInfoKHR", "PRESENT_INFO_KHR" },
   { "VkBindSparseInfo", "BIND_SPARSE_INFO" },
   { "VkBindBufferMemoryInfo", "BIND_BUFFER_MEMORY_INFO" },
   { "VkBindImagePlaneMemoryInfoKHR", "BIND_IMAGE_PLANE_MEMORY_INFO_KHR" },
   { "VkBindImageMemoryInfo", "BIND_IMAGE_MEMORY_INFO" },
   { "VkWriteDescriptorSet", "WRITE_DESCRIPTOR_SET" },
   { "VkCopyDescriptorSet", "COPY_DESCRIPTOR_SET" },
   { "VkDebugUtilsObjectNameInfoEXT", "DEBUG_UTILS_OBJECT_NAME_INFO_EXT" },
   { "VkDebugUtilsObjectTagInfoEXT", "DEBUG_UTILS_OBJECT_TAG_INFO_EXT" },
   { "VkDebugUtilsLabelEXT", "DEBUG_UTILS_LABEL_EXT" },
   { "VkDebugUtilsMessengerCallbackDataEXT", "DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT" },
   { "VkDisplayPowerInfoEXT", "DISPLAY_POWER_INFO_EXT" },
   { "VkMultisamplePropertiesEXT", "MULTISAMPLE_PROPERTIES_EXT" },
   { "VkConditionalRenderingBeginInfoEXT", "CONDITIONAL_RENDERING_BEGIN_INFO_EXT" },
   { "VkDescriptorSetLayoutSupport", "DESCRIPTOR_SET_LAYOUT_SUPPORT" },
   { "VkPipelineShaderStageCreateInfo", "PIPELINE_SHADER_STAGE_CREATE_INFO" },
   { "VkComputePipelineCreateInfo", "COMPUTE_PIPELINE_CREATE_INFO" },
   { "VkPipelineInputAssemblyStateCreateInfo", "PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO" },
   { "VkPipelineTessellationDomainOriginStateCreateInfoKHR", "PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR" },
   { "VkPipelineTessellationStateCreateInfo", "PIPELINE_TESSELLATION_STATE_CREATE_INFO" },
   { "VkPipelineViewportStateCreateInfo", "PIPELINE_VIEWPORT_STATE_CREATE_INFO" },
   { "VkPipelineRasterizationStateCreateInfo", "PIPELINE_RASTERIZATION_STATE_CREATE_INFO" },
   { "VkPipelineSampleLocationsStateCreateInfoEXT", "PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT" },
   { "VkSampleLocationsInfoEXT", "SAMPLE_LOCATIONS_INFO_EXT" },
   { "VkPipelineMultisampleStateCreateInfo", "PIPELINE_MULTISAMPLE_STATE_CREATE_INFO" },
   { "VkPipelineVertexInputStateCreateInfo", "PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO" },
   { "VkPipelineDepthStencilStateCreateInfo", "PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO" },
   { "VkPipelineColorBlendAdvancedStateCreateInfoEXT", "PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT" },
   { "VkPipelineColorBlendStateCreateInfo", "PIPELINE_COLOR_BLEND_STATE_CREATE_INFO" },
   { "VkPipelineDynamicStateCreateInfo", "PIPELINE_DYNAMIC_STATE_CREATE_INFO" },
   { "VkPipelineDiscardRectangleStateCreateInfoEXT", "PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT" },
   { "VkGraphicsPipelineCreateInfo", "GRAPHICS_PIPELINE_CREATE_INFO" },
   { "VkMemoryFdPropertiesKHR", "MEMORY_FD_PROPERTIES_KHR" },
   { "VkMemoryHostPointerPropertiesEXT", "MEMORY_HOST_POINTER_PROPERTIES_EXT" },
   { "VkMemoryGetFdInfoKHR", "MEMORY_GET_FD_INFO_KHR" },
   { "VkDeviceEventInfoEXT", "DEVICE_EVENT_INFO_EXT" },
   { "VkDisplayEventInfoEXT", "DISPLAY_EVENT_INFO_EXT" },
   { "VkImportFenceFdInfoKHR", "IMPORT_FENCE_FD_INFO_KHR" },
   { "VkFenceGetFdInfoKHR", "FENCE_GET_FD_INFO_KHR" },
   { "VkImportSemaphoreFdInfoKHR", "IMPORT_SEMAPHORE_FD_INFO_KHR" },
   { "VkSemaphoreGetFdInfoKHR", "SEMAPHORE_GET_FD_INFO_KHR" },
   { "VkExternalBufferPropertiesKHR", "EXTERNAL_BUFFER_PROPERTIES_KHR" },
   { "VkPhysicalDeviceExternalBufferInfoKHR", "PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR" },
   { "VkExternalFencePropertiesKHR", "EXTERNAL_FENCE_PROPERTIES_KHR" },
   { "VkPhysicalDeviceExternalFenceInfoKHR", "PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR" },
   { "VkExternalSemaphorePropertiesKHR", "EXTERNAL_SEMAPHORE_PROPERTIES_KHR" },
   { "VkPhysicalDeviceExternalSemaphoreInfo", "PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO" },
   { "VkPhysicalDeviceSparseImageFormatInfo2", "PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2" },
   { "VkRenderPassSampleLocationsBeginInfoEXT", "RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT" },
   { "VkRenderPassBeginInfo", "RENDER_PASS_BEGIN_INFO" },
   { "VkPhysicalDeviceExternalImageFormatInfoKHR", "PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR" },
   { "VkPhysicalDeviceImageFormatInfo2", "PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2" },
   { "VkDisplayModeCreateInfoKHR", "DISPLAY_MODE_CREATE_INFO_KHR" },
   { "VkPhysicalDeviceSurfaceInfo2KHR", "PHYSICAL_DEVICE_SURFACE_INFO_2_KHR" },
   { "VkProtectedSubmitInfo", "PROTECTED_SUBMIT_INFO" },
   { "VkPipelineRasterizationConservativeStateCreateInfoEXT", "PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT" },
   { "VkPipelineRasterizationStateStreamCreateInfoEXT", "PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT" },
   { "VkImageViewASTCDecodeModeEXT", "IMAGE_VIEW_ASTC_DECODE_MODE_EXT" },
   { "VkPipelineVertexInputDivisorStateCreateInfoEXT", "PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT" },
   { "VkWriteDescriptorSetInlineUniformBlockEXT", "WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT" },
   { "VkDescriptorPoolInlineUniformBlockCreateInfoEXT", "DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT" },
   { "VkDescriptorSetLayoutBindingFlagsCreateInfo", "DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO" },
   { "VkDescriptorSetVariableDescriptorCountAllocateInfo", "DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO" },
   { "VkRenderPassMultiviewCreateInfoKHR", "RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR" },
   { "VkAttachmentDescription2", "ATTACHMENT_DESCRIPTION_2" },
   { "VkAttachmentReference2", "ATTACHMENT_REFERENCE_2" },
   { "VkSubpassDescription2", "SUBPASS_DESCRIPTION_2" },
   { "VkSubpassDependency2", "SUBPASS_DEPENDENCY_2" },
   { "VkRenderPassCreateInfo2", "RENDER_PASS_CREATE_INFO_2" },
   { "VkSubpassBeginInfo", "SUBPASS_BEGIN_INFO" },
   { "VkSubpassEndInfo", "SUBPASS_END_INFO" },
   { "VkDisplayProperties2KHR", "DISPLAY_PROPERTIES_2_KHR" },
   { "VkDisplayPlaneProperties2KHR", "DISPLAY_PLANE_PROPERTIES_2_KHR" },
   { "VkDisplayModeProperties2KHR", "DISPLAY_MODE_PROPERTIES_2_KHR" },
   { "VkDisplayPlaneCapabilities2KHR", "DISPLAY_PLANE_CAPABILITIES_2_KHR" },
   { "VkDisplayPlaneInfo2KHR", "DISPLAY_PLANE_INFO_2_KHR" },
   { "VkDeviceGroupDeviceCreateInfoKHR", "DEVICE_GROUP_DEVICE_CREATE_INFO_KHR" },
   { "VkPhysicalDeviceGroupPropertiesKHR", "PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR" },
   { "VkMemoryAllocateFlagsInfoKHR", "MEMORY_ALLOCATE_FLAGS_INFO_KHR" },
   { "VkDeviceGroupRenderPassBeginInfoKHR", "DEVICE_GROUP_RENDER_PASS_BEGIN_INFO_KHR" },
   { "VkDeviceGroupCommandBufferBeginInfoKHR", "DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO_KHR" },
   { "VkDeviceGroupSubmitInfoKHR", "DEVICE_GROUP_SUBMIT_INFO_KHR" },
   { "VkDeviceGroupBindSparseInfoKHR", "DEVICE_GROUP_BIND_SPARSE_INFO_KHR" },
   { "VkBindImageMemoryDeviceGroupInfoKHR", "BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO_KHR" },
   { "VkBindBufferMemoryDeviceGroupInfoKHR", "BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO_KHR" },
   { "VkImageSwapchainCreateInfoKHR", "IMAGE_SWAPCHAIN_CREATE_INFO_KHR" },
   { "VkBindImageMemorySwapchainInfoKHR", "BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR" },
   { "VkDeviceGroupSwapchainCreateInfoKHR", "DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR" },
   { "VkDeviceGroupPresentInfoKHR", "DEVICE_GROUP_PRESENT_INFO_KHR" },
   { "VkDeviceGroupPresentCapabilitiesKHR", "DEVICE_GROUP_PRESENT_CAPABILITIES_KHR" },
   { "VkAcquireNextImageInfoKHR", "ACQUIRE_NEXT_IMAGE_INFO_KHR" },
   { "VkImportMemoryHostPointerInfoEXT", "IMPORT_MEMORY_HOST_POINTER_INFO_EXT" },
   { "VkCalibratedTimestampInfoEXT", "CALIBRATED_TIMESTAMP_INFO_EXT" },
   { "VkDrmFormatModifierPropertiesListEXT", "DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT" },
   { "VkPhysicalDeviceImageDrmFormatModifierInfoEXT", "PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT" },
   { "VkImageDrmFormatModifierListCreateInfoEXT", "IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT" },
   { "VkImageDrmFormatModifierExplicitCreateInfoEXT", "IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT" },
   { "VkImageDrmFormatModifierPropertiesEXT", "IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT" },
   { "VkFramebufferAttachmentImageInfo", "FRAMEBUFFER_ATTACHMENT_IMAGE_INFO" },
   { "VkFramebufferAttachmentsCreateInfo", "FRAMEBUFFER_ATTACHMENTS_CREATE_INFO" },
   { "VkRenderPassAttachmentBeginInfo", "RENDER_PASS_ATTACHMENT_BEGIN_INFO" },
   { "VkSubpassDescriptionDepthStencilResolve", "SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE" },
   { "VkSurfaceProtectedCapabilitiesKHR", "SURFACE_PROTECTED_CAPABILITIES_KHR" },
   { "VkPipelineRasterizationDepthClipStateCreateInfoEXT", "PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT" },
   { "VkPhysicalDeviceImageViewImageFormatInfoEXT", "PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT" },
   { "VkFilterCubicImageViewImageFormatPropertiesEXT", "FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT" },
   { "VkPipelineCreationFeedbackCreateInfoEXT", "PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT" },
   { "VkRenderPassFragmentDensityMapCreateInfoEXT", "RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT" },
   { "VkMemoryPriorityAllocateInfoEXT", "MEMORY_PRIORITY_ALLOCATE_INFO_EXT" },
   { "VkBufferDeviceAddressInfo", "BUFFER_DEVICE_ADDRESS_INFO" },
   { "VkImageStencilUsageCreateInfo", "IMAGE_STENCIL_USAGE_CREATE_INFO" },
   { "VkValidationFeaturesEXT", "VALIDATION_FEATURES_EXT" },
   { "VkHeadlessSurfaceCreateInfoEXT", "HEADLESS_SURFACE_CREATE_INFO_EXT" },
--@@  { "VkSurfaceFullScreenExclusiveInfoEXT", "SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT" },
--@@  { "VkSurfaceCapabilitiesFullScreenExclusiveEXT", "SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT" },
   { "VkPerformanceCounterKHR", "PERFORMANCE_COUNTER_KHR" },
   { "VkPerformanceCounterDescriptionKHR", "PERFORMANCE_COUNTER_DESCRIPTION_KHR" },
   { "VkQueryPoolPerformanceCreateInfoKHR", "QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR" },
   { "VkAcquireProfilingLockInfoKHR", "ACQUIRE_PROFILING_LOCK_INFO_KHR" },
   { "VkPerformanceQuerySubmitInfoKHR", "PERFORMANCE_QUERY_SUBMIT_INFO_KHR" },
   { "VkSemaphoreTypeCreateInfo", "SEMAPHORE_TYPE_CREATE_INFO" },
   { "VkTimelineSemaphoreSubmitInfo", "TIMELINE_SEMAPHORE_SUBMIT_INFO" },
   { "VkSemaphoreWaitInfo", "SEMAPHORE_WAIT_INFO" },
   { "VkSemaphoreSignalInfo", "SEMAPHORE_SIGNAL_INFO" },
   { "VkFragmentShadingRateAttachmentInfoKHR", "FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR" },
   { "VkPipelineFragmentShadingRateStateCreateInfoKHR", "PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR" },
   { "VkPhysicalDeviceFragmentShadingRateKHR", "PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR" },
   { "VkAttachmentReferenceStencilLayout", "ATTACHMENT_REFERENCE_STENCIL_LAYOUT" },
   { "VkAttachmentDescriptionStencilLayout", "ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT" },
   { "VkBufferOpaqueCaptureAddressCreateInfo", "BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO" },
   { "VkMemoryOpaqueCaptureAddressAllocateInfo", "MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO" },
   { "VkDeviceMemoryOpaqueCaptureAddressInfo", "DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO" },
   { "VkPipelineInfoKHR", "PIPELINE_INFO_KHR" },
   { "VkPipelineExecutablePropertiesKHR", "PIPELINE_EXECUTABLE_PROPERTIES_KHR" },
   { "VkPipelineExecutableInfoKHR", "PIPELINE_EXECUTABLE_INFO_KHR" },
   { "VkPipelineExecutableStatisticKHR", "PIPELINE_EXECUTABLE_STATISTIC_KHR" },
   { "VkPipelineExecutableInternalRepresentationKHR", "PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR" },
   { "VkPipelineLibraryCreateInfoKHR", "PIPELINE_LIBRARY_CREATE_INFO_KHR" },
   { "VkPresentIdKHR", "PRESENT_ID_KHR" },
   { "VkMemoryBarrier2KHR", "MEMORY_BARRIER_2_KHR" },
   { "VkBufferMemoryBarrier2KHR", "BUFFER_MEMORY_BARRIER_2_KHR" },
   { "VkImageMemoryBarrier2KHR", "IMAGE_MEMORY_BARRIER_2_KHR" },
   { "VkDependencyInfoKHR", "DEPENDENCY_INFO_KHR" },
   { "VkSemaphoreSubmitInfoKHR", "SEMAPHORE_SUBMIT_INFO_KHR" },
   { "VkCommandBufferSubmitInfoKHR", "COMMAND_BUFFER_SUBMIT_INFO_KHR" },
   { "VkSubmitInfo2KHR", "SUBMIT_INFO_2_KHR" },
   { "VkBufferCopy2KHR", "BUFFER_COPY_2_KHR" },
   { "VkCopyBufferInfo2KHR", "COPY_BUFFER_INFO_2_KHR" },
   { "VkImageCopy2KHR", "IMAGE_COPY_2_KHR" },
   { "VkCopyImageInfo2KHR", "COPY_IMAGE_INFO_2_KHR" },
   { "VkBufferImageCopy2KHR", "BUFFER_IMAGE_COPY_2_KHR" },
   { "VkCopyBufferToImageInfo2KHR", "COPY_BUFFER_TO_IMAGE_INFO_2_KHR" },
   { "VkCopyImageToBufferInfo2KHR", "COPY_IMAGE_TO_BUFFER_INFO_2_KHR" },
   { "VkImageBlit2KHR", "IMAGE_BLIT_2_KHR" },
   { "VkBlitImageInfo2KHR", "BLIT_IMAGE_INFO_2_KHR" },
   { "VkImageResolve2KHR", "IMAGE_RESOLVE_2_KHR" },
   { "VkResolveImageInfo2KHR", "RESOLVE_IMAGE_INFO_2_KHR" },
   { "VkAccelerationStructureGeometryTrianglesDataKHR", "ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR" },
   { "VkAccelerationStructureGeometryAabbsDataKHR", "ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR" },
   { "VkAccelerationStructureGeometryInstancesDataKHR", "ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR" },
   { "VkAccelerationStructureGeometryKHR", "ACCELERATION_STRUCTURE_GEOMETRY_KHR" },
   { "VkAccelerationStructureBuildGeometryInfoKHR", "ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR" },
   { "VkAccelerationStructureCreateInfoKHR", "ACCELERATION_STRUCTURE_CREATE_INFO_KHR" },
   { "VkWriteDescriptorSetAccelerationStructureKHR", "WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR" },
   { "VkAccelerationStructureDeviceAddressInfoKHR", "ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR" },
   { "VkAccelerationStructureVersionInfoKHR", "ACCELERATION_STRUCTURE_VERSION_INFO_KHR" },
   { "VkCopyAccelerationStructureToMemoryInfoKHR", "COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR" },
   { "VkCopyMemoryToAccelerationStructureInfoKHR", "COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR" },
   { "VkCopyAccelerationStructureInfoKHR", "COPY_ACCELERATION_STRUCTURE_INFO_KHR" },
   { "VkAccelerationStructureBuildSizesInfoKHR", "ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR" },
   { "VkRayTracingShaderGroupCreateInfoKHR", "RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR" },
   { "VkRayTracingPipelineInterfaceCreateInfoKHR", "RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR" },
   { "VkRayTracingPipelineCreateInfoKHR", "RAY_TRACING_PIPELINE_CREATE_INFO_KHR" },
   { "VkTraceRaysIndirectCommandKHR", "TRACE_RAYS_INDIRECT_COMMAND_KHR" },
   { "VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT", "PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT" },
   { "VkPhysicalDeviceToolPropertiesEXT", "PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT" },
   { "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT", "PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT" },
   { "VkPipelineRasterizationLineStateCreateInfoEXT", "PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT" },
   { "VkDeviceMemoryReportCallbackDataEXT", "DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT" },
   { "VkDeviceDeviceMemoryReportCreateInfoEXT", "DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT" },
   { "VkSamplerCustomBorderColorCreateInfoEXT", "SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT" },
   { "VkDevicePrivateDataCreateInfoEXT", "DEVICE_PRIVATE_DATA_CREATE_INFO_EXT" },
   { "VkPrivateDataSlotCreateInfoEXT", "PRIVATE_DATA_SLOT_CREATE_INFO_EXT" },
   { "VkVertexInputBindingDescription2EXT", "VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT" },
   { "VkVertexInputAttributeDescription2EXT", "VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT" },
   { "VkPipelineColorWriteCreateInfoEXT", "PIPELINE_COLOR_WRITE_CREATE_INFO_EXT" },
   { "VkQueueFamilyGlobalPriorityPropertiesEXT", "QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_EXT" },
}

local TYPED_WIN32 = {
   { "VkImportSemaphoreWin32HandleInfoKHR", "IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR" },
   { "VkSemaphoreGetWin32HandleInfoKHR", "SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR" },
   { "VkImportFenceWin32HandleInfoKHR", "IMPORT_FENCE_WIN32_HANDLE_INFO_KHR" },
   { "VkExportFenceWin32HandleInfoKHR", "EXPORT_FENCE_WIN32_HANDLE_INFO_KHR" },
   { "VkFenceGetWin32HandleInfoKHR", "FENCE_GET_WIN32_HANDLE_INFO_KHR" },
   { "VkImportMemoryWin32HandleInfoKHR", "IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR" },
   { "VkExportMemoryWin32HandleInfoKHR", "EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR" },
   { "VkMemoryWin32HandlePropertiesKHR", "MEMORY_WIN32_HANDLE_PROPERTIES_KHR" },
   { "VkMemoryGetWin32HandleInfoKHR", "MEMORY_GET_WIN32_HANDLE_INFO_KHR" },
   { "VkWin32KeyedMutexAcquireReleaseInfoKHR", "WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR" },
   { "VkSurfaceFullScreenExclusiveWin32InfoEXT", "SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT" },
}


-----------------------------------------------------------------------------
local template_typed =[[
#define znewXxx(L, err) (Xxx*)znew((L), VK_STRUCTURE_TYPE_XXX, sizeof(Xxx), (err))
#define znewarrayXxx(L, count, err) (Xxx*)znewarray((L), VK_STRUCTURE_TYPE_XXX, sizeof(Xxx), (count), (err))
#define znewchainXxx moonvulkan_znewchainXxx
Xxx* znewchainXxx(lua_State *L, int *err);
#define znewchainarrayXxx moonvulkan_znewchainarrayXxx
Xxx* znewchainarrayXxx(lua_State *L, uint32_t count, int *err);
#define zcheckXxx moonvulkan_zcheckXxx
Xxx* zcheckXxx(lua_State *L, int arg, int *err);
#define zcheckarrayXxx moonvulkan_zcheckarrayXxx
Xxx* zcheckarrayXxx(lua_State *L, int arg, uint32_t *count, int *err);
#define zinitXxx moonvulkan_zinitXxx
int zinitXxx(lua_State *L, Xxx* p, int *err);
#define zfreeXxx(L, p, base) zfree((L), (p), (base))
#define zfreearrayXxx(L, p, count, base) zfreearray((L), (p), sizeof(Xxx), (count), (base))
#define zpushXxx moonvulkan_zpushXxx
int zpushXxx(lua_State *L, const Xxx *p);
]]

local function create_decl_typed(Xxx, XXX)
   local s = string.gsub(template_typed, "Xxx", Xxx)
   local s = string.gsub(s, "XXX", XXX)
   return s
end

-----------------------------------------------------------------------------
local template_untyped =[[
#define znewXxx(L, err) (Xxx*)znew((L), -1, sizeof(Xxx),(err))
#define znewarrayXxx(L, count, err) (Xxx*)znewarray((L),-1, sizeof(Xxx), (count), (err))
#define zcheckXxx moonvulkan_zcheckXxx
Xxx* zcheckXxx(lua_State *L, int arg, int *err);
#define zcheckarrayXxx moonvulkan_zcheckarrayXxx
Xxx* zcheckarrayXxx(lua_State *L, int arg, uint32_t *count, int *err);
#define zinitXxx moonvulkan_zinitXxx
int zinitXxx(lua_State *L, Xxx* p, int *err);
#define zfreeXxx(L, p, base) zfree_untyped((L), (p), (base), zclearXxx)
#define zfreearrayXxx(L, p, count, base) zfreearray_untyped((L), (p), sizeof(Xxx), (count), (base), zclearXxx)
#define zpushXxx moonvulkan_zpushXxx
int zpushXxx(lua_State *L, const Xxx *p);
]]

local template_untyped0 = [[
#define zclearXxx NULL
]]..template_untyped

local template_untyped1 = [[
#define zclearXxx moonvulkan_zclearXxx 
void zclearXxx(lua_State *L, const void *p);
]]..template_untyped

local function create_decl_untyped(Xxx, clearfunc)
   local s = clearfunc and string.gsub(template_untyped1, "Xxx", Xxx) 
               or string.gsub(template_untyped0, "Xxx", Xxx)
   return s
end

-----------------------------------------------------------------------------

print(HEADER)

print([[
/*------------------------------------------------------------------------------*
 | Typed structs                                                                |
 *------------------------------------------------------------------------------*/
]])

for i, t in ipairs(TYPED) do print(create_decl_typed(t[1], t[2])) end
print("\n#ifdef VK_USE_PLATFORM_WIN32_KHR\n")
for i, t in ipairs(TYPED_WIN32) do print(create_decl_typed(t[1], t[2])) end
print("\n#endif /* VK_USE_PLATFORM_WIN32_KHR */\n")

print([[
/*------------------------------------------------------------------------------*
 | Untyped structs                                                              |
 *------------------------------------------------------------------------------*/
]])

for i, t in ipairs(UNTYPED) do print(create_decl_untyped(t[1], t[2])) end
print("\n#ifdef VK_USE_PLATFORM_WIN32_KHR\n")
for i, t in ipairs(UNTYPED_WIN32) do print(create_decl_untyped(t[1], t[2])) end
print("\n#endif /* VK_USE_PLATFORM_WIN32_KHR */\n")

print(TRAILER)

