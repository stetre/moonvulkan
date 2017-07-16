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

/* 
 * echeckxxx() functions return 0 (ERR_SUCCESS) on success, otherwise they
 * push a message on the Lua stack and return err!=0 (ERR_XXX).
 * The GetXxx() macros conform to this behaviour.
 *
 * This applies only to echeckxxx() functions defined in this file:
 * - checkxxxlist() functions defined in utils.c only return an error code,
 * - checkxxx() functions defined elsewhere (enums, bitfields, etc) follow the
 *   Lua convention to raise errors.
 */

#define ECHECK_PREAMBLE                                                         \
    memset(p, 0, sizeof(*p));                                                   \
    if(lua_isnoneornil(L, arg))                                                 \
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); return ERR_NOTPRESENT; }\
    if(lua_type(L, arg) != LUA_TTABLE)                                          \
        { lua_pushstring(L, errstring(ERR_TABLE)); return ERR_TABLE; }
        
#define POPERROR()  lua_pop(L, 1)

static int pusherror(lua_State *L, int errcode)
    { 
    lua_pushstring(L, errstring(errcode)); 
    return ERR_GENERIC; 
    }

static int fielderror(lua_State *L, const char *fieldname, int errcode)
    { 
    lua_pushfstring(L, "%s: %s", fieldname, errstring(errcode)); 
    return ERR_GENERIC; 
    }

static int efielderror(lua_State *L, const char *fieldname)
    { 
    lua_pushfstring(L, "%s.%s", fieldname, lua_tostring(L, -1));
    lua_remove(L, -2);
    return ERR_GENERIC; 
    }

#define PUSHFIELD(sname)    \
    do { lua_pushstring(L, sname); lua_rawget(L, arg); arg1 = lua_gettop(L); } while(0)

#define POPFIELD()          \
    do { lua_remove(L, arg1); } while(0)

/* Flags ---------------------------------------------------------------------*/

#define GetFlags(name, sname)  /* always opt., defaults to 0 */ \
do {                                                \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = testflags(L, -1, &err);               \
    lua_pop(L, 1);                                  \
    if(err == ERR_NOTPRESENT)                       \
        p->name = 0;                                \
    else if(err)                                    \
        return fielderror(L, sname, err);           \
} while(0)

#define GetBits(name, sname, T) do {                \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = (T)testflags(L, -1, &err);            \
    lua_pop(L, 1);                                  \
    if(err == ERR_NOTPRESENT)                       \
        p->name = (T)0;                             \
    else if(err)                                    \
        return fielderror(L, sname, err);           \
} while(0)

#define GetShaderStageFlagBits(name, sname) GetBits(name, sname, VkShaderStageFlagBits)
#define GetSurfaceTransformFlagBits(name, sname) GetBits(name, sname, VkSurfaceTransformFlagBitsKHR)
#define GetCompositeAlphaFlagBits(name, sname) GetBits(name, sname, VkCompositeAlphaFlagBitsKHR)
#define GetDisplayPlaneAlphaFlagBits(name, sname) GetBits(name, sname, VkDisplayPlaneAlphaFlagBitsKHR)

/* Numbers and strings -------------------------------------------------------*/

#define GetSamples(name, sname) do {                \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = (VkSampleCountFlagBits)testflags(L, -1, &err); \
    lua_pop(L, 1);                                  \
    if(err == ERR_NOTPRESENT)                       \
        p->name = VK_SAMPLE_COUNT_1_BIT;            \
    else if(err)                                    \
        return fielderror(L, sname, err);           \
} while(0)

#define GetNumberOpt(name, sname, defval) do {      \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isnumber(L, -1))                         \
        p->name = lua_tonumber(L, -1);              \
    else if(lua_isnoneornil(L, -1))                 \
        p->name = defval;                           \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetNumber(name, sname) GetNumberOpt(name, sname, 0) 

#define GetIntegerOpt(name, sname, defval) do {     \
    int isnum_;                                     \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isnoneornil(L, -1))                      \
        p->name = defval;                           \
    else                                            \
        {                                           \
        p->name = lua_tointegerx(L, -1, &isnum_);   \
        if(!isnum_) err = ERR_TYPE;                 \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetInteger(name, sname) GetIntegerOpt(name, sname, 0)

#define GetBoolean(name, sname)                     \
do {                                                \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isboolean(L, -1))                        \
        p->name = lua_toboolean(L, -1);             \
    else if(lua_isnoneornil(L, -1))                 \
        p->name = 0;                                \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetString(name, sname) do {                 \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_type(L, -1) == LUA_TSTRING)              \
        p->name = Strdup(L, lua_tostring(L, -1)); /* REMEMBER TO FREE THIS */\
    else                                            \
        {                                           \
        p->name = NULL;                             \
        err = lua_isnoneornil(L, -1) ? ERR_NOTPRESENT : ERR_TYPE;   \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetStringOpt(name, sname) do {              \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        p->name = Strdup(L, lua_tostring(L, -1)); /* REMEMBER TO FREE THIS */\
    else                                            \
        {                                           \
        p->name = NULL;                             \
        err = ((t_ == LUA_TNONE)||(t_ == LUA_TNIL)) ? 0 : ERR_TYPE;    \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetStringDef(name, sname, defval) do {      \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        p->name = Strdup(L, lua_tostring(L, -1)); /* REMEMBER TO FREE THIS */\
    else                                            \
        {                                           \
        p->name = Strdup(L, (defval)); /* REMEMBER TO FREE THIS */\
        err = ((t_ == LUA_TNONE)||(t_ == LUA_TNIL)) ? 0 : ERR_TYPE;    \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

/* Enums ---------------------------------------------------------------------*/

#define GetEnum(name, sname, testfunc) do {         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = testfunc(L, -1, &err);                \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetEnumOpt(name, sname, testfunc, defval) do {      \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = testfunc(L, -1, &err);                \
    lua_pop(L, 1);                                  \
    if(err == ERR_NOTPRESENT)                       \
        p->name = (defval);                         \
    else if(err < 0)                                \
        return fielderror(L, sname, err);           \
} while(0)

/* enums without defval (ie required) */
#define GetDescriptorType(name, sname) GetEnum(name, sname, testdescriptortype)
#define GetImageType(name, sname) GetEnum(name, sname, testimagetype)
#define GetImageViewType(name, sname) GetEnum(name, sname, testimageviewtype)
#define GetDescriptorUpdateTemplateType(name, sname) GetEnum(name, sname, testdescriptorupdatetemplatetype)

/* optional enums with defval */
#define GetPipelineBindPoint(name, sname) GetEnumOpt(name, sname, testpipelinebindpoint, VK_PIPELINE_BIND_POINT_GRAPHICS)
#define GetSharingMode(name, sname) GetEnumOpt(name, sname, testsharingmode, VK_SHARING_MODE_EXCLUSIVE)
#define GetImageLayout(name, sname) GetEnumOpt(name, sname, testimagelayout, VK_IMAGE_LAYOUT_UNDEFINED)
#define GetCompareOp(name, sname) GetEnumOpt(name, sname, testcompareop, VK_COMPARE_OP_NEVER)
#define GetBlendFactor(name, sname) GetEnumOpt(name, sname, testblendfactor, VK_BLEND_FACTOR_ZERO)
#define GetBlendOp(name, sname) GetEnumOpt(name, sname, testblendop, VK_BLEND_OP_ADD)
#define GetStencilOp(name, sname) GetEnumOpt(name, sname, teststencilop, VK_STENCIL_OP_KEEP)
#define GetFormat(name, sname) GetEnumOpt(name, sname, testformat, VK_FORMAT_UNDEFINED)
#define GetAttachmentLoadOp(name, sname) GetEnumOpt(name, sname, testattachmentloadop, VK_ATTACHMENT_LOAD_OP_DONT_CARE)
#define GetAttachmentStoreOp(name, sname) GetEnumOpt(name, sname, testattachmentstoreop, VK_ATTACHMENT_STORE_OP_DONT_CARE)
#define GetFilter(name, sname) GetEnumOpt(name, sname, testfilter, VK_FILTER_NEAREST)
#define GetSamplerMipmapMode(name, sname) GetEnumOpt(name, sname, testsamplermipmapmode, VK_SAMPLER_MIPMAP_MODE_NEAREST)
#define GetSamplerAddressMode(name, sname) GetEnumOpt(name, sname, testsampleraddressmode, VK_SAMPLER_ADDRESS_MODE_REPEAT)
#define GetBorderColor(name, sname) GetEnumOpt(name, sname, testbordercolor, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
#define GetComponentSwizzle(name, sname) GetEnumOpt(name, sname, testcomponentswizzle, VK_COMPONENT_SWIZZLE_IDENTITY)
#define GetImageTiling(name, sname) GetEnumOpt(name, sname, testimagetiling, VK_IMAGE_TILING_OPTIMAL)
#define GetTopology(name, sname) GetEnumOpt(name, sname, testprimitivetopology, VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
#define GetPolygonMode(name, sname) GetEnumOpt(name, sname, testpolygonmode, VK_POLYGON_MODE_FILL)
#define GetFrontFace(name, sname) GetEnumOpt(name, sname, testfrontface, VK_FRONT_FACE_COUNTER_CLOCKWISE)
#define GetVertexInputRate(name, sname) GetEnumOpt(name, sname, testvertexinputrate, VK_VERTEX_INPUT_RATE_VERTEX)
#define GetLogicOp(name, sname) GetEnumOpt(name, sname, testlogicop, VK_LOGIC_OP_CLEAR)
#define GetColorSpace(name, sname) GetEnumOpt(name, sname, testcolorspace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
#define GetPresentMode(name, sname) GetEnumOpt(name, sname, testpresentmode, VK_PRESENT_MODE_FIFO_KHR)

/* Structs -------------------------------------------------------------------*/

#define GetStruct(name, sname, echeckfunc) do {     \
    int arg2;                                       \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    arg2 = lua_gettop(L);                           \
    err = echeckfunc(L, arg2, &(p->name));          \
    lua_remove(L, arg2);                            \
    if(err)                                         \
        {                                           \
        switch(err)                                 \
            {                                       \
            case ERR_NOTPRESENT:                    \
            case ERR_TABLE:                         \
            case ERR_MEMORY:                        \
            case ERR_EMPTY:                         \
                return fielderror(L, sname, err);   \
            default:                                \
                return efielderror(L, sname);       \
            }                                       \
        }                                           \
} while(0)

#define GetStructOpt(name, sname, echeckfunc) do {  \
    int arg2;                                       \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    arg2 = lua_gettop(L);                           \
    err = echeckfunc(L, arg2, &(p->name));          \
    lua_remove(L, arg2);                            \
    if(err < 0)                                     \
        {                                           \
        switch(err)                                 \
            {                                       \
            case ERR_TABLE:                         \
            case ERR_MEMORY:                        \
            case ERR_EMPTY:                         \
                return fielderror(L, sname, err);   \
            default:                                \
                return efielderror(L, sname);       \
            }                                       \
        }                                           \
} while(0)

#define GetStructArrayOpt(name, sname, n, echeckfunc) do {  \
    int arg2, arg3, t_, i_;                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    arg2 = lua_gettop(L);                           \
    t_ = lua_type(L, arg2);                         \
    if(t_ != LUA_TTABLE)                            \
        {                                           \
        lua_remove(L, arg2);                        \
        if(t_ == LUA_TNIL || t_ == LUA_TNONE)       \
            err = ERR_NOTPRESENT;                   \
        else                                        \
            return fielderror(L, sname, ERR_TABLE); \
        }                                           \
    else {                                          \
        for(i_=0; i_ <(n); i_++)                    \
            {                                       \
            lua_rawgeti(L, arg2, i_+1);             \
            arg3 = lua_gettop(L);                   \
            err = echeckfunc(L, arg3, &(p->name[i_]));\
            lua_remove(L, arg3);                    \
            if(err < 0) break;                      \
            }                                       \
        lua_remove(L, arg2);                        \
        if(err < 0)                                 \
            {                                       \
            switch(err)                             \
                {                                   \
                case ERR_TABLE:                     \
                case ERR_MEMORY:                    \
                case ERR_EMPTY:                     \
                    return fielderror(L, sname, err);   \
                default:                            \
                    return efielderror(L, sname);   \
                }                                   \
            }                                       \
        }                                           \
} while(0)

#define GetOffset2d(name, sname) GetStruct(name, sname, echeckoffset2d)
#define GetOffset2dOpt(name, sname) GetStructOpt(name, sname, echeckoffset2d)
#define GetOffset3d(name, sname) GetStruct(name, sname, echeckoffset3d)
#define GetOffset3dOpt(name, sname) GetStructOpt(name, sname, echeckoffset3d)
#define GetExtent2d(name, sname) GetStruct(name, sname, echeckextent2d)
#define GetExtent2dOpt(name, sname) GetStructOpt(name, sname, echeckextent2d)
#define GetExtent3d(name, sname) GetStruct(name, sname, echeckextent3d)
#define GetExtent3dOpt(name, sname) GetStructOpt(name, sname, echeckextent3d)
#define GetRect2d(name, sname) GetStruct(name, sname, echeckrect2d)
#define GetRect2dOpt(name, sname) GetStructOpt(name, sname, echeckrect2d)
#define GetImageSubresourceLayers(name, sname) GetStruct(name, sname, echeckimagesubresourcelayers)
#define GetComponentMappingOpt(name, sname) GetStructOpt(name, sname, echeckcomponentmapping)
#define GetImageSubresourceRangeOpt(name, sname) GetStructOpt(name, sname, echeckimagesubresourcerange)
#define GetClearColorValue(name, sname) GetStruct(name, sname, echeckclearcolorvalue)
#define GetClearValue(name, sname) GetStruct(name, sname, echeckclearvalue)
#define GetImageSubresource(name, sname) GetStruct(name, sname, echeckimagesubresource)
#define GetStencilOpStateOpt(name, sname) GetStructOpt(name, sname, echeckstencilopstate)
#define GetPipelineShaderStageCreateInfo(name, sname) GetStruct(name, sname, echeckpipelineshaderstagecreateinfo)

/* Objects -------------------------------------------------------------------*/

#define GetObject(name, sname, TTT, ttt) do {       \
/* eg: TTT = VkRenderPass, ttt = render_pass */     \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = test##ttt(L, -1, NULL);               \
    lua_pop(L, 1);                                  \
    if(!p->name)                                    \
        return fielderror(L, sname, ERR_TYPE);      \
} while(0)

#define GetObjectOpt(name, sname, TTT, ttt) do {    \
/* eg: TTT = VkRenderPass, ttt = render_pass */     \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isnoneornil(L, -1))                      \
        p->name = 0;                                \
    else                                            \
        {                                           \
        p->name = test##ttt(L, -1, NULL);           \
        if(!p->name) err = ERR_TYPE;                \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetRenderPass(name, sname) GetObject(name, sname, VkRenderPass, render_pass)
#define GetRenderPassOpt(name, sname) GetObjectOpt(name, sname, VkRenderPass, render_pass)
#define GetFramebuffer(name, sname) GetObject(name, sname, VkFramebuffer, framebuffer)
#define GetFramebufferOpt(name, sname) GetObjectOpt(name, sname, VkFramebuffer, framebuffer)
#define GetBuffer(name, sname) GetObject(name, sname, VkBuffer, buffer)
#define GetImage(name, sname) GetObject(name, sname, VkImage, image)
#define GetDeviceMemory(name, sname) GetObject(name, sname, VkDeviceMemory, device_memory)
#define GetShaderModule(name, sname) GetObject(name, sname, VkShaderModule, shader_module)
#define GetPipelineLayout(name, sname) GetObject(name, sname, VkPipelineLayout, pipeline_layout)
#define GetPipelineLayoutOpt(name, sname) GetObjectOpt(name, sname, VkPipelineLayout, pipeline_layout)
#define GetPipelineOpt(name, sname) GetObjectOpt(name, sname, VkPipeline, pipeline)
#define GetSurface(name, sname) GetObject(name, sname, VkSurfaceKHR, surface)
#define GetSwapchainOpt(name, sname) GetObjectOpt(name, sname, VkSwapchainKHR, swapchain)
#define GetSampler(name, sname) GetObject(name, sname, VkSampler, sampler)
#define GetImageView(name, sname) GetObject(name, sname, VkImageView, image_view)
#define GetDescriptorSet(name, sname) GetObject(name, sname, VkDescriptorSet, descriptor_set)
#define GetDescriptorSetLayoutOpt(name, sname) GetObject(name, sname, VkDescriptorSetLayout, descriptor_set_layout)
#if 0
#define Get(name, sname) GetObject(name, sname, Vk, )
#define GetOpt(name, sname) GetObjectOpt(name, sname, Vk, )
#endif

/* Integers with special values ----------------------------------------------*/

#define GetSubpass(name, sname) /* integer or 'external' */ do {\
    const char *s_;                                 \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, -1);                   \
        if(strcmp(s_, "external") == 0)             \
            p->name = VK_SUBPASS_EXTERNAL;          \
        else                                        \
            err = ERR_VALUE;                        \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = 0;                                \
    else if(lua_isinteger(L, -1))                   \
        p->name = lua_tointeger(L, -1);             \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)


/* 'remaining' stands for  VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS, ... */
#define GetIntegerOrRemaining(name, sname, defval) /* integer or 'remaining' */ do {\
    const char *s_;                                 \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, -1);                   \
        if(strcmp(s_, "remaining") == 0)            \
            p->name = ~0U; /* VK_REMAINING_XXX */   \
        else                                        \
            err = ERR_VALUE;                        \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = defval;                           \
    else if(lua_isinteger(L, -1))                   \
        p->name = lua_tointeger(L, -1);             \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetIntegerOrWholeSize(name, sname) /* integer or 'whole size' */ do {\
    const char *s_;                                 \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, -1);                   \
        if(strcmp(s_, "whole size") == 0)           \
            p->name = VK_WHOLE_SIZE;                \
        else                                        \
            err = ERR_VALUE;                        \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_WHOLE_SIZE;                    \
    else if(lua_isinteger(L, -1))                   \
        p->name = lua_tointeger(L, -1);             \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetAttachment(name, sname) /* integer or 'unused', defval = 'unused' */ do {\
    const char *s_;                                 \
    int t_;                                         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    t_ = lua_type(L, -1);                           \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, -1);                   \
        if(strcmp(s_, "remaining") == 0)            \
            p->name = VK_ATTACHMENT_UNUSED;         \
        else                                        \
            err = ERR_VALUE;                        \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_ATTACHMENT_UNUSED;             \
    else if(lua_isinteger(L, -1))                   \
        p->name = lua_tointeger(L, -1);             \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

/* Lists ---------------------------------------------------------------------*
 *
 * VkXxx* echeckxxxlist(lua_State *L, int arg, uint32_t *count, int *err)
 *
 * Checks if the variable at arg on the Lua stack is a list of VkXxx structures.
 * On success, returns an array of VkXxx handles and sets its length in *count.
 * The array is Malloc'd and must be released by the caller using Free(L, ...)
 * or the proper freexxxlist() (see below).
 *
 * On error, sets *err to ERR_XXX, *count to 0, leaves an error message on the
 * Lua stack and returns NULL. 
 *
 * This function espects the existence of a 'echeck' function defined as:
 * int echeckxxx(lua_State *L, int arg, VkXxx *dst)
 * which is expected to fill *dst and/or return an ERR_XXX code + pushing an error
 * message on the stack on error.
 *
 * If the echeckxxx() function itselfs Mallocates memory, a non-NULL freexxxlist 
 * function must be defined with the FREELISTFUNC macro.
 *
 * If the echeckxxx() function does not allocate memory, then freexxxlist = NULL
 * may be passed to the ECHECKLISTFUNC macro.
 *
 */
typedef void (*FREELISTFUNCp)(lua_State *L, void *list, uint32_t count);

#define FREELISTFUNC(VkXxx, xxx)                                                \
void free##xxx##list(lua_State *L, void *list, uint32_t count)                  \
    {                                                                           \
    uint32_t i;                                                                 \
    VkXxx *p = (VkXxx*)list;                                                    \
    if(!p) return;                                                              \
    for(i=0; i<count; i++)                                                      \
        free##xxx(L, &p[i]);                                                    \
    Free(L, p);                                                                 \
    }

#define ECHECKLISTFUNC(VkXxx, xxx, freexxxlist)                             \
VkXxx* echeck##xxx##list(lua_State *L, int arg, uint32_t *count, int *err)  \
    {                                                                       \
    int arg1;                                                               \
    VkXxx* list;                                                            \
    uint32_t i;                                                             \
    FREELISTFUNCp freelist = freexxxlist;                                   \
                                                                            \
    *count = 0;                                                             \
    *err = 0;                                                               \
    if(lua_isnoneornil(L, arg))                                             \
        { *err = ERR_NOTPRESENT; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; }    \
    if(lua_type(L, arg) != LUA_TTABLE)                                              \
        { *err = ERR_TABLE; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; } \
    *count = lua_rawlen(L, arg);                                                    \
    if(*count == 0)                                                                 \
        { *err = ERR_EMPTY; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; } \
                                                                            \
    list = (VkXxx*)MallocNoErr(L, sizeof(VkXxx) * (*count));                \
    if(!list)                                                               \
        { *count = 0; *err = ERR_MEMORY; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; }\
                                                                            \
    for(i=0; i<*count; i++)                                                 \
        {                                                                   \
        lua_rawgeti(L, arg, i+1);                                           \
        arg1 = lua_gettop(L);                                               \
        *err = echeck##xxx(L, arg1, &list[i]);                              \
        lua_remove(L, arg1);                                                \
        if(*err)                                                            \
            {                                                               \
            if(freelist)                                                    \
                freelist(L, list, i);                                       \
            else                                                            \
                Free(L, list);                                              \
            *count = 0;                                                     \
            /* an error message has been already pushed by echeckxxx() */   \
            lua_pushfstring(L, "%d.%s", i+1, lua_tostring(L, -1));          \
            lua_remove(L, -2);                                              \
            *err = ERR_GENERIC;                                             \
            return NULL;                                                    \
            }                                                               \
        }                                                                   \
    return list;                                                            \
    }


/* Set macros (for push functions) ----------------------------------------------------*/
#define SetInteger(name, sname) do { lua_pushinteger(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetNumber(name, sname) do { lua_pushnumber(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetFlags(name, sname) do { pushflags(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetBits SetFlags
#define SetBoolean(name, sname) do { lua_pushboolean(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetString(name, sname) do { lua_pushstring(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetUUID(name, sname, len) do { lua_pushlstring(L, (char*)p->name,(len)); lua_setfield(L, -2, sname); } while(0)
#define SetEnum(name, sname, pushfunc) do { pushfunc(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetStruct(name, sname, pushfunc) do { pushfunc(L, &(p->name)); lua_setfield(L, -2, sname); } while(0)
#define SetIntegerArray(name, sname, n) do { int i_;    \
            lua_newtable(L);                            \
            for(i_=0; i_<(n); i_++) { lua_pushinteger(L, p->name[i_]); lua_seti(L, -2, i_+1); } \
            lua_setfield(L, -2, sname);                 \
} while(0)
#define SetNumberArray(name, sname, n) do { int i_;     \
            lua_newtable(L);                            \
            for(i_=0; i_<(n); i_++) { lua_pushnumber(L, p->name[i_]); lua_seti(L, -2, i_+1); }  \
            lua_setfield(L, -2, sname);                 \
} while(0)


/*------------------------------------------------------------------------------*/

static void freeapplicationinfo(lua_State *L, VkApplicationInfo *p)
    {
    if(!p) return;
    if(p->pApplicationName) Free(L, (char*)p->pApplicationName);
    if(p->pEngineName) Free(L, (char*)p->pEngineName);
    }

static int echeckapplicationinfo(lua_State *L, int arg, VkApplicationInfo *p)
/* freeapplicationinfo() must be called in any case, even on error,
 * except for err = ERR_NOTPRESENT.
 */
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    p->pNext = NULL;
    GetStringOpt(pApplicationName, "application_name");
    GetInteger(applicationVersion, "application_version");
    GetStringOpt(pEngineName, "engine_name");
    GetInteger(engineVersion, "engine_version");
    GetInteger(apiVersion, "api_version");
    return 0;
    }

void freeinstancecreateinfo(lua_State *L, VkInstanceCreateInfo *p)
    {
    if(!p) return;
    if(p->pApplicationInfo)
        {
        freeapplicationinfo(L, (VkApplicationInfo*)p->pApplicationInfo);
        Free(L, (void*)p->pApplicationInfo);
        }
    if(p->ppEnabledLayerNames) freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames) freestringlist(L, (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
    }

int echeckinstancecreateinfo(lua_State *L, int arg, VkInstanceCreateInfo *p)
    {
    int arg1, err;
    VkApplicationInfo *appinfo;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");

#define F "application_info"
    appinfo = MALLOC(L, VkApplicationInfo);
    if(!appinfo) return pusherror(L, ERR_MEMORY);
    PUSHFIELD(F);
    err = echeckapplicationinfo(L, arg1, appinfo);
    POPFIELD();
    if(err < 0) return efielderror(L, F);
    if(err == ERR_NOTPRESENT)
        { POPERROR(); Free(L, appinfo); }
    else
        p->pApplicationInfo = appinfo; 
#undef F
#define F "enabled_layer_names"
    PUSHFIELD(F);
    p->ppEnabledLayerNames = (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, &err);
    POPFIELD();
    if(err < 0 && err != ERR_EMPTY) 
        { freeinstancecreateinfo(L, p); return fielderror(L, F, err); }
#undef F
#define F "enabled_extension_names"
    PUSHFIELD(F);
    p->ppEnabledExtensionNames = (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, &err);
    POPFIELD();
    if(err < 0 && err != ERR_EMPTY) 
        { freeinstancecreateinfo(L, p); return fielderror(L, F, err); }
#undef F
    
    return 0;
    }


/*------------------------------------------------------------------------------*/

static int echeckdevicequeuecreateinfo(lua_State *L, int arg, VkDeviceQueueCreateInfo *p)
    {
    int arg1, err;
    uint32_t count;
    p->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
#define F   "queue_priorities"
    PUSHFIELD(F);
    p->pQueuePriorities = (const float*)checkfloatlist(L, arg1, &count, &err);
    p->queueCount = count;
    POPFIELD();
    if(err) return fielderror(L, F, err);
#undef F
    return 0;
    }

static void freedevicequeuecreateinfolist(lua_State *L, void *list, uint32_t count)
    {
    uint32_t i;
    VkDeviceQueueCreateInfo *p = (VkDeviceQueueCreateInfo*)list;
    if((!p)||(count==0)) return;
    for(i=0; i<count; i++)
        {
        if(p[i].pQueuePriorities)
            Free(L, (void*)(p[i].pQueuePriorities));
        }
    Free(L, p);
    }


/* echeckdevicequeuecreateinfolist() */
static ECHECKLISTFUNC(VkDeviceQueueCreateInfo, devicequeuecreateinfo, freedevicequeuecreateinfolist) 

    
/*------------------------------------------------------------------------------*/

static int echeckdescriptorpoolsize(lua_State *L, int arg, VkDescriptorPoolSize *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetDescriptorType(type, "type");
    GetIntegerOpt(descriptorCount, "descriptor_count", 1);
    return 0;
    }

/* echeckdescriptorpoolsizelist() */
ECHECKLISTFUNC(VkDescriptorPoolSize, descriptorpoolsize, NULL) 

/*------------------------------------------------------------------------------*/

static void freedescriptorsetlayoutbinding(lua_State *L, VkDescriptorSetLayoutBinding *p)
    {
    if(!p)
        return;
    if(p->pImmutableSamplers)
        Free(L, (void*)p->pImmutableSamplers);
    }

static int echeckdescriptorsetlayoutbinding(lua_State *L, int arg, VkDescriptorSetLayoutBinding *p)
    {
    int err, arg1;
    uint32_t count;

    ECHECK_PREAMBLE
    GetInteger(binding, "binding");
    GetDescriptorType(descriptorType, "descriptor_type");
    GetInteger(descriptorCount, "descriptor_count");
    GetFlags(stageFlags, "stage_flags");

    if((p->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
        p->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
        p->descriptorCount != 0)
        {
#define F   "immutable_samplers"
        PUSHFIELD(F);
        p->pImmutableSamplers = checksamplerlist(L, arg1, &count, &err, NULL);
        POPFIELD();
        if(err == ERR_NOTPRESENT) 
            return 0;
        if(err < 0)
            { freedescriptorsetlayoutbinding(L, p); return fielderror(L, F, err); }
        if(count != p->descriptorCount)
            { freedescriptorsetlayoutbinding(L, p); return fielderror(L, F, ERR_LENGTH); }
#undef F
        }
    return 0;
    }

/* echeckdescriptorsetlayoutbindinglist() */
FREELISTFUNC(VkDescriptorSetLayoutBinding, descriptorsetlayoutbinding) 
ECHECKLISTFUNC(VkDescriptorSetLayoutBinding, descriptorsetlayoutbinding, freedescriptorsetlayoutbindinglist) 

/*------------------------------------------------------------------------------*/

static int echeckattachmentdescription(lua_State *L, int arg, VkAttachmentDescription *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(flags, "flags");
    GetFormat(format, "format");
    GetSamples(samples, "samples");
    GetAttachmentLoadOp(loadOp, "load_op");
    GetAttachmentStoreOp(storeOp, "store_op");
    GetAttachmentLoadOp(stencilLoadOp, "stencil_load_op");
    GetAttachmentStoreOp(stencilStoreOp, "stencil_store_op");
    GetImageLayout(initialLayout, "initial_layout");
    GetImageLayout(finalLayout, "final_layout");
    return 0;
    }

/* echeckattachmentdescriptionlist() */
static ECHECKLISTFUNC(VkAttachmentDescription, attachmentdescription, NULL) 

static int echecksubpassdependency(lua_State *L, int arg, VkSubpassDependency *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetSubpass(srcSubpass, "src_subpass");
    GetSubpass(dstSubpass, "dst_subpass");
    GetFlags(srcStageMask, "src_stage_mask");
    GetFlags(dstStageMask, "dst_stage_mask");
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetFlags(dependencyFlags, "dependency_flags");
    return 0;
    }

/* echecksubpassdependencylist() */
static ECHECKLISTFUNC(VkSubpassDependency, subpassdependency, NULL) 

static int echeckattachmentreference(lua_State *L, int arg, VkAttachmentReference *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetAttachment(attachment, "attachment");
    GetImageLayout(layout, "layout");
    return 0;
    }

/* echeckattachmentreferencelist() */
static ECHECKLISTFUNC(VkAttachmentReference, attachmentreference, NULL) 

static void freesubpassdescription(lua_State *L, VkSubpassDescription *p)
    {
    if(p->pInputAttachments) Free(L, (void*)p->pInputAttachments);
    if(p->pColorAttachments) Free(L, (void*)p->pColorAttachments);
    if(p->pResolveAttachments) Free(L, (void*)p->pResolveAttachments);
    if(p->pPreserveAttachments) Free(L, (void*)p->pPreserveAttachments);
    if(p->pDepthStencilAttachment ) Free(L, (void*)p->pDepthStencilAttachment );
    }

static int echecksubpassdescription(lua_State *L, int arg, VkSubpassDescription *p)
    {
    int err, arg1;
    uint32_t count;
    VkAttachmentReference ref;

    ECHECK_PREAMBLE
    GetFlags(flags, "flags");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
#define F "input_attachments"
    PUSHFIELD(F);
    p->pInputAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    p->inputAttachmentCount = count;
    POPFIELD();
    if(err < 0) { freesubpassdescription(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "color_attachments"
    PUSHFIELD(F);
    p->pColorAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    p->colorAttachmentCount = count;
    POPFIELD();
    if(err < 0) { freesubpassdescription(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "resolve_attachments"
    PUSHFIELD(F);
    p->pResolveAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    POPFIELD();
    if(err < 0) { freesubpassdescription(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) 
        POPERROR();
    else if(count != p->colorAttachmentCount)
        { err = ERR_LENGTH; lua_pushstring(L, errstring(err)); }
#undef F
#define F "depth_stencil_attachment"
    PUSHFIELD(F);
    err = echeckattachmentreference(L, arg1, &ref);
    POPFIELD();
    if(err < 0) { freesubpassdescription(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
    else
        {
        p->pDepthStencilAttachment = MALLOC(L, VkAttachmentReference);
        memcpy((void*)p->pDepthStencilAttachment, &ref, sizeof(VkAttachmentReference));
        }
#undef F
#define F "preserve_attachments"
    PUSHFIELD(F);
    p->pPreserveAttachments = checkuint32list(L, arg1, &count, &err);
    p->inputAttachmentCount = count;
    POPFIELD();
    if(err < 0) { freesubpassdescription(L, p); return fielderror(L, F, err); }
#undef F
    return 0;
    }

/* echecksubpassdescriptionlist() */
static FREELISTFUNC(VkSubpassDescription, subpassdescription) 
static ECHECKLISTFUNC(VkSubpassDescription, subpassdescription, freesubpassdescriptionlist) 


void freerenderpasscreateinfo(lua_State *L, VkRenderPassCreateInfo *p)
    {
    if(!p) return;
    if(p->pAttachments) Free(L, (VkAttachmentDescription*)p->pAttachments);
    if(p->pSubpasses) freesubpassdescriptionlist(L, (void*)p->pSubpasses, p->subpassCount);
    if(p->pDependencies) Free(L, (VkSubpassDependency*)p->pDependencies);
    }

int echeckrenderpasscreateinfo(lua_State *L, int arg, VkRenderPassCreateInfo *p)
    {
    int err, arg1;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");

#define F "attachments"
    PUSHFIELD(F);
    p->pAttachments = echeckattachmentdescriptionlist(L, arg1, &p->attachmentCount, &err);
    POPFIELD();
    if(err < 0) { freerenderpasscreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "subpasses"
    PUSHFIELD(F);
    p->pSubpasses = echecksubpassdescriptionlist(L, arg1, &p->subpassCount, &err);
    POPFIELD();
    if(err) { freerenderpasscreateinfo(L, p); return efielderror(L, F); }
#undef F
#define F "dependencies"
    PUSHFIELD(F);
    p->pDependencies = echecksubpassdependencylist(L, arg1, &p->dependencyCount, &err);
    POPFIELD();
    if(err < 0) { freerenderpasscreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freeframebuffercreateinfo(lua_State *L, VkFramebufferCreateInfo *p)
    {
    if(!p) return;
    if(p->pAttachments) Free(L, (void*)p->pAttachments);
    }

int echeckframebuffercreateinfo(lua_State *L, int arg, VkFramebufferCreateInfo *p)
    {
    int err, arg1;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");

    GetRenderPass(renderPass, "render_pass");
    GetInteger(width, "width");
    GetInteger(height, "height");
    GetIntegerOpt(layers, "layers", 1);

#define F "attachments"
    PUSHFIELD(F);
    p->pAttachments = checkimage_viewlist(L, arg1, &p->attachmentCount, &err, NULL);
    POPFIELD();
    if(err < 0) return fielderror(L, F, err);
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freebuffercreateinfo(lua_State *L, VkBufferCreateInfo *p)
    {
    if(!p) return;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    }

int echeckbuffercreateinfo(lua_State *L, int arg, VkBufferCreateInfo *p)
    {
    int err, arg1;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetInteger(size, "size");
    GetFlags(usage, "usage");
    GetSharingMode(sharingMode, "sharing_mode");
#define F "queue_family_indices"
    PUSHFIELD(F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    POPFIELD();
    if(err < 0) return fielderror(L, F, err);
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckbufferviewcreateinfo(lua_State *L, int arg, VkBufferViewCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
/*  p->buffer = set by caller */
    GetFormat(format, "format");
    GetInteger(offset, "offset");
    GetInteger(range, "range");
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freeimagecreateinfo(lua_State *L, VkImageCreateInfo *p)
    {
    if(!p) return;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    }

int echeckimagecreateinfo(lua_State *L, int arg, VkImageCreateInfo *p)
    {
    int err, arg1;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetImageType(imageType, "image_type");
    GetFormat(format, "format");
    GetExtent3dOpt(extent, "extent");
    GetIntegerOpt(mipLevels, "mip_levels", 1);
    GetIntegerOpt(arrayLayers, "array_layers", 1);
    GetSamples(samples, "samples");
    GetImageTiling(tiling, "tiling");
    GetFlags(usage, "usage");
    GetImageLayout(initialLayout, "initial_layout");
    GetSharingMode(sharingMode, "sharing_mode");
#define F "queue_family_indices"
    PUSHFIELD(F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    POPFIELD();
    if(err < 0) return fielderror(L, F, err);
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckimageviewcreateinfo(lua_State *L, int arg, VkImageViewCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
/*  p->image = set by caller */
    GetImageViewType(viewType, "view_type");
    GetFormat(format, "format");
    GetComponentMappingOpt(components, "components");
    GetImageSubresourceRangeOpt(subresourceRange, "subresource_range");
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echecksamplercreateinfo(lua_State *L, int arg, VkSamplerCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetFilter(magFilter, "mag_filter");
    GetFilter(minFilter, "min_filter");
    GetSamplerMipmapMode(mipmapMode, "mipmap_mode");
    GetSamplerAddressMode(addressModeU, "address_mode_u");
    GetSamplerAddressMode(addressModeV, "address_mode_v");
    GetSamplerAddressMode(addressModeW, "address_mode_w");
    GetNumber(mipLodBias, "mip_lod_bias");
    GetBoolean(anisotropyEnable, "anisotropy_enable");
    GetNumber(maxAnisotropy, "max_anisotropy");
    GetBoolean(compareEnable, "compare_enable");
    GetCompareOp(compareOp, "compare_op");
    GetNumber(minLod, "min_lod");
    GetNumber(maxLod, "max_lod");
    GetBorderColor(borderColor, "border_color");
    GetBoolean(unnormalizedCoordinates, "unnormalized_coordinates");
    return 0;
    }


/*------------------------------------------------------------------------------*/
static ECHECKLISTFUNC(VkClearValue, clearvalue, NULL) /* echeckclearvaluelist() */

void freerenderpassbegininfo(lua_State *L, VkRenderPassBeginInfo *p)
    {
    if(!p) return;
    if(p->pClearValues) Free(L, (void*)p->pClearValues);
    }

int echeckrenderpassbegininfo(lua_State *L, int arg, VkRenderPassBeginInfo *p)
    {
    int err, arg1;
    uint32_t count;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    p->pNext = NULL;

    GetRenderPass(renderPass, "render_pass");
    GetFramebuffer(framebuffer, "framebuffer");
    GetRect2dOpt(renderArea, "render_area");
#define F "clear_values"
    PUSHFIELD(F);
    p->pClearValues = echeckclearvaluelist(L, arg1, &count, &err);
    p->clearValueCount = count;
    POPFIELD();
    if(err < 0) return efielderror(L, F);
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
        
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckcommandbufferinheritanceinfo(lua_State *L, int arg, VkCommandBufferInheritanceInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    p->pNext = NULL;
    GetRenderPassOpt(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetFramebufferOpt(framebuffer, "framebuffer");
    GetBoolean(occlusionQueryEnable, "occlusion_query_enable");
    GetFlags(queryFlags, "query_flags");
    GetFlags(pipelineStatistics, "pipeline_statistics");
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckphysicaldevicefeatures(lua_State *L, int arg, VkPhysicalDeviceFeatures *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetBoolean(robustBufferAccess, "robust_buffer_access");
    GetBoolean(fullDrawIndexUint32, "full_draw_index_uint_32");
    GetBoolean(imageCubeArray, "image_cube_array");
    GetBoolean(independentBlend, "independent_blend");
    GetBoolean(geometryShader, "geometry_shader");
    GetBoolean(tessellationShader, "tessellation_shader");
    GetBoolean(sampleRateShading, "sample_rate_shading");
    GetBoolean(dualSrcBlend, "dual_src_blend");
    GetBoolean(logicOp, "logic_op");
    GetBoolean(multiDrawIndirect, "multi_draw_indirect");
    GetBoolean(drawIndirectFirstInstance, "draw_indirect_first_instance");
    GetBoolean(depthClamp, "depth_clamp");
    GetBoolean(depthBiasClamp, "depth_bias_clamp");
    GetBoolean(fillModeNonSolid, "fill_mode_non_solid");
    GetBoolean(depthBounds, "depth_bounds");
    GetBoolean(wideLines, "wide_lines");
    GetBoolean(largePoints, "large_points");
    GetBoolean(alphaToOne, "alpha_to_one");
    GetBoolean(multiViewport, "multi_viewport");
    GetBoolean(samplerAnisotropy, "sampler_anisotropy");
    GetBoolean(textureCompressionETC2, "texture_compression_etc2");
    GetBoolean(textureCompressionASTC_LDR, "texture_compression_astc_ldr");
    GetBoolean(textureCompressionBC, "texture_compression_bc");
    GetBoolean(occlusionQueryPrecise, "occlusion_query_precise");
    GetBoolean(pipelineStatisticsQuery, "pipeline_statistics_query");
    GetBoolean(vertexPipelineStoresAndAtomics, "vertex_pipeline_stores_and_atomics");
    GetBoolean(fragmentStoresAndAtomics, "fragment_stores_and_atomics");
    GetBoolean(shaderTessellationAndGeometryPointSize, "shader_tessellation_and_geometry_point_size");
    GetBoolean(shaderImageGatherExtended, "shader_image_gather_extended");
    GetBoolean(shaderStorageImageExtendedFormats, "shader_storage_image_extended_formats");
    GetBoolean(shaderStorageImageMultisample, "shader_storage_image_multisample");
    GetBoolean(shaderStorageImageReadWithoutFormat, "shader_storage_image_read_without_format");
    GetBoolean(shaderStorageImageWriteWithoutFormat, "shader_storage_image_write_without_format");
    GetBoolean(shaderUniformBufferArrayDynamicIndexing, "shader_uniform_buffer_array_dynamic_indexing");
    GetBoolean(shaderSampledImageArrayDynamicIndexing, "shader_sampled_image_array_dynamic_indexing");
    GetBoolean(shaderStorageBufferArrayDynamicIndexing, "shader_storage_buffer_array_dynamic_indexing");
    GetBoolean(shaderStorageImageArrayDynamicIndexing, "shader_storage_image_array_dynamic_indexing");
    GetBoolean(shaderClipDistance, "shader_clip_distance");
    GetBoolean(shaderCullDistance, "shader_cull_distance");
    GetBoolean(shaderFloat64, "shader_float_64");
    GetBoolean(shaderInt64, "shader_int_64");
    GetBoolean(shaderInt16, "shader_int_16");
    GetBoolean(shaderResourceResidency, "shader_resource_residency");
    GetBoolean(shaderResourceMinLod, "shader_resource_min_lod");
    GetBoolean(sparseBinding, "sparse_binding");
    GetBoolean(sparseResidencyBuffer, "sparse_residency_buffer");
    GetBoolean(sparseResidencyImage2D, "sparse_residency_image_2d");
    GetBoolean(sparseResidencyImage3D, "sparse_residency_image_3d");
    GetBoolean(sparseResidency2Samples, "sparse_residency_2_samples");
    GetBoolean(sparseResidency4Samples, "sparse_residency_4_samples");
    GetBoolean(sparseResidency8Samples, "sparse_residency_8_samples");
    GetBoolean(sparseResidency16Samples, "sparse_residency_16_samples");
    GetBoolean(sparseResidencyAliased, "sparse_residency_aliased");
    GetBoolean(variableMultisampleRate, "variable_multisample_rate");
    GetBoolean(inheritedQueries, "inherited_queries");
    return 0;
    }
 
static int echeckphysicaldevice16bitstoragefeatures(lua_State *L, int arg, VkPhysicalDevice16BitStorageFeaturesKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    GetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    GetBoolean(storagePushConstant16, "storage_push_constant_16");
    GetBoolean(storageInputOutput16, "storage_input_output_16");
    return 0;
    }

static int echeckphysicaldevicevariablepointerfeatures(lua_State *L, int arg, VkPhysicalDeviceVariablePointerFeaturesKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    GetBoolean(variablePointers, "variable_pointers");
    return 0;
    }

typedef struct {
    VkPhysicalDeviceFeatures2KHR p1;
    VkPhysicalDevice16BitStorageFeaturesKHR p2;
    VkPhysicalDeviceVariablePointerFeaturesKHR p3;
} VkPhysicalDeviceFeatures2KHR_CHAIN;

VkPhysicalDeviceFeatures2KHR* newphysicaldevicefeatures2(lua_State *L)
    {
    VkPhysicalDeviceFeatures2KHR_CHAIN *p = MALLOC_NOERR(L, VkPhysicalDeviceFeatures2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
    p->p3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = &p->p3;
    p->p3.pNext = NULL;
    return (VkPhysicalDeviceFeatures2KHR*)p;
    }

void freephysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2KHR *p)
    {
    Free(L, (void*)p);
    }

static int echeckphysicaldevicefeatures2(lua_State *L, int arg, VkPhysicalDeviceFeatures2KHR *pp)
    {
    int err;
    VkPhysicalDeviceFeatures2KHR_CHAIN *p = (VkPhysicalDeviceFeatures2KHR_CHAIN*)pp;
    err = echeckphysicaldevicefeatures(L, arg, &p->p1.features);
    if(err) return err;
    err = echeckphysicaldevice16bitstoragefeatures(L, arg, &p->p2);
    if(err) return err;
    err = echeckphysicaldevicevariablepointerfeatures(L, arg, &p->p3);
    if(err) return err;
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
    p->p3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = &p->p3;
    p->p3.pNext = NULL;
    return 0;
    }

int pushphysicaldevicefeatures(lua_State *L, VkPhysicalDeviceFeatures *p)
    {
    lua_newtable(L);
    SetBoolean(robustBufferAccess, "robust_buffer_access");
    SetBoolean(fullDrawIndexUint32, "full_draw_index_uint_32");
    SetBoolean(imageCubeArray, "image_cube_array");
    SetBoolean(independentBlend, "independent_blend");
    SetBoolean(geometryShader, "geometry_shader");
    SetBoolean(tessellationShader, "tessellation_shader");
    SetBoolean(sampleRateShading, "sample_rate_shading");
    SetBoolean(dualSrcBlend, "dual_src_blend");
    SetBoolean(logicOp, "logic_op");
    SetBoolean(multiDrawIndirect, "multi_draw_indirect");
    SetBoolean(drawIndirectFirstInstance, "draw_indirect_first_instance");
    SetBoolean(depthClamp, "depth_clamp");
    SetBoolean(depthBiasClamp, "depth_bias_clamp");
    SetBoolean(fillModeNonSolid, "fill_mode_non_solid");
    SetBoolean(depthBounds, "depth_bounds");
    SetBoolean(wideLines, "wide_lines");
    SetBoolean(largePoints, "large_points");
    SetBoolean(alphaToOne, "alpha_to_one");
    SetBoolean(multiViewport, "multi_viewport");
    SetBoolean(samplerAnisotropy, "sampler_anisotropy");
    SetBoolean(textureCompressionETC2, "texture_compression_etc2");
    SetBoolean(textureCompressionASTC_LDR, "texture_compression_astc_ldr");
    SetBoolean(textureCompressionBC, "texture_compression_bc");
    SetBoolean(occlusionQueryPrecise, "occlusion_query_precise");
    SetBoolean(pipelineStatisticsQuery, "pipeline_statistics_query");
    SetBoolean(vertexPipelineStoresAndAtomics, "vertex_pipeline_stores_and_atomics");
    SetBoolean(fragmentStoresAndAtomics, "fragment_stores_and_atomics");
    SetBoolean(shaderTessellationAndGeometryPointSize, "shader_tessellation_and_geometry_point_size");
    SetBoolean(shaderImageGatherExtended, "shader_image_gather_extended");
    SetBoolean(shaderStorageImageExtendedFormats, "shader_storage_image_extended_formats");
    SetBoolean(shaderStorageImageMultisample, "shader_storage_image_multisample");
    SetBoolean(shaderStorageImageReadWithoutFormat, "shader_storage_image_read_without_format");
    SetBoolean(shaderStorageImageWriteWithoutFormat, "shader_storage_image_write_without_format");
    SetBoolean(shaderUniformBufferArrayDynamicIndexing, "shader_uniform_buffer_array_dynamic_indexing");
    SetBoolean(shaderSampledImageArrayDynamicIndexing, "shader_sampled_image_array_dynamic_indexing");
    SetBoolean(shaderStorageBufferArrayDynamicIndexing, "shader_storage_buffer_array_dynamic_indexing");
    SetBoolean(shaderStorageImageArrayDynamicIndexing, "shader_storage_image_array_dynamic_indexing");
    SetBoolean(shaderClipDistance, "shader_clip_distance");
    SetBoolean(shaderCullDistance, "shader_cull_distance");
    SetBoolean(shaderFloat64, "shader_float_64");
    SetBoolean(shaderInt64, "shader_int_64");
    SetBoolean(shaderInt16, "shader_int_16");
    SetBoolean(shaderResourceResidency, "shader_resource_residency");
    SetBoolean(shaderResourceMinLod, "shader_resource_min_lod");
    SetBoolean(sparseBinding, "sparse_binding");
    SetBoolean(sparseResidencyBuffer, "sparse_residency_buffer");
    SetBoolean(sparseResidencyImage2D, "sparse_residency_image_2d");
    SetBoolean(sparseResidencyImage3D, "sparse_residency_image_3d");
    SetBoolean(sparseResidency2Samples, "sparse_residency_2_samples");
    SetBoolean(sparseResidency4Samples, "sparse_residency_4_samples");
    SetBoolean(sparseResidency8Samples, "sparse_residency_8_samples");
    SetBoolean(sparseResidency16Samples, "sparse_residency_16_samples");
    SetBoolean(sparseResidencyAliased, "sparse_residency_aliased");
    SetBoolean(variableMultisampleRate, "variable_multisample_rate");
    SetBoolean(inheritedQueries, "inherited_queries");

    return 1;
    } 

static int pushphysicaldevice16bitstoragefeatures(lua_State *L, VkPhysicalDevice16BitStorageFeaturesKHR *p)
    {
    SetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    SetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    SetBoolean(storagePushConstant16, "storage_push_constant_16");
    SetBoolean(storageInputOutput16, "storage_input_output_16");
    return 1;
    }

static int pushphysicaldevicevariablepointerfeatures(lua_State *L, VkPhysicalDeviceVariablePointerFeaturesKHR *p)
    {
    SetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    SetBoolean(variablePointers, "variable_pointers");
    return 1;
    }

int pushphysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2KHR *pp)
    {
    VkPhysicalDeviceFeatures2KHR_CHAIN *p = (VkPhysicalDeviceFeatures2KHR_CHAIN*)pp;
    pushphysicaldevicefeatures(L, &p->p1.features);
    pushphysicaldevice16bitstoragefeatures(L, &p->p2);
    pushphysicaldevicevariablepointerfeatures(L, &p->p3);
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushphysicaldevicelimits(lua_State *L, VkPhysicalDeviceLimits *p)
    {
    lua_newtable(L);
    SetInteger(maxImageDimension1D, "max_image_dimension_1d");
    SetInteger(maxImageDimension2D, "max_image_dimension_2d");
    SetInteger(maxImageDimension3D, "max_image_dimension_3d");
    SetInteger(maxImageDimensionCube, "max_image_dimension_cube");
    SetInteger(maxImageArrayLayers, "max_image_array_layers");
    SetInteger(maxTexelBufferElements, "max_texel_buffer_elements");
    SetInteger(maxUniformBufferRange, "max_uniform_buffer_range");
    SetInteger(maxStorageBufferRange, "max_storage_buffer_range");
    SetInteger(maxPushConstantsSize, "max_push_constants_size");
    SetInteger(maxMemoryAllocationCount, "max_memory_allocation_count");
    SetInteger(maxSamplerAllocationCount, "max_sampler_allocation_count");
    SetInteger(bufferImageGranularity, "buffer_image_granularity");
    SetInteger(sparseAddressSpaceSize, "sparse_address_space_size");
    SetInteger(maxBoundDescriptorSets, "max_bound_descriptor_sets");
    SetInteger(maxPerStageDescriptorSamplers, "max_per_stage_descriptor_samplers");
    SetInteger(maxPerStageDescriptorUniformBuffers, "max_per_stage_descriptor_uniform_buffers");
    SetInteger(maxPerStageDescriptorStorageBuffers, "max_per_stage_descriptor_storage_buffers");
    SetInteger(maxPerStageDescriptorSampledImages, "max_per_stage_descriptor_sampled_images");
    SetInteger(maxPerStageDescriptorStorageImages, "max_per_stage_descriptor_storage_images");
    SetInteger(maxPerStageDescriptorInputAttachments, "max_per_stage_descriptor_input_attachments");
    SetInteger(maxPerStageResources, "max_per_stage_resources");
    SetInteger(maxDescriptorSetSamplers, "max_descriptor_set_samplers");
    SetInteger(maxDescriptorSetUniformBuffers, "max_descriptor_set_uniform_buffers");
    SetInteger(maxDescriptorSetUniformBuffersDynamic, "max_descriptor_set_uniform_buffers_dynamic");
    SetInteger(maxDescriptorSetStorageBuffers, "max_descriptor_set_storage_buffers");
    SetInteger(maxDescriptorSetStorageBuffersDynamic, "max_descriptor_set_storage_buffers_dynamic");
    SetInteger(maxDescriptorSetSampledImages, "max_descriptor_set_sampled_images");
    SetInteger(maxDescriptorSetStorageImages, "max_descriptor_set_storage_images");
    SetInteger(maxDescriptorSetInputAttachments, "max_descriptor_set_input_attachments");
    SetInteger(maxVertexInputAttributes, "max_vertex_input_attributes");
    SetInteger(maxVertexInputBindings, "max_vertex_input_bindings");
    SetInteger(maxVertexInputAttributeOffset, "max_vertex_input_attribute_offset");
    SetInteger(maxVertexInputBindingStride, "max_vertex_input_binding_stride");
    SetInteger(maxVertexOutputComponents, "max_vertex_output_components");
    SetInteger(maxTessellationGenerationLevel, "max_tessellation_generation_level");
    SetInteger(maxTessellationPatchSize, "max_tessellation_patch_size");
    SetInteger(maxTessellationControlPerVertexInputComponents, "max_tessellation_control_per_vertex_input_components");
    SetInteger(maxTessellationControlPerVertexOutputComponents, "max_tessellation_control_per_vertex_output_components");
    SetInteger(maxTessellationControlPerPatchOutputComponents, "max_tessellation_control_per_patch_output_components");
    SetInteger(maxTessellationControlTotalOutputComponents, "max_tessellation_control_total_output_components");
    SetInteger(maxTessellationEvaluationInputComponents, "max_tessellation_evaluation_input_components");
    SetInteger(maxTessellationEvaluationOutputComponents, "max_tessellation_evaluation_output_components");
    SetInteger(maxGeometryShaderInvocations, "max_geometry_shader_invocations");
    SetInteger(maxGeometryInputComponents, "max_geometry_input_components");
    SetInteger(maxGeometryOutputComponents, "max_geometry_output_components");
    SetInteger(maxGeometryOutputVertices, "max_geometry_output_vertices");
    SetInteger(maxGeometryTotalOutputComponents, "max_geometry_total_output_components");
    SetInteger(maxFragmentInputComponents, "max_fragment_input_components");
    SetInteger(maxFragmentOutputAttachments, "max_fragment_output_attachments");
    SetInteger(maxFragmentDualSrcAttachments, "max_fragment_dual_src_attachments");
    SetInteger(maxFragmentCombinedOutputResources, "max_fragment_combined_output_resources");
    SetInteger(maxComputeSharedMemorySize, "max_compute_shared_memory_size");
    SetIntegerArray(maxComputeWorkGroupCount, "max_compute_work_group_count", 3);
    SetInteger(maxComputeWorkGroupInvocations, "max_compute_work_group_invocations");
    SetIntegerArray(maxComputeWorkGroupSize, "max_compute_work_group_size", 3);
    SetInteger(subPixelPrecisionBits, "sub_pixel_precision_bits");
    SetInteger(subTexelPrecisionBits, "sub_texel_precision_bits");
    SetInteger(mipmapPrecisionBits, "mipmap_precision_bits");
    SetInteger(maxDrawIndexedIndexValue, "max_draw_indexed_index_value");
    SetInteger(maxDrawIndirectCount, "max_draw_indirect_count");
    SetInteger(maxSamplerLodBias, "max_sampler_lod_bias");
    SetNumber(maxSamplerAnisotropy, "max_sampler_anisotropy");
    SetInteger(maxViewports, "max_viewports");
    SetIntegerArray(maxViewportDimensions, "max_viewport_dimensions", 2);
    SetNumberArray(viewportBoundsRange, "viewport_bounds_range", 2);
    SetInteger(viewportSubPixelBits, "viewport_sub_pixel_bits");
    SetInteger(minMemoryMapAlignment, "min_memory_map_alignment");
    SetInteger(minTexelBufferOffsetAlignment, "min_texel_buffer_offset_alignment");
    SetInteger(minUniformBufferOffsetAlignment, "minuniform_buffer_offset_alignment");
    SetInteger(minStorageBufferOffsetAlignment, "min_storage_buffer_offset_alignment");
    SetInteger(minTexelOffset, "min_texel_offset");
    SetInteger(maxTexelOffset, "max_texel_offset");
    SetInteger(minTexelGatherOffset, "min_texel_gather_offset");
    SetInteger(maxTexelGatherOffset, "max_texel_gather_offset");
    SetNumber(minInterpolationOffset, "min_interpolation_offset");
    SetNumber(maxInterpolationOffset, "max_interpolation_offset");
    SetInteger(subPixelInterpolationOffsetBits, "sub_pixel_interpolation_offset_bits");
    SetInteger(maxFramebufferWidth, "max_framebuffer_width");
    SetInteger(maxFramebufferHeight, "max_framebuffer_height");
    SetInteger(maxFramebufferLayers, "max_framebuffer_layers");
    SetFlags(framebufferColorSampleCounts, "framebuffer_color_sample_counts");
    SetFlags(framebufferDepthSampleCounts, "framebuffer_depth_sample_counts");
    SetFlags(framebufferStencilSampleCounts, "framebuffer_stencil_sample_counts");
    SetFlags(framebufferNoAttachmentsSampleCounts, "framebuffer_no_attachments_sample_counts");
    SetInteger(maxColorAttachments, "max_color_attachments");
    SetFlags(sampledImageColorSampleCounts, "sampled_image_color_sample_counts");
    SetFlags(sampledImageIntegerSampleCounts, "sampled_image_integer_sample_counts");
    SetFlags(sampledImageDepthSampleCounts, "sampled_image_depth_sample_counts");
    SetFlags(sampledImageStencilSampleCounts, "sampled_image_stencil_sample_counts");
    SetFlags(storageImageSampleCounts, "storage_image_sample_counts");
    SetInteger(maxSampleMaskWords, "max_sample_mask_words");
    SetBoolean(timestampComputeAndGraphics, "timestamp_compute_and_graphics");
    SetNumber(timestampPeriod, "timestamp_period");
    SetInteger(maxClipDistances, "max_clip_distances");
    SetInteger(maxCullDistances, "max_cull_distances");
    SetInteger(maxCombinedClipAndCullDistances, "max_combined_clip_and_cull_distances");
    SetInteger(discreteQueuePriorities, "discrete_queue_priorities");
    SetNumberArray(pointSizeRange, "point_size_range", 2);
    SetNumberArray(lineWidthRange, "line_width_range", 2);
    SetNumber(pointSizeGranularity, "point_size_granularity");
    SetNumber(lineWidthGranularity, "line_width_granularity");
    SetBoolean(strictLines, "strict_lines");
    SetBoolean(standardSampleLocations, "standard_sample_locations");
    SetInteger(optimalBufferCopyOffsetAlignment, "optimal_buffer_copy_offset_alignment");
    SetInteger(optimalBufferCopyRowPitchAlignment, "optimal_buffer_copy_row_pitch_alignment");
    SetInteger(nonCoherentAtomSize, "non_coherent_atom_size");
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushphysicaldevicesparseproperties(lua_State *L, VkPhysicalDeviceSparseProperties *p)
    {
    lua_newtable(L);
    SetBoolean(residencyStandard2DBlockShape, "residency_standard_2d_block_shape");
    SetBoolean(residencyStandard2DMultisampleBlockShape, "residency_standard_2d_multisample_block_shape");
    SetBoolean(residencyStandard3DBlockShape, "residency_standard_3d_block_shape");
    SetBoolean(residencyAlignedMipSize, "residency_aligned_mip_size");
    SetBoolean(residencyNonResidentStrict, "residency_non_resident_strict");
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushphysicaldeviceproperties(lua_State *L, VkPhysicalDeviceProperties *p)
    {
    lua_newtable(L);
    SetInteger(apiVersion, "api_version");
    SetInteger(driverVersion, "driver_version");
    SetInteger(vendorID, "vendor_id");
    SetInteger(deviceID, "device_id");
    SetEnum(deviceType, "device_type", pushphysicaldevicetype);
    SetString(deviceName, "device_name");
    SetUUID(pipelineCacheUUID, "pipeline_cache_uuid", VK_UUID_SIZE);
    SetStruct(limits, "limits", pushphysicaldevicelimits);
    SetStruct(sparseProperties, "sparse_properties", pushphysicaldevicesparseproperties);
    return 1;
    }


static int pushphysicaldevicepushdescriptorproperties(lua_State *L, VkPhysicalDevicePushDescriptorPropertiesKHR  *p)
    {
    SetInteger(maxPushDescriptors, "max_push_descriptors");
    return 0;
    }

typedef struct {
    VkPhysicalDeviceProperties2KHR p1;
    VkPhysicalDevicePushDescriptorPropertiesKHR p2;
} VkPhysicalDeviceProperties2KHR_CHAIN;

VkPhysicalDeviceProperties2KHR* newphysicaldeviceproperties2(lua_State *L)
    {
    VkPhysicalDeviceProperties2KHR_CHAIN *p = MALLOC_NOERR(L, VkPhysicalDeviceProperties2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = NULL;
    return (VkPhysicalDeviceProperties2KHR*)p;
    }

void freephysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushphysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2KHR *pp)
    {
    VkPhysicalDeviceProperties2KHR_CHAIN *p = (VkPhysicalDeviceProperties2KHR_CHAIN*)pp;
    pushphysicaldeviceproperties(L, &p->p1.properties);
    pushphysicaldevicepushdescriptorproperties(L, &p->p2);
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushformatproperties(lua_State *L, VkFormatProperties *p)
    {
    lua_newtable(L);
    SetFlags(linearTilingFeatures, "linear_tiling_features");
    SetFlags(optimalTilingFeatures, "optimal_tiling_features");
    SetFlags(bufferFeatures, "buffer_features");
    return 1;
    }

typedef struct {
    VkFormatProperties2KHR p1;
} VkFormatProperties2KHR_CHAIN;

VkFormatProperties2KHR* newformatproperties2(lua_State *L)
    {
    VkFormatProperties2KHR_CHAIN *p = MALLOC_NOERR(L, VkFormatProperties2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR;
    p->p1.pNext = NULL;
    return (VkFormatProperties2KHR*)p;
    }

void freeformatproperties2(lua_State *L, VkFormatProperties2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushformatproperties2(lua_State *L, VkFormatProperties2KHR *pp)
    {
    VkFormatProperties2KHR_CHAIN *p = (VkFormatProperties2KHR_CHAIN*)pp;
    pushformatproperties(L, &p->p1.formatProperties);
    return 1;
    }

/*------------------------------------------------------------------------------*/
int pushimageformatproperties(lua_State *L, VkImageFormatProperties *p)
    {
    lua_newtable(L);
    SetStruct(maxExtent, "max_extent", pushextent3d);
    SetInteger(maxMipLevels, "max_mip_levels");
    SetInteger(maxArrayLayers, "max_array_layers");
    SetInteger(sampleCounts, "sample_counts");
    SetInteger(maxResourceSize, "max_resource_size");
    return 1;
    }

typedef struct {
    VkImageFormatProperties2KHR p1;
} VkImageFormatProperties2KHR_CHAIN;

VkImageFormatProperties2KHR* newimageformatproperties2(lua_State *L)
    {
    VkImageFormatProperties2KHR_CHAIN *p = MALLOC_NOERR(L, VkImageFormatProperties2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
    p->p1.pNext = NULL;
    return (VkImageFormatProperties2KHR*)p;
    }

void freeimageformatproperties2(lua_State *L, VkImageFormatProperties2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushimageformatproperties2(lua_State *L, VkImageFormatProperties2KHR *pp)
    {
    VkImageFormatProperties2KHR_CHAIN *p = (VkImageFormatProperties2KHR_CHAIN*)pp;
    pushimageformatproperties(L, &p->p1.imageFormatProperties);
    return 1;
    }

/*------------------------------------------------------------------------------*/
int pushsparseimageformatproperties(lua_State *L, VkSparseImageFormatProperties *p)
    {
    lua_newtable(L);
    SetFlags(aspectMask, "aspect_mask");
    SetStruct(imageGranularity, "image_granularity", pushextent3d);
    SetFlags(flags, "flags");
    return 1;
    }

VkSparseImageFormatProperties2KHR *newsparseimageformatproperties2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkSparseImageFormatProperties2KHR *p = NMALLOC_NOERR(L, VkSparseImageFormatProperties2KHR, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR;
        p[i].pNext = NULL;
        }
    return p;
    }

void freesparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2KHR *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushsparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2KHR *p)
    {
    pushsparseimageformatproperties(L, &p->properties);
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushqueuefamilyproperties(lua_State *L, VkQueueFamilyProperties *p, uint32_t index)
    {
    lua_newtable(L);
    lua_pushinteger(L, index); lua_setfield(L, -2, "queue_family_index");
    SetFlags(queueFlags, "queue_flags");
    SetInteger(queueCount, "queue_count");
    SetInteger(timestampValidBits, "timestamp_valid_bits");
    SetStruct(minImageTransferGranularity, "min_image_transfer_granularity", pushextent3d);
    return 1;
    }

VkQueueFamilyProperties2KHR *newqueuefamilyproperties2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkQueueFamilyProperties2KHR *p = NMALLOC_NOERR(L, VkQueueFamilyProperties2KHR, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR;
        p[i].pNext = NULL;
        }
    return p;
    }

void freequeuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2KHR *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushqueuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2KHR *p, uint32_t index)
    {
    pushqueuefamilyproperties(L, &p->queueFamilyProperties, index);
    return 1;
    }

/*------------------------------------------------------------------------------*/
static int pushmemorytype(lua_State *L, VkMemoryType *p, uint32_t index)
    {
    lua_newtable(L);
    lua_pushinteger(L, index); lua_setfield(L, -2, "memory_type_index");
    SetFlags(propertyFlags, "property_flags");
    SetInteger(heapIndex, "heap_index");
    return 1;
    }

static int pushmemoryheap(lua_State *L, VkMemoryHeap *p, uint32_t index)
    {
    lua_newtable(L);
    lua_pushinteger(L, index); lua_setfield(L, -2, "memory_heap_index");
    SetInteger(size, "size");
    SetFlags(flags, "flags");
    return 1;
    }

int pushphysicaldevicememoryproperties(lua_State *L, VkPhysicalDeviceMemoryProperties *p)
    {
    uint32_t i;
    lua_newtable(L);
    lua_newtable(L);
    for(i = 0; i < p->memoryTypeCount; i++)
        {
        pushmemorytype(L, &(p->memoryTypes[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "memory_types");
    lua_newtable(L);
    for(i = 0; i < p->memoryHeapCount; i++)
        {
        pushmemoryheap(L, &(p->memoryHeaps[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "memory_heaps");
    return 1;
    }

typedef struct {
    VkPhysicalDeviceMemoryProperties2KHR p1;
} VkPhysicalDeviceMemoryProperties2KHR_CHAIN; 

VkPhysicalDeviceMemoryProperties2KHR* newphysicaldevicememoryproperties2(lua_State *L)
    {
   VkPhysicalDeviceMemoryProperties2KHR_CHAIN *p = MALLOC_NOERR(L, VkPhysicalDeviceMemoryProperties2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
    p->p1.pNext = NULL;
    return (VkPhysicalDeviceMemoryProperties2KHR*)p;
    }

void freephysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushphysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2KHR *pp)
    {
    VkPhysicalDeviceMemoryProperties2KHR_CHAIN *p = (VkPhysicalDeviceMemoryProperties2KHR_CHAIN*)pp;
    pushphysicaldevicememoryproperties(L, &p->p1.memoryProperties);
    return 1;
    }

/*------------------------------------------------------------------------------*/
int echeckphysicaldeviceimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceImageFormatInfo2KHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
    p->pNext = NULL;
    GetFormat(format, "format");
    GetImageType(type, "type");
    GetImageTiling(tiling, "tiling");
    GetFlags(usage, "usage");
    GetFlags(flags, "flags");
    return 0;
    }

int echeckphysicaldevicesparseimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceSparseImageFormatInfo2KHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2_KHR;
    p->pNext = NULL;
    GetFormat(format, "format");
    GetImageType(type, "type");
    GetSamples(samples, "samples");
    GetFlags(usage, "usage");
    GetImageTiling(tiling, "tiling");
    return 0;
    }

/*------------------------------------------------------------------------------*/
int pushextensionproperties(lua_State *L, VkExtensionProperties *p)
    {
    lua_newtable(L);
    SetString(extensionName, "extension_name");
    SetInteger(specVersion, "spec_version");
    return 1;
    }

int pushlayerproperties(lua_State *L, VkLayerProperties *p)
    {
    lua_newtable(L);
    SetString(layerName, "layer_name");
    SetInteger(specVersion, "spec_version");
    SetInteger(implementationVersion, "implementation_version");
    SetString(description, "description");
    return 1;
    }

/*------------------------------------------------------------------------------*/

int echeckviewport(lua_State *L, int arg, VkViewport *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetNumber(x, "x");
    GetNumber(y, "y");
    GetNumber(width, "width");
    GetNumber(height, "height");
    GetNumber(minDepth, "min_depth");
    GetNumber(maxDepth, "max_depth");
    return 0;
    }

/* echeckviewportlist() */
ECHECKLISTFUNC(VkViewport, viewport, NULL) 

int pushviewport(lua_State *L, VkViewport *p)
    {
    lua_newtable(L);
    SetNumber(x, "x");
    SetNumber(y, "y");
    SetNumber(width, "width");
    SetNumber(height, "height");
    SetNumber(minDepth, "min_depth");
    SetNumber(maxDepth, "max_depth");
    return 1;
    }

/*------------------------------------------------------------------------------*/

int echeckoffset2d(lua_State *L, int arg, VkOffset2D *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(x, "x");
    GetInteger(y, "y");
    return 0;
    }

int echeckoffset3d(lua_State *L, int arg, VkOffset3D *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(x, "x");
    GetInteger(y, "y");
    GetInteger(z, "z");
    return 0;
    }

int echeckextent2d(lua_State *L, int arg, VkExtent2D *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(width, "width");
    GetInteger(height, "height");
    return 0;
    }

int echeckextent3d(lua_State *L, int arg, VkExtent3D *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(width, "width");
    GetInteger(height, "height");
    GetInteger(depth, "depth");
    return 0;
    }

int pushoffset2d(lua_State *L, VkOffset2D *p)
    {
    lua_newtable(L);
    SetInteger(x, "x");
    SetInteger(y, "y");
    return 1;
    }

int pushoffset3d(lua_State *L, VkOffset3D *p)
    {
    lua_newtable(L);
    SetInteger(x, "x");
    SetInteger(y, "y");
    SetInteger(z, "z");
    return 1;
    }

int pushextent2d(lua_State *L, VkExtent2D *p)
    {
    lua_newtable(L);
    SetInteger(width, "width");
    SetInteger(height, "height");
    return 1;
    }

int pushextent3d(lua_State *L, VkExtent3D *p)
    {
    lua_newtable(L);
    SetInteger(width, "width");
    SetInteger(height, "height");
    SetInteger(depth, "depth");
    return 1;
    }


/*------------------------------------------------------------------------------*/

int echeckrect2d(lua_State *L, int arg, VkRect2D *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetOffset2dOpt(offset, "offset");
    GetExtent2dOpt(extent, "extent");
    return 0;
    }

/* echeckrect2dlist() */
ECHECKLISTFUNC(VkRect2D, rect2d, NULL) 

int pushrect2d(lua_State *L, VkRect2D *p)
    {
    lua_newtable(L);
    SetStruct(offset, "offset", pushoffset2d);
    SetStruct(extent, "extent", pushextent2d);
    return 1;
    }

/*------------------------------------------------------------------------------*/

int echeckcomponentmapping(lua_State *L, int arg, VkComponentMapping *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetComponentSwizzle(r, "r");
    GetComponentSwizzle(g, "g");
    GetComponentSwizzle(b, "b");
    GetComponentSwizzle(a, "a");
    return 0;
    }

int pushcomponentmapping(lua_State *L, VkComponentMapping *p)
    {
    lua_newtable(L);
    SetEnum(r, "r", pushcomponentswizzle);
    SetEnum(g, "g", pushcomponentswizzle);
    SetEnum(b, "b", pushcomponentswizzle);
    SetEnum(a, "a", pushcomponentswizzle);
    return 1;
    }


/*------------------------------------------------------------------------------*/

int echeckimagesubresourcerange(lua_State *L, int arg, VkImageSubresourceRange *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(baseMipLevel, "base_mip_level");
    GetIntegerOrRemaining(levelCount, "level_count", 1);
    GetInteger(baseArrayLayer, "base_array_layer");
    GetIntegerOrRemaining(layerCount, "layer_count", 1);
    return 0;
    }

/* echeckimagesubresourcerangelist() */
ECHECKLISTFUNC(VkImageSubresourceRange, imagesubresourcerange, NULL) 


int pushimagesubresourcerange(lua_State *L, VkImageSubresourceRange *p)
    {
    lua_newtable(L);
    SetFlags(aspectMask, "aspect_mask");
    SetInteger(baseMipLevel, "base_mip_level");
    SetInteger(levelCount, "level_count");
    SetInteger(baseArrayLayer, "base_array_layer");
    SetInteger(layerCount, "layer_count");
    return 1;
    }


/*------------------------------------------------------------------------------*/

int echeckclearcolorvalue(lua_State *L, int arg, VkClearColorValue *p)
    {
    int i, t;
    const char* s;
    int colortype = -1;

    ECHECK_PREAMBLE
    lua_pushstring(L, "t"); lua_rawget(L, arg);
    s = luaL_optstring(L, -1, NULL);
    if(!s || (strcmp(s, "float32")==0))
        colortype = 0;
    else if(strcmp(s, "int32")==0)
        colortype = 1;
    else if(strcmp(s, "uint32")==0)
        colortype = 2;
    else 
        { lua_pop(L, 1); lua_pushstring(L, "t"); return ERR_VALUE; }
    lua_pop(L, 1);
    
    if(colortype == 0) /* float32 */
        {
        for(i = 0; i < 4; i++)
            {
            t = lua_rawgeti(L, arg, i + 1);
            if(t != LUA_TNUMBER)
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1); return ERR_TYPE; } 
            p->float32[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
            }
        }
    else if(colortype == 1) /* int32 */
        {
        for(i = 0; i < 4; i++)
            {
            lua_rawgeti(L, arg, i + 1);
            if(!lua_isinteger(L, -1))
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1); return ERR_TYPE; } 
            p->int32[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
            }
        }
    else if(colortype == 2) /* uint32 */
        {
        for(i = 0; i < 4; i++)
            {
            lua_rawgeti(L, arg, i + 1);
            if(!lua_isinteger(L, -1))
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1); return ERR_TYPE; } 
            p->uint32[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
            }
        }

    return 0;
    }

#if 0 
/* depth and stencil are passed from Lua separately */
int echeckcleardepthstencilvalue(lua_State *L, int arg, VkClearDepthStencilValue *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetNumber(depth, "depth");
    GetInteger(stencil, "stencil");
    return 0;
    }
#endif

/* e.g. clearvalue = { color = { 1, 1, 1, 1, t='uint32' } } 
 *      clearvalue = { depth = 0.8, stencil = 12 } 
 */
int echeckclearvalue(lua_State *L, int arg, VkClearValue *p)
    {
    int err, t;
    ECHECK_PREAMBLE
    lua_pushstring(L, "depth");
    t = lua_rawget(L, arg);
    lua_pop(L, 1);
    if(t==LUA_TNIL)
        {
        GetClearColorValue(color, "color");
        return 0;
        }
    GetNumber(depthStencil.depth, "depth");
    GetInteger(depthStencil.stencil, "stencil");
    return 0;
    }


int echeckclearattachment(lua_State *L, int arg, VkClearAttachment *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(aspectMask, "aspect_mask");
    GetAttachment(colorAttachment, "color_attachment");
    GetClearValue(clearValue, "clear_value");
    return 0;
    }

/* echeckclearattachmentlist() */
ECHECKLISTFUNC(VkClearAttachment, clearattachment, NULL) 

int echeckclearrect(lua_State *L, int arg, VkClearRect *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetRect2dOpt(rect, "rect");
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
    return 0;
    }

/* echeckclearrectlist() */
ECHECKLISTFUNC(VkClearRect, clearrect, NULL) 

int echeckimagesubresourcelayers(lua_State *L, int arg, VkImageSubresourceLayers *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
    return 0;
    }

int echeckimagecopy(lua_State *L, int arg, VkImageCopy *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetImageSubresourceLayers(srcSubresource, "src_subresource");
    GetOffset3dOpt(srcOffset, "src_offset");
    GetImageSubresourceLayers(dstSubresource, "dst_subresource");
    GetOffset3dOpt(dstOffset, "dst_offset");
    GetExtent3dOpt(extent, "extent");
    return 0;
    }

/* echeckimagecopylist() */
ECHECKLISTFUNC(VkImageCopy, imagecopy, NULL) 


int echeckimageblit(lua_State *L, int arg, VkImageBlit *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetImageSubresourceLayers(srcSubresource, "src_subresource");
    GetStructArrayOpt(srcOffsets, "src_offsets", 2, echeckoffset3d);
    GetImageSubresourceLayers(dstSubresource, "dst_subresource");
    GetStructArrayOpt(dstOffsets, "dst_offsets", 2, echeckoffset3d);
    return 0;
    }

/* echeckimageblitlist() */
ECHECKLISTFUNC(VkImageBlit, imageblit, NULL) 

int echeckbufferimagecopy(lua_State *L, int arg, VkBufferImageCopy *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(bufferOffset, "buffer_offset");
    GetInteger(bufferRowLength, "buffer_row_length");
    GetInteger(bufferImageHeight, "buffer_image_height");
    GetImageSubresourceLayers(imageSubresource, "image_subresource");
    GetOffset3dOpt(imageOffset, "image_offset");
    GetExtent3dOpt(imageExtent, "image_extent");
    return 0;
    }

/* echeckbufferimagecopylist() */
ECHECKLISTFUNC(VkBufferImageCopy, bufferimagecopy, NULL) 

int echeckimageresolve(lua_State *L, int arg, VkImageResolve *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetImageSubresourceLayers(srcSubresource, "src_subresource");
    GetOffset3dOpt(srcOffset, "src_offset");
    GetImageSubresourceLayers(dstSubresource, "dst_subresource");
    GetOffset3dOpt(dstOffset, "dst_offset");
    GetExtent3dOpt(extent, "extent");
    return 0;
    }

/* echeckimageresolvelist() */
ECHECKLISTFUNC(VkImageResolve, imageresolve, NULL) 

int echeckbuffercopy(lua_State *L, int arg, VkBufferCopy *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(srcOffset, "src_offset");
    GetInteger(dstOffset, "dst_offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckbuffercopylist() */
ECHECKLISTFUNC(VkBufferCopy, buffercopy, NULL) 

int echeckmemorybarrier(lua_State *L, int arg, VkMemoryBarrier *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    p->pNext = NULL;
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    return 0;
    }

/* echeckmemorybarrierlist() */
ECHECKLISTFUNC(VkMemoryBarrier, memorybarrier, NULL) 

int echeckbuffermemorybarrier(lua_State *L, int arg, VkBufferMemoryBarrier *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    p->pNext = NULL;
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetIntegerOpt(srcQueueFamilyIndex, "src_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetIntegerOpt(dstQueueFamilyIndex, "dst_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckbuffermemorybarrierlist() */
ECHECKLISTFUNC(VkBufferMemoryBarrier, buffermemorybarrier, NULL) 

int echeckimagememorybarrier(lua_State *L, int arg, VkImageMemoryBarrier *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    p->pNext = NULL;
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetImageLayout(oldLayout, "old_layout");
    GetImageLayout(newLayout, "new_layout");
    GetIntegerOpt(srcQueueFamilyIndex, "src_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetIntegerOpt(dstQueueFamilyIndex, "dst_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetImage(image, "image");
    GetImageSubresourceRangeOpt(subresourceRange, "subresource_range");
    return 0;
    }

/* echeckimagememorybarrierlist() */
ECHECKLISTFUNC(VkImageMemoryBarrier, imagememorybarrier, NULL) 

static int echeckpushconstantrange(lua_State *L, int arg, VkPushConstantRange *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(stageFlags, "stage_flags");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckpushconstantrangelist() */
ECHECKLISTFUNC(VkPushConstantRange, pushconstantrange, NULL) 

/*------------------------------------------------------------------------------*/

static int echeckmappedmemoryrange(lua_State *L, int arg, VkMappedMemoryRange *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    p->pNext = NULL;
    GetDeviceMemory(memory, "memory");
    GetInteger(offset, "offset");
    GetIntegerOrWholeSize(size, "size");
    return 0;
    }

/* echeckmappedmemoryrangelist() */
ECHECKLISTFUNC(VkMappedMemoryRange, mappedmemoryrange, NULL) 


/*------------------------------------------------------------------------------*/

int pushmemoryrequirements(lua_State *L, VkMemoryRequirements *p)
    {
    lua_newtable(L);
    SetInteger(size, "size");
    SetInteger(alignment, "alignment");
    SetInteger(memoryTypeBits, "memory_type_bits");
    return 1;
    }

int pushmemorydedicatedrequirements(lua_State *L, VkMemoryDedicatedRequirementsKHR *p)
    {
    SetBoolean(prefersDedicatedAllocation, "prefers_dedicated_allocation");
    SetBoolean(requiresDedicatedAllocation, "requires_dedicated_allocation");
    return 1;
    }

typedef struct {
    VkMemoryRequirements2KHR p1;
    VkMemoryDedicatedRequirementsKHR p2;
} VkMemoryRequirements2KHR_CHAIN;

VkMemoryRequirements2KHR* newmemoryrequirements2(lua_State *L)
    {
    VkMemoryRequirements2KHR_CHAIN *p = MALLOC_NOERR(L, VkMemoryRequirements2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = NULL;
    return (VkMemoryRequirements2KHR*)p;
    }

void freememoryrequirements2(lua_State *L, VkMemoryRequirements2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushmemoryrequirements2(lua_State *L, VkMemoryRequirements2KHR *pp)
    {
    VkMemoryRequirements2KHR_CHAIN *p = (VkMemoryRequirements2KHR_CHAIN*)pp;
    pushmemoryrequirements(L, &p->p1.memoryRequirements);
    pushmemorydedicatedrequirements(L, &p->p2);
    return 1;
    }

/*-------------------------------------------------------------------------------------*/
int pushsparseimagememoryrequirements(lua_State *L, VkSparseImageMemoryRequirements *p)
    {
    lua_newtable(L);
    SetStruct(formatProperties, "format_properties", pushsparseimageformatproperties);
    SetInteger(imageMipTailFirstLod, "image_mip_tail_first_lod");
    SetInteger(imageMipTailSize, "image_mip_tail_size");
    SetInteger(imageMipTailOffset, "image_mip_tail_offset");
    SetInteger(imageMipTailStride, "image_mip_tail_stride");
    return 1;
    }

VkSparseImageMemoryRequirements2KHR* newsparseimagememoryrequirements2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkSparseImageMemoryRequirements2KHR *p = NMALLOC_NOERR(L, VkSparseImageMemoryRequirements2KHR, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2_KHR;
        p[i].pNext = NULL;
        }
    return p;
    }

void freesparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2KHR *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushsparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2KHR *p)
    {
    pushsparseimagememoryrequirements(L, &p->memoryRequirements);
    return 1;
    }

/*-------------------------------------------------------------------------------------*/
static int echecksparsememorybind(lua_State *L, int arg, VkSparseMemoryBind *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(resourceOffset, "resource_offset");
    GetInteger(size, "size");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "memory_offset");
    GetFlags(flags, "flags");
    return 0;
    }

static ECHECKLISTFUNC(VkSparseMemoryBind, sparsememorybind, NULL) /* echecksparsememorybindlist() */
/*-------------------------------------------------------------------------------------*/

int echeckimagesubresource(lua_State *L, int arg, VkImageSubresource *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(arrayLayer, "array_layer");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/
static int echecksparseimagememorybind(lua_State *L, int arg, VkSparseImageMemoryBind *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetImageSubresource(subresource, "subresource");
    GetOffset3dOpt(offset, "offset");
    GetExtent3dOpt(extent, "extent");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "memory_offset");
    GetFlags(flags, "flags");
    return 0;
    }

/* echecksparseimagememorybindlist() */
static ECHECKLISTFUNC(VkSparseImageMemoryBind, sparseimagememorybind, NULL) 

/*-------------------------------------------------------------------------------------*/

static void freesparsebuffermemorybindinfo(lua_State *L, VkSparseBufferMemoryBindInfo *p)
    {
    if(!p) return;
    if(p->pBinds) Free(L, (void*)p->pBinds);
    }

static int echecksparsebuffermemorybindinfo(lua_State *L, int arg, VkSparseBufferMemoryBindInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE

    GetBuffer(buffer, "buffer");
#define F "binds"
    PUSHFIELD(F);
    p->pBinds = echecksparsememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    POPFIELD();
    if(err) return efielderror(L, F);
#undef F
    return 0;
    }

/* echecksparsebuffermemorybindinfolist() */
static FREELISTFUNC(VkSparseBufferMemoryBindInfo, sparsebuffermemorybindinfo) 
static ECHECKLISTFUNC(VkSparseBufferMemoryBindInfo, sparsebuffermemorybindinfo, freesparsebuffermemorybindinfolist) 


/*-------------------------------------------------------------------------------------*/

static void freesparseimageopaquememorybindinfo(lua_State *L, VkSparseImageOpaqueMemoryBindInfo *p)
    {
    if(!p) return;
    if(p->pBinds) Free(L, (void*)p->pBinds);
    }

static int echecksparseimageopaquememorybindinfo(lua_State *L, int arg, VkSparseImageOpaqueMemoryBindInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE

    GetImage(image, "image");
#define F "binds"
    PUSHFIELD(F);
    p->pBinds = echecksparsememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    POPFIELD();
    if(err) return efielderror(L, F);
#undef F
    return 0;
    }


/* echecksparseimageopaquememorybindinfolist() */
static FREELISTFUNC(VkSparseImageOpaqueMemoryBindInfo, sparseimageopaquememorybindinfo) 
static ECHECKLISTFUNC(VkSparseImageOpaqueMemoryBindInfo, sparseimageopaquememorybindinfo, freesparseimageopaquememorybindinfolist) 

/*-------------------------------------------------------------------------------------*/

static void freesparseimagememorybindinfo(lua_State *L, VkSparseImageMemoryBindInfo *p)
    {
    if(!p) return;
    if(p->pBinds) Free(L, (void*)p->pBinds);
    }

static int echecksparseimagememorybindinfo(lua_State *L, int arg, VkSparseImageMemoryBindInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE

    GetImage(image, "image");
#define F "binds"
    PUSHFIELD(F);
    p->pBinds = echecksparseimagememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    POPFIELD();
    if(err) return efielderror(L, F);
#undef F
    return 0;
    }

/* echecksparseimagememorybindinfolist() */
static FREELISTFUNC(VkSparseImageMemoryBindInfo, sparseimagememorybindinfo) 
static ECHECKLISTFUNC(VkSparseImageMemoryBindInfo, sparseimagememorybindinfo, freesparseimagememorybindinfolist) 

/*-------------------------------------------------------------------------------------*/

int pushsubresourcelayout(lua_State *L, VkSubresourceLayout *p)
    {
    lua_newtable(L);
    SetInteger(offset, "offset");
    SetInteger(size, "size");
    SetInteger(rowPitch, "row_pitch");
    SetInteger(arrayPitch, "array_pitch");
    SetInteger(depthPitch, "depth_pitch");
    return 1;
    }


static void freesubmitinfo(lua_State *L, VkSubmitInfo *p)
    {
    if(!p) return;
    if(p->pWaitSemaphores) Free(L, (void*)p->pWaitSemaphores);
    if(p->pWaitDstStageMask) Free(L, (void*)p->pWaitDstStageMask);
    if(p->pCommandBuffers) Free(L, (void*)p->pCommandBuffers);
    if(p->pSignalSemaphores) Free(L, (void*)p->pSignalSemaphores);
    }

static int echecksubmitinfo(lua_State *L, int arg, VkSubmitInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    p->pNext = NULL;

#define F "wait_semaphores"
    PUSHFIELD(F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    POPFIELD();
    if(err < 0)
        return fielderror(L, F, err);
#undef F
#define F "wait_dst_stage_mask"
    PUSHFIELD(F);
    p->pWaitDstStageMask = checkflaglist(L, arg1, &count, &err);
    POPFIELD();
    if(err < 0)
        { freesubmitinfo(L, p); return fielderror(L, F, err); }
    if(count != p->waitSemaphoreCount)
        { freesubmitinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
#undef F
#define F "command_buffers"
    PUSHFIELD(F);
    p->pCommandBuffers = checkcommand_bufferlist(L, arg1, &count, &err);
    p->commandBufferCount = count;
    POPFIELD();
    if(err < 0)
        { freesubmitinfo(L, p); return fielderror(L, F, err); }
#undef F
#define F "signal_semaphores"
    PUSHFIELD(F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->signalSemaphoreCount = count;
    POPFIELD();
    if(err < 0)
        { freesubmitinfo(L, p); return fielderror(L, F, err); }
#undef F
    return 0;
    }

/* echecksubmitinfolist() */
FREELISTFUNC(VkSubmitInfo, submitinfo) 
ECHECKLISTFUNC(VkSubmitInfo, submitinfo, freesubmitinfolist) 


static void freebindsparseinfo(lua_State *L, VkBindSparseInfo *p)
    {
    if(!p) return;
    if(p->pWaitSemaphores) Free(L, (void*)p->pWaitSemaphores);
    if(p->pBufferBinds) freesparsebuffermemorybindinfolist(L, (void*)p->pBufferBinds, p->bufferBindCount);
    if(p->pImageOpaqueBinds) 
        freesparseimageopaquememorybindinfolist(L, (void*)p->pImageOpaqueBinds, p->imageOpaqueBindCount);
    if(p->pImageBinds) freesparseimagememorybindinfolist(L, (void*)p->pImageBinds, p->imageBindCount);
    if(p->pSignalSemaphores) Free(L, (void*)p->pSignalSemaphores);
    }

static int echeckbindsparseinfo(lua_State *L, int arg, VkBindSparseInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
    p->pNext = NULL;

#define F "wait_semaphores"
    PUSHFIELD(F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    POPFIELD();
    if(err < 0) { freebindsparseinfo(L, p); return fielderror(L, F, err); }
#undef F
#define F "buffer_binds"
    PUSHFIELD(F);
    p->pBufferBinds = echecksparsebuffermemorybindinfolist(L, arg1, &count, &err);
    p->bufferBindCount = count;
    POPFIELD();
    if(err < 0) { freebindsparseinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "image_opaque_binds"
    PUSHFIELD(F);
    p->pImageOpaqueBinds = echecksparseimageopaquememorybindinfolist(L, arg1, &count, &err);
    p->imageOpaqueBindCount = count;
    POPFIELD();
    if(err < 0) { freebindsparseinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "image_binds"
    PUSHFIELD(F);
    p->pImageBinds = echecksparseimagememorybindinfolist(L, arg1, &count, &err);
    p->imageBindCount = count;
    POPFIELD();
    if(err < 0) { freebindsparseinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "signal_semaphores"
    PUSHFIELD(F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->signalSemaphoreCount = count;
    POPFIELD();
    if(err < 0) { freebindsparseinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }

/* echeckbindsparseinfolist() */
FREELISTFUNC(VkBindSparseInfo, bindsparseinfo) 
ECHECKLISTFUNC(VkBindSparseInfo, bindsparseinfo, freebindsparseinfolist) 

/*-------------------------------------------------------------------------------------*/

int pushsurfacecapabilities(lua_State *L, VkSurfaceCapabilitiesKHR *p)
    {
    lua_newtable(L);
    SetInteger(minImageCount, "min_image_count");
    SetInteger(maxImageCount, "max_image_count");
    if(p->currentExtent.width != (uint32_t)-1) 
        SetStruct(currentExtent, "current_extent", pushextent2d);
        /* width and height are either both -1 or ~=-1
         * the first case means 'not present', so instead of setting the width and height 
         * table fields with 0xffffffff or -1 (which are not the same, in lua_Integers),
         * we don't set current_extent at all
         */
    SetStruct(minImageExtent, "min_image_extent", pushextent2d);
    SetStruct(maxImageExtent, "max_image_extent", pushextent2d);
    SetInteger(maxImageArrayLayers, "max_image_array_layers");
    SetFlags(supportedTransforms, "supported_transforms");
    SetBits(currentTransform, "current_transform");
    SetFlags(supportedCompositeAlpha, "supported_composite_alpha");
    SetFlags(supportedUsageFlags, "supported_usage_flags");
    return 1;
    }

static int pushsharedpresentsurfacecapabilities(lua_State *L, VkSharedPresentSurfaceCapabilitiesKHR *p)
    {
    SetFlags(sharedPresentSupportedUsageFlags, "shared_present_supported_usage_flags");
    return 1;
    }

typedef struct {
    VkSurfaceCapabilities2KHR p1;
    VkSharedPresentSurfaceCapabilitiesKHR p2;
} VkSurfaceCapabilities2KHR_CHAIN;

VkSurfaceCapabilities2KHR* newsurfacecapabilities2(lua_State *L)
    {
    VkSurfaceCapabilities2KHR_CHAIN *p = MALLOC_NOERR(L, VkSurfaceCapabilities2KHR_CHAIN);
    if(!p) return NULL;
    p->p1.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = NULL;
    return (VkSurfaceCapabilities2KHR*)p;
    }

void freesurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR *p)
    {
    Free(L, (void*)p);
    }

int pushsurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR *pp)
    {
    VkSurfaceCapabilities2KHR_CHAIN *p = (VkSurfaceCapabilities2KHR_CHAIN*)pp;
    pushsurfacecapabilities(L, &p->p1.surfaceCapabilities);
    pushsharedpresentsurfacecapabilities(L, &p->p2);
    return 1;
    }

/*-------------------------------------------------------------------------------------*/

int pushsurfaceformat(lua_State *L, VkSurfaceFormatKHR *p)
    {
    lua_newtable(L);
    SetEnum(format, "format", pushformat);
    SetEnum(colorSpace, "color_space", pushcolorspace);
    return 1;
    }

VkSurfaceFormat2KHR *newsurfaceformat2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkSurfaceFormat2KHR *p = NMALLOC_NOERR(L, VkSurfaceFormat2KHR, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        p[i].pNext = NULL;
        }
    return p;
    }

void freesurfaceformat2(lua_State *L, VkSurfaceFormat2KHR *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }


int pushsurfaceformat2(lua_State *L, VkSurfaceFormat2KHR *p)
    {
    pushsurfaceformat(L, &p->surfaceFormat);
    return 1;
    }


/*-------------------------------------------------------------------------------------*/

static int echeckspecializationmapentry(lua_State *L, int arg, VkSpecializationMapEntry *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(constantID, "constant_id");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckspecializationmapentrylist() */
static ECHECKLISTFUNC(VkSpecializationMapEntry, specializationmapentry, NULL) 

static void freespecializationinfo(lua_State *L, VkSpecializationInfo *p)
    {
    if(!p) return;
    if(p->pMapEntries) Free(L, (void*)p->pMapEntries);
    if(p->pData) Free(L, (void*)p->pData);
    }

static int echeckspecializationinfo(lua_State *L, int arg, VkSpecializationInfo *p)
    {
    size_t size;
    int err, arg1;
    uint32_t count;
    const char* data;

    ECHECK_PREAMBLE
#define F "map_entries"
    PUSHFIELD(F);
    p->pMapEntries = echeckspecializationmapentrylist(L, arg1, &count, &err);
    p->mapEntryCount = count;
    POPFIELD();
    if(err < 0) { freespecializationinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "data"
    PUSHFIELD(F);
    data = lua_tolstring(L, arg1, &size);
    if(!data || size == 0)
        { POPFIELD(); freespecializationinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
    p->pData = MallocNoErr(L, size);
    if(!p->pData)
        { POPFIELD(); freespecializationinfo(L, p); return fielderror(L, F, ERR_MEMORY); }
    memcpy((void*)p->pData, data, size);
    p->dataSize = size;
    POPFIELD();
#undef F
    
    return 0;
    }

/*-------------------------------------------------------------------------------------*/


static void freepipelineshaderstagecreateinfo(lua_State *L, VkPipelineShaderStageCreateInfo *p)
    {
    if(!p) return;
    if(!p->pName) Free(L, (void*)p->pName);
    if(!p->pSpecializationInfo)
        {
        freespecializationinfo(L, (VkSpecializationInfo*)p->pSpecializationInfo);
        Free(L, (void*)p->pSpecializationInfo);
        }
    }

static int echeckpipelineshaderstagecreateinfo(lua_State *L, int arg, VkPipelineShaderStageCreateInfo *p)
    {
    int err, arg1;
    VkSpecializationInfo* specinfo;
    
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetShaderStageFlagBits(stage, "stage");
    GetShaderModule(module, "module");
    GetStringDef(pName, "name", "main");

#define F "specialization_info"
    PUSHFIELD(F);
    if(lua_isnoneornil(L, arg1))
        { 
        POPFIELD(); 
        p->pSpecializationInfo = NULL; 
        return 0; 
        }

    specinfo = (VkSpecializationInfo*)MallocNoErr(L, sizeof(VkSpecializationInfo));
    if(!specinfo)
        { POPFIELD(); freepipelineshaderstagecreateinfo(L, p); return fielderror(L, F, ERR_MEMORY); }
    err = echeckspecializationinfo(L, arg1, specinfo);
    p->pSpecializationInfo = specinfo;
    POPFIELD();
    if(err) { freepipelineshaderstagecreateinfo(L, p); return efielderror(L, F); }
#undef F
    return 0;
    }

/* echeckpipelineshaderstagecreateinfolist() */
static FREELISTFUNC(VkPipelineShaderStageCreateInfo, pipelineshaderstagecreateinfo) 
static ECHECKLISTFUNC(VkPipelineShaderStageCreateInfo, pipelineshaderstagecreateinfo, freepipelineshaderstagecreateinfolist) 

/*-------------------------------------------------------------------------------------*/

#define freepipelineinputassemblystatecreateinfo(L, p) do { } while(0)
static int echeckpipelineinputassemblystatecreateinfo(lua_State *L, int arg, VkPipelineInputAssemblyStateCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetTopology(topology, "topology");
    GetBoolean(primitiveRestartEnable, "primitive_restart_enable");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

#define freepipelinetessellationstatecreateinfo(L, p) do { } while(0)
static int echeckpipelinetessellationstatecreateinfo(lua_State *L, int arg, VkPipelineTessellationStateCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetInteger(patchControlPoints, "patch_control_points");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freepipelineviewportstatecreateinfo(lua_State *L, VkPipelineViewportStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pViewports) Free(L, (void*)p->pViewports);
    if(p->pScissors) Free(L, (void*)p->pScissors);
    }

static int echeckpipelineviewportstatecreateinfo(lua_State *L, int arg, VkPipelineViewportStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetIntegerOpt(viewportCount, "viewport_count", 1);
    GetIntegerOpt(scissorCount, "scissor_count", 1);
    /* scissorCount and viewportCount must be identical, and they may be > 0 
     * even if scissors and/or viewports lists are not given
     */
#define F "viewports"
    PUSHFIELD(F);
    p->pViewports = echeckviewportlist(L, arg1, &count, &err);
    POPFIELD();
    if(err < 0) { freepipelineviewportstatecreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT)
        POPERROR();
    else
        p->viewportCount = count;
/*
    else if(count != p->viewportCount)
        { freepipelineviewportstatecreateinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
*/
#undef F
#define F "scissors"
    PUSHFIELD(F);
    p->pScissors = echeckrect2dlist(L, arg1, &count, &err);
    POPFIELD();
    if(err < 0) { freepipelineviewportstatecreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT)
        POPERROR();
    else
        p->scissorCount = count;
/*
    else if(count != p->scissorCount)
        { freepipelineviewportstatecreateinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
*/
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/


#define freepipelinerasterizationstatecreateinfo(L, p) do { } while(0)
static int echeckpipelinerasterizationstatecreateinfo(lua_State *L, int arg, VkPipelineRasterizationStateCreateInfo *p)
    {
    int err;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetBoolean(depthClampEnable, "depth_clamp_enable");
    GetBoolean(rasterizerDiscardEnable, "rasterizer_discard_enable");
    GetPolygonMode(polygonMode, "polygon_mode");
    GetFlags(cullMode, "cull_mode");
    GetFrontFace(frontFace, "front_face");
    GetBoolean(depthBiasEnable, "depth_bias_enable");
    GetNumber(depthBiasConstantFactor, "depth_bias_constant_factor");
    GetNumber(depthBiasClamp, "depth_bias_clamp");
    GetNumber(depthBiasSlopeFactor, "depth_bias_slope_factor");
    GetNumberOpt(lineWidth, "line_width", 1.0);
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freepipelinemultisamplestatecreateinfo(lua_State *L, VkPipelineMultisampleStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pSampleMask) Free(L, (void*)p->pSampleMask);
    }

static int echeckpipelinemultisamplestatecreateinfo(lua_State *L, int arg, VkPipelineMultisampleStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetSamples(rasterizationSamples, "rasterization_samples");
    GetBoolean(sampleShadingEnable, "sample_shading_enable");
    GetNumber(minSampleShading, "min_sample_shading");
    GetBoolean(alphaToCoverageEnable, "alpha_to_coverage_enable");
    GetBoolean(alphaToOneEnable, "alpha_to_one_enable");

#define F "sample_mask"
    PUSHFIELD(F);
    p->pSampleMask = (VkSampleMask*)checkuint32list(L, arg1, &count, &err);
    POPFIELD();
    if(err < 0)
        { freepipelinemultisamplestatecreateinfo(L, p); return fielderror(L, F, err); }
    if((count > 0) && (count != p->rasterizationSamples / 32)) 
        { freepipelinemultisamplestatecreateinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckvertexinputbindingdescription(lua_State *L, int arg, VkVertexInputBindingDescription *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(binding, "binding");
    GetInteger(stride, "stride");
    GetVertexInputRate(inputRate, "input_rate");
    return 0;
    }

/* echeckvertexinputbindingdescriptionlist() */
static ECHECKLISTFUNC(VkVertexInputBindingDescription, vertexinputbindingdescription, NULL) 

static int echeckvertexinputattributedescription(lua_State *L, int arg, VkVertexInputAttributeDescription *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(location, "location");
    GetInteger(binding, "binding");
    GetFormat(format, "format");
    GetInteger(offset, "offset");
    return 0;
    }

/* echeckvertexinputattributedescriptionlist() */
static ECHECKLISTFUNC(VkVertexInputAttributeDescription, vertexinputattributedescription, NULL) 


static void freepipelinevertexinputstatecreateinfo(lua_State *L, VkPipelineVertexInputStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pVertexBindingDescriptions) Free(L, (void*)p->pVertexBindingDescriptions);
    if(p->pVertexAttributeDescriptions) Free(L, (void*)p->pVertexAttributeDescriptions);
    }

static int echeckpipelinevertexinputstatecreateinfo(lua_State *L, int arg, VkPipelineVertexInputStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
#define F "vertex_binding_descriptions"
    PUSHFIELD(F);
    p->pVertexBindingDescriptions = echeckvertexinputbindingdescriptionlist(L, arg1, &count, &err);
    p->vertexBindingDescriptionCount = count;
    POPFIELD();
    if(err < 0) { freepipelinevertexinputstatecreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "vertex_attribute_descriptions"
    PUSHFIELD(F);
    p->pVertexAttributeDescriptions = echeckvertexinputattributedescriptionlist(L, arg1, &count, &err);
    p->vertexAttributeDescriptionCount = count;
    POPFIELD();
    if(err < 0) { freepipelinevertexinputstatecreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/
static int echeckstencilopstate(lua_State *L, int arg, VkStencilOpState *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetStencilOp(failOp, "fail_op");
    GetStencilOp(passOp, "pass_op");
    GetStencilOp(depthFailOp, "depth_fail_op");
    GetCompareOp(compareOp, "compare_op");
    GetInteger(compareMask, "compare_mask");
    GetInteger(writeMask, "write_mask");
    GetInteger(reference, "reference");
    return 0;
    }

#define freepipelinedepthstencilstatecreateinfo(L, p) do { } while(0)
static int echeckpipelinedepthstencilstatecreateinfo(lua_State *L, int arg, VkPipelineDepthStencilStateCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetBoolean(depthTestEnable, "depth_test_enable");
    GetBoolean(depthWriteEnable, "depth_write_enable");
    GetCompareOp(depthCompareOp, "depth_compare_op");
    GetBoolean(depthBoundsTestEnable, "depth_bounds_test_enable");
    GetBoolean(stencilTestEnable, "stencil_test_enable");
    GetStencilOpStateOpt(front, "front");
    GetStencilOpStateOpt(back, "back");
    GetNumber(minDepthBounds, "min_depth_bounds");
    GetNumber(maxDepthBounds, "max_depth_bounds");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckpipelinecolorblendattachmentstate(lua_State *L, int arg, VkPipelineColorBlendAttachmentState *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetBoolean(blendEnable, "blend_enable");
    GetBlendFactor(srcColorBlendFactor, "src_color_blend_factor");
    GetBlendFactor(dstColorBlendFactor, "dst_color_blend_factor");
    GetBlendOp(colorBlendOp, "color_blend_op");
    GetBlendFactor(srcAlphaBlendFactor, "src_alpha_blend_factor");
    GetBlendFactor(dstAlphaBlendFactor, "dst_alpha_blend_factor");
    GetBlendOp(alphaBlendOp, "alpha_blend_op");
    GetFlags(colorWriteMask, "color_write_mask");
    return 0;
    }

/* echeckpipelinecolorblendattachmentstatelist() */
static ECHECKLISTFUNC(VkPipelineColorBlendAttachmentState, pipelinecolorblendattachmentstate, NULL) 

static void freepipelinecolorblendstatecreateinfo(lua_State *L, VkPipelineColorBlendStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pAttachments) Free(L, (void*)p->pAttachments);
    }

static int echeckpipelinecolorblendstatecreateinfo(lua_State *L, int arg, VkPipelineColorBlendStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t i, count;
    int isnum;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetBoolean(logicOpEnable, "logic_op_enable");
    GetLogicOp(logicOp, "logic_op");
#define F "attachments"
    PUSHFIELD(F);
    p->pAttachments = echeckpipelinecolorblendattachmentstatelist(L, arg1, &count, &err);
    p->attachmentCount = count;
    POPFIELD();
    if(err < 0) { freepipelinecolorblendstatecreateinfo(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
#define F "blend_constants"
    PUSHFIELD(F);
    if(lua_isnoneornil(L, arg1))
        { POPFIELD(); return 0; }
    if(lua_type(L, arg1) != LUA_TTABLE)
        { POPFIELD(); freepipelinecolorblendstatecreateinfo(L, p); return fielderror(L, F, ERR_TABLE); }
    for(i = 0; i < 4; i++)
        { 
        lua_rawgeti(L, arg1, i+1);
        p->blendConstants[i] = lua_tonumberx(L, -1, &isnum);
        lua_pop(L, 1);
        if(!isnum)
            { 
            POPFIELD(); 
            freepipelinecolorblendstatecreateinfo(L, p); 
            return fielderror(L, F, ERR_TYPE); 
            }
        }
    POPFIELD();
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freepipelinedynamicstatecreateinfo(lua_State *L, VkPipelineDynamicStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pDynamicStates) freedynamicstatelist(L, (VkDynamicState*)p->pDynamicStates);
    }

static int echeckpipelinedynamicstatecreateinfo(lua_State *L, int arg, VkPipelineDynamicStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    PUSHFIELD("dynamic_states");
    p->pDynamicStates = checkdynamicstatelist(L, arg1, &count, &err);
    p->dynamicStateCount = count;
    POPFIELD();
    if(err) 
        { lua_pushstring(L, "dynamic_states"); return ERR_GENERIC; }
 
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freegraphicspipelinecreateinfo(lua_State *L, VkGraphicsPipelineCreateInfo *p)
    {
    if(!p) return;
    if(p->pStages) freepipelineshaderstagecreateinfolist(L, (void*)p->pStages, p->stageCount);
    if(p->pVertexInputState)
        {
        freepipelinevertexinputstatecreateinfo(L, 
            (VkPipelineVertexInputStateCreateInfo*)p->pVertexInputState);
        Free(L, (void*)p->pVertexInputState);
        }
    if(p->pInputAssemblyState) 
        {
        freepipelineinputassemblystatecreateinfo(L, 
            (VkPipelineInputAssemblyStateCreateInfo*)p->pInputAssemblyState);
        Free(L, (void*)p->pInputAssemblyState);
        }
    if(p->pTessellationState) 
        {
        freepipelinetessellationstatecreateinfo(L, 
            (VkPipelineTessellationStateCreateInfo*)p->pTessellationState);
        Free(L, (void*)p->pTessellationState);
        }
    if(p->pViewportState) 
        {
        freepipelineviewportstatecreateinfo(L, 
            (VkPipelineViewportStateCreateInfo*)p->pViewportState);
        Free(L, (void*)p->pViewportState);
        }
    if(p->pRasterizationState) 
        {
        freepipelinerasterizationstatecreateinfo(L, 
            (VkPipelineRasterizationStateCreateInfo*)p->pRasterizationState);
        Free(L, (void*)p->pRasterizationState);
        }
    if(p->pMultisampleState) 
        {
        freepipelinemultisamplestatecreateinfo(L, 
            (VkPipelineMultisampleStateCreateInfo*)p->pMultisampleState);
        Free(L, (void*)p->pMultisampleState);
        }
    if(p->pDepthStencilState) 
        {
        freepipelinedepthstencilstatecreateinfo(L, 
            (VkPipelineDepthStencilStateCreateInfo*)p->pDepthStencilState);
        Free(L, (void*)p->pDepthStencilState);
        }
    if(p->pColorBlendState) 
        {
        freepipelinecolorblendstatecreateinfo(L, 
            (VkPipelineColorBlendStateCreateInfo*)p->pColorBlendState);
        Free(L, (void*)p->pColorBlendState);
        }
    if(p->pDynamicState) 
        {
        freepipelinedynamicstatecreateinfo(L, 
            (VkPipelineDynamicStateCreateInfo*)p->pDynamicState);
        Free(L, (void*)p->pDynamicState);
        }
    }

static int echeckgraphicspipelinecreateinfo(lua_State *L, int arg, VkGraphicsPipelineCreateInfo *p)
    {
    int err, arg1;
    VkPipelineVertexInputStateCreateInfo       *VertexInputState;
    VkPipelineInputAssemblyStateCreateInfo     *InputAssemblyState;
    VkPipelineTessellationStateCreateInfo      *TessellationState;
    VkPipelineViewportStateCreateInfo          *ViewportState;
    VkPipelineRasterizationStateCreateInfo     *RasterizationState;
    VkPipelineMultisampleStateCreateInfo       *MultisampleState;
    VkPipelineDepthStencilStateCreateInfo      *DepthStencilState;
    VkPipelineColorBlendStateCreateInfo        *ColorBlendState;
    VkPipelineDynamicStateCreateInfo           *DynamicState;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetPipelineLayout(layout, "layout");
    GetRenderPass(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetPipelineOpt(basePipelineHandle, "base_pipeline_handle");
    GetIntegerOpt(basePipelineIndex, "base_pipeline_index", -1);

#define F "stages"
    PUSHFIELD(F);
    p->pStages = echeckpipelineshaderstagecreateinfolist(L, arg1, &p->stageCount, &err);
    POPFIELD();
    if(err) return efielderror(L, F);
#undef F

#define BEGIN(xxx) do {                                                             \
    xxx = (VkPipeline##xxx##CreateInfo*)Malloc(L, sizeof(VkPipeline##xxx##CreateInfo)); \
    if(!xxx) { freegraphicspipelinecreateinfo(L, p); return pusherror(L, ERR_MEMORY); } \
    PUSHFIELD(F);                                                                   \
} while (0)

#define END_MANDATORY(xxx)  do {                                                    \
    POPFIELD();                                                                     \
    if(err) { freegraphicspipelinecreateinfo(L, p); return efielderror(L, F); }     \
    p->p##xxx = xxx;                                                                \
} while(0)

#define END_OPTIONAL(xxx)   do {                                                    \
    POPFIELD();                                                                     \
    if(err < 0) { freegraphicspipelinecreateinfo(L, p); return efielderror(L, F); } \
    if(err == ERR_NOTPRESENT)                                                       \
        POPERROR();                                                                 \
    else                                                                            \
        p->p##xxx = xxx;                                                            \
} while(0)

#define F "vertex_input_state"
    BEGIN(VertexInputState);
    err = echeckpipelinevertexinputstatecreateinfo(L, arg1, VertexInputState);
    END_MANDATORY(VertexInputState);
#undef F
#define F "input_assembly_state"
    BEGIN(InputAssemblyState);
    err = echeckpipelineinputassemblystatecreateinfo(L, arg1, InputAssemblyState);
    END_MANDATORY(InputAssemblyState);
#undef F
#define F "tessellation_state"
    BEGIN(TessellationState);
    err = echeckpipelinetessellationstatecreateinfo(L, arg1, TessellationState);
    END_OPTIONAL(TessellationState);
#undef F
#define F "viewport_state"
    BEGIN(ViewportState);
    err = echeckpipelineviewportstatecreateinfo(L, arg1, ViewportState);
    END_OPTIONAL(ViewportState);
#undef F
#define F "rasterization_state"
    BEGIN(RasterizationState);
    err = echeckpipelinerasterizationstatecreateinfo(L, arg1, RasterizationState);
    END_MANDATORY(RasterizationState);
#undef F
#define F "multisample_state"
    BEGIN(MultisampleState);
    err = echeckpipelinemultisamplestatecreateinfo(L, arg1, MultisampleState);
    END_OPTIONAL(MultisampleState);
#undef F
#define F "depth_stencil_state"
    BEGIN(DepthStencilState);
    err = echeckpipelinedepthstencilstatecreateinfo(L, arg1, DepthStencilState);
    END_OPTIONAL(DepthStencilState);
#undef F
#define F "color_blend_state"
    BEGIN(ColorBlendState);
    err = echeckpipelinecolorblendstatecreateinfo(L, arg1, ColorBlendState);
    END_OPTIONAL(ColorBlendState);
#undef F
#define F "dynamic_state"
    BEGIN(DynamicState);
    err = echeckpipelinedynamicstatecreateinfo(L, arg1, DynamicState);
    END_OPTIONAL(DynamicState);
#undef F
    return 0;
#undef BEGIN
#undef END_MANDATORY
#undef END_OPTIONAL
    }

/* echeckgraphicspipelinecreateinfolist() */
FREELISTFUNC(VkGraphicsPipelineCreateInfo, graphicspipelinecreateinfo) 
ECHECKLISTFUNC(VkGraphicsPipelineCreateInfo, graphicspipelinecreateinfo, freegraphicspipelinecreateinfolist) 

/*-------------------------------------------------------------------------------------*/
static void freecomputepipelinecreateinfo(lua_State *L, VkComputePipelineCreateInfo *p)
    {
    if(!p) return;
    freepipelineshaderstagecreateinfo(L, &p->stage);
    }

static int echeckcomputepipelinecreateinfo(lua_State *L, int arg, VkComputePipelineCreateInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetPipelineLayout(layout, "layout");
    GetPipelineOpt(basePipelineHandle, "base_pipeline_handle");
    GetIntegerOpt(basePipelineIndex, "base_pipeline_index", -1);
    GetPipelineShaderStageCreateInfo(stage, "stage");
    return 0;
    }

/* echeckcomputepipelinecreateinfolist() */
FREELISTFUNC(VkComputePipelineCreateInfo, computepipelinecreateinfo) 
ECHECKLISTFUNC(VkComputePipelineCreateInfo, computepipelinecreateinfo, freecomputepipelinecreateinfolist) 

/*-------------------------------------------------------------------------------------*/

void freeswapchaincreateinfo(lua_State *L, VkSwapchainCreateInfoKHR *p)
    {
    if(!p) return;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    }

int echeckswapchaincreateinfo(lua_State *L, int arg, VkSwapchainCreateInfoKHR *p)
    {
    int err, arg1;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetSurface(surface, "surface");
    GetInteger(minImageCount, "min_image_count");
    GetFormat(imageFormat, "image_format");
    GetColorSpace(imageColorSpace, "image_color_space");
    GetExtent2dOpt(imageExtent, "image_extent");
    GetInteger(imageArrayLayers, "image_array_layers");
    GetFlags(imageUsage, "image_usage");
    GetSharingMode(imageSharingMode, "image_sharing_mode");
    GetSurfaceTransformFlagBits(preTransform, "pre_transform");
    GetCompositeAlphaFlagBits(compositeAlpha, "composite_alpha");
    GetPresentMode(presentMode, "present_mode");
    GetBoolean(clipped, "clipped");
    GetSwapchainOpt(oldSwapchain, "old_swapchain");
#define F "queue_family_indices"
    PUSHFIELD(F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    POPFIELD();
    if(err < 0) return fielderror(L, F, err);
#undef F
    return 0;
    }

/* echeckswapchaincreateinfolist() */
FREELISTFUNC(VkSwapchainCreateInfoKHR, swapchaincreateinfo) 
ECHECKLISTFUNC(VkSwapchainCreateInfoKHR, swapchaincreateinfo, freeswapchaincreateinfolist) 




void freepresentinfo(lua_State *L, VkPresentInfoKHR *p)
    {
    if(!p) return;
    if(p->pWaitSemaphores) Free(L, (void*)p->pWaitSemaphores);
    if(p->pSwapchains) Free(L, (void*)p->pSwapchains);
    if(p->pImageIndices) Free(L, (void*)p->pImageIndices);
    if(p->pResults) Free(L, (void*)p->pResults);
    }

int echeckpresentinfo(lua_State *L, int arg, VkPresentInfoKHR *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    p->pNext = NULL;

#define F "wait_semaphores"
    PUSHFIELD(F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    POPFIELD();
    if(err < 0)
        return fielderror(L, F, err);
#undef F
#define F "swapchains"
    PUSHFIELD(F);
    p->pSwapchains = checkswapchainlist(L, arg1, &count, &err, NULL);
    p->swapchainCount = count;
    POPFIELD();
    if(err)
        { freepresentinfo(L, p); return fielderror(L, F, err); }
#undef F
#define F "image_indices"
    PUSHFIELD(F);
    p->pImageIndices = checkuint32list(L, arg1, &count, &err);
    POPFIELD();
    if(err) { freepresentinfo(L, p); return fielderror(L, F, err); }
    if(p->swapchainCount != count)
        { freepresentinfo(L, p); return fielderror(L, F, ERR_LENGTH); }
#undef F
/*  p->pResults = NULL; */

    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckdescriptorimageinfo(lua_State *L, int arg, VkDescriptorImageInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetSampler(sampler, "sampler");
    GetImageView(imageView, "image_view");
    GetImageLayout(imageLayout, "image_layout");
    return 0;
    }

/* echeckdescriptorimageinfolist() */
static ECHECKLISTFUNC(VkDescriptorImageInfo, descriptorimageinfo, NULL) 

static int echeckdescriptorbufferinfo(lua_State *L, int arg, VkDescriptorBufferInfo *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetIntegerOrWholeSize(range, "range");
    return 0;
    }

/* echeckdescriptorbufferinfolist() */
static ECHECKLISTFUNC(VkDescriptorBufferInfo, descriptorbufferinfo, NULL) 


static void freewritedescriptorset(lua_State *L, VkWriteDescriptorSet *p)
    {
    if(!p) return;
    if(!p->pImageInfo) Free(L, (void*)p->pImageInfo);
    if(!p->pBufferInfo) Free(L, (void*)p->pBufferInfo);
    if(!p->pTexelBufferView) Free(L, (void*)p->pTexelBufferView);
    }

static int echeckwritedescriptorset(lua_State *L, int arg, VkWriteDescriptorSet *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    p->pNext = NULL;
    GetDescriptorSet(dstSet, "dst_set");
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetDescriptorType(descriptorType, "descriptor_type");
    /* image_info, buffer_info and texel_buffer_view are exclusive and their
     * presence depends on p->descriptorType: */
    switch(p->descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
#define F "image_info"
            PUSHFIELD(F);
            p->pImageInfo = echeckdescriptorimageinfolist(L, arg1, &count, &err);
            POPFIELD();
            if(err) { freewritedescriptorset(L, p); return efielderror(L, F); }
            p->descriptorCount = count;
            return 0;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
#define F "buffer_info"
            PUSHFIELD(F);
            p->pBufferInfo = echeckdescriptorbufferinfolist(L, arg1, &count, &err);
            POPFIELD();
            if(err) { freewritedescriptorset(L, p); return efielderror(L, F); }
            p->descriptorCount = count;
            return 0;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
#define F "texel_buffer_view"
            PUSHFIELD(F);
            p->pTexelBufferView = checkbuffer_viewlist(L, arg1, &count, &err, NULL);
            POPFIELD();
            if(err) { freewritedescriptorset(L, p); return fielderror(L, F, err); }
            if(err == ERR_NOTPRESENT) POPERROR();
            p->descriptorCount = count;
            return 0;
#undef F
        default:
            unexpected(L); /* unhandled descriptorType ? */
        }
    return 0;
    }

/* echeckwritedescriptorsetlist() */
FREELISTFUNC(VkWriteDescriptorSet, writedescriptorset) 
ECHECKLISTFUNC(VkWriteDescriptorSet, writedescriptorset, freewritedescriptorsetlist) 


static void freecopydescriptorset(lua_State *L, VkCopyDescriptorSet *p)
    {
    (void)L; (void)p;
    }

static int echeckcopydescriptorset(lua_State *L, int arg, VkCopyDescriptorSet *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    p->pNext = NULL;
    GetDescriptorSet(srcSet, "src_set");
    GetInteger(srcBinding, "src_binding");
    GetInteger(srcArrayElement, "src_array_element");
    GetDescriptorSet(dstSet, "dst_set");
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetInteger(descriptorCount, "descriptor_count");
    return 0;
    }

/* echeckcopydescriptorsetlist() */
FREELISTFUNC(VkCopyDescriptorSet, copydescriptorset) 
ECHECKLISTFUNC(VkCopyDescriptorSet, copydescriptorset, freecopydescriptorsetlist) 

/*-------------------------------------------------------------------------------------*/
int pushdisplayproperties(lua_State *L, VkDisplayPropertiesKHR *p)
    {
    lua_newtable(L);
/*  p->display = set by caller */
    SetString(displayName, "display_name");
    SetStruct(physicalDimensions, "physical_dimensions", pushextent2d);
    SetStruct(physicalResolution, "physical_resolution", pushextent2d);
    SetFlags(supportedTransforms, "supported_transforms");
    SetBoolean(planeReorderPossible, "plane_reorder_possible");
    SetBoolean(persistentContent, "persistent_content");
    return 1;
    }

int pushdisplayplaneproperties(lua_State *L, VkDisplayPlanePropertiesKHR *p)
    {
    lua_newtable(L);
/*  p->currentDisplay = set by caller */
    SetInteger(currentStackIndex, "current_stack_index");
    return 1;
    }


/*-------------------------------------------------------------------------------------*/

int echeckdisplaymodeparameters(lua_State *L, int arg, VkDisplayModeParametersKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetExtent2d(visibleRegion, "visible_region");
    GetInteger(refreshRate, "refresh_rate");
    return 0;
    }

int pushdisplaymodeparameters(lua_State *L, VkDisplayModeParametersKHR *p)
    {
    lua_newtable(L);
    SetStruct(visibleRegion, "visible_region", pushextent2d);
    SetInteger(refreshRate, "refresh_rate");
    return 1;
    }


int pushdisplaymodeproperties(lua_State *L, VkDisplayModePropertiesKHR *p)
    {
    lua_newtable(L);
/*  p->displayMode = set by caller */
    SetStruct(parameters, "parameters", pushdisplaymodeparameters);
    return 1;
    }

int pushdisplayplanecapabilities(lua_State *L, VkDisplayPlaneCapabilitiesKHR *p)
    {
    lua_newtable(L);
    SetFlags(supportedAlpha, "supported_alpha");
    SetStruct(minSrcPosition, "min_src_position", pushoffset2d);
    SetStruct(maxSrcPosition, "max_src_position", pushoffset2d);
    SetStruct(minSrcExtent, "min_src_extent", pushextent2d);
    SetStruct(maxSrcExtent, "max_src_extent", pushextent2d);
    SetStruct(minDstPosition, "min_dst_position", pushoffset2d);
    SetStruct(maxDstPosition, "max_dst_position", pushoffset2d);
    SetStruct(minDstExtent, "min_dst_extent", pushextent2d);
    SetStruct(maxDstExtent, "max_dst_extent", pushextent2d);
    return 1;
    }


void freedisplaysurfacecreateinfo(lua_State *L, VkDisplaySurfaceCreateInfoKHR *p)
    {
    (void)L;
    if(!p) return;
    }

int echeckdisplaysurfacecreateinfo(lua_State *L, int arg, VkDisplaySurfaceCreateInfoKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR ;
    p->pNext = NULL;
    GetFlags(flags, "flags");
/*  p->displayMode = set by caller */
    GetInteger(planeIndex, "plane_index");
    GetInteger(planeStackIndex, "plane_stack_index");
    GetSurfaceTransformFlagBits(transform, "transform");
    GetNumber(globalAlpha, "global_alpha");
    GetDisplayPlaneAlphaFlagBits(alphaMode, "alpha_mode");
    GetExtent2d(imageExtent, "image_extent");
    return 0;
    }

void freedisplaypresentinfo(lua_State *L, VkDisplayPresentInfoKHR *p)
    {
    (void)p; (void)L; 
    }

int echeckdisplaypresentinfo(lua_State *L, int arg, VkDisplayPresentInfoKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR;
    p->pNext = NULL;
    GetRect2d(srcRect, "src_rect");
    GetRect2d(dstRect, "dst_rect");
    GetBoolean(persistent, "persistent");
    return 0;
    }
 
/*-------------------------------------------------------------------------------------*/

static int echeckrectlayer(lua_State *L, int arg, VkRectLayerKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetOffset2d(offset, "offset");
    GetExtent2d(extent, "extent");
    GetInteger(layer, "layer");
    return 0;
    }

/* echeckrectlayerlist() */
static ECHECKLISTFUNC(VkRectLayerKHR, rectlayer, NULL) 

static void freepresentregion(lua_State *L, VkPresentRegionKHR *p)
    {
    if(!p) return;
    if(p->pRectangles)
        Free(L, (void*)p->pRectangles);
    }

static int echeckpresentregion(lua_State *L, int arg, VkPresentRegionKHR *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
#define F "rectangles"
    PUSHFIELD(F);
    p->pRectangles = echeckrectlayerlist(L, arg1, &count, &err);
    p->rectangleCount = count;
    POPFIELD();
    if(err < 0) { freepresentregion(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }
/* echeckpresentregionlist() */
static FREELISTFUNC(VkPresentRegionKHR, presentregion) 
static ECHECKLISTFUNC(VkPresentRegionKHR, presentregion, freepresentregionlist) 

void freepresentregions(lua_State *L, VkPresentRegionsKHR *p)
    {
    if(!p) return;
    if(p->pRegions)
        freepresentregionlist(L, (void*)p->pRegions, p->swapchainCount);
    }

int echeckpresentregions(lua_State *L, int arg, VkPresentRegionsKHR *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
    p->pNext = NULL;
#define F "regions"
    PUSHFIELD(F);
    p->pRegions = echeckpresentregionlist(L, arg1, &count, &err);
    p->swapchainCount = count;
    POPFIELD();
    if(err < 0) { freepresentregions(L, p); return efielderror(L, F); }
    if(err == ERR_NOTPRESENT) POPERROR();
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freedescriptorupdatetemplateentry(lua_State *L, VkDescriptorUpdateTemplateEntryKHR *p)
    {
    (void)L; (void)p;
    }

static int echeckdescriptorupdatetemplateentry(lua_State *L, int arg, VkDescriptorUpdateTemplateEntryKHR *p)
    {
    int err;
    ECHECK_PREAMBLE
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetInteger(descriptorCount, "descriptor_count");
    GetDescriptorType(descriptorType, "descriptor_type");
    GetInteger(offset, "offset");
    GetInteger(stride, "stride");
    return 0;
    }

/* echeckdescriptorupdatetemplateentrylist() */
static FREELISTFUNC(VkDescriptorUpdateTemplateEntryKHR, descriptorupdatetemplateentry) 
static ECHECKLISTFUNC(VkDescriptorUpdateTemplateEntryKHR, descriptorupdatetemplateentry, freedescriptorupdatetemplateentrylist) 


void freedescriptorupdatetemplatecreateinfo(lua_State *L, VkDescriptorUpdateTemplateCreateInfoKHR *p)
    {
    if(!p) return;
    if(p->pDescriptorUpdateEntries)
        freedescriptorupdatetemplateentrylist(L, (VkDescriptorUpdateTemplateEntryKHR*)p->pDescriptorUpdateEntries, p->descriptorUpdateEntryCount);
    }

int echeckdescriptorupdatetemplatecreateinfo(lua_State *L, int arg, VkDescriptorUpdateTemplateCreateInfoKHR *p)
    {
    int err, arg1;
    uint32_t count;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_KHR;
    p->pNext = NULL;
    GetFlags(flags, "flags");
    GetDescriptorUpdateTemplateType(templateType, "template_type");
    GetDescriptorSetLayoutOpt(descriptorSetLayout, "descriptor_set_layout");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
    GetPipelineLayoutOpt(pipelineLayout, "pipeline_layout");
    GetFlags(set, "set");
#define F "descriptor_update_entries"
    PUSHFIELD(F);
    p->pDescriptorUpdateEntries = echeckdescriptorupdatetemplateentrylist(L, arg1, &count, &err);
    p->descriptorUpdateEntryCount = count;
    POPFIELD();
    if(err) { freedescriptorupdatetemplatecreateinfo(L, p); return efielderror(L, F); }
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

void freedevicecreateinfo(lua_State *L, VkDeviceCreateInfo *p)
    {
    if(!p) return;
    if(p->pQueueCreateInfos)
        freedevicequeuecreateinfolist(L, (void*)p->pQueueCreateInfos,  p->queueCreateInfoCount);
    if(p->ppEnabledLayerNames)
        freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames)
        freestringlist(L,  (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
    if(p->pEnabledFeatures)
        Free(L, (void*)p->pEnabledFeatures);
    if(p->pNext)
        freephysicaldevicefeatures2(L, (VkPhysicalDeviceFeatures2KHR*)p->pNext);
    }



int echeckdevicecreateinfo(lua_State *L, int arg, VkDeviceCreateInfo *p, ud_t *ud)
    {
    int arg1, err;
    uint32_t count;
    VkPhysicalDeviceFeatures *features;
    VkPhysicalDeviceFeatures2KHR *features2;

    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    p->pNext = NULL;

    GetFlags(flags, "flags");

#define F "queue_create_infos"
    PUSHFIELD(F);
    p->pQueueCreateInfos = echeckdevicequeuecreateinfolist(L, arg1, &count, &err);
    p->queueCreateInfoCount = count;
    POPFIELD();
    if(err) return efielderror(L, F);
#undef F
#define F "enabled_layer_names" /* deprecated: aggiungere comunque */
    PUSHFIELD(F);
    p->ppEnabledLayerNames = (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, &err);
    POPFIELD();
    if(err < 0 && err != ERR_EMPTY)
        { freedevicecreateinfo(L, p); return fielderror(L, F, err); }
#undef F
#define F "enabled_extension_names"
    PUSHFIELD(F);
    p->ppEnabledExtensionNames = (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, &err);
    POPFIELD();
    if(err < 0 && err != ERR_EMPTY)
        { freedevicecreateinfo(L, p); return fielderror(L, F, err); }
#undef F

    if(!ud->idt->GetPhysicalDeviceFeatures2KHR)
        {
#define F "enabled_features"
        features = MALLOC_NOERR(L, VkPhysicalDeviceFeatures);
        if(!features) { freedevicecreateinfo(L, p); return pusherror(L, ERR_MEMORY); }
        PUSHFIELD(F);
        err = echeckphysicaldevicefeatures(L, arg1, features);
        POPFIELD();
        if(err < 0) 
            { freedevicecreateinfo(L, p); return efielderror(L, F); }
        if(err == ERR_NOTPRESENT)
            { POPERROR(); Free(L, features); }
        else
            p->pEnabledFeatures = features;
#undef F
        }
    else
        {
#define F "enabled_features"
//      p->pEnabledFeatures = NULL;
        features2 = newphysicaldevicefeatures2(L);
        if(!features2) { freedevicecreateinfo(L, p); return pusherror(L, ERR_MEMORY); }
        PUSHFIELD(F);
        err = echeckphysicaldevicefeatures2(L, arg1, features2);
        POPFIELD();
        if(err < 0)
            { freedevicecreateinfo(L, p); return efielderror(L, F); }
        if(err == ERR_NOTPRESENT)
            { POPERROR(); Free(L, features2); }
        else
            p->pNext = features2;
#undef F
        }
    return 0;
    }


/*-------------------------------------------------------------------------------------*/
#if 0 // scaffolding

#define  moonvulkan_
void free(lua_State *L, Vk *p) //@@
    {
    if(!p) return;
    }

#define  moonvulkan_
int echeck(lua_State *L, int arg, Vk *p) //@@
    {
    int err;
    ECHECK_PREAMBLE
    p->sType = VK_STRUCTURE_TYPE_;
    p->pNext = NULL;
    GetFlags(flags, "flags");

    return 0;
    }

#define  moonvulkan_
int push(lua_State *L, Vk *p) //@@
    {
    lua_newtable(L);
    Set(, "");
    return 1;
    }

#endif

