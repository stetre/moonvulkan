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

#ifndef structsDEFINED
#define structsDEFINED

#if 0 /* scaffolding */
#define  moonvulkan_

/* echeckzzzlist() */
ECHECKLISTFUNC(VkZzz, zzz, NULL)
#define echeckzzzlist moonvulkan_echeckzzzlist
VkZzz* echeckzzzlist(lua_State *L, int arg, uint32_t *count, int *err);

/* echeckzzzlist() */
FREELISTFUNC(VkZzz, zzz)
ECHECKLISTFUNC(VkZzz, zzz, freezzzlist)
#define freezzzlist moonvulkan_freezzzlist
void freezzzlist(lua_State *L, void *list, uint32_t count);
#define echeckzzzlist moonvulkan_echeckzzzlist
VkZzz* echeckzzzlist(lua_State *L, int arg, uint32_t *count, int *err);

#endif

#define echeckcommandpoolcreateinfo moonvulkan_echeckcommandpoolcreateinfo
int echeckcommandpoolcreateinfo(lua_State *L, int arg, VkCommandPoolCreateInfo *p);

#define echeckcommandbufferallocateinfo moonvulkan_echeckcommandbufferallocateinfo
int echeckcommandbufferallocateinfo(lua_State *L, int arg, VkCommandBufferAllocateInfo *p);

typedef struct {
    VkCommandBufferBeginInfo p1;
    VkCommandBufferInheritanceInfo p2;
} VkCommandBufferBeginInfo_CHAIN;

#define echeckcommandbufferbegininfo moonvulkan_echeckcommandbufferbegininfo
int echeckcommandbufferbegininfo(lua_State *L, int arg, VkCommandBufferBeginInfo_CHAIN *pp);

typedef struct {
    VkFenceCreateInfo p1;
    VkExportFenceCreateInfoKHR p2;
} VkFenceCreateInfo_CHAIN;

#define echeckfencecreateinfo moonvulkan_echeckfencecreateinfo
int echeckfencecreateinfo(lua_State *L, int arg, VkFenceCreateInfo_CHAIN *p);

#define echeckdeviceeventinfo moonvulkan_echeckdeviceeventinfo
int echeckdeviceeventinfo(lua_State *L, int arg, VkDeviceEventInfoEXT *p);
#define echeckdisplayeventinfo moonvulkan_echeckdisplayeventinfo
int echeckdisplayeventinfo(lua_State *L, int arg, VkDisplayEventInfoEXT *p);
#define echeckimportfencefdinfo moonvulkan_echeckimportfencefdinfo
int echeckimportfencefdinfo(lua_State *L, int arg, VkImportFenceFdInfoKHR *p);
#define echeckfencegetfdinfo moonvulkan_echeckfencegetfdinfo
int echeckfencegetfdinfo(lua_State *L, int arg, VkFenceGetFdInfoKHR *p);


typedef struct {
    VkSemaphoreCreateInfo p1;
    VkExportSemaphoreCreateInfoKHR p2;
} VkSemaphoreCreateInfo_CHAIN;

#define echecksemaphorecreateinfo moonvulkan_echecksemaphorecreateinfo
int echecksemaphorecreateinfo(lua_State *L, int arg, VkSemaphoreCreateInfo_CHAIN *p);
#define echeckimportsemaphorefdinfo moonvulkan_echeckimportsemaphorefdinfo
int echeckimportsemaphorefdinfo(lua_State *L, int arg, VkImportSemaphoreFdInfoKHR *p);
#define echecksemaphoregetfdinfo moonvulkan_echecksemaphoregetfdinfo
int echecksemaphoregetfdinfo(lua_State *L, int arg, VkSemaphoreGetFdInfoKHR *p);

#define echeckeventcreateinfo moonvulkan_echeckeventcreateinfo
int echeckeventcreateinfo(lua_State *L, int arg, VkEventCreateInfo *p);

#define freedescriptorpoolcreateinfo moonvulkan_freedescriptorpoolcreateinfo
void freedescriptorpoolcreateinfo(lua_State *L, VkDescriptorPoolCreateInfo *p);
#define echeckdescriptorpoolcreateinfo moonvulkan_echeckdescriptorpoolcreateinfo
int echeckdescriptorpoolcreateinfo(lua_State *L, int arg, VkDescriptorPoolCreateInfo *p);
#define echeckdescriptorpoolsizelist moonvulkan_echeckdescriptorpoolsizelist
VkDescriptorPoolSize* echeckdescriptorpoolsizelist(lua_State *L, int arg, uint32_t *count, int *err);

#define freedescriptorsetallocateinfo moonvulkan_freedescriptorsetallocateinfo
void freedescriptorsetallocateinfo(lua_State *L, VkDescriptorSetAllocateInfo *p);
#define echeckdescriptorsetallocateinfo moonvulkan_echeckdescriptorsetallocateinfo
int echeckdescriptorsetallocateinfo(lua_State *L, int arg, VkDescriptorSetAllocateInfo *p);

#define freedescriptorsetlayoutbindinglist moonvulkan_freedescriptorsetlayoutbindinglist
void freedescriptorsetlayoutbindinglist(lua_State *L, void* list, uint32_t count);
#define echeckdescriptorsetlayoutbindinglist moonvulkan_echeckdescriptorsetlayoutbindinglist
VkDescriptorSetLayoutBinding* echeckdescriptorsetlayoutbindinglist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckdescriptorsetlayoutcreateinfo moonvulkan_echeckdescriptorsetlayoutcreateinfo
int echeckdescriptorsetlayoutcreateinfo(lua_State *L, int arg, VkDescriptorSetLayoutCreateInfo *p);
#define freedescriptorsetlayoutcreateinfo moonvulkan_freedescriptorsetlayoutcreateinfo
void freedescriptorsetlayoutcreateinfo(lua_State *L, VkDescriptorSetLayoutCreateInfo *p);

#define echeckpushconstantrangelist moonvulkan_echeckpushconstantrangelist
VkPushConstantRange* echeckpushconstantrangelist(lua_State *L, int arg, uint32_t *count, int *err);
#define freepipelinelayoutcreateinfo moonvulkan_freepipelinelayoutcreateinfo
void freepipelinelayoutcreateinfo(lua_State *L, VkPipelineLayoutCreateInfo *p);
#define echeckpipelinelayoutcreateinfo moonvulkan_echeckpipelinelayoutcreateinfo
int echeckpipelinelayoutcreateinfo(lua_State *L, int arg, VkPipelineLayoutCreateInfo *p);

#define echeckquerypoolcreateinfo moonvulkan_echeckquerypoolcreateinfo
int echeckquerypoolcreateinfo(lua_State *L, int arg, VkQueryPoolCreateInfo *p);


typedef struct {
    VkRenderPassCreateInfo p1;
    VkRenderPassInputAttachmentAspectCreateInfoKHR p2;
} VkRenderPassCreateInfo_CHAIN;

#define freerenderpasscreateinfo moonvulkan_freerenderpasscreateinfo
void freerenderpasscreateinfo(lua_State *L, VkRenderPassCreateInfo_CHAIN *p);
#define echeckrenderpasscreateinfo moonvulkan_echeckrenderpasscreateinfo
int echeckrenderpasscreateinfo(lua_State *L, int arg, VkRenderPassCreateInfo_CHAIN *p);

#define freerenderpassbegininfo moonvulkan_freerenderpassbegininfo
void freerenderpassbegininfo(lua_State *L, VkRenderPassBeginInfo *p);
#define echeckrenderpassbegininfo moonvulkan_echeckrenderpassbegininfo
int echeckrenderpassbegininfo(lua_State *L, int arg, VkRenderPassBeginInfo *p);

#define freeframebuffercreateinfo moonvulkan_freeframebuffercreateinfo
void freeframebuffercreateinfo(lua_State *L, VkFramebufferCreateInfo *p);
#define echeckframebuffercreateinfo moonvulkan_echeckframebuffercreateinfo
int echeckframebuffercreateinfo(lua_State *L, int arg, VkFramebufferCreateInfo *p);

#define echeckshadermodulecreateinfo moonvulkan_echeckshadermodulecreateinfo
int echeckshadermodulecreateinfo(lua_State *L, int arg, VkShaderModuleCreateInfo *p);

#define echeckpipelinecachecreateinfo moonvulkan_echeckpipelinecachecreateinfo
int echeckpipelinecachecreateinfo(lua_State *L, int arg, VkPipelineCacheCreateInfo *p);

typedef struct {
    VkBufferCreateInfo p1;
    VkExternalMemoryBufferCreateInfoKHR p2;
} VkBufferCreateInfo_CHAIN;

#define freebuffercreateinfo moonvulkan_freebuffercreateinfo
void freebuffercreateinfo(lua_State *L, VkBufferCreateInfo_CHAIN *p);
#define echeckbuffercreateinfo moonvulkan_echeckbuffercreateinfo
int echeckbuffercreateinfo(lua_State *L, int arg, VkBufferCreateInfo_CHAIN *p);

#define freebufferviewcreateinfo(L, p) do { } while(0)
#define echeckbufferviewcreateinfo moonvulkan_echeckbufferviewcreateinfo
int echeckbufferviewcreateinfo(lua_State *L, int arg, VkBufferViewCreateInfo *p);

typedef struct {
    VkImageCreateInfo p1;
    VkExternalMemoryImageCreateInfoKHR p2;
    VkImageFormatListCreateInfoKHR p3;
} VkImageCreateInfo_CHAIN;

#define freeimagecreateinfo moonvulkan_freeimagecreateinfo
void freeimagecreateinfo(lua_State *L, VkImageCreateInfo_CHAIN *p);
#define echeckimagecreateinfo moonvulkan_echeckimagecreateinfo
int echeckimagecreateinfo(lua_State *L, int arg, VkImageCreateInfo_CHAIN *p);

typedef struct {
    VkImageViewCreateInfo p1;
    VkImageViewUsageCreateInfoKHR p2;
} VkImageViewCreateInfo_CHAIN;

#define freeimageviewcreateinfo(L, p) do { } while(0)
#define echeckimageviewcreateinfo moonvulkan_echeckimageviewcreateinfo
int echeckimageviewcreateinfo(lua_State *L, int arg, VkImageViewCreateInfo_CHAIN *p);

typedef struct {
    VkSamplerCreateInfo p1;
    VkSamplerReductionModeCreateInfoEXT p2;
} VkSamplerCreateInfo_CHAIN;

#define freesamplercreateinfo moonvulkan_freesamplercreateinfo
void freesamplercreateinfo(lua_State *L, VkSamplerCreateInfo_CHAIN *p);
#define echecksamplercreateinfo moonvulkan_echecksamplercreateinfo
int echecksamplercreateinfo(lua_State *L, int arg, VkSamplerCreateInfo_CHAIN *p);

#define pushphysicaldevicelimits moonvulkan_pushphysicaldevicelimits
int pushphysicaldevicelimits(lua_State *L, VkPhysicalDeviceLimits *p);

#define pushextensionproperties  moonvulkan_pushextensionproperties 
int pushextensionproperties (lua_State *L, VkExtensionProperties *p);

#define pushlayerproperties moonvulkan_pushlayerproperties
int pushlayerproperties(lua_State *L, VkLayerProperties *p);

#define echeckviewport moonvulkan_echeckviewport
int echeckviewport(lua_State *L, int arg, VkViewport *dst);
#define pushviewport moonvulkan_pushviewport
int pushviewport(lua_State *L, VkViewport *p);
#define echeckviewportlist moonvulkan_echeckviewportlist
VkViewport* echeckviewportlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echeckoffset3d moonvulkan_echeckoffset3d
int echeckoffset3d(lua_State *L, int arg, VkOffset3D *dst);
#define pushoffset3d moonvulkan_pushoffset3d
int pushoffset3d(lua_State *L, VkOffset3D *p);

#define echeckoffset2d moonvulkan_echeckoffset2d
int echeckoffset2d(lua_State *L, int arg, VkOffset2D *dst);
#define pushoffset2d moonvulkan_pushoffset2d
int pushoffset2d(lua_State *L, VkOffset2D *p);

#define echeckextent2d moonvulkan_echeckextent2d
int echeckextent2d(lua_State *L, int arg, VkExtent2D *dst);
#define pushextent2d moonvulkan_pushextent2d
int pushextent2d(lua_State *L, VkExtent2D *p);

#define echeckextent3d moonvulkan_echeckextent3d
int echeckextent3d(lua_State *L, int arg, VkExtent3D *dst);
#define pushextent3d moonvulkan_pushextent3d
int pushextent3d(lua_State *L, VkExtent3D *p);

typedef struct {
    VkPhysicalDeviceProperties2KHR p1;
    VkPhysicalDevicePushDescriptorPropertiesKHR p2;
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT p3;
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT p4;
    VkPhysicalDeviceDiscardRectanglePropertiesEXT p5;
    VkPhysicalDeviceIDPropertiesKHR p6;
    VkPhysicalDevicePointClippingPropertiesKHR p7;
    VkPhysicalDeviceSampleLocationsPropertiesEXT p8;
} VkPhysicalDeviceProperties2KHR_CHAIN;

#define initphysicaldeviceproperties2 moonvulkan_initphysicaldeviceproperties2
void initphysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2KHR_CHAIN *p);
#define pushphysicaldeviceproperties2 moonvulkan_pushphysicaldeviceproperties2
int pushphysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2KHR_CHAIN *p);
#define pushphysicaldeviceproperties moonvulkan_pushphysicaldeviceproperties
int pushphysicaldeviceproperties(lua_State *L, VkPhysicalDeviceProperties *p);

typedef struct {
    VkPhysicalDeviceFeatures2KHR p1;
    VkPhysicalDevice16BitStorageFeaturesKHR p2;
    VkPhysicalDeviceVariablePointerFeaturesKHR p3;
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT p4;
} VkPhysicalDeviceFeatures2KHR_CHAIN;

#define initphysicaldevicefeatures2 moonvulkan_initphysicaldevicefeatures2
void initphysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2KHR_CHAIN*);
#define pushphysicaldevicefeatures2 moonvulkan_pushphysicaldevicefeatures2
int pushphysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2KHR_CHAIN *p);
#define pushphysicaldevicefeatures moonvulkan_pushphysicaldevicefeatures
int pushphysicaldevicefeatures(lua_State *L, VkPhysicalDeviceFeatures *p);

typedef struct {
    VkFormatProperties2KHR p1;
} VkFormatProperties2KHR_CHAIN;

#define initformatproperties2 moonvulkan_initformatproperties2
void initformatproperties2(lua_State *L, VkFormatProperties2KHR_CHAIN* p);
#define pushformatproperties2 moonvulkan_pushformatproperties2
int pushformatproperties2(lua_State *L, VkFormatProperties2KHR_CHAIN *p);
#define pushformatproperties moonvulkan_pushformatproperties
int pushformatproperties(lua_State *L, VkFormatProperties *p);

typedef struct {
    VkImageFormatProperties2KHR p1;
    VkExternalImageFormatPropertiesKHR p2;
} VkImageFormatProperties2KHR_CHAIN;

#define initimageformatproperties2 moonvulkan_initimageformatproperties2
void initimageformatproperties2(lua_State *L, VkImageFormatProperties2KHR_CHAIN *p);
#define pushimageformatproperties2 moonvulkan_pushimageformatproperties2
int pushimageformatproperties2(lua_State *L, VkImageFormatProperties2KHR_CHAIN *p);
#define pushimageformatproperties moonvulkan_pushimageformatproperties
int pushimageformatproperties(lua_State *L, VkImageFormatProperties *p);

#define pushexternalbufferproperties moonvulkan_pushexternalbufferproperties
int pushexternalbufferproperties(lua_State *L, VkExternalBufferPropertiesKHR *p);
#define echeckphysicaldeviceexternalbufferinfo moonvulkan_echeckphysicaldeviceexternalbufferinfo
int echeckphysicaldeviceexternalbufferinfo(lua_State *L, int arg, VkPhysicalDeviceExternalBufferInfoKHR *p);

#define pushexternalfenceproperties moonvulkan_pushexternalfenceproperties
int pushexternalfenceproperties(lua_State *L, VkExternalFencePropertiesKHR *p);
#define echeckphysicaldeviceexternalfenceinfo moonvulkan_echeckphysicaldeviceexternalfenceinfo
int echeckphysicaldeviceexternalfenceinfo(lua_State *L, int arg, VkPhysicalDeviceExternalFenceInfoKHR *p);

#define pushexternalsemaphoreproperties moonvulkan_pushexternalsemaphoreproperties
int pushexternalsemaphoreproperties(lua_State *L, VkExternalSemaphorePropertiesKHR *p);
#define echeckphysicaldeviceexternalsemaphoreinfo moonvulkan_echeckphysicaldeviceexternalsemaphoreinfo
int echeckphysicaldeviceexternalsemaphoreinfo(lua_State *L, int arg, VkPhysicalDeviceExternalSemaphoreInfoKHR *p);

typedef struct {
    VkPhysicalDeviceImageFormatInfo2KHR p1;
    VkPhysicalDeviceExternalImageFormatInfoKHR p2;
} VkPhysicalDeviceImageFormatInfo2KHR_CHAIN;

#define echeckphysicaldeviceimageformatinfo2 moonvulkan_echeckphysicaldeviceimageformatinfo2
int echeckphysicaldeviceimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceImageFormatInfo2KHR_CHAIN *p);

#define echeckphysicaldevicesparseimageformatinfo2 moonvulkan_echeckphysicaldevicesparseimageformatinfo2
int echeckphysicaldevicesparseimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceSparseImageFormatInfo2KHR *p);

#define pushphysicaldevicesparseproperties moonvulkan_pushphysicaldevicesparseproperties
int pushphysicaldevicesparseproperties(lua_State *L, VkPhysicalDeviceSparseProperties *p);

#define newqueuefamilyproperties2 moonvulkan_newqueuefamilyproperties2
VkQueueFamilyProperties2KHR *newqueuefamilyproperties2(lua_State *L, uint32_t count);
#define freequeuefamilyproperties2 moonvulkan_freequeuefamilyproperties2
void freequeuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2KHR *p, uint32_t count);
#define pushqueuefamilyproperties moonvulkan_pushqueuefamilyproperties
int pushqueuefamilyproperties(lua_State *L, VkQueueFamilyProperties *p, uint32_t family_index);
#define pushqueuefamilyproperties2 moonvulkan_pushqueuefamilyproperties2
int pushqueuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2KHR *p, uint32_t index);

typedef struct {
    VkPhysicalDeviceMemoryProperties2KHR p1;
} VkPhysicalDeviceMemoryProperties2KHR_CHAIN;

#define initphysicaldevicememoryproperties2 moonvulkan_initphysicaldevicememoryproperties2
void initphysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2KHR_CHAIN *p);
#define pushphysicaldevicememoryproperties2 moonvulkan_pushphysicaldevicememoryproperties2
int pushphysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2KHR_CHAIN *p);
#define pushphysicaldevicememoryproperties moonvulkan_pushphysicaldevicememoryproperties
int pushphysicaldevicememoryproperties(lua_State *L, VkPhysicalDeviceMemoryProperties *p);

#define newsparseimageformatproperties2 moonvulkan_newsparseimageformatproperties2
VkSparseImageFormatProperties2KHR *newsparseimageformatproperties2(lua_State *L, uint32_t count);
#define freesparseimageformatproperties2 moonvulkan_freesparseimageformatproperties2
void freesparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2KHR *p, uint32_t count);
#define pushsparseimageformatproperties moonvulkan_pushsparseimageformatproperties
int pushsparseimageformatproperties(lua_State *L, VkSparseImageFormatProperties *p);
#define pushsparseimageformatproperties2 moonvulkan_pushsparseimageformatproperties2
int pushsparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2KHR *p);

#define echeckcomponentmapping moonvulkan_echeckcomponentmapping
int echeckcomponentmapping(lua_State *L, int arg, VkComponentMapping *dst);
#define pushcomponentmapping moonvulkan_pushcomponentmapping
int pushcomponentmapping(lua_State *L, VkComponentMapping *p);

#define echeckimagesubresourcerange moonvulkan_echeckimagesubresourcerange
int echeckimagesubresourcerange(lua_State *L, int arg, VkImageSubresourceRange *dst);
#define echeckimagesubresourcerangelist moonvulkan_echeckimagesubresourcerangelist
VkImageSubresourceRange* echeckimagesubresourcerangelist(lua_State *L, int arg, uint32_t *count, int *err);
#define pushimagesubresourcerange moonvulkan_pushimagesubresourcerange
int pushimagesubresourcerange(lua_State *L, VkImageSubresourceRange *p);

#define echeckrect2d moonvulkan_echeckrect2d
int echeckrect2d(lua_State *L, int arg, VkRect2D *p);
#define pushrect2d moonvulkan_pushrect2d
int pushrect2d(lua_State *L, VkRect2D *p);
#define echeckrect2dlist moonvulkan_echeckrect2dlist
VkRect2D* echeckrect2dlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echecksamplelocation moonvulkan_echecksamplelocation
int echecksamplelocation(lua_State *L, int arg, VkSampleLocationEXT *p);
#define pushsamplelocation moonvulkan_pushsamplelocation
int pushsamplelocation(lua_State *L, VkSampleLocationEXT *p);
#define echecksamplelocationlist moonvulkan_echecksamplelocationlist
VkSampleLocationEXT* echecksamplelocationlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echeckxycolor moonvulkan_echeckxycolor
int echeckxycolor(lua_State *L, int arg, VkXYColorEXT *p);
#define echeckhdrmetadata moonvulkan_echeckhdrmetadata
int echeckhdrmetadata(lua_State *L, int arg, VkHdrMetadataEXT *p);

#define echeckclearcolorvalue moonvulkan_echeckclearcolorvalue
int echeckclearcolorvalue(lua_State *L, int arg, VkClearColorValue *p);
#define echeckclearvalue moonvulkan_echeckclearvalue
int echeckclearvalue(lua_State *L, int arg, VkClearValue *p);
#define echeckclearattachment moonvulkan_echeckclearattachment
int echeckclearattachment(lua_State *L, int arg, VkClearAttachment *p);
#define echeckclearattachmentlist moonvulkan_echeckclearattachmentlist
VkClearAttachment* echeckclearattachmentlist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckclearrect moonvulkan_echeckclearrect
int echeckclearrect(lua_State *L, int arg, VkClearRect *p);
#define echeckclearrectlist moonvulkan_echeckclearrectlist
VkClearRect* echeckclearrectlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echeckimagesubresource moonvulkan_echeckimagesubresource
int echeckimagesubresource(lua_State *L, int arg, VkImageSubresource *p);
#define echeckimagesubresourcelayers moonvulkan_echeckimagesubresourcelayers
int echeckimagesubresourcelayers(lua_State *L, int arg, VkImageSubresourceLayers *p);
#define echeckimagecopy moonvulkan_echeckimagecopy
int echeckimagecopy(lua_State *L, int arg, VkImageCopy *p);
#define echeckimagecopylist moonvulkan_echeckimagecopylist
VkImageCopy* echeckimagecopylist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckimageblit moonvulkan_echeckimageblit
int echeckimageblit(lua_State *L, int arg, VkImageBlit *p);
#define echeckimageblitlist moonvulkan_echeckimageblitlist
VkImageBlit* echeckimageblitlist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckbufferimagecopy moonvulkan_echeckbufferimagecopy
int echeckbufferimagecopy(lua_State *L, int arg, VkBufferImageCopy *p);
#define echeckbufferimagecopylist moonvulkan_echeckbufferimagecopylist
VkBufferImageCopy* echeckbufferimagecopylist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckimageresolve moonvulkan_echeckimageresolve 
int echeckimageresolve(lua_State *L, int arg, VkImageResolve *p);
#define echeckimageresolvelist moonvulkan_echeckimageresolvelist
VkImageResolve* echeckimageresolvelist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckbuffercopy moonvulkan_echeckbuffercopy
int echeckbuffercopy(lua_State *L, int arg, VkBufferCopy *p);
#define echeckbuffercopylist moonvulkan_echeckbuffercopylist
VkBufferCopy* echeckbuffercopylist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckmemorybarrier moonvulkan_echeckmemorybarrier
int echeckmemorybarrier(lua_State *L, int arg, VkMemoryBarrier *p);
#define echeckmemorybarrierlist moonvulkan_echeckmemorybarrierlist
VkMemoryBarrier* echeckmemorybarrierlist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckbuffermemorybarrier moonvulkan_echeckbuffermemorybarrier
int echeckbuffermemorybarrier(lua_State *L, int arg, VkBufferMemoryBarrier *p);
#define echeckbuffermemorybarrierlist moonvulkan_echeckbuffermemorybarrierlist
VkBufferMemoryBarrier* echeckbuffermemorybarrierlist(lua_State *L, int arg, uint32_t *count, int *err);
#define echeckimagememorybarrier moonvulkan_echeckimagememorybarrier
int echeckimagememorybarrier(lua_State *L, int arg, VkImageMemoryBarrier *p);
#define echeckimagememorybarrierlist moonvulkan_echeckimagememorybarrierlist
VkImageMemoryBarrier* echeckimagememorybarrierlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echeckmappedmemoryrangelist moonvulkan_echeckmappedmemoryrangelist
VkMappedMemoryRange* echeckmappedmemoryrangelist(lua_State *L, int arg, uint32_t *count, int *err);

typedef struct {
    VkMemoryAllocateInfo p1;
    VkMemoryDedicatedAllocateInfoKHR p2;
    VkExportMemoryAllocateInfoKHR p3;
    VkImportMemoryFdInfoKHR p4;
} VkMemoryAllocateInfo_CHAIN;

#define echeckmemoryallocateinfo moonvulkan_echeckmemoryallocateinfo
int echeckmemoryallocateinfo(lua_State *L, int arg, VkMemoryAllocateInfo_CHAIN *pp);


#define pushmemoryfdproperties moonvulkan_pushmemoryfdproperties
int pushmemoryfdproperties(lua_State *L, VkMemoryFdPropertiesKHR *p);
#define echeckmemorygetfdinfo moonvulkan_echeckmemorygetfdinfo
int echeckmemorygetfdinfo(lua_State *L, int arg, VkMemoryGetFdInfoKHR *p);

typedef struct {
    VkBufferMemoryRequirementsInfo2KHR p1;
} VkBufferMemoryRequirementsInfo2KHR_CHAIN;

#define echeckbuffermemoryrequirementsinfo2 moonvulkan_echeckbuffermemoryrequirementsinfo2
int echeckbuffermemoryrequirementsinfo2(lua_State *L, int arg, VkBufferMemoryRequirementsInfo2KHR_CHAIN *pp);

typedef struct {
    VkImageMemoryRequirementsInfo2KHR p1;
} VkImageMemoryRequirementsInfo2KHR_CHAIN;

#define echeckimagememoryrequirementsinfo2 moonvulkan_echeckimagememoryrequirementsinfo2
int echeckimagememoryrequirementsinfo2(lua_State *L, int arg, VkImageMemoryRequirementsInfo2KHR_CHAIN *pp);

typedef struct {
    VkImageSparseMemoryRequirementsInfo2KHR p1;
} VkImageSparseMemoryRequirementsInfo2KHR_CHAIN;

#define echeckimagesparsememoryrequirementsinfo2 moonvulkan_echeckimagesparsememoryrequirementsinfo2
int echeckimagesparsememoryrequirementsinfo2(lua_State *L, int arg, VkImageSparseMemoryRequirementsInfo2KHR_CHAIN *pp);

typedef struct {
    VkMemoryRequirements2KHR p1;
    VkMemoryDedicatedRequirementsKHR p2;
} VkMemoryRequirements2KHR_CHAIN;

#define initmemoryrequirements2 moonvulkan_initmemoryrequirements2
void initmemoryrequirements2(lua_State *L, VkMemoryRequirements2KHR_CHAIN *p);
#define pushmemoryrequirements2 moonvulkan_pushmemoryrequirements2
int pushmemoryrequirements2(lua_State *L, VkMemoryRequirements2KHR_CHAIN *p);
#define pushmemorydedicatedrequirements moonvulkan_pushmemorydedicatedrequirements
int pushmemorydedicatedrequirements(lua_State *L, VkMemoryDedicatedRequirementsKHR *p);
#define pushmemoryrequirements moonvulkan_pushmemoryrequirements
int pushmemoryrequirements(lua_State *L, VkMemoryRequirements *p);

#define newsparseimagememoryrequirements2 moonvulkan_newsparseimagememoryrequirements2
VkSparseImageMemoryRequirements2KHR* newsparseimagememoryrequirements2(lua_State *L, uint32_t count);
#define freesparseimagememoryrequirements2 moonvulkan_freesparseimagememoryrequirements2
void freesparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2KHR *p, uint32_t count);
#define pushsparseimagememoryrequirements moonvulkan_pushsparseimagememoryrequirements
int pushsparseimagememoryrequirements(lua_State *L, VkSparseImageMemoryRequirements *p);
#define pushsparseimagememoryrequirements2 moonvulkan_pushsparseimagememoryrequirements2
int pushsparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2KHR *p);

#define pushsubresourcelayout moonvulkan_pushsubresourcelayout
int pushsubresourcelayout(lua_State *L, VkSubresourceLayout *p);

#define freesubmitinfolist moonvulkan_freesubmitinfolist
void freesubmitinfolist(lua_State *L, void *list, uint32_t count);
#define echecksubmitinfolist moonvulkan_echecksubmitinfolist
VkSubmitInfo* echecksubmitinfolist(lua_State *L, int arg, uint32_t *count, int *err);

#define freebindsparseinfolist moonvulkan_freebindsparseinfolist
void freebindsparseinfolist(lua_State *L, void *list, uint32_t count);
#define echeckbindsparseinfolist moonvulkan_echeckbindsparseinfolist
VkBindSparseInfo* echeckbindsparseinfolist(lua_State *L, int arg, uint32_t *count, int *err);

typedef struct {
    VkSurfaceCapabilities2KHR p1;
    VkSharedPresentSurfaceCapabilitiesKHR p2;
} VkSurfaceCapabilities2KHR_CHAIN;

#define initsurfacecapabilities2 moonvulkan_initsurfacecapabilities2
void initsurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR_CHAIN *p);
#define pushsurfacecapabilities2 moonvulkan_pushsurfacecapabilities2
int pushsurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR_CHAIN *p);
#define pushsurfacecapabilities moonvulkan_pushsurfacecapabilities
int pushsurfacecapabilities(lua_State *L, VkSurfaceCapabilitiesKHR *p);

#define newsurfaceformat2 moonvulkan_newsurfaceformat2
VkSurfaceFormat2KHR *newsurfaceformat2(lua_State *L, uint32_t count);
#define freesurfaceformat2 moonvulkan_freesurfaceformat2
void freesurfaceformat2(lua_State *L, VkSurfaceFormat2KHR *p, uint32_t count);
#define pushsurfaceformat moonvulkan_pushsurfaceformat
int pushsurfaceformat(lua_State *L, VkSurfaceFormatKHR *p);
#define pushsurfaceformat2 moonvulkan_pushsurfaceformat2
int pushsurfaceformat2(lua_State *L, VkSurfaceFormat2KHR *p);

#define freegraphicspipelinecreateinfolist moonvulkan_freegraphicspipelinecreateinfolist
void freegraphicspipelinecreateinfolist(lua_State *L, void *list, uint32_t count);
#define echeckgraphicspipelinecreateinfolist moonvulkan_echeckgraphicspipelinecreateinfolist
VkGraphicsPipelineCreateInfo* echeckgraphicspipelinecreateinfolist(lua_State *L, int arg, uint32_t *count, int *err);

#define freecomputepipelinecreateinfolist moonvulkan_freecomputepipelinecreateinfolist
void freecomputepipelinecreateinfolist(lua_State *L, void *list, uint32_t count);
#define echeckcomputepipelinecreateinfolist moonvulkan_echeckcomputepipelinecreateinfolist
VkComputePipelineCreateInfo* echeckcomputepipelinecreateinfolist(lua_State *L, int arg, uint32_t *count, int *err);

typedef struct {
    VkSwapchainCreateInfoKHR p1;
    VkSwapchainCounterCreateInfoEXT p2;
} VkSwapchainCreateInfoKHR_CHAIN;

typedef struct {
    VkSwapchainCreateInfoKHR *p;
    VkSwapchainCreateInfoKHR_CHAIN *chain;
} VkSwapchainCreateInfoKHR_ARRAY;

#define freeswapchaincreateinfo moonvulkan_freeswapchaincreateinfo
void freeswapchaincreateinfo(lua_State *L, VkSwapchainCreateInfoKHR_CHAIN *pp);
#define echeckswapchaincreateinfo moonvulkan_echeckswapchaincreateinfo
int echeckswapchaincreateinfo(lua_State *L, int arg, VkSwapchainCreateInfoKHR_CHAIN *pp);
#define freeswapchaincreateinfoarray moonvulkan_freeswapchaincreateinfoarray
void freeswapchaincreateinfoarray(lua_State *L, VkSwapchainCreateInfoKHR_ARRAY *pp, uint32_t count);
#define echeckswapchaincreateinfoarray moonvulkan_echeckswapchaincreateinfoarray
int echeckswapchaincreateinfoarray(lua_State *L, int arg, VkSwapchainCreateInfoKHR_ARRAY *pp, uint32_t *count);

typedef struct {
    VkPresentInfoKHR p1;
    VkDisplayPresentInfoKHR p2;
    VkPresentRegionsKHR p3;
} VkPresentInfoKHR_CHAIN;

#define freepresentinfo moonvulkan_freepresentinfo
void freepresentinfo(lua_State *L, VkPresentInfoKHR_CHAIN *p);
#define echeckpresentinfo moonvulkan_echeckpresentinfo
int echeckpresentinfo(lua_State *L, int arg, VkPresentInfoKHR_CHAIN *pp, int results);

#define freewritedescriptorsetlist moonvulkan_freewritedescriptorsetlist
void freewritedescriptorsetlist(lua_State *L, void *list, uint32_t count);
#define echeckwritedescriptorsetlist moonvulkan_echeckwritedescriptorsetlist
VkWriteDescriptorSet* echeckwritedescriptorsetlist(lua_State *L, int arg, uint32_t *count, int *err);

#define freecopydescriptorsetlist moonvulkan_freecopydescriptorsetlist
void freecopydescriptorsetlist(lua_State *L, void *list, uint32_t count);
#define echeckcopydescriptorsetlist moonvulkan_echeckcopydescriptorsetlist
VkCopyDescriptorSet* echeckcopydescriptorsetlist(lua_State *L, int arg, uint32_t *count, int *err);

#define echeckdisplaypowerinfo moonvulkan_echeckdisplaypowerinfo
int echeckdisplaypowerinfo(lua_State *L, int arg, VkDisplayPowerInfoEXT *p);

#define pushdisplayproperties moonvulkan_pushdisplayproperties
int pushdisplayproperties(lua_State *L, VkDisplayPropertiesKHR *p);
#define pushdisplayplaneproperties moonvulkan_pushdisplayplaneproperties
int pushdisplayplaneproperties(lua_State *L, VkDisplayPlanePropertiesKHR *p);

#define pushmultisampleproperties moonvulkan_pushmultisampleproperties
int pushmultisampleproperties(lua_State *L, VkMultisamplePropertiesEXT *p);

#define echeckdisplaymodeparameters moonvulkan_echeckdisplaymodeparameters
int echeckdisplaymodeparameters(lua_State *L, int arg, VkDisplayModeParametersKHR *p);
#define pushdisplaymodeparameters moonvulkan_pushdisplaymodeparameters
int pushdisplaymodeparameters(lua_State *L, VkDisplayModeParametersKHR *p);
#define pushdisplaymodeproperties moonvulkan_pushdisplaymodeproperties
int pushdisplaymodeproperties(lua_State *L, VkDisplayModePropertiesKHR *p);
#define pushdisplayplanecapabilities moonvulkan_pushdisplayplanecapabilities
int pushdisplayplanecapabilities(lua_State *L, VkDisplayPlaneCapabilitiesKHR *p);
#define freedisplaysurfacecreateinfo moonvulkan_freedisplaysurfacecreateinfo
void freedisplaysurfacecreateinfo(lua_State *L, VkDisplaySurfaceCreateInfoKHR *p);
#define echeckdisplaysurfacecreateinfo moonvulkan_echeckdisplaysurfacecreateinfo
int echeckdisplaysurfacecreateinfo(lua_State *L, int arg, VkDisplaySurfaceCreateInfoKHR *p);

#define freedescriptorupdatetemplatecreateinfo moonvulkan_freedescriptorupdatetemplatecreateinfo
void freedescriptorupdatetemplatecreateinfo(lua_State *L, VkDescriptorUpdateTemplateCreateInfoKHR *p);
#define echeckdescriptorupdatetemplatecreateinfo moonvulkan_echeckdescriptorupdatetemplatecreateinfo
int echeckdescriptorupdatetemplatecreateinfo(lua_State *L, int arg, VkDescriptorUpdateTemplateCreateInfoKHR *p);

typedef struct {
    VkDeviceCreateInfo p1;
    VkPhysicalDeviceFeatures p2a;
    VkPhysicalDeviceFeatures2KHR_CHAIN p2b;
} VkDeviceCreateInfo_CHAIN;

#define freedevicecreateinfo moonvulkan_freedevicecreateinfo
void freedevicecreateinfo(lua_State *L, VkDeviceCreateInfo_CHAIN *p);
#define echeckdevicecreateinfo moonvulkan_echeckdevicecreateinfo
int echeckdevicecreateinfo(lua_State *L, int arg, VkDeviceCreateInfo_CHAIN *p, ud_t *ud);

typedef struct {
    VkInstanceCreateInfo p1;
    VkApplicationInfo appinfo;
    VkValidationFlagsEXT p2;
} VkInstanceCreateInfo_CHAIN;

#define freeinstancecreateinfo moonvulkan_freeinstancecreateinfo
void freeinstancecreateinfo(lua_State *L, VkInstanceCreateInfo_CHAIN *p);
#define echeckinstancecreateinfo moonvulkan_echeckinstancecreateinfo
int echeckinstancecreateinfo(lua_State *L, int arg, VkInstanceCreateInfo_CHAIN *p);

#define freebindbuffermemoryinfolist moonvulkan_freebindbuffermemoryinfolist
void freebindbuffermemoryinfolist(lua_State *L, void *list, uint32_t count);
#define echeckbindbuffermemoryinfolist moonvulkan_echeckbindbuffermemoryinfolist
VkBindBufferMemoryInfoKHR* echeckbindbuffermemoryinfolist(lua_State *L, int arg, uint32_t *count, int *err);

#define freebindimagememoryinfolist moonvulkan_freebindimagememoryinfolist
void freebindimagememoryinfolist(lua_State *L, void *list, uint32_t count);
#define echeckbindimagememoryinfolist moonvulkan_echeckbindimagememoryinfolist
VkBindImageMemoryInfoKHR* echeckbindimagememoryinfolist(lua_State *L, int arg, uint32_t *count, int *err);


#ifdef VK_USE_PLATFORM_WIN32_KHR

#define echeckimportsemaphorewin32handleinfo moonvulkan_echeckimportsemaphorewin32handleinfo
int echeckimportsemaphorewin32handleinfo(lua_State *L, int arg, VkImportSemaphoreWin32HandleInfoKHR *p);
#define echecksemaphoregetwin32handleinfo moonvulkan_echecksemaphoregetwin32handleinfo
int echecksemaphoregetwin32handleinfo(lua_State *L, int arg, VkSemaphoreGetWin32HandleInfoKHR *p);

#endif /* VK_USE_PLATFORM_WIN32_KHR */

#endif /* structsDEFINED */
