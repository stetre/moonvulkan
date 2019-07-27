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

#ifndef enumsDEFINED
#define enumsDEFINED

/* enums.c */
#define enums_free_all moonvulkan_enums_free_all
void enums_free_all(lua_State *L);
#define enums_test moonvulkan_enums_test
uint32_t enums_test(lua_State *L, uint32_t domain, int arg, int *err);
#define enums_check moonvulkan_enums_check
uint32_t enums_check(lua_State *L, uint32_t domain, int arg);
#define enums_push moonvulkan_enums_push
int enums_push(lua_State *L, uint32_t domain, uint32_t code);
#define enums_values moonvulkan_enums_values
int enums_values(lua_State *L, uint32_t domain);
#define enums_checklist moonvulkan_enums_checklist
uint32_t* enums_checklist(lua_State *L, uint32_t domain, int arg, uint32_t *count, int *err);
#define enums_freelist moonvulkan_enums_freelist
void enums_freelist(lua_State *L, uint32_t *list);


/* Enum domains */
#define DOMAIN_RESULT                           0
#define DOMAIN_PIPELINE_CACHE_HEADER_VERSION    1
#define DOMAIN_SYSTEM_ALLOCATION_SCOPE          2
#define DOMAIN_INTERNAL_ALLOCATION_TYPE         3
#define DOMAIN_FORMAT                           4
#define DOMAIN_IMAGE_TYPE                       5
#define DOMAIN_IMAGE_TILING                     6
#define DOMAIN_PHYSICAL_DEVICE_TYPE             7
#define DOMAIN_QUERY_TYPE                       8
#define DOMAIN_SHARING_MODE                     9
#define DOMAIN_IMAGE_LAYOUT                     10
#define DOMAIN_IMAGE_VIEW_TYPE                  11
#define DOMAIN_COMPONENT_SWIZZLE                12
#define DOMAIN_VERTEX_INPUT_RATE                13
#define DOMAIN_PRIMITIVE_TOPOLOGY               14
#define DOMAIN_POLYGON_MODE                     15
#define DOMAIN_FRONT_FACE                       16
#define DOMAIN_COMPARE_OP                       17
#define DOMAIN_STENCIL_OP                       18
#define DOMAIN_LOGIC_OP                         19
#define DOMAIN_BLEND_FACTOR                     20
#define DOMAIN_BLEND_OP                         21
#define DOMAIN_DYNAMIC_STATE                    22
#define DOMAIN_FILTER                           23
#define DOMAIN_SAMPLER_MIPMAP_MODE              24
#define DOMAIN_SAMPLER_ADDRESS_MODE             25
#define DOMAIN_BORDER_COLOR                     26
#define DOMAIN_DESCRIPTOR_TYPE                  27
#define DOMAIN_ATTACHMENT_LOAD_OP               28
#define DOMAIN_ATTACHMENT_STORE_OP              29
#define DOMAIN_PIPELINE_BIND_POINT              30
#define DOMAIN_COMMAND_BUFFER_LEVEL             31
#define DOMAIN_INDEX_TYPE                       32
#define DOMAIN_SUBPASS_CONTENTS                 33
#define DOMAIN_COLOR_SPACE                      34
#define DOMAIN_PRESENT_MODE                     35
#define DOMAIN_DEBUG_REPORT_OBJECT_TYPE         36
#define DOMAIN_DESCRIPTOR_UPDATE_TEMPLATE_TYPE  37
#define DOMAIN_VALIDATION_CHECK                 38
#define DOMAIN_DISPLAY_POWER_STATE              39
#define DOMAIN_DEVICE_EVENT_TYPE                40
#define DOMAIN_DISPLAY_EVENT_TYPE               41
#define DOMAIN_OBJECT_TYPE                      42
#define DOMAIN_BLEND_OVERLAP                    43
#define DOMAIN_SAMPLER_REDUCTION_MODE           44
#define DOMAIN_DISCARD_RECTANGLE_MODE           45
#define DOMAIN_POINT_CLIPPING_BEHAVIOR          46
#define DOMAIN_TESSELLATION_DOMAIN_ORIGIN       47
#define DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION   48
#define DOMAIN_SAMPLER_YCBCR_RANGE              49
#define DOMAIN_CHROMA_LOCATION                  50
#define DOMAIN_VALIDATION_CACHE_HEADER_VERSION  51
#define DOMAIN_QUEUE_GLOBAL_PRIORITY            52
#define DOMAIN_CONSERVATIVE_RASTERIZATION_MODE  53
#define DOMAIN_VENDOR_ID                        54
#define DOMAIN_DRIVER_ID                        55
#define DOMAIN_TIME_DOMAIN                      56
#define DOMAIN_VALIDATION_FEATURE_ENABLE        57
#define DOMAIN_VALIDATION_FEATURE_DISABLE       58
#define DOMAIN_FULL_SCREEN_EXCLUSIVE            59

/* NONVK additions */
#define DOMAIN_NONVK_TYPE                       101


/* Types for vk.sizeof() & friends */
#define NONVK_TYPE_INT8         1
#define NONVK_TYPE_UINT8        2
#define NONVK_TYPE_INT16        3
#define NONVK_TYPE_UINT16       4
#define NONVK_TYPE_INT32        5
#define NONVK_TYPE_UINT32       6
#define NONVK_TYPE_INT64        7
#define NONVK_TYPE_UINT64       8
#define NONVK_TYPE_BYTE         9
#define NONVK_TYPE_UBYTE        10
#define NONVK_TYPE_SHORT        11
#define NONVK_TYPE_USHORT       12
#define NONVK_TYPE_INT          13
#define NONVK_TYPE_UINT         14
#define NONVK_TYPE_LONG         15
#define NONVK_TYPE_ULONG        16
#define NONVK_TYPE_FLOAT        17
#define NONVK_TYPE_DOUBLE       18

#define testtype(L, arg, err) enums_test((L), DOMAIN_NONVK_TYPE, (arg), (err))
#define checktype(L, arg) enums_check((L), DOMAIN_NONVK_TYPE, (arg))
#define pushtype(L, val) enums_push((L), DOMAIN_NONVK_TYPE, (uint32_t)(val))
#define valuestype(L) enums_values((L), DOMAIN_NONVK_TYPE)


#define testresult(L, arg, err) (VkResult)enums_test((L), DOMAIN_RESULT, (arg), (err))
#define checkresult(L, arg) (VkResult)enums_check((L), DOMAIN_RESULT, (arg))
#define pushresult(L, val) enums_push((L), DOMAIN_RESULT, (uint32_t)(val))
#define valuesresult(L) enums_values((L), DOMAIN_RESULT)


#define testsubpasscontents(L, arg, err) (VkSubpassContents)enums_test((L), DOMAIN_SUBPASS_CONTENTS, (arg), (err))
#define checksubpasscontents(L, arg) (VkSubpassContents)enums_check((L), DOMAIN_SUBPASS_CONTENTS, (arg))
#define pushsubpasscontents(L, val) enums_push((L), DOMAIN_SUBPASS_CONTENTS, (uint32_t)(val))
#define valuessubpasscontents(L) enums_values((L), DOMAIN_SUBPASS_CONTENTS)

#define testindextype(L, arg, err) (VkIndexType)enums_test((L), DOMAIN_INDEX_TYPE, (arg), (err))
#define checkindextype(L, arg) (VkIndexType)enums_check((L), DOMAIN_INDEX_TYPE, (arg))
#define pushindextype(L, val) enums_push((L), DOMAIN_INDEX_TYPE, (uint32_t)(val))
#define valuesindextype(L) enums_values((L), DOMAIN_INDEX_TYPE)

#define testformat(L, arg, err) (VkFormat)enums_test((L), DOMAIN_FORMAT, (arg), (err))
#define checkformat(L, arg) (VkFormat)enums_check((L), DOMAIN_FORMAT, (arg))
#define pushformat(L, val) enums_push((L), DOMAIN_FORMAT, (uint32_t)(val))
#define valuesformat(L) enums_values((L), DOMAIN_FORMAT)
#define checkformatlist(L, arg, count, err) (VkFormat*)enums_checklist((L), DOMAIN_FORMAT, (arg), (count), (err))
#define freeformatlist(L, list) enums_freelist((L), (uint32_t*)(list))


#define testcommandbufferlevel(L, arg, err) (VkCommandBufferLevel)enums_test((L), DOMAIN_COMMAND_BUFFER_LEVEL, (arg), (err))
#define checkcommandbufferlevel(L, arg) (VkCommandBufferLevel)enums_check((L), DOMAIN_COMMAND_BUFFER_LEVEL, (arg))
#define pushcommandbufferlevel(L, val) enums_push((L), DOMAIN_COMMAND_BUFFER_LEVEL, (uint32_t)(val))
#define valuescommandbufferlevel(L) enums_values((L), DOMAIN_COMMAND_BUFFER_LEVEL)

#define testpipelinebindpoint(L, arg, err) (VkPipelineBindPoint)enums_test((L), DOMAIN_PIPELINE_BIND_POINT, (arg), (err))
#define checkpipelinebindpoint(L, arg) (VkPipelineBindPoint)enums_check((L), DOMAIN_PIPELINE_BIND_POINT, (arg))
#define pushpipelinebindpoint(L, val) enums_push((L), DOMAIN_PIPELINE_BIND_POINT, (uint32_t)(val))
#define valuespipelinebindpoint(L) enums_values((L), DOMAIN_PIPELINE_BIND_POINT)

#define testattachmentstoreop(L, arg, err) (VkAttachmentStoreOp)enums_test((L), DOMAIN_ATTACHMENT_STORE_OP, (arg), (err))
#define checkattachmentstoreop(L, arg) (VkAttachmentStoreOp)enums_check((L), DOMAIN_ATTACHMENT_STORE_OP, (arg))
#define pushattachmentstoreop(L, val) enums_push((L), DOMAIN_ATTACHMENT_STORE_OP, (uint32_t)(val))
#define valuesattachmentstoreop(L) enums_values((L), DOMAIN_ATTACHMENT_STORE_OP)

#define testattachmentloadop(L, arg, err) (VkAttachmentLoadOp)enums_test((L), DOMAIN_ATTACHMENT_LOAD_OP, (arg), (err))
#define checkattachmentloadop(L, arg) (VkAttachmentLoadOp)enums_check((L), DOMAIN_ATTACHMENT_LOAD_OP, (arg))
#define pushattachmentloadop(L, val) enums_push((L), DOMAIN_ATTACHMENT_LOAD_OP, (uint32_t)(val))
#define valuesattachmentloadop(L) enums_values((L), DOMAIN_ATTACHMENT_LOAD_OP)

#define testdescriptortype(L, arg, err) (VkDescriptorType)enums_test((L), DOMAIN_DESCRIPTOR_TYPE, (arg), (err))
#define checkdescriptortype(L, arg) (VkDescriptorType)enums_check((L), DOMAIN_DESCRIPTOR_TYPE, (arg))
#define pushdescriptortype(L, val) enums_push((L), DOMAIN_DESCRIPTOR_TYPE, (uint32_t)(val))
#define valuesdescriptortype(L) enums_values((L), DOMAIN_DESCRIPTOR_TYPE)

#define testbordercolor(L, arg, err) (VkBorderColor)enums_test((L), DOMAIN_BORDER_COLOR, (arg), (err))
#define checkbordercolor(L, arg) (VkBorderColor)enums_check((L), DOMAIN_BORDER_COLOR, (arg))
#define pushbordercolor(L, val) enums_push((L), DOMAIN_BORDER_COLOR, (uint32_t)(val))
#define valuesbordercolor(L) enums_values((L), DOMAIN_BORDER_COLOR)

#define testsampleraddressmode(L, arg, err) (VkSamplerAddressMode)enums_test((L), DOMAIN_SAMPLER_ADDRESS_MODE, (arg), (err))
#define checksampleraddressmode(L, arg) (VkSamplerAddressMode)enums_check((L), DOMAIN_SAMPLER_ADDRESS_MODE, (arg))
#define pushsampleraddressmode(L, val) enums_push((L), DOMAIN_SAMPLER_ADDRESS_MODE, (uint32_t)(val))
#define valuessampleraddressmode(L) enums_values((L), DOMAIN_SAMPLER_ADDRESS_MODE)

#define testsamplermipmapmode(L, arg, err) (VkSamplerMipmapMode)enums_test((L), DOMAIN_SAMPLER_MIPMAP_MODE, (arg), (err))
#define checksamplermipmapmode(L, arg) (VkSamplerMipmapMode)enums_check((L), DOMAIN_SAMPLER_MIPMAP_MODE, (arg))
#define pushsamplermipmapmode(L, val) enums_push((L), DOMAIN_SAMPLER_MIPMAP_MODE, (uint32_t)(val))
#define valuessamplermipmapmode(L) enums_values((L), DOMAIN_SAMPLER_MIPMAP_MODE)

#define testfilter(L, arg, err) (VkFilter)enums_test((L), DOMAIN_FILTER, (arg), (err))
#define checkfilter(L, arg) (VkFilter)enums_check((L), DOMAIN_FILTER, (arg))
#define pushfilter(L, val) enums_push((L), DOMAIN_FILTER, (uint32_t)(val))
#define valuesfilter(L) enums_values((L), DOMAIN_FILTER)

#define testdynamicstate(L, arg, err) (VkDynamicState)enums_test((L), DOMAIN_DYNAMIC_STATE, (arg), (err))
#define checkdynamicstate(L, arg) (VkDynamicState)enums_check((L), DOMAIN_DYNAMIC_STATE, (arg))
#define pushdynamicstate(L, val) enums_push((L), DOMAIN_DYNAMIC_STATE, (uint32_t)(val))
#define valuesdynamicstate(L) enums_values((L), DOMAIN_DYNAMIC_STATE)
#define checkdynamicstatelist(L, arg, count, err) (VkDynamicState*)enums_checklist((L), DOMAIN_DYNAMIC_STATE, (arg), (count), (err))
#define freedynamicstatelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testblendop(L, arg, err) (VkBlendOp)enums_test((L), DOMAIN_BLEND_OP, (arg), (err))
#define checkblendop(L, arg) (VkBlendOp)enums_check((L), DOMAIN_BLEND_OP, (arg))
#define pushblendop(L, val) enums_push((L), DOMAIN_BLEND_OP, (uint32_t)(val))
#define valuesblendop(L) enums_values((L), DOMAIN_BLEND_OP)

#define testblendfactor(L, arg, err) (VkBlendFactor)enums_test((L), DOMAIN_BLEND_FACTOR, (arg), (err))
#define checkblendfactor(L, arg) (VkBlendFactor)enums_check((L), DOMAIN_BLEND_FACTOR, (arg))
#define pushblendfactor(L, val) enums_push((L), DOMAIN_BLEND_FACTOR, (uint32_t)(val))
#define valuesblendfactor(L) enums_values((L), DOMAIN_BLEND_FACTOR)

#define testlogicop(L, arg, err) (VkLogicOp)enums_test((L), DOMAIN_LOGIC_OP, (arg), (err))
#define checklogicop(L, arg) (VkLogicOp)enums_check((L), DOMAIN_LOGIC_OP, (arg))
#define pushlogicop(L, val) enums_push((L), DOMAIN_LOGIC_OP, (uint32_t)(val))
#define valueslogicop(L) enums_values((L), DOMAIN_LOGIC_OP)

#define teststencilop(L, arg, err) (VkStencilOp)enums_test((L), DOMAIN_STENCIL_OP, (arg), (err))
#define checkstencilop(L, arg) (VkStencilOp)enums_check((L), DOMAIN_STENCIL_OP, (arg))
#define pushstencilop(L, val) enums_push((L), DOMAIN_STENCIL_OP, (uint32_t)(val))
#define valuesstencilop(L) enums_values((L), DOMAIN_STENCIL_OP)

#define testcompareop(L, arg, err) (VkCompareOp)enums_test((L), DOMAIN_COMPARE_OP, (arg), (err))
#define checkcompareop(L, arg) (VkCompareOp)enums_check((L), DOMAIN_COMPARE_OP, (arg))
#define pushcompareop(L, val) enums_push((L), DOMAIN_COMPARE_OP, (uint32_t)(val))
#define valuescompareop(L) enums_values((L), DOMAIN_COMPARE_OP)

#define testfrontface(L, arg, err) (VkFrontFace)enums_test((L), DOMAIN_FRONT_FACE, (arg), (err))
#define checkfrontface(L, arg) (VkFrontFace)enums_check((L), DOMAIN_FRONT_FACE, (arg))
#define pushfrontface(L, val) enums_push((L), DOMAIN_FRONT_FACE, (uint32_t)(val))
#define valuesfrontface(L) enums_values((L), DOMAIN_FRONT_FACE)

#define testpolygonmode(L, arg, err) (VkPolygonMode)enums_test((L), DOMAIN_POLYGON_MODE, (arg), (err))
#define checkpolygonmode(L, arg) (VkPolygonMode)enums_check((L), DOMAIN_POLYGON_MODE, (arg))
#define pushpolygonmode(L, val) enums_push((L), DOMAIN_POLYGON_MODE, (uint32_t)(val))
#define valuespolygonmode(L) enums_values((L), DOMAIN_POLYGON_MODE)

#define testprimitivetopology(L, arg, err) (VkPrimitiveTopology)enums_test((L), DOMAIN_PRIMITIVE_TOPOLOGY, (arg), (err))
#define checkprimitivetopology(L, arg) (VkPrimitiveTopology)enums_check((L), DOMAIN_PRIMITIVE_TOPOLOGY, (arg))
#define pushprimitivetopology(L, val) enums_push((L), DOMAIN_PRIMITIVE_TOPOLOGY, (uint32_t)(val))
#define valuesprimitivetopology(L) enums_values((L), DOMAIN_PRIMITIVE_TOPOLOGY)

#define testvertexinputrate(L, arg, err) (VkVertexInputRate)enums_test((L), DOMAIN_VERTEX_INPUT_RATE, (arg), (err))
#define checkvertexinputrate(L, arg) (VkVertexInputRate)enums_check((L), DOMAIN_VERTEX_INPUT_RATE, (arg))
#define pushvertexinputrate(L, val) enums_push((L), DOMAIN_VERTEX_INPUT_RATE, (uint32_t)(val))
#define valuesvertexinputrate(L) enums_values((L), DOMAIN_VERTEX_INPUT_RATE)

#define testcomponentswizzle(L, arg, err) (VkComponentSwizzle)enums_test((L), DOMAIN_COMPONENT_SWIZZLE, (arg), (err))
#define checkcomponentswizzle(L, arg) (VkComponentSwizzle)enums_check((L), DOMAIN_COMPONENT_SWIZZLE, (arg))
#define pushcomponentswizzle(L, val) enums_push((L), DOMAIN_COMPONENT_SWIZZLE, (uint32_t)(val))
#define valuescomponentswizzle(L) enums_values((L), DOMAIN_COMPONENT_SWIZZLE)

#define testimageviewtype(L, arg, err) (VkImageViewType)enums_test((L), DOMAIN_IMAGE_VIEW_TYPE, (arg), (err))
#define checkimageviewtype(L, arg) (VkImageViewType)enums_check((L), DOMAIN_IMAGE_VIEW_TYPE, (arg))
#define pushimageviewtype(L, val) enums_push((L), DOMAIN_IMAGE_VIEW_TYPE, (uint32_t)(val))
#define valuesimageviewtype(L) enums_values((L), DOMAIN_IMAGE_VIEW_TYPE)

#define testimagelayout(L, arg, err) (VkImageLayout)enums_test((L), DOMAIN_IMAGE_LAYOUT, (arg), (err))
#define checkimagelayout(L, arg) (VkImageLayout)enums_check((L), DOMAIN_IMAGE_LAYOUT, (arg))
#define pushimagelayout(L, val) enums_push((L), DOMAIN_IMAGE_LAYOUT, (uint32_t)(val))
#define valuesimagelayout(L) enums_values((L), DOMAIN_IMAGE_LAYOUT)

#define testsharingmode(L, arg, err) (VkSharingMode)enums_test((L), DOMAIN_SHARING_MODE, (arg), (err))
#define checksharingmode(L, arg) (VkSharingMode)enums_check((L), DOMAIN_SHARING_MODE, (arg))
#define pushsharingmode(L, val) enums_push((L), DOMAIN_SHARING_MODE, (uint32_t)(val))
#define valuessharingmode(L) enums_values((L), DOMAIN_SHARING_MODE)

#define testquerytype(L, arg, err) (VkQueryType)enums_test((L), DOMAIN_QUERY_TYPE, (arg), (err))
#define checkquerytype(L, arg) (VkQueryType)enums_check((L), DOMAIN_QUERY_TYPE, (arg))
#define pushquerytype(L, val) enums_push((L), DOMAIN_QUERY_TYPE, (uint32_t)(val))
#define valuesquerytype(L) enums_values((L), DOMAIN_QUERY_TYPE)

#define testphysicaldevicetype(L, arg, err) (VkPhysicalDeviceType)enums_test((L), DOMAIN_PHYSICAL_DEVICE_TYPE, (arg), (err))
#define checkphysicaldevicetype(L, arg) (VkPhysicalDeviceType)enums_check((L), DOMAIN_PHYSICAL_DEVICE_TYPE, (arg))
#define pushphysicaldevicetype(L, val) enums_push((L), DOMAIN_PHYSICAL_DEVICE_TYPE, (uint32_t)(val))
#define valuesphysicaldevicetype(L) enums_values((L), DOMAIN_PHYSICAL_DEVICE_TYPE)

#define testimagetiling(L, arg, err) (VkImageTiling)enums_test((L), DOMAIN_IMAGE_TILING, (arg), (err))
#define checkimagetiling(L, arg) (VkImageTiling)enums_check((L), DOMAIN_IMAGE_TILING, (arg))
#define pushimagetiling(L, val) enums_push((L), DOMAIN_IMAGE_TILING, (uint32_t)(val))
#define valuesimagetiling(L) enums_values((L), DOMAIN_IMAGE_TILING)

#define testimagetype(L, arg, err) (VkImageType)enums_test((L), DOMAIN_IMAGE_TYPE, (arg), (err))
#define checkimagetype(L, arg) (VkImageType)enums_check((L), DOMAIN_IMAGE_TYPE, (arg))
#define pushimagetype(L, val) enums_push((L), DOMAIN_IMAGE_TYPE, (uint32_t)(val))
#define valuesimagetype(L) enums_values((L), DOMAIN_IMAGE_TYPE)

#define testpresentmode(L, arg, err) (VkPresentModeKHR)enums_test((L), DOMAIN_PRESENT_MODE, (arg), (err))
#define checkpresentmode(L, arg) (VkPresentModeKHR)enums_check((L), DOMAIN_PRESENT_MODE, (arg))
#define pushpresentmode(L, val) enums_push((L), DOMAIN_PRESENT_MODE, (uint32_t)(val))
#define valuespresentmode(L) enums_values((L), DOMAIN_PRESENT_MODE)

#define testcolorspace(L, arg, err) (VkColorSpaceKHR)enums_test((L), DOMAIN_COLOR_SPACE, (arg), (err))
#define checkcolorspace(L, arg) (VkColorSpaceKHR)enums_check((L), DOMAIN_COLOR_SPACE, (arg))
#define pushcolorspace(L, val) enums_push((L), DOMAIN_COLOR_SPACE, (uint32_t)(val))
#define valuescolorspace(L) enums_values((L), DOMAIN_COLOR_SPACE)

#define testdebugreportobjecttype(L, arg, err) (VkDebugReportObjectTypeEXT)enums_test((L), DOMAIN_DEBUG_REPORT_OBJECT_TYPE, (arg), (err))
#define checkdebugreportobjecttype(L, arg) (VkDebugReportObjectTypeEXT)enums_check((L), DOMAIN_DEBUG_REPORT_OBJECT_TYPE, (arg))
#define pushdebugreportobjecttype(L, val) enums_push((L), DOMAIN_DEBUG_REPORT_OBJECT_TYPE, (uint32_t)(val))
#define valuesdebugreportobjecttype(L) enums_values((L), DOMAIN_DEBUG_REPORT_OBJECT_TYPE)

#define testdescriptorupdatetemplatetype(L, arg, err) (VkDescriptorUpdateTemplateType)enums_test((L), DOMAIN_DESCRIPTOR_UPDATE_TEMPLATE_TYPE, (arg), (err))
#define checkdescriptorupdatetemplatetype(L, arg) (VkDescriptorUpdateTemplateType)enums_check((L), DOMAIN_DESCRIPTOR_UPDATE_TEMPLATE_TYPE, (arg))
#define pushdescriptorupdatetemplatetype(L, val) enums_push((L), DOMAIN_DESCRIPTOR_UPDATE_TEMPLATE_TYPE, (uint32_t)(val))
#define valuesdescriptorupdatetemplatetype(L) enums_values((L), DOMAIN_DESCRIPTOR_UPDATE_TEMPLATE_TYPE)

#define testobjecttype(L, arg, err) (VkObjectType)enums_test((L), DOMAIN_OBJECT_TYPE, (arg), (err))
#define checkobjecttype(L, arg) (VkObjectType)enums_check((L), DOMAIN_OBJECT_TYPE, (arg))
#define pushobjecttype(L, val) enums_push((L), DOMAIN_OBJECT_TYPE, (uint32_t)(val))
#define valuesobjecttype(L) enums_values((L), DOMAIN_OBJECT_TYPE)
#define checkobjecttypelist(L, arg, count, err) (VkObjectType*)enums_checklist((L), DOMAIN_OBJECT_TYPE, (arg), (count), (err))
#define freeobjecttypelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testblendoverlap(L, arg, err) (VkBlendOverlapEXT)enums_test((L), DOMAIN_BLEND_OVERLAP, (arg), (err))
#define checkblendoverlap(L, arg) (VkBlendOverlapEXT)enums_check((L), DOMAIN_BLEND_OVERLAP, (arg))
#define pushblendoverlap(L, val) enums_push((L), DOMAIN_BLEND_OVERLAP, (uint32_t)(val))
#define valuesblendoverlap(L) enums_values((L), DOMAIN_BLEND_OVERLAP)

#define testsamplerreductionmode(L, arg, err) (VkSamplerReductionModeEXT)enums_test((L), DOMAIN_SAMPLER_REDUCTION_MODE, (arg), (err))
#define checksamplerreductionmode(L, arg) (VkSamplerReductionModeEXT)enums_check((L), DOMAIN_SAMPLER_REDUCTION_MODE, (arg))
#define pushsamplerreductionmode(L, val) enums_push((L), DOMAIN_SAMPLER_REDUCTION_MODE, (uint32_t)(val))
#define valuessamplerreductionmode(L) enums_values((L), DOMAIN_SAMPLER_REDUCTION_MODE)

#define testvalidationcheck(L, arg, err) (VkValidationCheckEXT)enums_test((L), DOMAIN_VALIDATION_CHECK, (arg), (err))
#define checkvalidationcheck(L, arg) (VkValidationCheckEXT)enums_check((L), DOMAIN_VALIDATION_CHECK, (arg))
#define pushvalidationcheck(L, val) enums_push((L), DOMAIN_VALIDATION_CHECK, (uint32_t)(val))
#define valuesvalidationcheck(L) enums_values((L), DOMAIN_VALIDATION_CHECK)
#define checkvalidationchecklist(L, arg, count, err) (VkValidationCheckEXT*)enums_checklist((L), DOMAIN_VALIDATION_CHECK, (arg), (count), (err))
#define freevalidationchecklist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testdiscardrectanglemode(L, arg, err) (VkDiscardRectangleModeEXT)enums_test((L), DOMAIN_DISCARD_RECTANGLE_MODE, (arg), (err))
#define checkdiscardrectanglemode(L, arg) (VkDiscardRectangleModeEXT)enums_check((L), DOMAIN_DISCARD_RECTANGLE_MODE, (arg))
#define pushdiscardrectanglemode(L, val) enums_push((L), DOMAIN_DISCARD_RECTANGLE_MODE, (uint32_t)(val))
#define valuesdiscardrectanglemode(L) enums_values((L), DOMAIN_DISCARD_RECTANGLE_MODE)
#define checkdiscardrectanglemodelist(L, arg, count, err) (VkDiscardRectangleModeEXT*)enums_checklist((L), DOMAIN_DISCARD_RECTANGLE_MODE, (arg), (count), (err))
#define freediscardrectanglemodelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testdisplaypowerstate(L, arg, err) (VkDisplayPowerStateEXT)enums_test((L), DOMAIN_DISPLAY_POWER_STATE, (arg), (err))
#define checkdisplaypowerstate(L, arg) (VkDisplayPowerStateEXT)enums_check((L), DOMAIN_DISPLAY_POWER_STATE, (arg))
#define pushdisplaypowerstate(L, val) enums_push((L), DOMAIN_DISPLAY_POWER_STATE, (uint32_t)(val))
#define valuesdisplaypowerstate(L) enums_values((L), DOMAIN_DISPLAY_POWER_STATE)
#define checkdisplaypowerstatelist(L, arg, count, err) (VkDisplayPowerStateEXT*)enums_checklist((L), DOMAIN_DISPLAY_POWER_STATE, (arg), (count), (err))
#define freedisplaypowerstatelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testdeviceeventtype(L, arg, err) (VkDeviceEventTypeEXT)enums_test((L), DOMAIN_DEVICE_EVENT_TYPE, (arg), (err))
#define checkdeviceeventtype(L, arg) (VkDeviceEventTypeEXT)enums_check((L), DOMAIN_DEVICE_EVENT_TYPE, (arg))
#define pushdeviceeventtype(L, val) enums_push((L), DOMAIN_DEVICE_EVENT_TYPE, (uint32_t)(val))
#define valuesdeviceeventtype(L) enums_values((L), DOMAIN_DEVICE_EVENT_TYPE)
#define checkdeviceeventtypelist(L, arg, count, err) (VkDeviceEventTypeEXT*)enums_checklist((L), DOMAIN_DEVICE_EVENT_TYPE, (arg), (count), (err))
#define freedeviceeventtypelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testdisplayeventtype(L, arg, err) (VkDisplayEventTypeEXT)enums_test((L), DOMAIN_DISPLAY_EVENT_TYPE, (arg), (err))
#define checkdisplayeventtype(L, arg) (VkDisplayEventTypeEXT)enums_check((L), DOMAIN_DISPLAY_EVENT_TYPE, (arg))
#define pushdisplayeventtype(L, val) enums_push((L), DOMAIN_DISPLAY_EVENT_TYPE, (uint32_t)(val))
#define valuesdisplayeventtype(L) enums_values((L), DOMAIN_DISPLAY_EVENT_TYPE)
#define checkdisplayeventtypelist(L, arg, count, err) (VkDisplayEventTypeEXT*)enums_checklist((L), DOMAIN_DISPLAY_EVENT_TYPE, (arg), (count), (err))
#define freedisplayeventtypelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testpointclippingbehavior(L, arg, err) (VkPointClippingBehavior)enums_test((L), DOMAIN_POINT_CLIPPING_BEHAVIOR, (arg), (err))
#define checkpointclippingbehavior(L, arg) (VkPointClippingBehavior)enums_check((L), DOMAIN_POINT_CLIPPING_BEHAVIOR, (arg))
#define pushpointclippingbehavior(L, val) enums_push((L), DOMAIN_POINT_CLIPPING_BEHAVIOR, (uint32_t)(val))
#define valuespointclippingbehavior(L) enums_values((L), DOMAIN_POINT_CLIPPING_BEHAVIOR)
#define checkpointclippingbehaviorlist(L, arg, count, err) (VkPointClippingBehavior*)enums_checklist((L), DOMAIN_POINT_CLIPPING_BEHAVIOR, (arg), (count), (err))
#define freepointclippingbehaviorlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testtessellationdomainorigin(L, arg, err) (VkTessellationDomainOrigin)enums_test((L), DOMAIN_TESSELLATION_DOMAIN_ORIGIN, (arg), (err))
#define checktessellationdomainorigin(L, arg) (VkTessellationDomainOrigin)enums_check((L), DOMAIN_TESSELLATION_DOMAIN_ORIGIN, (arg))
#define pushtessellationdomainorigin(L, val) enums_push((L), DOMAIN_TESSELLATION_DOMAIN_ORIGIN, (uint32_t)(val))
#define valuestessellationdomainorigin(L) enums_values((L), DOMAIN_TESSELLATION_DOMAIN_ORIGIN)
#define checktessellationdomainoriginlist(L, arg, count, err) (VkTessellationDomainOrigin*)enums_checklist((L), DOMAIN_TESSELLATION_DOMAIN_ORIGIN, (arg), (count), (err))
#define freetessellationdomainoriginlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testsamplerycbcrmodelconversion(L, arg, err) (VkSamplerYcbcrModelConversion)enums_test((L), DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION, (arg), (err))
#define checksamplerycbcrmodelconversion(L, arg) (VkSamplerYcbcrModelConversion)enums_check((L), DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION, (arg))
#define pushsamplerycbcrmodelconversion(L, val) enums_push((L), DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION, (uint32_t)(val))
#define valuessamplerycbcrmodelconversion(L) enums_values((L), DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION)
#define checksamplerycbcrmodelconversionlist(L, arg, count, err) (VkSamplerYcbcrModelConversion*)enums_checklist((L), DOMAIN_SAMPLER_YCBCR_MODEL_CONVERSION, (arg), (count), (err))
#define freesamplerycbcrmodelconversionlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testsamplerycbcrrange(L, arg, err) (VkSamplerYcbcrRange)enums_test((L), DOMAIN_SAMPLER_YCBCR_RANGE, (arg), (err))
#define checksamplerycbcrrange(L, arg) (VkSamplerYcbcrRange)enums_check((L), DOMAIN_SAMPLER_YCBCR_RANGE, (arg))
#define pushsamplerycbcrrange(L, val) enums_push((L), DOMAIN_SAMPLER_YCBCR_RANGE, (uint32_t)(val))
#define valuessamplerycbcrrange(L) enums_values((L), DOMAIN_SAMPLER_YCBCR_RANGE)
#define checksamplerycbcrrangelist(L, arg, count, err) (VkSamplerYcbcrRange*)enums_checklist((L), DOMAIN_SAMPLER_YCBCR_RANGE, (arg), (count), (err))
#define freesamplerycbcrrangelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testchromalocation(L, arg, err) (VkChromaLocation)enums_test((L), DOMAIN_CHROMA_LOCATION, (arg), (err))
#define checkchromalocation(L, arg) (VkChromaLocation)enums_check((L), DOMAIN_CHROMA_LOCATION, (arg))
#define pushchromalocation(L, val) enums_push((L), DOMAIN_CHROMA_LOCATION, (uint32_t)(val))
#define valueschromalocation(L) enums_values((L), DOMAIN_CHROMA_LOCATION)
#define checkchromalocationlist(L, arg, count, err) (VkChromaLocation*)enums_checklist((L), DOMAIN_CHROMA_LOCATION, (arg), (count), (err))
#define freechromalocationlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testvalidationcacheheaderversion(L, arg, err) (VkValidationCacheHeaderVersionEXT)enums_test((L), DOMAIN_VALIDATION_CACHE_HEADER_VERSION, (arg), (err))
#define checkvalidationcacheheaderversion(L, arg) (VkValidationCacheHeaderVersionEXT)enums_check((L), DOMAIN_VALIDATION_CACHE_HEADER_VERSION, (arg))
#define pushvalidationcacheheaderversion(L, val) enums_push((L), DOMAIN_VALIDATION_CACHE_HEADER_VERSION, (uint32_t)(val))
#define valuesvalidationcacheheaderversion(L) enums_values((L), DOMAIN_VALIDATION_CACHE_HEADER_VERSION)
#define checkvalidationcacheheaderversionlist(L, arg, count, err) (VkValidationCacheHeaderVersionEXT*)enums_checklist((L), DOMAIN_VALIDATION_CACHE_HEADER_VERSION, (arg), (count), (err))
#define freevalidationcacheheaderversionlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testqueueglobalpriority(L, arg, err) (VkQueueGlobalPriorityEXT)enums_test((L), DOMAIN_QUEUE_GLOBAL_PRIORITY, (arg), (err))
#define checkqueueglobalpriority(L, arg) (VkQueueGlobalPriorityEXT)enums_check((L), DOMAIN_QUEUE_GLOBAL_PRIORITY, (arg))
#define pushqueueglobalpriority(L, val) enums_push((L), DOMAIN_QUEUE_GLOBAL_PRIORITY, (uint32_t)(val))
#define valuesqueueglobalpriority(L) enums_values((L), DOMAIN_QUEUE_GLOBAL_PRIORITY)
#define checkqueueglobalprioritylist(L, arg, count, err) (VkQueueGlobalPriorityEXT*)enums_checklist((L), DOMAIN_QUEUE_GLOBAL_PRIORITY, (arg), (count), (err))
#define freequeueglobalprioritylist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testconservativerasterizationmode(L, arg, err) (VkConservativeRasterizationModeEXT)enums_test((L), DOMAIN_CONSERVATIVE_RASTERIZATION_MODE, (arg), (err))
#define checkconservativerasterizationmode(L, arg) (VkConservativeRasterizationModeEXT)enums_check((L), DOMAIN_CONSERVATIVE_RASTERIZATION_MODE, (arg))
#define pushconservativerasterizationmode(L, val) enums_push((L), DOMAIN_CONSERVATIVE_RASTERIZATION_MODE, (uint32_t)(val))
#define valuesconservativerasterizationmode(L) enums_values((L), DOMAIN_CONSERVATIVE_RASTERIZATION_MODE)
#define checkconservativerasterizationmodelist(L, arg, count, err) (VkConservativeRasterizationModeEXT*)enums_checklist((L), DOMAIN_CONSERVATIVE_RASTERIZATION_MODE, (arg), (count), (err))
#define freeconservativerasterizationmodelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testvendorid(L, arg, err) (VkVendorId)enums_test((L), DOMAIN_VENDOR_ID, (arg), (err))
#define checkvendorid(L, arg) (VkVendorId)enums_check((L), DOMAIN_VENDOR_ID, (arg))
#define pushvendorid(L, val) enums_push((L), DOMAIN_VENDOR_ID, (uint32_t)(val))
#define valuesvendorid(L) enums_values((L), DOMAIN_VENDOR_ID)
#define checkvendoridlist(L, arg, count, err) (VkVendorId*)enums_checklist((L), DOMAIN_VENDOR_ID, (arg), (count), (err))
#define freevendoridlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testdriverid(L, arg, err) (VkDriverIdKHR)enums_test((L), DOMAIN_DRIVER_ID, (arg), (err))
#define checkdriverid(L, arg) (VkDriverIdKHR)enums_check((L), DOMAIN_DRIVER_ID, (arg))
#define pushdriverid(L, val) enums_push((L), DOMAIN_DRIVER_ID, (uint32_t)(val))
#define valuesdriverid(L) enums_values((L), DOMAIN_DRIVER_ID)
#define checkdriveridlist(L, arg, count, err) (VkDriverIdKHR*)enums_checklist((L), DOMAIN_DRIVER_ID, (arg), (count), (err))
#define freedriveridlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testtimedomain(L, arg, err) (VkTimeDomainEXT)enums_test((L), DOMAIN_TIME_DOMAIN, (arg), (err))
#define checktimedomain(L, arg) (VkTimeDomainEXT)enums_check((L), DOMAIN_TIME_DOMAIN, (arg))
#define pushtimedomain(L, val) enums_push((L), DOMAIN_TIME_DOMAIN, (uint32_t)(val))
#define valuestimedomain(L) enums_values((L), DOMAIN_TIME_DOMAIN)
#define checktimedomainlist(L, arg, count, err) (VkTimeDomainEXT*)enums_checklist((L), DOMAIN_TIME_DOMAIN, (arg), (count), (err))
#define freetimedomainlist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testvalidationfeatureenable(L, arg, err) (VkValidationFeatureEnableEXT)enums_test((L), DOMAIN_VALIDATION_FEATURE_ENABLE, (arg), (err))
#define checkvalidationfeatureenable(L, arg) (VkValidationFeatureEnableEXT)enums_check((L), DOMAIN_VALIDATION_FEATURE_ENABLE, (arg))
#define pushvalidationfeatureenable(L, val) enums_push((L), DOMAIN_VALIDATION_FEATURE_ENABLE, (uint32_t)(val))
#define valuesvalidationfeatureenable(L) enums_values((L), DOMAIN_VALIDATION_FEATURE_ENABLE)
#define checkvalidationfeatureenablelist(L, arg, count, err) (VkValidationFeatureEnableEXT*)enums_checklist((L), DOMAIN_VALIDATION_FEATURE_ENABLE, (arg), (count), (err))
#define freevalidationfeatureenablelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testvalidationfeaturedisable(L, arg, err) (VkValidationFeatureDisableEXT)enums_test((L), DOMAIN_VALIDATION_FEATURE_DISABLE, (arg), (err))
#define checkvalidationfeaturedisable(L, arg) (VkValidationFeatureDisableEXT)enums_check((L), DOMAIN_VALIDATION_FEATURE_DISABLE, (arg))
#define pushvalidationfeaturedisable(L, val) enums_push((L), DOMAIN_VALIDATION_FEATURE_DISABLE, (uint32_t)(val))
#define valuesvalidationfeaturedisable(L) enums_values((L), DOMAIN_VALIDATION_FEATURE_DISABLE)
#define checkvalidationfeaturedisablelist(L, arg, count, err) (VkValidationFeatureDisableEXT*)enums_checklist((L), DOMAIN_VALIDATION_FEATURE_DISABLE, (arg), (count), (err))
#define freevalidationfeaturedisablelist(L, list) enums_freelist((L), (uint32_t*)(list))

#define testfullscreenexclusive(L, arg, err) (VkFullScreenExclusiveEXT)enums_test((L), DOMAIN_FULL_SCREEN_EXCLUSIVE, (arg), (err))
#define checkfullscreenexclusive(L, arg) (VkFullScreenExclusiveEXT)enums_check((L), DOMAIN_FULL_SCREEN_EXCLUSIVE, (arg))
#define pushfullscreenexclusive(L, val) enums_push((L), DOMAIN_FULL_SCREEN_EXCLUSIVE, (uint32_t)(val))
#define valuesfullscreenexclusive(L) enums_values((L), DOMAIN_FULL_SCREEN_EXCLUSIVE)
#define checkfullscreenexclusivelist(L, arg, count, err) (VkFullScreenExclusiveEXT*)enums_checklist((L), DOMAIN_FULL_SCREEN_EXCLUSIVE, (arg), (count), (err))
#define freefullscreenexclusivelist(L, list) enums_freelist((L), (uint32_t*)(list))

#if 0 /* scaffolding 7yy */
#define testxxx(L, arg, err) (VkXxx)enums_test((L), DOMAIN_XXX, (arg), (err))
#define checkxxx(L, arg) (VkXxx)enums_check((L), DOMAIN_XXX, (arg))
#define pushxxx(L, val) enums_push((L), DOMAIN_XXX, (uint32_t)(val))
#define valuesxxx(L) enums_values((L), DOMAIN_XXX)
#define checkxxxlist(L, arg, count, err) (VkXxx*)enums_checklist((L), DOMAIN_XXX, (arg), (count), (err))
#define freexxxlist(L, list) enums_freelist((L), (uint32_t*)(list))
    CASE(xxx);
#endif

#endif /* enumsDEFINED */

