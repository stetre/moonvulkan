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

#ifndef objectsDEFINED
#define objectsDEFINED

#include "tree.h"
#include "udata.h"

/* Objects' metatable names */
#define INSTANCE_MT "moonvulkan_instance"
#define PHYSICAL_DEVICE_MT "moonvulkan_physical_device"
#define DEVICE_MT "moonvulkan_device"
#define QUEUE_MT "moonvulkan_queue"
#define COMMAND_POOL_MT "moonvulkan_command_pool"
#define COMMAND_BUFFER_MT "moonvulkan_command_buffer"
#define SEMAPHORE_MT "moonvulkan_semaphore"
#define FENCE_MT "moonvulkan_fence"
#define BUFFER_MT "moonvulkan_buffer"
#define DEVICE_MEMORY_MT "moonvulkan_device_memory"
#define IMAGE_MT "moonvulkan_image"
#define EVENT_MT "moonvulkan_event"
#define QUERY_POOL_MT "moonvulkan_query_pool"
#define BUFFER_VIEW_MT "moonvulkan_buffer_view"
#define IMAGE_VIEW_MT "moonvulkan_image_view"
#define SHADER_MODULE_MT "moonvulkan_shader_module"
#define PIPELINE_CACHE_MT "moonvulkan_pipeline_cache"
#define PIPELINE_MT "moonvulkan_pipeline"
#define PIPELINE_LAYOUT_MT "moonvulkan_pipeline_layout"
#define SAMPLER_MT "moonvulkan_sampler"
#define DESCRIPTOR_SET_LAYOUT_MT "moonvulkan_descriptor_set_layout"
#define DESCRIPTOR_POOL_MT "moonvulkan_descriptor_pool"
#define DESCRIPTOR_SET_MT "moonvulkan_descriptor_set"
#define FRAMEBUFFER_MT "moonvulkan_framebuffer"
#define RENDER_PASS_MT "moonvulkan_render_pass"
/* extensions objects */
#define SURFACE_MT "moonvulkan_surface" /* KHR */
#define SWAPCHAIN_MT "moonvulkan_swapchain" /* KHR */
#define DISPLAY_MT "moonvulkan_display" /* KHR */
#define DISPLAY_MODE_MT "moonvulkan_display_mode" /* KHR */
#define DEBUG_REPORT_CALLBACK_MT "moonvulkan_debug_report_callback" /* EXT */
#define DESCRIPTOR_UPDATE_TEMPLATE_MT "moonvulkan_descriptor_update_template"
#define VALIDATION_CACHE_MT "moonvulkan_validation_cache" /* EXT */
#define SAMPLER_YCBCR_CONVERSION_MT "moonvulkan_sampler_ycbcr_conversion"
#define DEBUG_UTILS_MESSENGER_MT "moonvulkan_debug_utils_messenger" /* EXT */

/* Userdata memory associated with objects */
#define ud_t moonvulkan_ud_t
typedef struct moonvulkan_ud_s ud_t;

struct moonvulkan_ud_s {
    uint64_t handle; /* the object handle bound to this userdata (see NOTE1 below) */
    int (*destructor)(lua_State *L, ud_t *ud);  /* self destructor */
    ud_t *parent_ud; /* the ud of the parent object */
    VkInstance instance; /* the instance it belongs to */
    VkDevice device; /* the device it belongs to */
    uint32_t marks;
    int ref1, ref2, ref3, ref4; /* references for callbacks, automatically unreference at deletion */
    const VkAllocationCallbacks* allocator; /* optional allocator */
    instance_dt_t *idt; /* instance dispatch table */
    device_dt_t *ddt; /* device dispatch table */
    void *info; /* object specific info (ud_info_t, subject to Free() at destruction, if not NULL) */
};
    
/* NOTE1: ud->handle is an uint64_t both for dispatchable and non-dispatchable
 *        objects, but for dispatchable objects it actually holds a pointer, 
 *        which may be 32-bit or 64-bit long depending on the system we are on.
 *        Independently of the system, it is safe to cast ud->handle to a pointer,
 *        but on a 32-bit system the gcc compiler issues a pointer-to-int warning
 *        (correctly, because we know that it is really a pointer but gcc does not
 *        know it).
 *        The trick to avoid the warning without disabling the check (which may
 *        be useful in other situations), is to first cast handle to uintptr_t, and
 *        then to the actual pointer type, eg:
 *        VkInstance instance = (VkInstance)ud->handle;            --> warning
 *        VkInstance instance = (VkInstance)(uintptr_t)ud->handle; --> no warning
 *        (VkInstance is a pointer: see vulkan.h)
 *
 *        Similarly, when we store a pointer to ud->handle we get a warning, which
 *        we can also avoid by casting the pointer to uintptr_t first, eg:
 *        ud->handle = (uint64_t)(uintptr_t)instance;
 *
 *        (Fortunately, few objects are dispatchable so we do not need to use this
 *        tick very often).
 */


/* Marks.  m_ = marks word (uint32_t) , i_ = bit number (0 .. 31)  */
#define MarkGet(m_,i_)  (((m_) & ((uint32_t)1<<(i_))) == ((uint32_t)1<<(i_)))
#define MarkSet(m_,i_)  do { (m_) = ((m_) | ((uint32_t)1<<(i_))); } while(0)
#define MarkReset(m_,i_) do { (m_) = ((m_) & (~((uint32_t)1<<(i_)))); } while(0)

#define IsValid(ud)             MarkGet((ud)->marks, 0)
#define MarkValid(ud)           MarkSet((ud)->marks, 0) 
#define CancelValid(ud)         MarkReset((ud)->marks, 0)

#define IsDispatchable(ud)      MarkGet((ud)->marks, 1)
#define MarkDispatchable(ud)    MarkSet((ud)->marks, 1) 
#define CancelDispatchable(ud)  MarkReset((ud)->marks, 1)

#define IsBorrowed(ud)          MarkGet((ud)->marks, 2)
#define MarkBorrowed(ud)        MarkSet((ud)->marks, 2) 
#define CancelBorrowed(ud)      MarkReset((ud)->marks, 2)

#define IsFreeDescriptorSetAllowed(ud)  MarkGet((ud)->marks, 3) /* descriptor_pool only */
#define MarkFreeDescriptorSetAllowed(ud) MarkSet((ud)->marks, 3) 
#define CancelFreeDescriptorSetAllowed(ud) MarkReset((ud)->marks, 3)

#if 0
/* .c */
#define  moonvulkan_
#endif

#define newuserdata moonvulkan_newuserdata
ud_t *newuserdata(lua_State *L, uint64_t handle, const char *mt, int dispatchable);
#define newuserdata_dispatchable(L, handle, mt) newuserdata((L), (uint64_t)(uintptr_t)(handle), (mt), 1)
#define newuserdata_nondispatchable(L, handle, mt) newuserdata((L), (uint64_t)(handle), (mt), 0)
#define freeuserdata moonvulkan_freeuserdata
int freeuserdata(lua_State *L, ud_t *ud);
#define pushuserdata moonvulkan_pushuserdata 
int pushuserdata(lua_State *L, ud_t *ud);
#define pushnondispatchable moonvulkan_pushnondispatchable
int pushnondispatchable(lua_State *L, uint64_t handle, ud_t *parent_ud, const char *mt);
#define checkxxxlist_dispatchable moonvulkan_checkxxxlist_dispatchable
void** checkxxxlist_dispatchable(lua_State *L, int arg, uint32_t *count, int *err, const char *mt);
#define checkxxxlist_nondispatchable moonvulkan_checkxxxlist_nondispatchable
uint64_t* checkxxxlist_nondispatchable(lua_State *L, int arg, uint32_t *count, int *err, ud_t ***ud, const char *mt);

#define userdata_unref(L, handle) udata_unref((L),(handle))

#define UD(handle) userdata((handle)) /* dispatchable objects only */
#define userdata moonvulkan_userdata
ud_t *userdata(void *handle);
#define testxxx moonvulkan_testxxx
uint64_t testxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define checkxxx moonvulkan_checkxxx
uint64_t checkxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define pushxxx moonvulkan_pushxxx
int pushxxx(lua_State *L, uint64_t handle); /* dispatchable objects only */
/* NOTE: nondispatchable objects' handles are not guaranteed to be unique, so we
 *       can not use an handle to retrieve the Lua userdata, which is 'pushed' 
 *       only at creation.
 *       That is, nondispatchable obiects are also non-pushable (if we think about
 *       it, it is the same thing, because we can see MoonVulkan as a 'layer' which
 *       is placed above Vulkan insteda of below). 
 */ 

#define freechildren moonvulkan_freechildren
int freechildren(lua_State *L,  const char *mt, ud_t *parent_ud);

/* instance.c (dispatchable) */
#define checkinstance(L, arg, udp) (VkInstance)(uintptr_t)checkxxx((L), (arg), (udp), INSTANCE_MT)
#define testinstance(L, arg, udp) (VkInstance)(uintptr_t)testxxx((L), (arg), (udp), INSTANCE_MT)
#define pushinstance(L, handle) pushxxx((L), (uint64_t)(uintptr_t)(handle))

/* physical_device.c (dispatchable) */
#define checkphysical_device(L, arg, udp) (VkPhysicalDevice)(uintptr_t)checkxxx((L), (arg), (udp), PHYSICAL_DEVICE_MT)
#define testphysical_device(L, arg, udp) (VkPhysicalDevice)(uintptr_t)testxxx((L), (arg), (udp), PHYSICAL_DEVICE_MT)
#define pushphysical_device moonvulkan_pushphysical_device
int pushphysical_device(lua_State *L, VkPhysicalDevice p, VkInstance instance);
#define checkphysical_devicelist(L, arg, count, err, notused) \
    (VkPhysicalDevice*)checkxxxlist_dispatchable((L), (arg), (count), (err), PHYSICAL_DEVICE_MT)

/* device.c (dispatchable) */
#define checkdevice(L, arg, udp) (VkDevice)(uintptr_t)checkxxx((L), (arg), (udp), DEVICE_MT)
#define testdevice(L, arg, udp) (VkDevice)(uintptr_t)testxxx((L), (arg), (udp), DEVICE_MT)
#define pushdevice(L, handle) pushxxx((L), (uint64_t)(uintptr_t)(handle))

/* command_buffer.c (dispatchable) */
#define checkcommand_buffer(L, arg, udp) (VkCommandBuffer)(uintptr_t)checkxxx((L), (arg), (udp), COMMAND_BUFFER_MT)
#define testcommand_buffer(L, arg, udp) (VkCommandBuffer)(uintptr_t)testxxx((L), (arg), (udp), COMMAND_BUFFER_MT)
#define pushcommand_buffer(L, handle) pushxxx((L), (uint64_t)(uintptr_t)(handle))
#define checkcommand_bufferlist(L, arg, count, err, notused) \
    (VkCommandBuffer*)checkxxxlist_dispatchable((L), (arg), (count), (err), COMMAND_BUFFER_MT)

/* command_pool.c (nondispatchable) */
#define checkcommand_pool(L, arg, udp) (VkCommandPool)checkxxx((L), (arg), (udp), COMMAND_POOL_MT)
#define testcommand_pool(L, arg, udp) (VkCommandPool)testxxx((L), (arg), (udp), COMMAND_POOL_MT)

/* queue.c (dispatchable) */
#define checkqueue(L, arg, udp) (VkQueue)(uintptr_t)checkxxx((L), (arg), (udp), QUEUE_MT)
#define testqueue(L, arg, udp) (VkQueue)(uintptr_t)testxxx((L), (arg), (udp), QUEUE_MT)
#define checkqueuelist(L, arg, count, err) \
    (VkQueue*)checkxxxlist_dispatchable((L), (arg), (count), (err), QUEUE_MT)
#define pushqueue moonvulkan_pushqueue
int pushqueue(lua_State *L, VkQueue p, VkDevice device);

/* semaphore.c (nondispatchable) */
#define checksemaphore(L, arg, udp) (VkSemaphore)checkxxx((L), (arg), (udp), SEMAPHORE_MT)
#define testsemaphore(L, arg, udp) (VkSemaphore)testxxx((L), (arg), (udp), SEMAPHORE_MT)
#define checksemaphorelist(L, arg, count, err, ud) \
        (VkSemaphore*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), SEMAPHORE_MT)

/* fence.c (nondispatchable) */
#define checkfence(L, arg, udp) (VkFence)checkxxx((L), (arg), (udp), FENCE_MT)
#define testfence(L, arg, udp) (VkFence)testxxx((L), (arg), (udp), FENCE_MT)
#define checkfencelist(L, arg, count, err, ud) \
    (VkFence*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), FENCE_MT)

/* device_memory.c (nondispatchable) */
#define checkdevice_memory(L, arg, udp) (VkDeviceMemory)checkxxx((L), (arg), (udp), DEVICE_MEMORY_MT)
#define testdevice_memory(L, arg, udp) (VkDeviceMemory)testxxx((L), (arg), (udp), DEVICE_MEMORY_MT)

/* event.c (nondispatchable) */
#define checkevent(L, arg, udp) (VkEvent)checkxxx((L), (arg), (udp), EVENT_MT)
#define testevent(L, arg, udp) (VkEvent)testxxx((L), (arg), (udp), EVENT_MT)
#define checkeventlist(L, arg, count, err, ud) \
    (VkEvent*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), EVENT_MT)

/* buffer_view.c (nondispatchable) */
#define checkbuffer_view(L, arg, udp) (VkBufferView)checkxxx((L), (arg), (udp), BUFFER_VIEW_MT)
#define testbuffer_view(L, arg, udp) (VkBufferView)testxxx((L), (arg), (udp), BUFFER_VIEW_MT)
#define checkbuffer_viewlist(L, arg, count, err, ud) \
    (VkBufferView*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), BUFFER_VIEW_MT)

/* buffer.c (nondispatchable) */
#define checkbuffer(L, arg, udp) (VkBuffer)checkxxx((L), (arg), (udp), BUFFER_MT)
#define testbuffer(L, arg, udp) (VkBuffer)testxxx((L), (arg), (udp), BUFFER_MT)
#define checkbufferlist(L, arg, count, err) \
    (VkBuffer*)checkxxxlist_dispatchable((L), (arg), (count), (err), BUFFER_MT)

/* framebuffer.c (nondispatchable) */
#define checkframebuffer(L, arg, udp) (VkFramebuffer)checkxxx((L), (arg), (udp), FRAMEBUFFER_MT)
#define testframebuffer(L, arg, udp) (VkFramebuffer)testxxx((L), (arg), (udp), FRAMEBUFFER_MT)

/* image_view.c (nondispatchable) */
#define checkimage_view(L, arg, udp) (VkImageView)checkxxx((L), (arg), (udp), IMAGE_VIEW_MT)
#define testimage_view(L, arg, udp) (VkImageView)testxxx((L), (arg), (udp), IMAGE_VIEW_MT)
#define checkimage_viewlist(L, arg, count, err, ud) \
    (VkImageView*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), IMAGE_VIEW_MT)

/* image.c (nondispatchable) */
#define checkimage(L, arg, udp) (VkImage)checkxxx((L), (arg), (udp), IMAGE_MT)
#define testimage(L, arg, udp) (VkImage)testxxx((L), (arg), (udp), IMAGE_MT)
#define pushimage_swapchain moonvulkan_pushimage_swapchain
int pushimage_swapchain(lua_State *L, VkImage image, ud_t *swapchain_ud);

/* shader_module.c (nondispatchable) */
#define checkshader_module(L, arg, udp) (VkShaderModule)checkxxx((L), (arg), (udp), SHADER_MODULE_MT)
#define testshader_module(L, arg, udp) (VkShaderModule)testxxx((L), (arg), (udp), SHADER_MODULE_MT)

/* sampler.c (nondispatchable) */
#define checksampler(L, arg, udp) (VkSampler)checkxxx((L), (arg), (udp), SAMPLER_MT)
#define testsampler(L, arg, udp) (VkSampler)testxxx((L), (arg), (udp), SAMPLER_MT)
#define checksamplerlist(L, arg, count, err, ud) \
    (VkSampler*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), SAMPLER_MT)

/* render_pass.c (nondispatchable) */
#define checkrender_pass(L, arg, udp) (VkRenderPass)checkxxx((L), (arg), (udp), RENDER_PASS_MT)
#define testrender_pass(L, arg, udp) (VkRenderPass)testxxx((L), (arg), (udp), RENDER_PASS_MT)

/* descriptor_set_layout.c (nondispatchable) */
#define checkdescriptor_set_layout(L, arg, udp) (VkDescriptorSetLayout)checkxxx((L), (arg), (udp), DESCRIPTOR_SET_LAYOUT_MT)
#define testdescriptor_set_layout(L, arg, udp) (VkDescriptorSetLayout)testxxx((L), (arg), (udp), DESCRIPTOR_SET_LAYOUT_MT)
#define checkdescriptor_set_layoutlist(L, arg, count, err, ud) \
    (VkDescriptorSetLayout*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), DESCRIPTOR_SET_LAYOUT_MT)

/* pipeline_layout.c (nondispatchable) */
#define checkpipeline_layout(L, arg, udp) (VkPipelineLayout)checkxxx((L), (arg), (udp), PIPELINE_LAYOUT_MT)
#define testpipeline_layout(L, arg, udp) (VkPipelineLayout)testxxx((L), (arg), (udp), PIPELINE_LAYOUT_MT)

/* descriptor_pool.c (nondispatchable) */
#define checkdescriptor_pool(L, arg, udp) (VkDescriptorPool)checkxxx((L), (arg), (udp), DESCRIPTOR_POOL_MT)
#define testdescriptor_pool(L, arg, udp) (VkDescriptorPool)testxxx((L), (arg), (udp), DESCRIPTOR_POOL_MT)

/* descriptor_set.c (nondispatchable) */
#define checkdescriptor_set(L, arg, udp) (VkDescriptorSet)checkxxx((L), (arg), (udp), DESCRIPTOR_SET_MT)
#define testdescriptor_set(L, arg, udp) (VkDescriptorSet)testxxx((L), (arg), (udp), DESCRIPTOR_SET_MT)
#define checkdescriptor_setlist(L, arg, count, err, ud) \
    (VkDescriptorSet*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), DESCRIPTOR_SET_MT)

/* pipeline_cache.c (nondispatchable) */
#define checkpipeline_cache(L, arg, udp) (VkPipelineCache)checkxxx((L), (arg), (udp), PIPELINE_CACHE_MT)
#define testpipeline_cache(L, arg, udp) (VkPipelineCache)testxxx((L), (arg), (udp), PIPELINE_CACHE_MT)
#define checkpipeline_cachelist(L, arg, count, err, ud) \
    (VkPipelineCache*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), PIPELINE_CACHE_MT)

/* query_pool.c (nondispatchable) */
#define checkquery_pool(L, arg, udp) (VkQueryPool)checkxxx((L), (arg), (udp), QUERY_POOL_MT)
#define testquery_pool(L, arg, udp) (VkQueryPool)testxxx((L), (arg), (udp), QUERY_POOL_MT)

/* pipeline.c (nondispatchable) */
#define checkpipeline(L, arg, udp) (VkPipeline)checkxxx((L), (arg), (udp), PIPELINE_MT)
#define testpipeline(L, arg, udp) (VkPipeline)testxxx((L), (arg), (udp), PIPELINE_MT)

/* surface.c (nondispatchable) */
#define checksurface(L, arg, udp) (VkSurfaceKHR)checkxxx((L), (arg), (udp), SURFACE_MT)
#define testsurface(L, arg, udp) (VkSurfaceKHR)testxxx((L), (arg), (udp), SURFACE_MT)

/* swapchain.c (nondispatchable) */
#define checkswapchain(L, arg, udp) (VkSwapchainKHR)checkxxx((L), (arg), (udp), SWAPCHAIN_MT)
#define testswapchain(L, arg, udp) (VkSwapchainKHR)testxxx((L), (arg), (udp), SWAPCHAIN_MT)
#define checkswapchainlist(L, arg, count, err, ud) \
    (VkSwapchainKHR*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), SWAPCHAIN_MT)

/* debug_report_callback.c */
#define checkdebug_report_callback(L, arg, udp) (VkDebugReportCallbackEXT)checkxxx((L), (arg), (udp), DEBUG_REPORT_CALLBACK_MT)
#define testdebug_report_callback(L, arg, udp) (VkDebugReportCallbackEXT)testxxx((L), (arg), (udp), DEBUG_REPORT_CALLBACK_MT)

/* debug_utils_messenger.c */
#define checkdebug_utils_messenger(L, arg, udp) (VkDebugUtilsMessengerEXT)checkxxx((L), (arg), (udp), DEBUG_UTILS_MESSENGER_MT)
#define testdebug_utils_messenger(L, arg, udp) (VkDebugUtilsMessengerEXT)testxxx((L), (arg), (udp), DEBUG_UTILS_MESSENGER_MT)

/* display.c */
#define checkdisplay(L, arg, udp) (VkDisplayKHR)checkxxx((L), (arg), (udp), DISPLAY_MT)
#define testdisplay(L, arg, udp) (VkDisplayKHR)testxxx((L), (arg), (udp), DISPLAY_MT)

/* display_mode.c */
#define checkdisplay_mode(L, arg, udp) (VkDisplayModeKHR)checkxxx((L), (arg), (udp), DISPLAY_MODE_MT)
#define testdisplay_mode(L, arg, udp) (VkDisplayModeKHR)testxxx((L), (arg), (udp), DISPLAY_MODE_MT)

/* descriptor_update_template.c */
#define checkdescriptor_update_template(L, arg, udp) (VkDescriptorUpdateTemplate)checkxxx((L), (arg), (udp), DESCRIPTOR_UPDATE_TEMPLATE_MT)
#define testdescriptor_update_template(L, arg, udp) (VkDescriptorUpdateTemplate)testxxx((L), (arg), (udp), DESCRIPTOR_UPDATE_TEMPLATE_MT)
#define pushdescriptor_update_template(L, handle) pushxxx((L), (uint64_t)(handle))

/* validation_cache.c */
#define checkvalidation_cache(L, arg, udp) (VkValidationCacheEXT)checkxxx((L), (arg), (udp), VALIDATION_CACHE_MT)
#define testvalidation_cache(L, arg, udp) (VkValidationCacheEXT)testxxx((L), (arg), (udp), VALIDATION_CACHE_MT)
#define pushvalidation_cache(L, handle) pushxxx((L), (uint64_t)(handle))
#define checkvalidation_cachelist(L, arg, count, err, ud) \
    (VkValidationCacheEXT*)checkxxxlist_nondispatchable((L), (arg), (count), (err), (ud), VALIDATION_CACHE_MT)

/* sampler_ycbcr_conversion.c */
#define checksampler_ycbcr_conversion(L, arg, udp) (VkSamplerYcbcrConversion)checkxxx((L), (arg), (udp), SAMPLER_YCBCR_CONVERSION_MT)
#define testsampler_ycbcr_conversion(L, arg, udp) (VkSamplerYcbcrConversion)testxxx((L), (arg), (udp), SAMPLER_YCBCR_CONVERSION_MT)
#define pushsampler_ycbcr_conversion(L, handle) pushxxx((L), (uint64_t)(handle))

/* used in main.c */
void moonvulkan_open_instance(lua_State *L);
void moonvulkan_open_physical_device(lua_State *L);
void moonvulkan_open_layers(lua_State *L);
void moonvulkan_open_device(lua_State *L);
void moonvulkan_open_queue(lua_State *L);
void moonvulkan_open_command_pool(lua_State *L);
void moonvulkan_open_command_buffer(lua_State *L);
void moonvulkan_open_semaphore(lua_State *L);
void moonvulkan_open_fence(lua_State *L);
void moonvulkan_open_buffer(lua_State *L);
void moonvulkan_open_device_memory(lua_State *L);
void moonvulkan_open_image(lua_State *L);
void moonvulkan_open_event(lua_State *L);
void moonvulkan_open_buffer_view(lua_State *L);
void moonvulkan_open_image_view(lua_State *L);
void moonvulkan_open_shader_module(lua_State *L);
void moonvulkan_open_sampler(lua_State *L);
void moonvulkan_open_render_pass(lua_State *L);
void moonvulkan_open_framebuffer(lua_State *L);
void moonvulkan_open_descriptor_set_layout(lua_State *L);
void moonvulkan_open_descriptor_pool(lua_State *L);
void moonvulkan_open_pipeline_layout(lua_State *L);
void moonvulkan_open_pipeline_cache(lua_State *L);
void moonvulkan_open_query_pool(lua_State *L);
void moonvulkan_open_descriptor_set(lua_State *L);
void moonvulkan_open_pipeline(lua_State *L);
void moonvulkan_open_surface(lua_State *L);
void moonvulkan_open_swapchain(lua_State *L);
void moonvulkan_open_debug_report_callback(lua_State *L);
void moonvulkan_open_display(lua_State *L);
void moonvulkan_open_display_mode(lua_State *L);
void moonvulkan_open_descriptor_update_template(lua_State *L);
void moonvulkan_open_validation_cache(lua_State *L);
void moonvulkan_open_sampler_ycbcr_conversion(lua_State *L);
void moonvulkan_open_debug_utils_messenger(lua_State *L);


#define RAW_FUNC(xxx)                       \
static int Raw(lua_State *L)                \
    {                                       \
    pushhandle(L, (uint64_t)check##xxx(L, 1, NULL));    \
    return 1;                               \
    }

#define RAW_FUNC_DISPATCHABLE(xxx)          \
static int Raw(lua_State *L)                \
    {                                       \
    pushhandle(L, (uint64_t)(uintptr_t)check##xxx(L, 1, NULL)); \
    return 1;                               \
    }

#define INSTANCE_FUNC(xxx)                  \
static int Instance(lua_State *L)           \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    return pushinstance(L, ud->instance);   \
    }

#define DEVICE_FUNC(xxx)                    \
static int Device(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->device)                         \
        return 0;                           \
    return pushdevice(L, ud->device);       \
    }

#define DELETE_FUNC(xxx)                    \
static int Delete(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)test##xxx(L, 1, &ud);             \
    if(!ud) return 0; /* already deleted */ \
    return ud->destructor(L, ud);           \
    }

#define DESTROY_FUNC(xxx)                   \
static int Destroy(lua_State *L)            \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    return ud->destructor(L, ud);           \
    }

#define PARENT_FUNC(xxx)                    \
static int Parent(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->parent_ud) return 0;            \
    return pushuserdata(L, ud->parent_ud);  \
    }

#define TYPE_FUNC(xxx) /* NONVK */          \
static int Type(lua_State *L)               \
    {                                       \
    (void)check##xxx(L, 1, NULL);           \
    lua_pushstring(L, ""#xxx);              \
    return 1;                               \
    }


#endif /* objectsDEFINED */
