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
 * - echeckxxx() functions return 0 (ERR_SUCCESS) on success, otherwise they
 *   push a message on the Lua stack and return err!=0 (ERR_XXX).
 *   (the GetXxx() macros conform to this behaviour)
 * - checkxxxlist() functions defined in utils.c only return an error code,
 * - checkxxx() functions defined elsewhere (enums, bitfields, etc) follow the
 *   usual Lua convention to raise errors.
 */

static int checktable(lua_State *L, int arg)
    {
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); return ERR_NOTPRESENT; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); return ERR_TABLE; }
    return 0;
    }

#define MEMZERO(p_) memset((p_), 0, sizeof(*(p_)))

#define CHECK_TABLE(L, arg, p_) do {    \
    int err_ = checktable((L), (arg));  \
    if(err_) return err_;               \
    memset((p_), 0, sizeof(*(p_)));     \
} while(0)

static int ispresent_(lua_State *L, int arg, const char *sname)
#define ispresent(sname) ispresent_(L, arg, (sname))
/* Checks if field 'sname' is present in the table at arg */
    {
    int rc;
    lua_pushstring(L, sname);
    lua_rawget(L, arg);
    rc = lua_isnoneornil(L, -1) ? 0 : 1;
    lua_pop(L, 1);
    return rc;
    }

static int getfield(lua_State *L, int arg, const char *sname)
/* Pushes field 'sname' from the table at arg, and returns its type (LUA_TXXX) */
    {
    lua_pushstring(L, sname);
    return lua_rawget(L, arg);
    }

static int pushfield(lua_State *L, int arg, const char *sname)
/* Pushes field 'sname' from the table at arg, and returns its stack index */
    {
    lua_pushstring(L, sname);
    lua_rawget(L, arg);
    return lua_gettop(L);
    }
#define popfield lua_remove

static int pusherror(lua_State *L, int errcode)
    { 
    lua_pushstring(L, errstring(errcode)); 
    return ERR_GENERIC; 
    }
#define poperror()  lua_pop(L, 1)

static int pushfielderror(lua_State *L, const char *fieldname, int errcode)
    { 
    lua_pushfstring(L, "%s: %s", fieldname, errstring(errcode)); 
    return ERR_GENERIC; 
    }

static int prependfield(lua_State *L, const char *fieldname)
    { 
    lua_pushfstring(L, "%s.%s", fieldname, lua_tostring(L, -1));
    lua_remove(L, -2);
    return ERR_GENERIC; 
    }

/* Structs chaining via pNext -----------------------------------------------*/
#define pnextof(p_) &((p_)->pNext)
#define addtochain(chain_, p_) do { *(chain_) = (p_); (chain_) = pnextof(p_); } while(0)
/* Appends the struct pointed to by p_ to the chain.
 * const void **chain_ must contains the address of the pNext field of the
 * structure that is currently last in the chain.
 */

/*------------------------------------------------------------------------------*
 | Get macros (for check functions)                                             |
 *------------------------------------------------------------------------------*/

/* Flags ---------------------------------------------------------------------*/

static VkFlags GetFlags_(lua_State *L, int arg, const char *sname, int *err)
    {
    int arg_ = pushfield(L, arg, sname);
    VkFlags flags = testflags(L, arg_, err);
    popfield(L, arg_);
    if(*err == ERR_NOTPRESENT) return 0;
    if(*err) pushfielderror(L, sname, *err);
    return flags;
    }

#define GetFlags(name, sname)  do { /* always opt., defaults to 0 */ \
    int err_;                                       \
    p->name = GetFlags_(L, arg, sname, &err_);      \
    if(err_ && err_ != ERR_NOTPRESENT) return err_; \
} while(0)

#define GetBits(name, sname, T) do {                \
    int err_;                                       \
    p->name = (T)GetFlags_(L, arg, sname, &err_);   \
    if(err_ && err_ != ERR_NOTPRESENT) return err_; \
} while(0)

#define GetShaderStageFlagBits(name, sname) GetBits(name, sname, VkShaderStageFlagBits)
#define GetSurfaceTransformFlagBits(name, sname) GetBits(name, sname, VkSurfaceTransformFlagBitsKHR)
#define GetCompositeAlphaFlagBits(name, sname) GetBits(name, sname, VkCompositeAlphaFlagBitsKHR)
#define GetDisplayPlaneAlphaFlagBits(name, sname) GetBits(name, sname, VkDisplayPlaneAlphaFlagBitsKHR)

/* Numbers and booleans ------------------------------------------------------*/

#define GetSamples(name, sname) do {                \
    int err_, arg_ = pushfield(L, arg, sname);      \
    p->name = (VkSampleCountFlagBits)testflags(L, arg_, &err_); \
    popfield(L, arg_);                              \
    if(err_ == ERR_NOTPRESENT)                      \
        p->name = VK_SAMPLE_COUNT_1_BIT;            \
    else if(err_)                                   \
        return pushfielderror(L, sname, err_);      \
} while(0)

static lua_Number GetNumber_(lua_State *L, int arg, const char *sname, lua_Number defval, int *err)
    {
    lua_Number val = defval;
    int arg_ = pushfield(L, arg, sname);
    *err = 0;
    if(lua_isnumber(L, arg_))
        val = lua_tonumber(L, arg_);
    else if(!lua_isnoneornil(L, arg_))
        *err = ERR_TYPE;
    popfield(L, arg_);
    if(*err)
        pushfielderror(L, sname, *err);
    return val;
    }

#define GetNumberOpt(name, sname, defval) do {          \
    int err_;                                           \
    p->name = GetNumber_(L, arg, sname, defval, &err_); \
    if(err_) return err_;                               \
} while(0)

#define GetNumber(name, sname) GetNumberOpt(name, sname, 0)


static lua_Integer GetInteger_(lua_State *L, int arg, const char *sname, lua_Integer defval, int *err)
    {
    int isnum;
    lua_Integer val = defval;
    int arg_ = pushfield(L, arg, sname);
    *err = 0;
    if(!lua_isnoneornil(L, arg_))
        {
        val = lua_tointegerx(L, arg_, &isnum);
        if(!isnum) *err = ERR_TYPE;
        }
    popfield(L, arg_);
    if(*err)
        pushfielderror(L, sname, *err);
    return val;
    }

#define GetIntegerOpt(name, sname, defval) do {             \
    int err_;                                               \
    p->name = GetInteger_(L, arg, sname, defval, &err_);    \
    if(err_) return err_;                                   \
} while(0)

#define GetInteger(name, sname) GetIntegerOpt(name, sname, 0)
#define GetHandle GetInteger /* uint64_t handle */

#define GetNumArray_(name, sname, n, towhatx) do {  \
    int err_, arg2, arg3, t_, i_, isnum_;           \
    arg2 = pushfield(L, arg, sname);                \
    t_ = lua_type(L, arg2);                         \
    err_ = 0;                                       \
    if(t_ != LUA_TTABLE)                            \
        {                                           \
        popfield(L, arg2);                          \
        if(t_ == LUA_TNIL || t_ == LUA_TNONE)       \
            err_ = ERR_NOTPRESENT;                  \
        else                                        \
            return pushfielderror(L, sname, ERR_TABLE); \
        }                                           \
    else {                                          \
        for(i_=0; i_ <(n); i_++)                    \
            {                                       \
            lua_rawgeti(L, arg2, i_+1);             \
            arg3 = lua_gettop(L);                   \
            p->name[i_] = towhatx(L, arg3, &isnum_); \
            popfield(L, arg3);                      \
            if(!isnum_) { err_=ERR_TYPE; break; }   \
            }                                       \
        popfield(L, arg2);                          \
        if(err_ < 0)                                \
            {                                       \
            switch(err_)                            \
                {                                   \
                case ERR_TABLE:                     \
                case ERR_MEMORY:                    \
                case ERR_EMPTY:                     \
                    return pushfielderror(L, sname, err_);   \
                default:                            \
                    return prependfield(L, sname);   \
                }                                   \
            }                                       \
        }                                           \
} while(0)

#define GetIntegerArray(name, sname, n) GetNumArray_(name, sname, n, lua_tointegerx)
#define GetNumberArray(name, sname, n) GetNumArray_(name, sname, n, lua_tonumberx)

static int GetBoolean_(lua_State *L, int arg, const char *sname, int *err)
    {
    int val = 0;
    int arg_ = pushfield(L, arg, sname);
    *err = 0;
    if(lua_isboolean(L, arg_))
        val = lua_toboolean(L, arg_);
    else if(!lua_isnoneornil(L, arg_))
        *err = ERR_TYPE;
    popfield(L, arg_);
    if(*err)
        pushfielderror(L, sname, *err);
    return val;
    }

#define GetBoolean(name, sname) do {                \
    int err_;                                       \
    p->name = GetBoolean_(L, arg, sname, &err_);    \
    if(err_) return err_;                           \
} while(0)

/* Strings -------------------------------------------------------------------*/

static const char *GetString_(lua_State *L, int arg, const char *sname, const char *defval, int *err, size_t *len)
/* The caller must Free() the returned string (if not NULL).
 * If len!=NULL, sets len with the string length or with 0 if defval is used.
 *
 */
    {
    const char *val = NULL;
    int arg_ = pushfield(L, arg, sname);
    int t_ = lua_type(L, arg_);
    *err = 0;
    if(len) *len = 0;
    if(t_ == LUA_TSTRING)
        val = Strdup(L, lua_tolstring(L, arg_, len));
    else if((t_ == LUA_TNONE)||(t_ == LUA_TNIL))
        {
        if(defval)
            val = Strdup(L, defval);
        else
            *err = ERR_NOTPRESENT;
        }
    else
        *err = ERR_TYPE;
    popfield(L, arg_);
    if(*err)
        pushfielderror(L, sname, *err);
    return val;
    }

#define GetString(name, sname) do {                     \
    int err_;                                           \
    p->name =  GetString_(L, arg, sname, NULL, &err_, NULL);  \
    if(err_) return err_;                               \
} while(0)

#define GetStringOpt(name, sname) do {                  \
    int err_;                                           \
    p->name =  GetString_(L, arg, sname, NULL, &err_, NULL);  \
    if(err_ < 0) return err_;                           \
    if(err_ == ERR_NOTPRESENT) poperror();              \
} while(0)

#define GetStringDef(name, sname, defval) do {          \
    int err_;                                           \
    p->name =  GetString_(L, arg, sname, defval, &err_, NULL);\
    if(err_ < 0) return err_;                           \
} while(0)

#define GetLString(name, sname, len) /* size_t* */ do { \
    int err_;                                           \
    p->name =  GetString_(L, arg, sname, NULL, &err_, len);  \
    if(err_) return err_;                               \
} while(0)


/* Lightuserdata -------------------------------------------------------------*/

#define GetLightuserdataOpt(name, sname, TTT) do {  \
    int err_, arg_ = pushfield(L, arg, sname);      \
    err_ = 0;                                       \
    if(lua_isnoneornil(L, arg_))                    \
        p->name = NULL;                             \
    else                                            \
        {                                           \
        if(lua_type(L, arg_) != LUA_TLIGHTUSERDATA) \
            err_ = ERR_TYPE;                        \
        else                                        \
            p->name = (TTT)lua_touserdata(L, arg_); \
        }                                           \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)

#define GetLightuserdata GetLightuserdataOpt

/* Enums ---------------------------------------------------------------------*/

#define GetEnum_(name, sname, testfunc, defval, opt) do {   \
    int err_, arg_ = pushfield(L, arg, sname);              \
    p->name = testfunc(L, arg_, &err_);                     \
    popfield(L, arg_);                                      \
    if(opt && (err_ == ERR_NOTPRESENT))                     \
        p->name = (defval);                                 \
    else if(err_)                                           \
        return pushfielderror(L, sname, err_);              \
} while(0)

#define GetEnum(name, sname, testfunc) GetEnum_(name, sname, testfunc, 0,  0)
#define GetEnumOpt(name, sname, testfunc, defval) GetEnum_(name, sname, testfunc, defval, 1)

/* enums without defval (ie required) */
#define GetDescriptorType(name, sname) GetEnum(name, sname, testdescriptortype)
#define GetImageType(name, sname) GetEnum(name, sname, testimagetype)
#define GetImageViewType(name, sname) GetEnum(name, sname, testimageviewtype)
#define GetDescriptorUpdateTemplateType(name, sname) GetEnum(name, sname, testdescriptorupdatetemplatetype)
#define GetQueueGlobalPriority(name, sname) GetEnum(name, sname, testqueueglobalpriority)

#define GetCommandBufferLevel(name, sname) GetEnum(name, sname, testcommandbufferlevel)
#define GetQueryType(name, sname) GetEnum(name, sname, testquerytype)
#define GetDiscardRectangleMode(name, sname) GetEnum(name, sname, testdiscardrectanglemode)
#define GetDeviceEventType(name, sname) GetEnum(name, sname, testdeviceeventtype)
#define GetDisplayEventType(name, sname) GetEnum(name, sname, testdisplayeventtype)
#define GetDisplayPowerState(name, sname) GetEnum(name, sname, testdisplaypowerstate)
#define GetTessellationDomainOrigin(name, sname) GetEnum(name, sname, testtessellationdomainorigin)
#define GetObjectType(name, sname) GetEnum(name, sname, testobjecttype)

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
#define GetBlendOverlap(name, sname) GetEnumOpt(name, sname, testblendoverlap, VK_BLEND_OVERLAP_UNCORRELATED_EXT)
#define GetSamplerReductionMode(name, sname) GetEnumOpt(name, sname, testsamplerreductionmode, VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_EXT)
#define GetSamplerYcbcrModelConversion(name, sname) GetEnumOpt(name, sname, testsamplerycbcrmodelconversion, VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
#define GetSamplerYcbcrRange(name, sname) GetEnumOpt(name, sname, testsamplerycbcrrange, VK_SAMPLER_YCBCR_RANGE_ITU_FULL)
#define GetChromaLocation(name, sname) GetEnumOpt(name, sname, testchromalocation, VK_CHROMA_LOCATION_COSITED_EVEN)

/* Structs -------------------------------------------------------------------*/

#define GetStruct_(name, sname, echeckfunc, opt) do { \
    int err_, arg_ = pushfield(L, arg, sname);      \
    err_ = echeckfunc(L, arg_, &(p->name));         \
    popfield(L, arg_);                              \
    if(err_)                                        \
        {                                           \
        switch(err_)                                \
            {                                       \
            case ERR_NOTPRESENT: if(opt) break; /* else fallthrough */\
            case ERR_TABLE:                         \
            case ERR_MEMORY:                        \
            case ERR_EMPTY:                         \
                return pushfielderror(L, sname, err_);  \
            default:                                \
                return prependfield(L, sname);  \
            }                                       \
        }                                           \
} while(0)

#define GetStruct(name, sname, echeckfunc) GetStruct_(name, sname, echeckfunc, 0)
#define GetStructOpt(name, sname, echeckfunc) GetStruct_(name, sname, echeckfunc, 1)

#define GetStructArrayOpt(name, sname, n, echeckfunc) do {  \
    int err_, arg2, arg3, t_, i_;                   \
    arg2 = pushfield(L, arg, sname);                \
    t_ = lua_type(L, arg2);                         \
    err_ = 0;                                       \
    if(t_ != LUA_TTABLE)                            \
        {                                           \
        popfield(L, arg2);                          \
        if(t_ == LUA_TNIL || t_ == LUA_TNONE)       \
            err_ = ERR_NOTPRESENT;                  \
        else                                        \
            return pushfielderror(L, sname, ERR_TABLE); \
        }                                           \
    else {                                          \
        for(i_=0; i_ <(n); i_++)                    \
            {                                       \
            lua_rawgeti(L, arg2, i_+1);             \
            arg3 = lua_gettop(L);                   \
            err_ = echeckfunc(L, arg3, &(p->name[i_]));\
            popfield(L, arg3);                    \
            if(err_ < 0) break;                      \
            }                                       \
        popfield(L, arg2);                          \
        if(err_ < 0)                                \
            {                                       \
            switch(err_)                            \
                {                                   \
                case ERR_TABLE:                     \
                case ERR_MEMORY:                    \
                case ERR_EMPTY:                     \
                    return pushfielderror(L, sname, err_);   \
                default:                            \
                    return prependfield(L, sname);   \
                }                                   \
            }                                       \
        }                                           \
} while(0)

#define GetXYColorOpt(name, sname) GetStructOpt(name, sname, echeckxycolor)
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
#define GetSampleLocationsInfo(name, sname) GetStruct(name, sname, echecksamplelocationsinfo)

/* Objects -------------------------------------------------------------------*/

#define GetObject_(name, sname, TTT, ttt, opt) do { \
/* eg: TTT = VkRenderPass, ttt = render_pass */     \
    int err_, arg_ = pushfield(L, arg, sname);      \
    err_ = 0;                                       \
    if(lua_isnoneornil(L, arg_))                    \
        {                                           \
        p->name = 0;                                \
        if(!opt) err_ = ERR_NOTPRESENT;             \
        }                                           \
    else                                            \
        {                                           \
        p->name = test##ttt(L, arg_, NULL);         \
        if(!p->name) err_ = ERR_TYPE;               \
        }                                           \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)

#define GetObject(name, sname, TTT, ttt) GetObject_(name, sname, TTT, ttt, 0)
#define GetObjectOpt(name, sname, TTT, ttt) GetObject_(name, sname, TTT, ttt, 1)

#define GetRenderPass(name, sname) GetObject(name, sname, VkRenderPass, render_pass)
#define GetRenderPassOpt(name, sname) GetObjectOpt(name, sname, VkRenderPass, render_pass)
#define GetFramebuffer(name, sname) GetObject(name, sname, VkFramebuffer, framebuffer)
#define GetFramebufferOpt(name, sname) GetObjectOpt(name, sname, VkFramebuffer, framebuffer)
#define GetBuffer(name, sname) GetObject(name, sname, VkBuffer, buffer)
#define GetBufferOpt(name, sname) GetObjectOpt(name, sname, VkBuffer, buffer)
#define GetImage(name, sname) GetObject(name, sname, VkImage, image)
#define GetImageOpt(name, sname) GetObjectOpt(name, sname, VkImage, image)
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
#define GetValidationCache(name, sname) GetObject(name, sname, VkValidationCacheEXT, validation_cache)
#define GetSamplerYcbcrConversion(name, sname) GetObject(name, sname, VkSamplerYcbcrConversion, sampler_ycbcr_conversion)

#if 0
#define Get(name, sname) GetObject(name, sname, Vk, )
#define GetOpt(name, sname) GetObjectOpt(name, sname, Vk, )
#endif

/* Integers with special values ----------------------------------------------*/

#define GetSubpass(name, sname) /* integer or 'external' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    int err_ = 0;                                   \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "external") == 0)             \
            p->name = VK_SUBPASS_EXTERNAL;          \
        else                                        \
            err_ = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = 0;                                \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        err_ = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)


/* 'remaining' stands for VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS, etc
 * (provided it has a value of ~0U) */
#define GetIntegerOrRemaining(name, sname, defval) /* integer or 'remaining' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    int err_ = 0;                                   \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "remaining") == 0)            \
            p->name = ~0U; /* VK_REMAINING_XXX */   \
        else                                        \
            err_ = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = defval;                           \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        err_ = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)

#define GetIntegerOrWholeSize(name, sname) /* integer or 'whole size' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    int err_ = 0;                                   \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "whole size") == 0)           \
            p->name = VK_WHOLE_SIZE;                \
        else                                        \
            err_ = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_WHOLE_SIZE;                    \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        err_ = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)

#define GetAttachment(name, sname) /* integer or 'unused', defval = 'unused' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    int err_ = 0;                                   \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "unused") == 0)               \
            p->name = VK_ATTACHMENT_UNUSED;         \
        else                                        \
            err_ = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_ATTACHMENT_UNUSED;             \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        err_ = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(err_)                                        \
        return pushfielderror(L, sname, err_);      \
} while(0)

/*------------------------------------------------------------------------------*
 | Lists                                                                        |
 *------------------------------------------------------------------------------*
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


/*------------------------------------------------------------------------------*
 | Set macros (for push functions)                                              |
 *------------------------------------------------------------------------------*/

#define SetInteger(name, sname) do { lua_pushinteger(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetHandle SetInteger /* uint64_t handle */
#define SetNumber(name, sname) do { lua_pushnumber(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetFlags(name, sname) do { pushflags(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetBits SetFlags
#define SetBoolean(name, sname) do { lua_pushboolean(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetString(name, sname) do { lua_pushstring(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetLString(name, sname, len) do { lua_pushlstring(L, p->name, len); lua_setfield(L, -2, sname); } while(0)
#define SetUUID(name, sname, len) do { lua_pushlstring(L, (char*)p->name,(len)); lua_setfield(L, -2, sname); } while(0)
#define SetEnum(name, sname, pushfunc) do { pushfunc(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetStruct(name, sname, pushfunc) do { pushfunc(L, &(p->name)); lua_setfield(L, -2, sname); } while(0)
#define SetIntegerArray(name, sname, n) do { int i_;                                    \
    lua_newtable(L);                                                                    \
    for(i_=0; i_<(n); i_++) { lua_pushinteger(L, p->name[i_]); lua_seti(L, -2, i_+1); } \
    lua_setfield(L, -2, sname);                                                         \
} while(0)
#define SetNumberArray(name, sname, n) do { int i_;                                     \
    lua_newtable(L);                                                                    \
    for(i_=0; i_<(n); i_++) { lua_pushnumber(L, p->name[i_]); lua_seti(L, -2, i_+1); }  \
    lua_setfield(L, -2, sname);                                                         \
} while(0)


/*------------------------------------------------------------------------------*
 | Vulkan structs                                                               |
 *------------------------------------------------------------------------------*/

int echeckcommandpoolcreateinfo(lua_State *L, int arg, VkCommandPoolCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckcommandbufferallocateinfo(lua_State *L, int arg, VkCommandBufferAllocateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    GetCommandBufferLevel(level, "level");
    GetInteger(commandBufferCount, "command_buffer_count");
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckcommandbufferinheritanceconditionalrenderinginfo(lua_State *L, int arg, VkCommandBufferInheritanceConditionalRenderingInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT;
    GetBoolean(conditionalRenderingEnable, "conditional_rendering_enable");
    return 0;
    }

static int echeckcommandbufferinheritanceinfo(lua_State *L, int arg, VkCommandBufferInheritanceInfo_CHAIN *pp)
    {
    int err;
    VkCommandBufferInheritanceInfo *p = &pp->p1;
    VkCommandBufferInheritanceConditionalRenderingInfoEXT *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    GetRenderPassOpt(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetFramebufferOpt(framebuffer, "framebuffer");
    GetBoolean(occlusionQueryEnable, "occlusion_query_enable");
    GetFlags(queryFlags, "query_flags");
    GetFlags(pipelineStatistics, "pipeline_statistics");
    if(ispresent("conditional_rendering_enable"))
        {
        err = echeckcommandbufferinheritanceconditionalrenderinginfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
    return 0;
    }

int echeckcommandbufferbegininfo(lua_State *L, int arg, VkCommandBufferBeginInfo_CHAIN *pp)
    {
    int err, arg1;
    VkCommandBufferBeginInfo *p = &pp->p1;
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    GetFlags(flags, "flags");
#define F "inheritance_info"
    arg1 = pushfield(L, arg, F);
    err = echeckcommandbufferinheritanceinfo(L, arg1, &pp->p2);
    popfield(L, arg1);
    if(err < 0) return prependfield(L, F);
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        p->pInheritanceInfo = &pp->p2.p1;
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckexportfencecreateinfo(lua_State *L, int arg, VkExportFenceCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
    GetFlags(handleTypes, "handle_types");
    return 0;
    }

int echeckfencecreateinfo(lua_State *L, int arg, VkFenceCreateInfo_CHAIN *pp)
    {
    int err;
    VkFenceCreateInfo *p = &pp->p1;
    VkExportFenceCreateInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    GetFlags(flags, "flags");
    if(ispresent("handle_types"))
        {
        err = echeckexportfencecreateinfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
    return 0;
    }

int echeckdeviceeventinfo(lua_State *L, int arg, VkDeviceEventInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT;
    GetDeviceEventType(deviceEvent, "device_event");
    return 0;
    }

int echeckdisplayeventinfo(lua_State *L, int arg, VkDisplayEventInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT;
    GetDisplayEventType(displayEvent, "display_event");
    return 0;
    }

int echeckimportfencefdinfo(lua_State *L, int arg, VkImportFenceFdInfoKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR;
    /* p->fence is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
    GetInteger(fd, "fd");
    return 0;
    }

int echeckfencegetfdinfo(lua_State *L, int arg, VkFenceGetFdInfoKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR;
    /* p->fence is set by the caller */
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
    return 0;
    }


/*------------------------------------------------------------------------------*/

#ifdef VK_USE_PLATFORM_WIN32_KHR
#if 0 //@@TODO VK_KHR_external_semaphore_win32
typedef struct VkExportSemaphoreWin32HandleInfoKHR {
    VkStructureType               sType;
    const void*                   pNext;
    const SECURITY_ATTRIBUTES*    pAttributes;
    DWORD                         dwAccess;
    LPCWSTR                       name;
} VkExportSemaphoreWin32HandleInfoKHR;

typedef struct VkD3D12FenceSubmitInfoKHR {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           waitSemaphoreValuesCount;
    const uint64_t*    pWaitSemaphoreValues;
    uint32_t           signalSemaphoreValuesCount;
    const uint64_t*    pSignalSemaphoreValues;
} VkD3D12FenceSubmitInfoKHR;
#endif
#endif /* VK_USE_PLATFORM_WIN32_KHR */

static int echeckexportsemaphorecreateinfo(lua_State *L, int arg, VkExportSemaphoreCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
    GetFlags(handleTypes, "handle_types");
    return 0;
    }


int echecksemaphorecreateinfo(lua_State *L, int arg, VkSemaphoreCreateInfo_CHAIN *pp)
    {
    int err;
    VkSemaphoreCreateInfo *p = &pp->p1;
    VkExportSemaphoreCreateInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    GetFlags(flags, "flags");
    if(ispresent("handle_types"))
        {
        err = echeckexportsemaphorecreateinfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
    return 0;
    }

int echeckimportsemaphorefdinfo(lua_State *L, int arg, VkImportSemaphoreFdInfoKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    /* p->semaphore is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    GetInteger(fd, "fd");
    return 0;
    }

int echecksemaphoregetfdinfo(lua_State *L, int arg, VkSemaphoreGetFdInfoKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    /* p->semaphore is set by the caller */
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    return 0;
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR

int echeckimportsemaphorewin32handleinfo(lua_State *L, int arg, VkImportSemaphoreWin32HandleInfoKHR *p) //@@DOC VK_KHR_external_semaphore_win32
    {
    int err;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
    /* p->semaphore is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    GetLightuserdata(handle, "handle", HANDLE);
    p->name = NULL; //@@ LPCWSTR name;
    return 0;
    }

int echecksemaphoregetwin32handleinfo(lua_State *L, int arg, VkSemaphoreGetWin32HandleInfoKHR *p) //@@ DOC VK_KHR_external_semaphore_win32
    {
    int err;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    /* p->semaphore is set by the caller */
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    return 0;
    }


#endif /* VK_USE_PLATFORM_WIN32_KHR */

/*------------------------------------------------------------------------------*/

int echeckeventcreateinfo(lua_State *L, int arg, VkEventCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    GetFlags(flags, "flags");
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckdescriptorpoolsize(lua_State *L, int arg, VkDescriptorPoolSize *p)
    {
    CHECK_TABLE(L, arg, p);
    GetDescriptorType(type, "type");
    GetIntegerOpt(descriptorCount, "descriptor_count", 1);
    return 0;
    }

/* echeckdescriptorpoolsizelist() */
ECHECKLISTFUNC(VkDescriptorPoolSize, descriptorpoolsize, NULL)

void freedescriptorpoolcreateinfo(lua_State *L, VkDescriptorPoolCreateInfo *p)
    {
    if(p->pPoolSizes) Free(L, (void*)p->pPoolSizes);
    }

int echeckdescriptorpoolcreateinfo(lua_State *L, int arg, VkDescriptorPoolCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    GetFlags(flags, "flags");
    GetInteger(maxSets, "max_sets");
#define F "pool_sizes"
    arg1 = pushfield(L, arg, F);
    p->pPoolSizes = echeckdescriptorpoolsizelist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err) return prependfield(L, F);
    p->poolSizeCount = count;
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freedescriptorsetallocateinfo(lua_State *L, VkDescriptorSetAllocateInfo *p)
    {
    if(p->pSetLayouts) Free(L, (void*)p->pSetLayouts);
    }

int echeckdescriptorsetallocateinfo(lua_State *L, int arg, VkDescriptorSetAllocateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
#define F "set_layouts"
    arg1 = pushfield(L, arg, F);
    p->pSetLayouts = checkdescriptor_set_layoutlist(L, arg1, &count, &err, NULL);
    p->descriptorSetCount = count;
    popfield(L, arg1);
    if(err)
        { freedescriptorsetallocateinfo(L, p); return pushfielderror(L, F, err); }
#undef F
    /* p->descriptorPool = set by caller */
    return 0;
    }

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

    CHECK_TABLE(L, arg, p);
    GetInteger(binding, "binding");
    GetDescriptorType(descriptorType, "descriptor_type");
    GetInteger(descriptorCount, "descriptor_count");
    GetFlags(stageFlags, "stage_flags");

    if((p->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
        p->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
        p->descriptorCount != 0)
        {
#define F   "immutable_samplers"
        arg1 = pushfield(L, arg, F);
        p->pImmutableSamplers = checksamplerlist(L, arg1, &count, &err, NULL);
        popfield(L, arg1);
        if(err == ERR_NOTPRESENT)
            return 0;
        if(err < 0)
            { freedescriptorsetlayoutbinding(L, p); return pushfielderror(L, F, err); }
        if(count != p->descriptorCount)
            { freedescriptorsetlayoutbinding(L, p); return pushfielderror(L, F, ERR_LENGTH); }
#undef F
        }
    return 0;
    }

/* echeckdescriptorsetlayoutbindinglist() */
FREELISTFUNC(VkDescriptorSetLayoutBinding, descriptorsetlayoutbinding)
ECHECKLISTFUNC(VkDescriptorSetLayoutBinding, descriptorsetlayoutbinding, freedescriptorsetlayoutbindinglist)

void freedescriptorsetlayoutcreateinfo(lua_State *L, VkDescriptorSetLayoutCreateInfo *p)
    {
    if(p->pBindings)
        freedescriptorsetlayoutbindinglist(L, (VkDescriptorSetLayoutBinding*)p->pBindings, p->bindingCount);
    }

int echeckdescriptorsetlayoutcreateinfo(lua_State *L, int arg, VkDescriptorSetLayoutCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    GetFlags(flags, "flags");
#define F "bindings"
    arg1 = pushfield(L, arg, F);
    p->pBindings = echeckdescriptorsetlayoutbindinglist(L, arg1, &count, &err);
    p->bindingCount = count;
    popfield(L, arg1);
    if(err<0) return prependfield(L, F);
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckpushconstantrange(lua_State *L, int arg, VkPushConstantRange *p)
    {
    CHECK_TABLE(L, arg, p);
    GetFlags(stageFlags, "stage_flags");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckpushconstantrangelist() */
ECHECKLISTFUNC(VkPushConstantRange, pushconstantrange, NULL)

void freepipelinelayoutcreateinfo(lua_State *L, VkPipelineLayoutCreateInfo *p)
    {
    if(p->pSetLayouts) Free(L, (void*)p->pSetLayouts);
    if(p->pPushConstantRanges) Free(L, (void*)p->pPushConstantRanges);
    }

int echeckpipelinelayoutcreateinfo(lua_State *L, int arg, VkPipelineLayoutCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    GetFlags(flags, "flags");
#define F "set_layouts"
    arg1 = pushfield(L, arg, F);
    p->pSetLayouts = checkdescriptor_set_layoutlist(L, arg1, &count, &err, NULL);
    p->setLayoutCount = count;
    popfield(L, arg1);
    if(err<0)
        { freepipelinelayoutcreateinfo(L, p); return pushfielderror(L, F, err); }
#undef F
#define F "push_constant_ranges"
    arg1 = pushfield(L, arg, F);
    p->pPushConstantRanges = echeckpushconstantrangelist(L, arg1, &count, &err);
    p->pushConstantRangeCount = count;
    popfield(L, arg1);
    if(err<0)
        { freepipelinelayoutcreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckquerypoolcreateinfo(lua_State *L, int arg, VkQueryPoolCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    GetFlags(flags, "flags");
    GetQueryType(queryType, "query_type");
    GetInteger(queryCount, "query_count");
    GetFlags(pipelineStatistics, "pipeline_statistics");
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckattachmentdescription(lua_State *L, int arg, VkAttachmentDescription *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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

    CHECK_TABLE(L, arg, p);
    GetFlags(flags, "flags");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
#define F "input_attachments"
    arg1 = pushfield(L, arg, F);
    p->pInputAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    p->inputAttachmentCount = count;
    popfield(L, arg1);
    if(err < 0) { freesubpassdescription(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "color_attachments"
    arg1 = pushfield(L, arg, F);
    p->pColorAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    p->colorAttachmentCount = count;
    popfield(L, arg1);
    if(err < 0) { freesubpassdescription(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "resolve_attachments"
    arg1 = pushfield(L, arg, F);
    p->pResolveAttachments = echeckattachmentreferencelist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0) { freesubpassdescription(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT)
        poperror();
    else if(count != p->colorAttachmentCount)
        { err = ERR_LENGTH; lua_pushstring(L, errstring(err)); }
#undef F
#define F "depth_stencil_attachment"
    arg1 = pushfield(L, arg, F);
    err = echeckattachmentreference(L, arg1, &ref);
    popfield(L, arg1);
    if(err < 0) { freesubpassdescription(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
    else
        {
        p->pDepthStencilAttachment = MALLOC(L, VkAttachmentReference);
        memcpy((void*)p->pDepthStencilAttachment, &ref, sizeof(VkAttachmentReference));
        }
#undef F
#define F "preserve_attachments"
    arg1 = pushfield(L, arg, F);
    p->pPreserveAttachments = checkuint32list(L, arg1, &count, &err);
    p->inputAttachmentCount = count;
    popfield(L, arg1);
    if(err < 0) { freesubpassdescription(L, p); return pushfielderror(L, F, err); }
#undef F
    return 0;
    }

/* echecksubpassdescriptionlist() */
static FREELISTFUNC(VkSubpassDescription, subpassdescription)
static ECHECKLISTFUNC(VkSubpassDescription, subpassdescription, freesubpassdescriptionlist)

static int echeckinputattachmentaspectreference(lua_State *L, int arg, VkInputAttachmentAspectReference *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(subpass, "subpass");
    GetInteger(inputAttachmentIndex, "input_attachment_index");
    GetFlags(aspectMask, "aspect_mask");
    return 0;
    }

/* echeckinputattachmentaspectreferencelist() */
static ECHECKLISTFUNC(VkInputAttachmentAspectReference, inputattachmentaspectreference, NULL)

void freerenderpasscreateinfo(lua_State *L, VkRenderPassCreateInfo_CHAIN *pp)
    {
    VkRenderPassCreateInfo *p = &pp->p1;
    VkRenderPassInputAttachmentAspectCreateInfo *p2 = &pp->p2;
    if(!p) return;
    if(p->pAttachments) Free(L, (VkAttachmentDescription*)p->pAttachments);
    if(p->pSubpasses) freesubpassdescriptionlist(L, (void*)p->pSubpasses, p->subpassCount);
    if(p->pDependencies) Free(L, (VkSubpassDependency*)p->pDependencies);
    if((p->pNext == &p2) && p2->pAspectReferences)
        Free(L, (void*)p2->pAspectReferences);
    }

int echeckrenderpasscreateinfo(lua_State *L, int arg, VkRenderPassCreateInfo_CHAIN *pp)
    {
    int err, arg1;
    VkRenderPassCreateInfo *p = &pp->p1;
    VkRenderPassInputAttachmentAspectCreateInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    GetFlags(flags, "flags");
#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = echeckattachmentdescriptionlist(L, arg1, &p->attachmentCount, &err);
    popfield(L, arg1);
    if(err < 0) { freerenderpasscreateinfo(L, pp); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "subpasses"
    arg1 = pushfield(L, arg, F);
    p->pSubpasses = echecksubpassdescriptionlist(L, arg1, &p->subpassCount, &err);
    popfield(L, arg1);
    if(err) { freerenderpasscreateinfo(L, pp); return prependfield(L, F); }
#undef F
#define F "dependencies"
    arg1 = pushfield(L, arg, F);
    p->pDependencies = echecksubpassdependencylist(L, arg1, &p->dependencyCount, &err);
    popfield(L, arg1);
    if(err < 0) { freerenderpasscreateinfo(L, pp); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "input_attachment_aspect_references"
    arg1 = pushfield(L, arg, F);
    p2->sType = VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO;
    p2->pAspectReferences = echeckinputattachmentaspectreferencelist(L, arg1, &p2->aspectReferenceCount, &err);
    popfield(L, arg1);
    if(err < 0) { freerenderpasscreateinfo(L, pp); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        addtochain(chain, p2);
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    GetFlags(flags, "flags");

    GetRenderPass(renderPass, "render_pass");
    GetInteger(width, "width");
    GetInteger(height, "height");
    GetIntegerOpt(layers, "layers", 1);

#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = checkimage_viewlist(L, arg1, &p->attachmentCount, &err, NULL);
    popfield(L, arg1);
    if(err < 0) return pushfielderror(L, F, err);
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }


/*------------------------------------------------------------------------------*/

static int echeckshadermodulevalidationcachecreateinfo(lua_State *L, int arg, VkShaderModuleValidationCacheCreateInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT;
    GetValidationCache(validationCache, "validation_cache");
    return 0;
    }

int echeckshadermodulecreateinfo(lua_State *L, int arg, VkShaderModuleCreateInfo_CHAIN *pp)
    {
    int err;
    VkShaderModuleCreateInfo *p = &pp->p1;
    VkShaderModuleValidationCacheCreateInfoEXT *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    GetFlags(flags, "flags");
    /* p->pCode, p->codeSize: retrieved by the caller */
#define F "validation_cache"
    if(ispresent(F))
        {
        err = echeckshadermodulevalidationcachecreateinfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckpipelinecachecreateinfo(lua_State *L, int arg, VkPipelineCacheCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    GetFlags(flags, "flags");
    /* p->pInitialData, p->initialDataSize: retrieved by the caller */
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckvalidationcachecreateinfo(lua_State *L, int arg, VkValidationCacheCreateInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT;
    GetFlags(flags, "flags");
    /* p->pInitialData, p->initialDataSize: retrieved by the caller */
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freebuffercreateinfo(lua_State *L, VkBufferCreateInfo_CHAIN *pp)
    {
    VkBufferCreateInfo *p = &pp->p1;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    }

static int echeckexternalmemorybuffercreateinfo(lua_State *L, int arg, VkExternalMemoryBufferCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    GetFlags(handleTypes, "handle_types");
    return 0;
    }

int echeckbuffercreateinfo(lua_State *L, int arg, VkBufferCreateInfo_CHAIN *pp)
    {
    int err, arg1;
    VkBufferCreateInfo *p = &pp->p1;
    VkExternalMemoryBufferCreateInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    GetFlags(flags, "flags");
    GetInteger(size, "size");
    GetFlags(usage, "usage");
    GetSharingMode(sharingMode, "sharing_mode");
#define F "queue_family_indices"
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    popfield(L, arg1);
    if(err < 0) return pushfielderror(L, F, err);
#undef F
    if(ispresent("handle_types"))
        {
        err = echeckexternalmemorybuffercreateinfo(L, arg, p2);
        if(err) { freebuffercreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckbufferviewcreateinfo(lua_State *L, int arg, VkBufferViewCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    GetFlags(flags, "flags");
/*  p->buffer = set by caller */
    GetFormat(format, "format");
    GetInteger(offset, "offset");
    GetInteger(range, "range");
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freeimagecreateinfo(lua_State *L, VkImageCreateInfo_CHAIN *pp)
    {
    VkImageCreateInfo *p = &pp->p1;
    VkImageFormatListCreateInfoKHR *p3 = &pp->p3;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    if(p3->pViewFormats) freeformatlist(L, p3->pViewFormats);
    }

static int echeckexternalmemoryimagecreateinfo(lua_State *L, int arg, VkExternalMemoryImageCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    GetFlags(handleTypes, "handle_types");
    return 0;
    }

int echeckimagecreateinfo(lua_State *L, int arg, VkImageCreateInfo_CHAIN *pp)
    {
    int err, arg1;
    VkImageCreateInfo *p = &pp->p1;
    VkExternalMemoryImageCreateInfo *p2 = &pp->p2;
    VkImageFormatListCreateInfoKHR *p3 = &pp->p3;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    popfield(L, arg1);
    if(err < 0) return pushfielderror(L, F, err);
#undef F
#define F "handle_types"
    if(ispresent(F))
        {
        err = echeckexternalmemoryimagecreateinfo(L, arg, p2);
        if(err) { freeimagecreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
#undef F
#define F "view_formats"
    if(ispresent(F))
        {
        arg1 = pushfield(L, arg, F);
        p3->sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
        p3->pViewFormats = checkformatlist(L, arg1, &p3->viewFormatCount, &err);
        popfield(L, arg1);
        if(err) { freeimagecreateinfo(L, pp); return pushfielderror(L, F, err); }
        addtochain(chain, p3);
        }
#undef F
    return 0;
    }


/*------------------------------------------------------------------------------*/

static int echeckimageviewusagecreateinfo(lua_State *L, int arg, VkImageViewUsageCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    GetFlags(usage, "usage");
    return 0;
    }

int echeckimageviewcreateinfo(lua_State *L, int arg, VkImageViewCreateInfo_CHAIN *pp)
    {
    int err;
    VkImageViewCreateInfo  *p = &pp->p1;
    VkImageViewUsageCreateInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    GetFlags(flags, "flags");
/*  p->image = set by caller */
    GetImageViewType(viewType, "view_type");
    GetFormat(format, "format");
    GetComponentMappingOpt(components, "components");
    GetImageSubresourceRangeOpt(subresourceRange, "subresource_range");
    if(ispresent("usage"))
        {
        err = echeckimageviewusagecreateinfo(L, arg, p2);
        if(err) { freeimageviewcreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freesamplercreateinfo(lua_State *L, VkSamplerCreateInfo_CHAIN *p)
    {
    (void)L; (void)p;
    }

static int echecksamplerreductionmodecreateinfo(lua_State *L, int arg, VkSamplerReductionModeCreateInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
    GetSamplerReductionMode(reductionMode, "reduction_mode");
    return 0;
    }

static int echecksamplerycbcrconversioninfo(lua_State *L, int arg, VkSamplerYcbcrConversionInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    GetSamplerYcbcrConversion(conversion, "conversion");
    return 0;
    }

int echecksamplercreateinfo(lua_State *L, int arg, VkSamplerCreateInfo_CHAIN *pp)
    {
    int err;
    VkSamplerCreateInfo *p = &pp->p1;
    VkSamplerReductionModeCreateInfoEXT *p2 = &pp->p2;
    VkSamplerYcbcrConversionInfo *p3 = &pp->p3;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
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
    if(ispresent("reduction_mode"))
        {
        err = echecksamplerreductionmodecreateinfo(L, arg, p2);
        if(err) { freesamplercreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    if(ispresent("conversion"))
        {
        err = echecksamplerycbcrconversioninfo(L, arg, p3);
        if(err) { freesamplercreateinfo(L, pp); return err; }
        addtochain(chain, p3);
        }
    return 0;
    }


/*------------------------------------------------------------------------------*/

static void freeattachmentsamplelocations(lua_State *L, VkAttachmentSampleLocationsEXT *p)
    {
    freesamplelocationsinfo(L, &p->sampleLocationsInfo);
    }

static int echeckattachmentsamplelocations(lua_State *L, int arg, VkAttachmentSampleLocationsEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(attachmentIndex, "attachment_index");
    GetSampleLocationsInfo(sampleLocationsInfo, "sample_locations_info");
    return 0;
    }

static FREELISTFUNC(VkAttachmentSampleLocationsEXT, attachmentsamplelocations)
static ECHECKLISTFUNC(VkAttachmentSampleLocationsEXT, attachmentsamplelocations, freeattachmentsamplelocationslist) /* echeckattachmentsamplelocationslist() */

static void freesubpasssamplelocations(lua_State *L, VkSubpassSampleLocationsEXT *p)
    {
    freesamplelocationsinfo(L, &p->sampleLocationsInfo);
    }

static int echecksubpasssamplelocations(lua_State *L, int arg, VkSubpassSampleLocationsEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(subpassIndex, "subpass_index");
    GetSampleLocationsInfo(sampleLocationsInfo, "sample_locations_info");
    return 0;
    }
static FREELISTFUNC(VkSubpassSampleLocationsEXT, subpasssamplelocations)
static ECHECKLISTFUNC(VkSubpassSampleLocationsEXT, subpasssamplelocations, freesubpasssamplelocationslist) /* echecksubpasssamplelocationslist() */


static void freerenderpasssamplelocationsbegininfo(lua_State *L, VkRenderPassSampleLocationsBeginInfoEXT *p)
    {
    if(p->pAttachmentInitialSampleLocations) Free(L, (void*)p->pAttachmentInitialSampleLocations);
    if(p->pPostSubpassSampleLocations) Free(L, (void*)p->pPostSubpassSampleLocations);
    }

static int echeckrenderpasssamplelocationsbegininfo(lua_State *L, int arg, VkRenderPassSampleLocationsBeginInfoEXT *p)
    {
    int err, arg1;
    p->sType = VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT;
#define F "attachment_initial_sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pAttachmentInitialSampleLocations = echeckattachmentsamplelocationslist(L, arg1, &p->attachmentInitialSampleLocationsCount, &err);
    popfield(L, arg1);
    if(err < 0) return prependfield(L, F);
#undef F
#define F "post_subpass_sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pPostSubpassSampleLocations = echecksubpasssamplelocationslist(L, arg1, &p->postSubpassSampleLocationsCount, &err);
    popfield(L, arg1);
    if(err < 0) return prependfield(L, F);
#undef F
    if((p->attachmentInitialSampleLocationsCount + p->postSubpassSampleLocationsCount) == 0)
        {
        lua_pushstring(L, errstring(ERR_NOTPRESENT));
        return ERR_NOTPRESENT;
        }
    return 0;
    }

void freerenderpassbegininfo(lua_State *L, VkRenderPassBeginInfo_CHAIN *pp)
    {
    VkRenderPassBeginInfo *p = &pp->p1;
    VkRenderPassSampleLocationsBeginInfoEXT *p2 = &pp->p2;
    if(p->pClearValues) Free(L, (void*)p->pClearValues);
    freerenderpasssamplelocationsbegininfo(L, p2);
    }

static VkClearValue* echeckclearvaluelist(lua_State*, int, uint32_t*, int*); /* forward decl. */
int echeckrenderpassbegininfo(lua_State *L, int arg, VkRenderPassBeginInfo_CHAIN *pp)
    {
    int err, arg1;
    uint32_t count;
    VkRenderPassBeginInfo *p = &pp->p1;
    VkRenderPassSampleLocationsBeginInfoEXT *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    GetRenderPass(renderPass, "render_pass");
    GetFramebuffer(framebuffer, "framebuffer");
    GetRect2dOpt(renderArea, "render_area");
#define F "clear_values"
    arg1 = pushfield(L, arg, F);
    p->pClearValues = echeckclearvaluelist(L, arg1, &count, &err);
    p->clearValueCount = count;
    popfield(L, arg1);
    if(err < 0) return prependfield(L, F);
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    err = echeckrenderpasssamplelocationsbegininfo(L, arg, p2);
    if(err < 0) { freerenderpassbegininfo(L, pp); return err; }
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        addtochain(chain, p2);
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckphysicaldevicefeatures(lua_State *L, int arg, VkPhysicalDeviceFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
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
 
static int echeckphysicaldevice16bitstoragefeatures(lua_State *L, int arg, VkPhysicalDevice16BitStorageFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    GetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    GetBoolean(storagePushConstant16, "storage_push_constant_16");
    GetBoolean(storageInputOutput16, "storage_input_output_16");
    return 0;
    }

static int echeckphysicaldevicevariablepointerfeatures(lua_State *L, int arg, VkPhysicalDeviceVariablePointerFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    GetBoolean(variablePointers, "variable_pointers");
    return 0;
    }

static int echeckphysicaldeviceblendoperationadvancedfeatures(lua_State *L, int arg, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(advancedBlendCoherentOperations, "advanced_blend_coherent_operations");
    return 0;
    }

static int echeckphysicaldevicesamplerycbcrconversionfeatures(lua_State *L, int arg, VkPhysicalDeviceSamplerYcbcrConversionFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(samplerYcbcrConversion, "sampler_ycbcr_conversion");
    return 0;
    }

static int echeckphysicaldeviceconditionalrenderingfeatures(lua_State *L, int arg, VkPhysicalDeviceConditionalRenderingFeaturesEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(conditionalRendering, "conditional_rendering");
    GetBoolean(inheritedConditionalRendering, "inherited_conditional_rendering");
    return 0;
    }

static int echeckphysicaldevice8bitstoragefeatures(lua_State *L, int arg, VkPhysicalDevice8BitStorageFeaturesKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(storageBuffer8BitAccess, "storage_buffer_8bit_access");
    GetBoolean(uniformAndStorageBuffer8BitAccess, "uniform_and_storage_buffer_8bit_access");
    GetBoolean(storagePushConstant8, "storage_push_constant_8");
    return 0;
    }

static int echeckphysicaldeviceprotectedmemoryfeatures(lua_State *L, int arg, VkPhysicalDeviceProtectedMemoryFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(protectedMemory, "protected_memory");
    return 0;
    }

static int echeckphysicaldeviceshaderdrawparameterfeatures(lua_State *L, int arg, VkPhysicalDeviceShaderDrawParameterFeatures *p)
    {
    CHECK_TABLE(L, arg, p);
    GetBoolean(shaderDrawParameters, "shader_draw_parameters");
    return 0;
    }

#define BUILD_CHAIN_VkPhysicalDeviceFeatures2(p) do { \
    (p)->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2; \
    (p)->p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES; \
    (p)->p3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES; \
    (p)->p4.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT; \
    (p)->p5.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES; \
    (p)->p6.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT; \
    (p)->p7.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR; \
    (p)->p8.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES; \
    (p)->p9.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES; \
    (p)->p1.pNext = &p->p2; \
    (p)->p2.pNext = &p->p3; \
    (p)->p3.pNext = &p->p4; \
    (p)->p4.pNext = &p->p5; \
    (p)->p5.pNext = &p->p6; \
    (p)->p6.pNext = &p->p7; \
    (p)->p7.pNext = &p->p8; \
    (p)->p8.pNext = &p->p9; \
    (p)->p9.pNext = NULL;   \
} while(0)

void initphysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    BUILD_CHAIN_VkPhysicalDeviceFeatures2(p);
    }

static int echeckphysicaldevicefeatures2(lua_State *L, int arg, VkPhysicalDeviceFeatures2_CHAIN *p)
    {
    int err;
    err = echeckphysicaldevicefeatures(L, arg, &p->p1.features);
    if(err) return err;
    err = echeckphysicaldevice16bitstoragefeatures(L, arg, &p->p2);
    if(err) return err;
    err = echeckphysicaldevicevariablepointerfeatures(L, arg, &p->p3);
    if(err) return err;
    err = echeckphysicaldeviceblendoperationadvancedfeatures(L, arg, &p->p4);
    if(err) return err;
    err = echeckphysicaldevicesamplerycbcrconversionfeatures(L, arg, &p->p5);
    if(err) return err;
    err = echeckphysicaldeviceconditionalrenderingfeatures(L, arg, &p->p6);
    if(err) return err;
    err = echeckphysicaldevice8bitstoragefeatures(L, arg, &p->p7);
    if(err) return err;
    err = echeckphysicaldeviceprotectedmemoryfeatures(L, arg, &p->p8);
    if(err) return err;
    err = echeckphysicaldeviceshaderdrawparameterfeatures(L, arg, &p->p9);
    if(err) return err;
    BUILD_CHAIN_VkPhysicalDeviceFeatures2(p);
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

static int pushphysicaldevice16bitstoragefeatures(lua_State *L, VkPhysicalDevice16BitStorageFeatures *p)
    {
    SetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    SetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    SetBoolean(storagePushConstant16, "storage_push_constant_16");
    SetBoolean(storageInputOutput16, "storage_input_output_16");
    return 1;
    }

static int pushphysicaldevicevariablepointerfeatures(lua_State *L, VkPhysicalDeviceVariablePointerFeatures *p)
    {
    SetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    SetBoolean(variablePointers, "variable_pointers");
    return 1;
    }

static int pushphysicaldeviceblendoperationadvancedfeatures(lua_State *L, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *p)
    {
    SetBoolean(advancedBlendCoherentOperations, "advanced_blend_coherent_operations");
    return 1;
    }

static int pushphysicaldevicesamplerycbcrconversionfeatures(lua_State *L, VkPhysicalDeviceSamplerYcbcrConversionFeatures *p)
    {
    SetBoolean(samplerYcbcrConversion, "sampler_ycbcr_conversion");
    return 0;
    }

static int pushphysicaldeviceconditionalrenderingfeatures(lua_State *L, VkPhysicalDeviceConditionalRenderingFeaturesEXT *p)
    {
    SetBoolean(conditionalRendering, "conditional_rendering");
    SetBoolean(inheritedConditionalRendering, "inherited_conditional_rendering");
    return 0;
    }

static int pushphysicaldevice8bitstoragefeatures(lua_State *L, VkPhysicalDevice8BitStorageFeaturesKHR *p)
    {
    SetBoolean(storageBuffer8BitAccess, "storage_buffer_8bit_access");
    SetBoolean(uniformAndStorageBuffer8BitAccess, "uniform_and_storage_buffer_8bit_access");
    SetBoolean(storagePushConstant8, "storage_push_constant_8");
    return 0;
    }

static int pushphysicaldeviceprotectedmemoryfeatures(lua_State *L, VkPhysicalDeviceProtectedMemoryFeatures *p)
    {
    SetBoolean(protectedMemory, "protected_memory");
    return 0;
    }

static int pushphysicaldeviceshaderdrawparameterfeatures(lua_State *L, VkPhysicalDeviceShaderDrawParameterFeatures *p)
    {
    SetBoolean(shaderDrawParameters, "shader_draw_parameters");
    return 0;
    }

int pushphysicaldevicefeatures2(lua_State *L, VkPhysicalDeviceFeatures2_CHAIN *p)
    {
    pushphysicaldevicefeatures(L, &p->p1.features);
    pushphysicaldevice16bitstoragefeatures(L, &p->p2);
    pushphysicaldevicevariablepointerfeatures(L, &p->p3);
    pushphysicaldeviceblendoperationadvancedfeatures(L, &p->p4);
    pushphysicaldevicesamplerycbcrconversionfeatures(L, &p->p5);
    pushphysicaldeviceconditionalrenderingfeatures(L, &p->p6);
    pushphysicaldevice8bitstoragefeatures(L, &p->p7);
    pushphysicaldeviceprotectedmemoryfeatures(L, &p->p8);
    pushphysicaldeviceshaderdrawparameterfeatures(L, &p->p9);
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

static int pushphysicaldeviceblendoperationadvancedproperties(lua_State *L, VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *p)
    {
    SetInteger(advancedBlendMaxColorAttachments, "advanced_blend_max_color_attachments");
    SetBoolean(advancedBlendIndependentBlend, "advanced_blend_independent_blend");
    SetBoolean(advancedBlendNonPremultipliedSrcColor, "advanced_blend_non_premultiplied_src_color");
    SetBoolean(advancedBlendNonPremultipliedDstColor, "advanced_blend_non_premultiplied_dst_color");
    SetBoolean(advancedBlendCorrelatedOverlap, "advanced_blend_correlated_overlap");
    SetBoolean(advancedBlendAllOperations, "advanced_blend_all_operations");
    return 0;
    }

static int pushphysicaldevicesamplerfilterminmaxproperties(lua_State *L, VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT *p)
    {
    SetBoolean(filterMinmaxSingleComponentFormats, "filter_minmax_single_component_formats");
    SetBoolean(filterMinmaxImageComponentMapping, "filter_minmax_image_component_mapping");
    return 0;
    }

static int pushphysicaldevicediscardrectangleproperties(lua_State *L, VkPhysicalDeviceDiscardRectanglePropertiesEXT *p)
    {
    SetInteger(maxDiscardRectangles, "max_discard_rectangles");
    return 0;
    }


static int pushphysicaldeviceidproperties(lua_State *L, VkPhysicalDeviceIDProperties *p)
    {
    SetUUID(deviceUUID, "device_uuid", VK_UUID_SIZE);
    SetUUID(driverUUID, "driver_uuid", VK_UUID_SIZE);
    SetUUID(deviceLUID, "device_luid", VK_LUID_SIZE);
    SetInteger(deviceNodeMask, "device_node_mask");
    SetBoolean(deviceLUIDValid, "device_luid_valid");
    return 0;
    }

static int pushphysicaldevicepointclippingproperties(lua_State *L, VkPhysicalDevicePointClippingProperties *p)
    {
    SetEnum(pointClippingBehavior, "point_clipping_behavior", pushpointclippingbehavior);
    return 0;
    }

static int pushphysicaldevicesamplelocationsproperties(lua_State *L, VkPhysicalDeviceSampleLocationsPropertiesEXT *p)
    {
    SetFlags(sampleLocationSampleCounts, "sample_location_sample_counts");
    SetStruct(maxSampleLocationGridSize, "max_sample_location_grid_size", pushextent2d);
    SetNumberArray(sampleLocationCoordinateRange, "sample_location_coordinate_range", 2);
    SetInteger(sampleLocationSubPixelBits, "sample_location_sub_pixel_bits");
    SetBoolean(variableSampleLocations, "variable_sample_locations");
    return 0;
    }

static int pushphysicaldevicemaintenance3properties(lua_State *L, VkPhysicalDeviceMaintenance3Properties *p)
    {
    SetInteger(maxPerSetDescriptors, "max_per_set_descriptors");
    SetInteger(maxMemoryAllocationSize, "max_memory_allocation_size");
    return 0;
    }

static int pushphysicaldevicesubgroupproperties(lua_State *L, VkPhysicalDeviceSubgroupProperties *p)
    {
    SetFlags(supportedStages, "supported_stages");
    SetFlags(supportedOperations, "supported_operations");
    SetBoolean(quadOperationsInAllStages, "quad_operations_in_all_stages");
    return 0;
    }

static int pushphysicaldeviceprotectedmemoryproperties(lua_State *L, VkPhysicalDeviceProtectedMemoryProperties *p)
    {
    SetBoolean(protectedNoFault, "protected_no_fault");
    return 0;
    }

//@@TODO: Investigate if it is legal to chain all structs, even if extensions are not enabled
void initphysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    p->p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;
    p->p3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
    p->p4.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT;
    p->p5.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT;
    p->p6.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
    p->p7.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES;
    p->p8.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;
    p->p9.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
    p->p10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    p->p11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
    p->p1.pNext = &p->p2;
    p->p2.pNext = &p->p3;
    p->p3.pNext = &p->p4;
    p->p4.pNext = &p->p5;
    p->p5.pNext = &p->p6;
    p->p6.pNext = &p->p7;
    p->p7.pNext = &p->p8;
    p->p8.pNext = &p->p9;
    p->p9.pNext = &p->p10;
    p->p10.pNext = &p->p11;
    p->p11.pNext = NULL;
    }

int pushphysicaldeviceproperties2(lua_State *L, VkPhysicalDeviceProperties2_CHAIN *p)
    {
    pushphysicaldeviceproperties(L, &p->p1.properties);
    pushphysicaldevicepushdescriptorproperties(L, &p->p2);
    pushphysicaldeviceblendoperationadvancedproperties(L, &p->p3);
    pushphysicaldevicesamplerfilterminmaxproperties(L, &p->p4);
    pushphysicaldevicediscardrectangleproperties(L, &p->p5);
    pushphysicaldeviceidproperties(L, &p->p6);
    pushphysicaldevicepointclippingproperties(L, &p->p7);
    pushphysicaldevicesamplelocationsproperties(L, &p->p8);
    pushphysicaldevicemaintenance3properties(L, &p->p9);
    pushphysicaldevicesubgroupproperties(L, &p->p10);
    pushphysicaldeviceprotectedmemoryproperties(L, &p->p11);
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

void initformatproperties2(lua_State *L, VkFormatProperties2_CHAIN* p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    }

int pushformatproperties2(lua_State *L, VkFormatProperties2_CHAIN *p)
    {
    pushformatproperties(L, &p->p1.formatProperties);
    return 1;
    }

/*------------------------------------------------------------------------------*/

static int pushexternalmemoryproperties(lua_State *L, VkExternalMemoryProperties *p)
    {
    lua_newtable(L);
    SetFlags(externalMemoryFeatures, "external_memory_features");
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
    return 1;
    }

static int pushsamplerycbcrconversionimageformatproperties(lua_State *L, VkSamplerYcbcrConversionImageFormatProperties *p)
    {
    SetInteger(combinedImageSamplerDescriptorCount, "combined_image_sampler_descriptor_count");
    return 1;
    }

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

void initimageformatproperties2(lua_State *L, VkImageFormatProperties2_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    p->p2.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
    p->p3.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES;
    }

int pushimageformatproperties2(lua_State *L, VkImageFormatProperties2_CHAIN *p)
    {
    pushimageformatproperties(L, &p->p1.imageFormatProperties);
    pushexternalmemoryproperties(L, &p->p2.externalMemoryProperties);
    lua_setfield(L, -2, "external_memory_properties");
    pushsamplerycbcrconversionimageformatproperties(L, &p->p3);
    return 1;
    }

/*------------------------------------------------------------------------------*/

int pushexternalbufferproperties(lua_State *L, VkExternalBufferProperties *p)
    {
    lua_newtable(L);
    SetStruct(externalMemoryProperties, "external_memory_properties", pushexternalmemoryproperties);
    return 1;
    }

int echeckphysicaldeviceexternalbufferinfo(lua_State *L, int arg, VkPhysicalDeviceExternalBufferInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    GetFlags(flags, "flags");
    GetFlags(usage, "usage");
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
    return 0;
    }

/*------------------------------------------------------------------------------*/

int pushexternalfenceproperties(lua_State *L, VkExternalFenceProperties *p)
    {
    lua_newtable(L);
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
    SetFlags(externalFenceFeatures, "external_fence_features");
    return 1;
    }

int echeckphysicaldeviceexternalfenceinfo(lua_State *L, int arg, VkPhysicalDeviceExternalFenceInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO;
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
    return 0;
    }

/*------------------------------------------------------------------------------*/

int pushexternalsemaphoreproperties(lua_State *L, VkExternalSemaphoreProperties *p)
    {
    lua_newtable(L);
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
    SetFlags(externalSemaphoreFeatures, "external_semaphore_features");
    return 1;
    }

int echeckphysicaldeviceexternalsemaphoreinfo(lua_State *L, int arg, VkPhysicalDeviceExternalSemaphoreInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    return 0;
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

VkSparseImageFormatProperties2 *newsparseimageformatproperties2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkSparseImageFormatProperties2 *p = NMALLOC_NOERR(L, VkSparseImageFormatProperties2, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2;
        }
    return p;
    }

void freesparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2 *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushsparseimageformatproperties2(lua_State *L, VkSparseImageFormatProperties2 *p)
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

VkQueueFamilyProperties2 *newqueuefamilyproperties2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkQueueFamilyProperties2 *p = NMALLOC_NOERR(L, VkQueueFamilyProperties2, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        }
    return p;
    }

void freequeuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2 *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushqueuefamilyproperties2(lua_State *L, VkQueueFamilyProperties2 *p, uint32_t index)
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

void initphysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    }

int pushphysicaldevicememoryproperties2(lua_State *L, VkPhysicalDeviceMemoryProperties2_CHAIN *p)
    {
    pushphysicaldevicememoryproperties(L, &p->p1.memoryProperties);
    return 1;
    }

/*------------------------------------------------------------------------------*/

static int echeckphysicaldeviceexternalimageformatinfo(lua_State *L, int arg, VkPhysicalDeviceExternalImageFormatInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
    return 0;
    }

int echeckphysicaldeviceimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceImageFormatInfo2_CHAIN *pp)
    {
    int err;
    VkPhysicalDeviceImageFormatInfo2 *p = &pp->p1;
    VkPhysicalDeviceExternalImageFormatInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    GetFormat(format, "format");
    GetImageType(type, "type");
    GetImageTiling(tiling, "tiling");
    GetFlags(usage, "usage");
    GetFlags(flags, "flags");
    if(ispresent("handle_type"))
        {
        err = echeckphysicaldeviceexternalimageformatinfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
    return 0;
    }

int echeckphysicaldevicesparseimageformatinfo2(lua_State *L, int arg, VkPhysicalDeviceSparseImageFormatInfo2 *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2;
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
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    GetInteger(x, "x");
    GetInteger(y, "y");
    return 0;
    }

int echeckoffset3d(lua_State *L, int arg, VkOffset3D *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(x, "x");
    GetInteger(y, "y");
    GetInteger(z, "z");
    return 0;
    }

int echeckextent2d(lua_State *L, int arg, VkExtent2D *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(width, "width");
    GetInteger(height, "height");
    return 0;
    }

int echeckextent3d(lua_State *L, int arg, VkExtent3D *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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

int echecksamplelocation(lua_State *L, int arg, VkSampleLocationEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetNumber(x, "x");
    GetNumber(y, "y");
    return 0;
    }

/* echecksamplelocationlist() */
ECHECKLISTFUNC(VkSampleLocationEXT, samplelocation, NULL)

int pushsamplelocation(lua_State *L, VkSampleLocationEXT *p)
    {
    lua_newtable(L);
    SetNumber(x, "x");
    SetNumber(y, "y");
    return 1;
    }

/*------------------------------------------------------------------------------*/

int echeckcomponentmapping(lua_State *L, int arg, VkComponentMapping *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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

int echeckxycolor(lua_State *L, int arg, VkXYColorEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    GetNumber(x, "x");
    GetNumber(y, "y");
    return 0;
    }

int echeckhdrmetadata(lua_State *L, int arg, VkHdrMetadataEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
    GetXYColorOpt(displayPrimaryRed, "display_primary_red");
    GetXYColorOpt(displayPrimaryGreen, "display_primary_green");
    GetXYColorOpt(displayPrimaryBlue, "display_primary_blue");
    GetXYColorOpt(whitePoint, "white_point");
    GetNumber(maxLuminance, "max_luminance");
    GetNumber(minLuminance, "min_luminance");
    GetNumber(maxContentLightLevel, "max_content_light_level");
    GetNumber(maxFrameAverageLightLevel, "max_frame_average_light_level");
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckclearcolorvalue(lua_State *L, int arg, VkClearColorValue *p)
    {
    int i, t;
    const char* s;
    int colortype = -1;

    CHECK_TABLE(L, arg, p);
    getfield(L, arg, "t");
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

int echeckclearvalue(lua_State *L, int arg, VkClearValue *p)
    {
    int t;
    CHECK_TABLE(L, arg, p);
    t = getfield(L, arg, "depth");
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
static ECHECKLISTFUNC(VkClearValue, clearvalue, NULL) /* echeckclearvaluelist() */

int echeckclearattachment(lua_State *L, int arg, VkClearAttachment *p)
    {
    CHECK_TABLE(L, arg, p);
    GetFlags(aspectMask, "aspect_mask");
    GetAttachment(colorAttachment, "color_attachment");
    GetClearValue(clearValue, "clear_value");
    return 0;
    }

/* echeckclearattachmentlist() */
ECHECKLISTFUNC(VkClearAttachment, clearattachment, NULL)

int echeckclearrect(lua_State *L, int arg, VkClearRect *p)
    {
    CHECK_TABLE(L, arg, p);
    GetRect2dOpt(rect, "rect");
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
    return 0;
    }

/* echeckclearrectlist() */
ECHECKLISTFUNC(VkClearRect, clearrect, NULL)

int echeckimagesubresourcelayers(lua_State *L, int arg, VkImageSubresourceLayers *p)
    {
    CHECK_TABLE(L, arg, p);
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
    return 0;
    }

int echeckimagecopy(lua_State *L, int arg, VkImageCopy *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    GetInteger(srcOffset, "src_offset");
    GetInteger(dstOffset, "dst_offset");
    GetInteger(size, "size");
    return 0;
    }

/* echeckbuffercopylist() */
ECHECKLISTFUNC(VkBufferCopy, buffercopy, NULL)

int echeckmemorybarrier(lua_State *L, int arg, VkMemoryBarrier *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    return 0;
    }

/* echeckmemorybarrierlist() */
ECHECKLISTFUNC(VkMemoryBarrier, memorybarrier, NULL)

int echeckbuffermemorybarrier(lua_State *L, int arg, VkBufferMemoryBarrier *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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

/*------------------------------------------------------------------------------*/

static int echeckmappedmemoryrange(lua_State *L, int arg, VkMappedMemoryRange *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    GetDeviceMemory(memory, "memory");
    GetInteger(offset, "offset");
    GetIntegerOrWholeSize(size, "size");
    return 0;
    }

/* echeckmappedmemoryrangelist() */
ECHECKLISTFUNC(VkMappedMemoryRange, mappedmemoryrange, NULL)


/*------------------------------------------------------------------------------*/

static int echeckmemorydedicatedallocateinfo(lua_State *L, int arg, VkMemoryDedicatedAllocateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    GetImageOpt(image, "image");
    GetBufferOpt(buffer, "buffer");
    return 0;
    }


static int echeckexportmemoryallocateinfo(lua_State *L, int arg, VkExportMemoryAllocateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    GetFlags(handleTypes, "handle_types");
    return 0;
    }

static int echeckimportmemoryfdinfo(lua_State *L, int arg, VkImportMemoryFdInfoKHR *p)
    {
    p->sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
    GetInteger(fd, "fd");
    return 0;
    }

int echeckmemoryallocateinfo(lua_State *L, int arg, VkMemoryAllocateInfo_CHAIN *pp)
    {
    int err, arg1;
    VkMemoryAllocateInfo *p = &pp->p1;
    VkMemoryDedicatedAllocateInfo *p2 = &pp->p2;
    VkExportMemoryAllocateInfo *p3 = &pp->p3;
    VkImportMemoryFdInfoKHR *p4 = &pp->p4;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    GetInteger(allocationSize, "allocation_size");
    GetInteger(memoryTypeIndex, "memory_type_index");
    /*---------- extensions --------------*/
    if(ispresent("image") || ispresent("buffer"))
        {
        err = echeckmemorydedicatedallocateinfo(L, arg, p2);
        if(err) return err;
        addtochain(chain, p2);
        }
    if(ispresent("handle_types"))
        {
        err = echeckexportmemoryallocateinfo(L, arg, &pp->p3);
        if(err) return err;
        addtochain(chain, p3);
        }
#define F "import_memory_fd_info"
    arg1 = pushfield(L, arg, F);
    err = echeckimportmemoryfdinfo(L, arg1, p4);
    popfield(L, arg1);
    if(err<0) return prependfield(L, F);
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        addtochain(chain, p4);
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/


int pushmemoryfdproperties(lua_State *L, VkMemoryFdPropertiesKHR *p)
    {
    lua_newtable(L);
    SetInteger(memoryTypeBits, "memory_type_bits");
    return 1;
    }

int echeckmemorygetfdinfo(lua_State *L, int arg, VkMemoryGetFdInfoKHR *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    /* p->memory = set by caller */
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
    return 0;
    }

/*------------------------------------------------------------------------------*/

int echeckbuffermemoryrequirementsinfo2(lua_State *L, int arg, VkBufferMemoryRequirementsInfo2_CHAIN *pp)
    {
    VkBufferMemoryRequirementsInfo2 *p = &pp->p1;
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
    /* p->buffer = set by caller */
    return 0;
    }

static int echeckimageplanememoryrequirementsinfo(lua_State *L, int arg, VkImagePlaneMemoryRequirementsInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
    GetFlags(planeAspect, "plane_aspect");
    return 0;
    }

int echeckimagememoryrequirementsinfo2(lua_State *L, int arg, VkImageMemoryRequirementsInfo2_CHAIN *pp)
    {
    int err;
    VkImageMemoryRequirementsInfo2 *p = &pp->p1;
    VkImagePlaneMemoryRequirementsInfo *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    /* p->image = set by caller */
#define F "plane_aspect"
    if(ispresent(F))
        {
        err = echeckimageplanememoryrequirementsinfo(L, arg, &pp->p2);
        if(err) return err;
        addtochain(chain, p2);
        }
#undef F
    return 0;
    }


int echeckimagesparsememoryrequirementsinfo2(lua_State *L, int arg, VkImageSparseMemoryRequirementsInfo2_CHAIN *pp)
    {
    VkImageSparseMemoryRequirementsInfo2 *p = &pp->p1;
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2;
    /* p->image = set by caller */
    return 0;
    }

int pushmemoryrequirements(lua_State *L, VkMemoryRequirements *p)
    {
    lua_newtable(L);
    SetInteger(size, "size");
    SetInteger(alignment, "alignment");
    SetInteger(memoryTypeBits, "memory_type_bits");
    return 1;
    }

int pushmemorydedicatedrequirements(lua_State *L, VkMemoryDedicatedRequirements *p)
    {
    SetBoolean(prefersDedicatedAllocation, "prefers_dedicated_allocation");
    SetBoolean(requiresDedicatedAllocation, "requires_dedicated_allocation");
    return 1;
    }

void initmemoryrequirements2(lua_State *L, VkMemoryRequirements2_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    p->p2.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
    p->p1.pNext = &p->p2;
    p->p2.pNext = NULL;
    }

int pushmemoryrequirements2(lua_State *L, VkMemoryRequirements2_CHAIN *p)
    {
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

VkSparseImageMemoryRequirements2* newsparseimagememoryrequirements2(lua_State *L, uint32_t count)
    {
    uint32_t i;
    VkSparseImageMemoryRequirements2 *p = NMALLOC_NOERR(L, VkSparseImageMemoryRequirements2, count);
    if(!p) return NULL;
    for(i = 0; i < count; i++)
        {
        p[i].sType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2;
        }
    return p;
    }

void freesparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2 *p, uint32_t count)
    {
    (void)count;
    Free(L, (void*)p);
    }

int pushsparseimagememoryrequirements2(lua_State *L, VkSparseImageMemoryRequirements2 *p)
    {
    pushsparseimagememoryrequirements(L, &p->memoryRequirements);
    return 1;
    }

/*-------------------------------------------------------------------------------------*/
static int echecksparsememorybind(lua_State *L, int arg, VkSparseMemoryBind *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(arrayLayer, "array_layer");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/
static int echecksparseimagememorybind(lua_State *L, int arg, VkSparseImageMemoryBind *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);

    GetBuffer(buffer, "buffer");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = echecksparsememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    popfield(L, arg1);
    if(err) return prependfield(L, F);
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
    CHECK_TABLE(L, arg, p);

    GetImage(image, "image");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = echecksparsememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    popfield(L, arg1);
    if(err) return prependfield(L, F);
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
    CHECK_TABLE(L, arg, p);

    GetImage(image, "image");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = echecksparseimagememorybindlist(L, arg1, &count, &err);
    p->bindCount = count;
    popfield(L, arg1);
    if(err) return prependfield(L, F);
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    popfield(L, arg1);
    if(err < 0)
        return pushfielderror(L, F, err);
#undef F
#define F "wait_dst_stage_mask"
    arg1 = pushfield(L, arg, F);
    p->pWaitDstStageMask = checkflaglist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0)
        { freesubmitinfo(L, p); return pushfielderror(L, F, err); }
    if(count != p->waitSemaphoreCount)
        { freesubmitinfo(L, p); return pushfielderror(L, F, ERR_LENGTH); }
#undef F
#define F "command_buffers"
    arg1 = pushfield(L, arg, F);
    p->pCommandBuffers = checkcommand_bufferlist(L, arg1, &count, &err);
    p->commandBufferCount = count;
    popfield(L, arg1);
    if(err < 0)
        { freesubmitinfo(L, p); return pushfielderror(L, F, err); }
#undef F
#define F "signal_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->signalSemaphoreCount = count;
    popfield(L, arg1);
    if(err < 0)
        { freesubmitinfo(L, p); return pushfielderror(L, F, err); }
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;

#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    popfield(L, arg1);
    if(err < 0) { freebindsparseinfo(L, p); return pushfielderror(L, F, err); }
#undef F
#define F "buffer_binds"
    arg1 = pushfield(L, arg, F);
    p->pBufferBinds = echecksparsebuffermemorybindinfolist(L, arg1, &count, &err);
    p->bufferBindCount = count;
    popfield(L, arg1);
    if(err < 0) { freebindsparseinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "image_opaque_binds"
    arg1 = pushfield(L, arg, F);
    p->pImageOpaqueBinds = echecksparseimageopaquememorybindinfolist(L, arg1, &count, &err);
    p->imageOpaqueBindCount = count;
    popfield(L, arg1);
    if(err < 0) { freebindsparseinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "image_binds"
    arg1 = pushfield(L, arg, F);
    p->pImageBinds = echecksparseimagememorybindinfolist(L, arg1, &count, &err);
    p->imageBindCount = count;
    popfield(L, arg1);
    if(err < 0) { freebindsparseinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "signal_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->signalSemaphoreCount = count;
    popfield(L, arg1);
    if(err < 0) { freebindsparseinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
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

void initsurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR_CHAIN *p)
    {
    (void)L;
    MEMZERO(p);
    p->p1.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    p->p2.sType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;
    p->p1.pNext = &p->p2;
    p->p2.pNext = NULL;
    }

int pushsurfacecapabilities2(lua_State *L, VkSurfaceCapabilities2KHR_CHAIN *p)
    {
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
    CHECK_TABLE(L, arg, p);
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

    CHECK_TABLE(L, arg, p);
#define F "map_entries"
    arg1 = pushfield(L, arg, F);
    p->pMapEntries = echeckspecializationmapentrylist(L, arg1, &count, &err);
    p->mapEntryCount = count;
    popfield(L, arg1);
    if(err < 0) { freespecializationinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "data"
    arg1 = pushfield(L, arg, F);
    data = lua_tolstring(L, arg1, &size);
    if(!data || size == 0)
        { popfield(L, arg1); freespecializationinfo(L, p); return pushfielderror(L, F, ERR_LENGTH); }
    p->pData = MallocNoErr(L, size);
    if(!p->pData)
        { popfield(L, arg1); freespecializationinfo(L, p); return pushfielderror(L, F, ERR_MEMORY); }
    memcpy((void*)p->pData, data, size);
    p->dataSize = size;
    popfield(L, arg1);
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
    
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetShaderStageFlagBits(stage, "stage");
    GetShaderModule(module, "module");
    GetStringDef(pName, "name", "main");

#define F "specialization_info"
    arg1 = pushfield(L, arg, F);
    if(lua_isnoneornil(L, arg1))
        { 
        popfield(L, arg1);
        p->pSpecializationInfo = NULL; 
        return 0; 
        }

    specinfo = (VkSpecializationInfo*)MallocNoErr(L, sizeof(VkSpecializationInfo));
    if(!specinfo)
        { popfield(L, arg1); freepipelineshaderstagecreateinfo(L, p); return pushfielderror(L, F, ERR_MEMORY); }
    err = echeckspecializationinfo(L, arg1, specinfo);
    p->pSpecializationInfo = specinfo;
    popfield(L, arg1);
    if(err) { freepipelineshaderstagecreateinfo(L, p); return prependfield(L, F); }
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetTopology(topology, "topology");
    GetBoolean(primitiveRestartEnable, "primitive_restart_enable");
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckpipelinetessellationdomainoriginstatecreateinfo(lua_State *L, int arg, VkPipelineTessellationDomainOriginStateCreateInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
    GetTessellationDomainOrigin(domainOrigin, "domain_origin");
    return 0;
    }

static void freepipelinetessellationstatecreateinfo(lua_State *L, VkPipelineTessellationStateCreateInfo *p)
    {
    if(p->pNext) Free(L, (void*)p->pNext);
    }

static int echeckpipelinetessellationstatecreateinfo(lua_State *L, int arg, VkPipelineTessellationStateCreateInfo *p)
    {
    int err;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetInteger(patchControlPoints, "patch_control_points");
#define F   "domain_origin"
    if(ispresent(F))
        {
        p->pNext = MALLOC_NOERR(L, VkPipelineTessellationDomainOriginStateCreateInfo);
        if(!p->pNext)
            { freepipelinetessellationstatecreateinfo(L, p); return pusherror(L, ERR_MEMORY); }
        err = echeckpipelinetessellationdomainoriginstatecreateinfo(L, arg,
                    (VkPipelineTessellationDomainOriginStateCreateInfo*)(p->pNext));
        if(err)
            { freepipelinetessellationstatecreateinfo(L, p); return err; /* field and error already pushed */ }
        }
#undef F
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

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetIntegerOpt(viewportCount, "viewport_count", 1);
    GetIntegerOpt(scissorCount, "scissor_count", 1);
    /* scissorCount and viewportCount must be identical, and they may be > 0 
     * even if scissors and/or viewports lists are not given
     */
#define F "viewports"
    arg1 = pushfield(L, arg, F);
    p->pViewports = echeckviewportlist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0) { freepipelineviewportstatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        p->viewportCount = count;
/*
    else if(count != p->viewportCount)
        { freepipelineviewportstatecreateinfo(L, p); return pushfielderror(L, F, ERR_LENGTH); }
*/
#undef F
#define F "scissors"
    arg1 = pushfield(L, arg, F);
    p->pScissors = echeckrect2dlist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0) { freepipelineviewportstatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        p->scissorCount = count;
/*
    else if(count != p->scissorCount)
        { freepipelineviewportstatecreateinfo(L, p); return pushfielderror(L, F, ERR_LENGTH); }
*/
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/


#define freepipelinerasterizationstatecreateinfo(L, p) do { } while(0)
static int echeckpipelinerasterizationstatecreateinfo(lua_State *L, int arg, VkPipelineRasterizationStateCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
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

static void freepipelinesamplelocationsstatecreateinfo(lua_State *L, VkPipelineSampleLocationsStateCreateInfoEXT *p)
    {
    if(!p) return;
    freesamplelocationsinfo(L, &p->sampleLocationsInfo);
    Free(L, (void*)p);
    }

static int echeckpipelinesamplelocationsstatecreateinfo(lua_State *L, int arg, VkPipelineSampleLocationsStateCreateInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT;
    GetBoolean(sampleLocationsEnable, "sample_locations_enable");
    GetSampleLocationsInfo(sampleLocationsInfo, "sample_locations_info");
    return 0;
    }

static void freepipelinemultisamplestatecreateinfo(lua_State *L, VkPipelineMultisampleStateCreateInfo *p)
    {
    if(!p) return;
    if(p->pSampleMask) Free(L, (void*)p->pSampleMask);
    if(p->pNext)
        freepipelinesamplelocationsstatecreateinfo(L, (VkPipelineSampleLocationsStateCreateInfoEXT*)p->pNext);
    }

static int echeckpipelinemultisamplestatecreateinfo(lua_State *L, int arg, VkPipelineMultisampleStateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetSamples(rasterizationSamples, "rasterization_samples");
    GetBoolean(sampleShadingEnable, "sample_shading_enable");
    GetNumber(minSampleShading, "min_sample_shading");
    GetBoolean(alphaToCoverageEnable, "alpha_to_coverage_enable");
    GetBoolean(alphaToOneEnable, "alpha_to_one_enable");

#define F "sample_mask"
    arg1 = pushfield(L, arg, F);
    p->pSampleMask = (VkSampleMask*)checkuint32list(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0)
        { freepipelinemultisamplestatecreateinfo(L, p); return pushfielderror(L, F, err); }
    if((count > 0) && (count != p->rasterizationSamples / 32))
        { freepipelinemultisamplestatecreateinfo(L, p); return pushfielderror(L, F, ERR_LENGTH); }
#undef F
#define F   "sample_locations_info"
    if(ispresent(F))
        {
        p->pNext = MALLOC_NOERR(L, VkPipelineSampleLocationsStateCreateInfoEXT);
        if(!p->pNext)
            { freepipelinemultisamplestatecreateinfo(L, p); return pusherror(L, ERR_MEMORY); }
        err = echeckpipelinesamplelocationsstatecreateinfo(L, arg,
                    (VkPipelineSampleLocationsStateCreateInfoEXT*)(p->pNext));
        if(err)
            { freepipelinemultisamplestatecreateinfo(L, p); return err; }
        }
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckvertexinputbindingdescription(lua_State *L, int arg, VkVertexInputBindingDescription *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(binding, "binding");
    GetInteger(stride, "stride");
    GetVertexInputRate(inputRate, "input_rate");
    return 0;
    }

/* echeckvertexinputbindingdescriptionlist() */
static ECHECKLISTFUNC(VkVertexInputBindingDescription, vertexinputbindingdescription, NULL)

static int echeckvertexinputattributedescription(lua_State *L, int arg, VkVertexInputAttributeDescription *p)
    {
    CHECK_TABLE(L, arg, p);
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

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
#define F "vertex_binding_descriptions"
    arg1 = pushfield(L, arg, F);
    p->pVertexBindingDescriptions = echeckvertexinputbindingdescriptionlist(L, arg1, &count, &err);
    p->vertexBindingDescriptionCount = count;
    popfield(L, arg1);
    if(err < 0) { freepipelinevertexinputstatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "vertex_attribute_descriptions"
    arg1 = pushfield(L, arg, F);
    p->pVertexAttributeDescriptions = echeckvertexinputattributedescriptionlist(L, arg1, &count, &err);
    p->vertexAttributeDescriptionCount = count;
    popfield(L, arg1);
    if(err < 0) { freepipelinevertexinputstatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/
static int echeckstencilopstate(lua_State *L, int arg, VkStencilOpState *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
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
    CHECK_TABLE(L, arg, p);
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

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetBoolean(logicOpEnable, "logic_op_enable");
    GetLogicOp(logicOp, "logic_op");
#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = echeckpipelinecolorblendattachmentstatelist(L, arg1, &count, &err);
    p->attachmentCount = count;
    popfield(L, arg1);
    if(err < 0) { freepipelinecolorblendstatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
#define F "blend_constants"
    arg1 = pushfield(L, arg, F);
    if(lua_isnoneornil(L, arg1))
        { popfield(L, arg1); return 0; }
    if(lua_type(L, arg1) != LUA_TTABLE)
        { popfield(L, arg1); freepipelinecolorblendstatecreateinfo(L, p); return pushfielderror(L, F, ERR_TABLE); }
    for(i = 0; i < 4; i++)
        { 
        lua_rawgeti(L, arg1, i+1);
        p->blendConstants[i] = lua_tonumberx(L, -1, &isnum);
        lua_pop(L, 1);
        if(!isnum)
            { 
            popfield(L, arg1);
            freepipelinecolorblendstatecreateinfo(L, p); 
            return pushfielderror(L, F, ERR_TYPE);
            }
        }
    popfield(L, arg1);
#undef F
    return 0;
    }

#define freepipelinecolorblendadvancedstatecreateinfo(L, p) do { } while(0)
static int echeckpipelinecolorblendadvancedstatecreateinfo(lua_State *L, int arg, VkPipelineColorBlendAdvancedStateCreateInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT ;
    GetBoolean(srcPremultiplied, "src_premultiplied");
    GetBoolean(dstPremultiplied, "dst_premultiplied");
    GetBlendOverlap(blendOverlap, "blend_overlap");
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

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    GetFlags(flags, "flags");
    arg1 = pushfield(L, arg, "dynamic_states");
    p->pDynamicStates = checkdynamicstatelist(L, arg1, &count, &err);
    p->dynamicStateCount = count;
    popfield(L, arg1);
    if(err)
        { lua_pushstring(L, "dynamic_states"); return ERR_GENERIC; }
 
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static void freediscardrectanglestatecreateinfo(lua_State *L, VkPipelineDiscardRectangleStateCreateInfoEXT *p)
    {
    if(!p) return;
    if(p->pDiscardRectangles) Free(L, (void*)p->pDiscardRectangles);
    }

static int echeckdiscardrectanglestatecreateinfo(lua_State *L, int arg, VkPipelineDiscardRectangleStateCreateInfoEXT *p)
    {
    int err, arg1;
    uint32_t count;

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT;
    GetFlags(flags, "flags");
    GetDiscardRectangleMode(discardRectangleMode, "discard_rectangle_mode");
#define F "discard_rectangles"
    arg1 = pushfield(L, arg, F);
    p->pDiscardRectangles = echeckrect2dlist(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err < 0) { freediscardrectanglestatecreateinfo(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        p->discardRectangleCount = count;
#undef F
    return 0;
    }


/*-------------------------------------------------------------------------------------*/

static void freegraphicspipelinecreateinfo(lua_State *L, VkGraphicsPipelineCreateInfo *p)
    {
    VkPipelineColorBlendAdvancedStateCreateInfoEXT *ColorBlendAdvancedState;
    VkPipelineDiscardRectangleStateCreateInfoEXT *DiscardRectangleState;
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
        if(p->pNext)
            {
            ColorBlendAdvancedState = (VkPipelineColorBlendAdvancedStateCreateInfoEXT*)(p->pNext);
            freepipelinecolorblendadvancedstatecreateinfo(L, ColorBlendAdvancedState);
            Free(L, (void*)ColorBlendAdvancedState);
            }
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
    /* extensions in the main chain */
    if(p->pNext)
        {
        DiscardRectangleState = (VkPipelineDiscardRectangleStateCreateInfoEXT*)p->pNext;
        freediscardrectanglestatecreateinfo(L, DiscardRectangleState);
        }
    }

/* Need these to trick the BEGIN macro: */
#define VkPipelineColorBlendAdvancedStateCreateInfo VkPipelineColorBlendAdvancedStateCreateInfoEXT
#define VkPipelineDiscardRectangleStateCreateInfo VkPipelineDiscardRectangleStateCreateInfoEXT

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
    VkPipelineColorBlendAdvancedStateCreateInfoEXT *ColorBlendAdvancedState;
    VkPipelineDynamicStateCreateInfo           *DynamicState;
    VkPipelineDiscardRectangleStateCreateInfoEXT *DiscardRectangleState;

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetPipelineLayout(layout, "layout");
    GetRenderPass(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetPipelineOpt(basePipelineHandle, "base_pipeline_handle");
    GetIntegerOpt(basePipelineIndex, "base_pipeline_index", -1);

#define F "stages"
    arg1 = pushfield(L, arg, F);
    p->pStages = echeckpipelineshaderstagecreateinfolist(L, arg1, &p->stageCount, &err);
    popfield(L, arg1);
    if(err) return prependfield(L, F);
#undef F

#define BEGIN(xxx) do {                                                                 \
    xxx = (VkPipeline##xxx##CreateInfo*)Malloc(L, sizeof(VkPipeline##xxx##CreateInfo)); \
    if(!xxx) { freegraphicspipelinecreateinfo(L, p); return pusherror(L, ERR_MEMORY); } \
    arg1 = pushfield(L, arg, F);                                                        \
} while (0)

#define END_MANDATORY(xxx)  do {                                                    \
    popfield(L, arg1);                                                                     \
    if(err) { freegraphicspipelinecreateinfo(L, p); return prependfield(L, F); }     \
} while(0)

#define END_OPTIONAL(xxx)   do {                                                    \
    popfield(L, arg1);                                                                     \
    if(err < 0) { freegraphicspipelinecreateinfo(L, p); return prependfield(L, F); } \
    if(err == ERR_NOTPRESENT) { poperror(); xxx = NULL; }                           \
} while(0)

#define F "vertex_input_state"
    BEGIN(VertexInputState);
    err = echeckpipelinevertexinputstatecreateinfo(L, arg1, VertexInputState);
    END_MANDATORY(VertexInputState);
    p->pVertexInputState = VertexInputState;
#undef F
#define F "input_assembly_state"
    BEGIN(InputAssemblyState);
    err = echeckpipelineinputassemblystatecreateinfo(L, arg1, InputAssemblyState);
    END_MANDATORY(InputAssemblyState);
    p->pInputAssemblyState = InputAssemblyState;
#undef F
#define F "tessellation_state"
    BEGIN(TessellationState);
    err = echeckpipelinetessellationstatecreateinfo(L, arg1, TessellationState);
    END_OPTIONAL(TessellationState);
    p->pTessellationState = TessellationState;
#undef F
#define F "viewport_state"
    BEGIN(ViewportState);
    err = echeckpipelineviewportstatecreateinfo(L, arg1, ViewportState);
    END_OPTIONAL(ViewportState);
    p->pViewportState = ViewportState;
#undef F
#define F "rasterization_state"
    BEGIN(RasterizationState);
    err = echeckpipelinerasterizationstatecreateinfo(L, arg1, RasterizationState);
    END_MANDATORY(RasterizationState);
    p->pRasterizationState = RasterizationState;
#undef F
#define F "multisample_state"
    BEGIN(MultisampleState);
    err = echeckpipelinemultisamplestatecreateinfo(L, arg1, MultisampleState);
    END_OPTIONAL(MultisampleState);
    p->pMultisampleState = MultisampleState;
#undef F
#define F "depth_stencil_state"
    BEGIN(DepthStencilState);
    err = echeckpipelinedepthstencilstatecreateinfo(L, arg1, DepthStencilState);
    END_OPTIONAL(DepthStencilState);
    p->pDepthStencilState = DepthStencilState;
#undef F
#define F "color_blend_state"
    BEGIN(ColorBlendState);
    err = echeckpipelinecolorblendstatecreateinfo(L, arg1, ColorBlendState);
    END_OPTIONAL(ColorBlendState);
    p->pColorBlendState = ColorBlendState;
#undef F
#define F "color_blend_advanced_state"
    if(ColorBlendState)
        {
        BEGIN(ColorBlendAdvancedState);
        err = echeckpipelinecolorblendadvancedstatecreateinfo(L, arg1, ColorBlendAdvancedState);
        END_OPTIONAL(ColorBlendAdvancedState);
        ColorBlendState->pNext = (const void*)ColorBlendAdvancedState;
        }
#undef F
#define F "dynamic_state"
    BEGIN(DynamicState);
    err = echeckpipelinedynamicstatecreateinfo(L, arg1, DynamicState);
    END_OPTIONAL(DynamicState);
    p->pDynamicState = DynamicState;
#undef F
#define F "discard_rectangle_state"
    BEGIN(DiscardRectangleState);
    err = echeckdiscardrectanglestatecreateinfo(L, arg1, DiscardRectangleState);
    END_OPTIONAL(DiscardRectangleState);
    if(!err) p->pNext = DiscardRectangleState;
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
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

void freeswapchaincreateinfo(lua_State *L, VkSwapchainCreateInfoKHR_CHAIN *pp)
    {
    VkSwapchainCreateInfoKHR *p = &pp->p1;
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
    }

static int echeckswapchaincountercreateinfo(lua_State *L, int arg, VkSwapchainCounterCreateInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT;
    GetFlags(surfaceCounters, "surface_counters");
    return 0;
    }

int echeckswapchaincreateinfo(lua_State *L, int arg, VkSwapchainCreateInfoKHR_CHAIN *pp)
    {
    int err, arg1;
    VkSwapchainCreateInfoKHR *p = &pp->p1;
    VkSwapchainCounterCreateInfoEXT *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
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
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, &err);
    popfield(L, arg1);
    if(err < 0) return pushfielderror(L, F, err);
#undef F
    /*-- extensions --------------------*/
    if(ispresent("surface_counters"))
        {
        err = echeckswapchaincountercreateinfo(L, arg, p2);
        if(err) { freeswapchaincreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    return 0;
    }

/* echeckswapchaincreateinfolist() */
static FREELISTFUNC(VkSwapchainCreateInfoKHR_CHAIN, swapchaincreateinfo)
static ECHECKLISTFUNC(VkSwapchainCreateInfoKHR_CHAIN, swapchaincreateinfo, freeswapchaincreateinfolist)

void freeswapchaincreateinfoarray(lua_State *L, VkSwapchainCreateInfoKHR_ARRAY *pp, uint32_t count)
    {
    Free(L, pp->p);
    freeswapchaincreateinfolist(L, pp->chain, count);
    MEMZERO(pp);
    }

int echeckswapchaincreateinfoarray(lua_State *L, int arg, VkSwapchainCreateInfoKHR_ARRAY *pp, uint32_t *count)
/* Constructs an array of contiguous p1 elements from the list. */
    {
    int err;
    uint32_t i;
    pp->chain = echeckswapchaincreateinfolist(L, arg, count, &err);
    pp->p = MALLOC_NOERR(L, VkSwapchainCreateInfoKHR);
    if(!pp->p)
        {
        freeswapchaincreateinfolist(L, pp->chain, *count);
        *count = 0;
        return pusherror(L, ERR_MEMORY);
        }

    for(i=0; i < *count; i++)
        memcpy(&pp->p[i], &(pp->chain[i].p1), sizeof(VkSwapchainCreateInfoKHR));

    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckrectlayer(lua_State *L, int arg, VkRectLayerKHR *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
#define F "rectangles"
    arg1 = pushfield(L, arg, F);
    p->pRectangles = echeckrectlayerlist(L, arg1, &count, &err);
    p->rectangleCount = count;
    popfield(L, arg1);
    if(err < 0) { freepresentregion(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }
/* echeckpresentregionlist() */
static FREELISTFUNC(VkPresentRegionKHR, presentregion)
static ECHECKLISTFUNC(VkPresentRegionKHR, presentregion, freepresentregionlist)

static void freepresentregions(lua_State *L, VkPresentRegionsKHR *p)
    {
    if(!p) return;
    if(p->pRegions)
        freepresentregionlist(L, (void*)p->pRegions, p->swapchainCount);
    }

static int echeckpresentregions(lua_State *L, int arg, VkPresentRegionsKHR *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
#define F "regions"
    arg1 = pushfield(L, arg, F);
    p->pRegions = echeckpresentregionlist(L, arg1, &count, &err);
    p->swapchainCount = count;
    popfield(L, arg1);
    if(err < 0) { freepresentregions(L, p); return prependfield(L, F); }
    if(err == ERR_NOTPRESENT) poperror();
#undef F
    return 0;
    }

void freepresentinfo(lua_State *L, VkPresentInfoKHR_CHAIN *p)
    {
    if(p->p1.pWaitSemaphores) Free(L, (void*)p->p1.pWaitSemaphores);
    if(p->p1.pSwapchains) Free(L, (void*)p->p1.pSwapchains);
    if(p->p1.pImageIndices) Free(L, (void*)p->p1.pImageIndices);
    if(p->p1.pResults) Free(L, (void*)p->p1.pResults);
    freepresentregions(L, &p->p3);
    }

static int echeckdisplaypresentinfo(lua_State *L, int arg, VkDisplayPresentInfoKHR *p)
    {
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR;
    GetRect2dOpt(srcRect, "src_rect");
    GetRect2dOpt(dstRect, "dst_rect");
    GetBoolean(persistent, "persistent");
    return 0;
    }

int echeckpresentinfo(lua_State *L, int arg, VkPresentInfoKHR_CHAIN *pp, int results)
    {
    int err, arg1;
    uint32_t count;
    VkPresentInfoKHR *p = &pp->p1;
    VkDisplayPresentInfoKHR *p2 = &pp->p2;
    VkPresentRegionsKHR *p3 = &pp->p3;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &count, &err, NULL);
    p->waitSemaphoreCount = count;
    popfield(L, arg1);
    if(err < 0)
        return pushfielderror(L, F, err);
#undef F
#define F "swapchains"
    arg1 = pushfield(L, arg, F);
    p->pSwapchains = checkswapchainlist(L, arg1, &count, &err, NULL);
    p->swapchainCount = count;
    popfield(L, arg1);
    if(err)
        { freepresentinfo(L, pp); return pushfielderror(L, F, err); }
    if(results) /* allocate memory for per-swapchain results */
        {
        p->pResults = (VkResult*)MallocNoErr(L, sizeof(VkResult)*(p->swapchainCount));
        if(!p->pResults)
            { freepresentinfo(L, pp); return pusherror(L, ERR_MEMORY); }
        }
#undef F
#define F "image_indices"
    arg1 = pushfield(L, arg, F);
    p->pImageIndices = checkuint32list(L, arg1, &count, &err);
    popfield(L, arg1);
    if(err) { freepresentinfo(L, pp); return pushfielderror(L, F, err); }
    if(p->swapchainCount != count)
        { freepresentinfo(L, pp); return pushfielderror(L, F, ERR_LENGTH); }
#undef F
    /*-- extensions --------------------*/
    if(ispresent("src_rect"))
        {
        err = echeckdisplaypresentinfo(L, arg, p2);
        if(err) { freepresentinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    if(ispresent("regions"))
        {
        err = echeckpresentregions(L, arg, p3);
        if(err) { freepresentinfo(L, pp); return err; }
        addtochain(chain, p3);
        }
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

static int echeckdescriptorimageinfo(lua_State *L, int arg, VkDescriptorImageInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    GetSampler(sampler, "sampler");
    GetImageView(imageView, "image_view");
    GetImageLayout(imageLayout, "image_layout");
    return 0;
    }

/* echeckdescriptorimageinfolist() */
static ECHECKLISTFUNC(VkDescriptorImageInfo, descriptorimageinfo, NULL)

static int echeckdescriptorbufferinfo(lua_State *L, int arg, VkDescriptorBufferInfo *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
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
            arg1 = pushfield(L, arg, F);
            p->pImageInfo = echeckdescriptorimageinfolist(L, arg1, &count, &err);
            popfield(L, arg1);
            if(err) { freewritedescriptorset(L, p); return prependfield(L, F); }
            p->descriptorCount = count;
            return 0;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
#define F "buffer_info"
            arg1 = pushfield(L, arg, F);
            p->pBufferInfo = echeckdescriptorbufferinfolist(L, arg1, &count, &err);
            popfield(L, arg1);
            if(err) { freewritedescriptorset(L, p); return prependfield(L, F); }
            p->descriptorCount = count;
            return 0;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
#define F "texel_buffer_view"
            arg1 = pushfield(L, arg, F);
            p->pTexelBufferView = checkbuffer_viewlist(L, arg1, &count, &err, NULL);
            popfield(L, arg1);
            if(err) { freewritedescriptorset(L, p); return pushfielderror(L, F, err); }
            if(err == ERR_NOTPRESENT) poperror();
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
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

int echeckdisplaypowerinfo(lua_State *L, int arg, VkDisplayPowerInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT;
    GetDisplayPowerState(powerState, "power_state");
    return 0;
    }

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

int pushmultisampleproperties(lua_State *L, VkMultisamplePropertiesEXT *p)
    {
    lua_newtable(L);
    SetStruct(maxSampleLocationGridSize, "max_sample_location_grid_size", pushextent2d);
    return 1;
    }

/*-------------------------------------------------------------------------------------*/

void freesamplelocationsinfo(lua_State *L, VkSampleLocationsInfoEXT *p)
    {
    if(p->pSampleLocations)
        Free(L, (void*)p->pSampleLocations);
    }

int echecksamplelocationsinfo(lua_State *L, int arg, VkSampleLocationsInfoEXT *p)
    {
    int err, arg1;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
    GetBits(sampleLocationsPerPixel, "sample_locations_per_pixel", VkSampleCountFlagBits);
    GetExtent2d(sampleLocationGridSize, "sample_location_grid_size");
#define F "sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pSampleLocations = echecksamplelocationlist(L, arg1, &p->sampleLocationsCount, &err);
    popfield(L, arg1);
    if(err) { freesamplelocationsinfo(L, p); return prependfield(L, F); }
#undef F
    return 0;
    }

/*-------------------------------------------------------------------------------------*/

int echeckdisplaymodeparameters(lua_State *L, int arg, VkDisplayModeParametersKHR *p)
    {
    CHECK_TABLE(L, arg, p);
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
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR ;
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
 
/*-------------------------------------------------------------------------------------*/

static void freedescriptorupdatetemplateentry(lua_State *L, VkDescriptorUpdateTemplateEntry *p)
    {
    (void)L; (void)p;
    }

static int echeckdescriptorupdatetemplateentry(lua_State *L, int arg, VkDescriptorUpdateTemplateEntry *p)
    {
    CHECK_TABLE(L, arg, p);
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetInteger(descriptorCount, "descriptor_count");
    GetDescriptorType(descriptorType, "descriptor_type");
    GetInteger(offset, "offset");
    GetInteger(stride, "stride");
    return 0;
    }

/* echeckdescriptorupdatetemplateentrylist() */
static FREELISTFUNC(VkDescriptorUpdateTemplateEntry, descriptorupdatetemplateentry)
static ECHECKLISTFUNC(VkDescriptorUpdateTemplateEntry, descriptorupdatetemplateentry, freedescriptorupdatetemplateentrylist)


void freedescriptorupdatetemplatecreateinfo(lua_State *L, VkDescriptorUpdateTemplateCreateInfo *p)
    {
    if(!p) return;
    if(p->pDescriptorUpdateEntries)
        freedescriptorupdatetemplateentrylist(L, (VkDescriptorUpdateTemplateEntry*)p->pDescriptorUpdateEntries, p->descriptorUpdateEntryCount);
    }

int echeckdescriptorupdatetemplatecreateinfo(lua_State *L, int arg, VkDescriptorUpdateTemplateCreateInfo *p)
    {
    int err, arg1;
    uint32_t count;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetDescriptorUpdateTemplateType(templateType, "template_type");
    GetDescriptorSetLayoutOpt(descriptorSetLayout, "descriptor_set_layout");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
    GetPipelineLayoutOpt(pipelineLayout, "pipeline_layout");
    GetFlags(set, "set");
#define F "descriptor_update_entries"
    arg1 = pushfield(L, arg, F);
    p->pDescriptorUpdateEntries = echeckdescriptorupdatetemplateentrylist(L, arg1, &count, &err);
    p->descriptorUpdateEntryCount = count;
    popfield(L, arg1);
    if(err) { freedescriptorupdatetemplatecreateinfo(L, p); return prependfield(L, F); }
#undef F
    return 0;
    }

/*------------------------------------------------------------------------------*/

static int echeckdevicequeueglobalprioritycreateinfo(lua_State *L, int arg, VkDeviceQueueGlobalPriorityCreateInfoEXT *p)
    {
    p->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT;
    GetQueueGlobalPriority(globalPriority, "global_priority");
    return 0;
    }

static void freedevicequeuecreateinfo(lua_State *L, VkDeviceQueueCreateInfo *p)
    {
    if(p->pQueuePriorities)
        Free(L, (void*)(p->pQueuePriorities));
    if(p->pNext) /* VkDeviceQueueGlobalPriorityCreateInfoEXT */
        Free(L, (void*)p->pNext);
    }

static int echeckdevicequeuecreateinfo(lua_State *L, int arg, VkDeviceQueueCreateInfo *p)
    {
    int arg1, err;
    uint32_t count;
    p->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
#define F   "queue_priorities"
    arg1 = pushfield(L, arg, F);
    p->pQueuePriorities = (const float*)checkfloatlist(L, arg1, &count, &err);
    p->queueCount = count;
    popfield(L, arg1);
    if(err) return pushfielderror(L, F, err);
#undef F
#define F   "global_priority"
    if(ispresent(F))
        {
        p->pNext = MALLOC_NOERR(L, VkDeviceQueueGlobalPriorityCreateInfoEXT);
        if(!p->pNext)
            { freedevicequeuecreateinfo(L, p); return pusherror(L, ERR_MEMORY); }
        err = echeckdevicequeueglobalprioritycreateinfo(L, arg,
                    (VkDeviceQueueGlobalPriorityCreateInfoEXT*)(p->pNext));
        if(err)
            { freedevicequeuecreateinfo(L, p); return err; /* field and error already pushed */ }
        }
#undef F
    return 0;
    }

/* echeckdevicequeuecreateinfolist() */
static FREELISTFUNC(VkDeviceQueueCreateInfo, devicequeuecreateinfo)
static ECHECKLISTFUNC(VkDeviceQueueCreateInfo, devicequeuecreateinfo, freedevicequeuecreateinfolist)

void freedevicecreateinfo(lua_State *L, VkDeviceCreateInfo_CHAIN *pp)
    {
    VkDeviceCreateInfo *p = &pp->p1;
    if(p->pQueueCreateInfos)
        freedevicequeuecreateinfolist(L, (void*)p->pQueueCreateInfos,  p->queueCreateInfoCount);
    if(p->ppEnabledLayerNames)
        freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames)
        freestringlist(L,  (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
    //freephysicaldevicefeatures(L, &pp->p2a);
    //freephysicaldevicefeatures2(L, &pp->p2b);
    }

int echeckdevicecreateinfo(lua_State *L, int arg, VkDeviceCreateInfo_CHAIN *pp, ud_t *ud)
    {
    int arg1, err;
    uint32_t count;
    VkDeviceCreateInfo *p = &pp->p1;

    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    GetFlags(flags, "flags");

#define F "queue_create_infos"
    arg1 = pushfield(L, arg, F);
    p->pQueueCreateInfos = echeckdevicequeuecreateinfolist(L, arg1, &count, &err);
    p->queueCreateInfoCount = count;
    popfield(L, arg1);
    if(err) return prependfield(L, F);
#undef F
#define F "enabled_layer_names" /* deprecated: aggiungere comunque */
    arg1 = pushfield(L, arg, F);
    p->ppEnabledLayerNames = (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, &err);
    popfield(L, arg1);
    if(err < 0 && err != ERR_EMPTY)
        { freedevicecreateinfo(L, pp); return pushfielderror(L, F, err); }
#undef F
#define F "enabled_extension_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledExtensionNames = (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, &err);
    popfield(L, arg1);
    if(err < 0 && err != ERR_EMPTY)
        { freedevicecreateinfo(L, pp); return pushfielderror(L, F, err); }
#undef F

    if(!ud->idt->GetPhysicalDeviceFeatures2KHR)
        {
#define F "enabled_features"
        arg1 = pushfield(L, arg, F);
        err = echeckphysicaldevicefeatures(L, arg1, &pp->p2a);
        popfield(L, arg1);
        if(err < 0)
            { freedevicecreateinfo(L, pp); return prependfield(L, F); }
        if(err == ERR_NOTPRESENT)
            poperror();
        else
            p->pEnabledFeatures = &pp->p2a;
#undef F
        }
    else
        {
#define F "enabled_features"
        p->pEnabledFeatures = NULL;
        arg1 = pushfield(L, arg, F);
        err = echeckphysicaldevicefeatures2(L, arg1, &pp->p2b);
        popfield(L, arg1);
        if(err < 0)
            { freedevicecreateinfo(L, pp); return prependfield(L, F); }
        if(err == ERR_NOTPRESENT)
            poperror();
        else
            p->pNext = &pp->p2b;
#undef F
        }
    return 0;
    }

/*------------------------------------------------------------------------------*/

static void freeapplicationinfo(lua_State *L, VkApplicationInfo *p)
    {
    if(p->pApplicationName) Free(L, (char*)p->pApplicationName);
    if(p->pEngineName) Free(L, (char*)p->pEngineName);
    }

static int echeckapplicationinfo(lua_State *L, int arg, VkApplicationInfo *p)
/* freeapplicationinfo() must be called in any case, even on error,
 * except for err = ERR_NOTPRESENT.
 */
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    GetStringOpt(pApplicationName, "application_name");
    GetInteger(applicationVersion, "application_version");
    GetStringOpt(pEngineName, "engine_name");
    GetInteger(engineVersion, "engine_version");
    GetInteger(apiVersion, "api_version");
    return 0;
    }

void freeinstancecreateinfo(lua_State *L, VkInstanceCreateInfo_CHAIN *pp)
    {
    VkInstanceCreateInfo *p = &pp->p1;
    VkValidationFlagsEXT *p2 = &pp->p2;
    freeapplicationinfo(L, &pp->appinfo);
    if(p->ppEnabledLayerNames)
        freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames)
        freestringlist(L, (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
    if(p2->pDisabledValidationChecks)
        freevalidationchecklist(L, p2->pDisabledValidationChecks);
    }

static int echeckvalidationflags(lua_State *L, int arg, VkValidationFlagsEXT *p)
    {
    int err, arg1;
    p->sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
#define F "disabled_validation_checks"
    arg1 = pushfield(L, arg, F);
    p->pDisabledValidationChecks = checkvalidationchecklist(L, arg1, &p->disabledValidationCheckCount, &err);
    popfield(L, arg1);
    if(err<0) return pushfielderror(L, F, err);
#undef F
    return 0;
    }

int echeckinstancecreateinfo(lua_State *L, int arg, VkInstanceCreateInfo_CHAIN *pp)
    {
    int arg1, err;
    VkInstanceCreateInfo *p = &pp->p1;
    VkApplicationInfo *appinfo = &pp->appinfo;
    VkValidationFlagsEXT *p2 = &pp->p2;
    const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    GetFlags(flags, "flags");

#define F "application_info"
    arg1 = pushfield(L, arg, F);
    err = echeckapplicationinfo(L, arg1, appinfo);
    popfield(L, arg1);
    if(err < 0) return prependfield(L, F);
    if(err == ERR_NOTPRESENT)
        poperror();
    else
        p->pApplicationInfo = appinfo;
#undef F
#define F "enabled_layer_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledLayerNames = (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, &err);
    popfield(L, arg1);
    if(err < 0 && err != ERR_EMPTY)
        { freeinstancecreateinfo(L, pp); return pushfielderror(L, F, err); }
#undef F
#define F "enabled_extension_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledExtensionNames = (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, &err);
    popfield(L, arg1);
    if(err < 0 && err != ERR_EMPTY)
        { freeinstancecreateinfo(L, pp); return pushfielderror(L, F, err); }
#undef F
    /* -- extensions ------ */
    if(ispresent("disabled_validation_checks"))
        {
        err = echeckvalidationflags(L, arg, p2);
        if(err) { freeinstancecreateinfo(L, pp); return err; }
        addtochain(chain, p2);
        }
    return 0;
    }

/*------------------------------------------------------------------------------*/

static void freebindbuffermemoryinfo(lua_State *L, VkBindBufferMemoryInfo *p)
    {
    (void)L; (void)p;
    }

static int echeckbindbuffermemoryinfo(lua_State *L, int arg, VkBindBufferMemoryInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
    GetBuffer(buffer, "buffer");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "offset");
    return 0;
    }

/* echeckbindbuffermemoryinfolist() */
FREELISTFUNC(VkBindBufferMemoryInfo, bindbuffermemoryinfo)
ECHECKLISTFUNC(VkBindBufferMemoryInfo, bindbuffermemoryinfo, freebindbuffermemoryinfolist)

static int echeckbindimageplanememoryinfo(lua_State *L, int arg, VkBindImagePlaneMemoryInfo *p)
    {
    p->sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
    GetFlags(planeAspect, "plane_aspect");
    return 0;
    }

static void freebindimagememoryinfo(lua_State *L, VkBindImageMemoryInfo *p)
    {
    VkBindImagePlaneMemoryInfo *p2 = (VkBindImagePlaneMemoryInfo*)p->pNext;
    if(p2) Free(L, (void*)p2);
    }

static int echeckbindimagememoryinfo(lua_State *L, int arg, VkBindImageMemoryInfo *p)
    {
    int err;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    GetImage(image, "image");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "offset");
#define F "plane_aspect"
    if(ispresent(F))
        {
        p->pNext = MALLOC_NOERR(L, VkBindImagePlaneMemoryInfo);
        if(!p->pNext) { freebindimagememoryinfo(L, p); return pusherror(L, ERR_MEMORY); }
        err = echeckbindimageplanememoryinfo(L, arg, (VkBindImagePlaneMemoryInfo*)(p->pNext));
        if(err) { freebindimagememoryinfo(L, p); return err; }
        }
#undef F
    return 0;
    }

/* echeckbindimagememoryinfolist() */
FREELISTFUNC(VkBindImageMemoryInfo, bindimagememoryinfo)
ECHECKLISTFUNC(VkBindImageMemoryInfo, bindimagememoryinfo, freebindimagememoryinfolist)

/*------------------------------------------------------------------------------*/

int echecksamplerycbcrconversioncreateinfo(lua_State *L, int arg, VkSamplerYcbcrConversionCreateInfo *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    GetFormat(format, "format");
    GetSamplerYcbcrModelConversion(ycbcrModel, "ycbcr_model");
    GetSamplerYcbcrRange(ycbcrRange, "ycbcr_range");
    GetComponentMappingOpt(components, "components");
    GetChromaLocation(xChromaOffset, "x_chroma_offset");
    GetChromaLocation(yChromaOffset, "y_chroma_offset");
    GetFilter(chromaFilter, "chroma_filter");
    GetBoolean(forceExplicitReconstruction, "force_explicit_reconstruction");
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freedebugutilsmessengercreateinfo(lua_State *L, VkDebugUtilsMessengerCreateInfo_CHAIN *p)
    {
    (void)L; (void)p;
    }

int echeckdebugutilsmessengercreateinfo(lua_State *L, int arg, VkDebugUtilsMessengerCreateInfo_CHAIN *pp)
    {
    VkDebugUtilsMessengerCreateInfoEXT *p = &pp->p1;
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    GetFlags(flags, "flags");
    GetFlags(messageSeverity, "message_severity");
    GetFlags(messageType, "message_type");
    /* pfnUserCallback and pUserData are set by the caller */
    return 0;
    }

/*------------------------------------------------------------------------------*/

void freedebugutilsobjectnameinfo(lua_State *L, VkDebugUtilsObjectNameInfoEXT *p)
    {
    if(p->pObjectName) Free(L, (char*)p->pObjectName);
    }

int echeckdebugutilsobjectnameinfo(lua_State *L, int arg, VkDebugUtilsObjectNameInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    GetObjectType(objectType, "object_type");
    GetHandle(objectHandle, "object_handle");
    GetString(pObjectName, "object_name");
    return 0;
    }

int pushdebugutilsobjectnameinfo(lua_State *L, const VkDebugUtilsObjectNameInfoEXT *p)
    {
    lua_newtable(L);
    SetEnum(objectType, "object_type", pushobjecttype);
    SetHandle(objectHandle, "object_handle");
    SetString(pObjectName, "object_name");
    return 1;
    }

/* echeckdebugutilsobjectnamelist() */
static FREELISTFUNC(VkDebugUtilsObjectNameInfoEXT, debugutilsobjectnameinfo)
static ECHECKLISTFUNC(VkDebugUtilsObjectNameInfoEXT, debugutilsobjectnameinfo, freedebugutilsobjectnameinfolist)

/*------------------------------------------------------------------------------*/

void freedebugutilsobjecttaginfo(lua_State *L, VkDebugUtilsObjectTagInfoEXT *p)
    {
    if(p->pTag) Free(L, (char*)p->pTag);
    }

int echeckdebugutilsobjecttaginfo(lua_State *L, int arg, VkDebugUtilsObjectTagInfoEXT *p)
    {
    size_t len;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
    GetObjectType(objectType, "object_type");
    GetHandle(objectHandle, "object_handle");
    GetInteger(tagName, "tag_name");
    GetLString(pTag, "tag", &len);
    p->tagSize = len;
    return 0;
    }

int pushdebugutilsobjecttaginfo(lua_State *L, const VkDebugUtilsObjectTagInfoEXT *p)
    {
    lua_newtable(L);
    SetEnum(objectType, "object_type", pushobjecttype);
    SetHandle(objectHandle, "object_handle");
    SetInteger(tagName, "tag_name");
    SetLString(pTag, "tag", p->tagSize);
    return 1;
    }

/*------------------------------------------------------------------------------*/

void freedebugutilslabel(lua_State *L, VkDebugUtilsLabelEXT *p)
    {
    if(p->pLabelName) Free(L, (char*)p->pLabelName);
    }

int echeckdebugutilslabel(lua_State *L, int arg, VkDebugUtilsLabelEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    GetString(pLabelName, "label_name");
    GetNumberArray(color, "color", 4);
    return 0;
    }

int pushdebugutilslabel(lua_State *L, const VkDebugUtilsLabelEXT *p)
    {
    lua_newtable(L);
    SetString(pLabelName, "label_name");
    SetNumberArray(color, "color", 4);
    return 1;
    }

/* echeckdebugutilslabellist() */
static FREELISTFUNC(VkDebugUtilsLabelEXT, debugutilslabel)
static ECHECKLISTFUNC(VkDebugUtilsLabelEXT, debugutilslabel, freedebugutilslabellist)

/*------------------------------------------------------------------------------*/

void freedebugutilsmessengercallbackdata(lua_State *L, VkDebugUtilsMessengerCallbackDataEXT *p)
    {
    if(!p) return;
    if(p->pMessageIdName) Free(L, (char*)p->pMessageIdName);
    if(p->pMessage) Free(L, (char*)p->pMessage);
    if(p->pQueueLabels) freedebugutilslabellist(L, (void*)p->pQueueLabels, p->queueLabelCount);
    if(p->pCmdBufLabels) freedebugutilslabellist(L, (void*)p->pCmdBufLabels, p->cmdBufLabelCount);
    if(p->pObjects) freedebugutilsobjectnameinfolist(L, (void*)p->pObjects, p->objectCount);
    }

int echeckdebugutilsmessengercallbackdata(lua_State *L, int arg, VkDebugUtilsMessengerCallbackDataEXT *p)
    {
    int err, arg1;
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
    GetFlags(flags, "flags");
    GetInteger(messageIdNumber, "message_id_number");
#define F "message_id_name"
    arg1 = pushfield(L, arg, F);
    p->pMessageIdName =  GetString_(L, arg1, F, NULL, &err, NULL);
    popfield(L, arg1);
    if(err) { freedebugutilsmessengercallbackdata(L, p); return prependfield(L, F); }
#undef F
#define F "message"
    arg1 = pushfield(L, arg, F);
    p->pMessage =  GetString_(L, arg1, F, NULL, &err, NULL);
    popfield(L, arg1);
    if(err) { freedebugutilsmessengercallbackdata(L, p); return prependfield(L, F); }
#undef F
#define F "queue_labels"
    arg1 = pushfield(L, arg, F);
    p->pQueueLabels = echeckdebugutilslabellist(L, arg1, &p->queueLabelCount, &err);
    popfield(L, arg1);
    if(err == ERR_NOTPRESENT || err == ERR_EMPTY) poperror();
    else if(err < 0) { freedebugutilsmessengercallbackdata(L, p); return prependfield(L, F); }
#undef F
#define F "cmd_buf_labels"
    arg1 = pushfield(L, arg, F);
    p->pCmdBufLabels = echeckdebugutilslabellist(L, arg1, &p->cmdBufLabelCount, &err);
    popfield(L, arg1);
    if(err == ERR_NOTPRESENT || err == ERR_EMPTY) poperror();
    else if(err < 0) { freedebugutilsmessengercallbackdata(L, p); return prependfield(L, F); }
#undef F
#define F "objects"
    arg1 = pushfield(L, arg, F);
    p->pObjects = echeckdebugutilsobjectnameinfolist(L, arg1, &p->objectCount, &err);
    popfield(L, arg1);
    if(err == ERR_NOTPRESENT || err == ERR_EMPTY) poperror();
    else if(err < 0) { freedebugutilsmessengercallbackdata(L, p); return prependfield(L, F); }
#undef F
    return 0;
    }

int pushdebugutilsmessengercallbackdata(lua_State *L, const VkDebugUtilsMessengerCallbackDataEXT *p)
    {
    uint32_t i;
    lua_newtable(L);
    SetFlags(flags, "flags");
    SetString(pMessageIdName, "message_id_name");
    SetInteger(messageIdNumber, "message_id_number");
    SetString(pMessage, "message");
#define F "queue_labels"
    lua_newtable(L);
    if(p->queueLabelCount > 0 && p->pQueueLabels)
        {
        for(i = 0; i < p->queueLabelCount; i++)
            {
            pushdebugutilslabel(L, &p->pQueueLabels[i]);
            lua_rawseti(L, -2, i+1);
            }
        }
    lua_setfield(L, -1, F);
#undef F
#define F "cmd_buf_labels"
    lua_newtable(L);
    if(p->cmdBufLabelCount > 0 && p->pCmdBufLabels)
        {
        for(i = 0; i < p->cmdBufLabelCount; i++)
            {
            pushdebugutilslabel(L, &p->pCmdBufLabels[i]);
            lua_rawseti(L, -2, i+1);
            }
        }
    lua_setfield(L, -1, F);
#undef F
#define F "objects"
    lua_newtable(L);
    if(p->objectCount > 0 && p->pObjects)
        {
        for(i = 0; i < p->objectCount; i++)
            {
            pushdebugutilsobjectnameinfo(L, &p->pObjects[i]);
            lua_rawseti(L, -2, i+1);
            }
        }
    lua_setfield(L, -1, F);
#undef F
    return 1;
    }

/*------------------------------------------------------------------------------*/

void freeconditionalrenderingbegininfo(lua_State *L, VkConditionalRenderingBeginInfoEXT *p)
    {
    (void)L; (void)p;
    }

int echeckconditionalrenderingbegininfo(lua_State *L, int arg, VkConditionalRenderingBeginInfoEXT *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetFlags(flags, "flags");
    return 0;
    }

/*------------------------------------------------------------------------------*/

int pushdescriptorsetlayoutsupport(lua_State *L, const VkDescriptorSetLayoutSupportKHR *p)
    {
    lua_newtable(L);
    SetBoolean(supported, "supported");
    return 1;
    }

/*------------------------------------------------------------------------------*/

void freedevicequeueinfo2(lua_State *L, VkDeviceQueueInfo2_CHAIN *p)
    {
    (void)L; (void)p;
    }

int echeckdevicequeueinfo2(lua_State *L, int arg, VkDeviceQueueInfo2_CHAIN *pp)
    {
    VkDeviceQueueInfo2 *p = &pp->p1;
    //const void **chain = pnextof(p);
    CHECK_TABLE(L, arg, pp);
    p->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
    GetInteger(queueIndex, "queue_index");
    return 0;
    }

#if 0 /* scaffolding 25yy */
#define freezzz moonvulkan_freezzz
void freezzz(lua_State *L, VkZzz *p);
#define echeckzzz moonvulkan_echeckzzz
int echeckzzz(lua_State *L, int arg, VkZzz *p);
#define pushzzz moonvulkan_pushzzz
int pushzzz(lua_State *L, const VkZzz *p);

void freezzz(lua_State *L, VkZzz *p)
    {
    (void)L; (void)p;
    }

int echeckzzz(lua_State *L, int arg, VkZzz *p)
    {
    CHECK_TABLE(L, arg, p);
    p->sType = ;
    return 0;
    }

int pushzzz(lua_State *L, const VkZzz *p)
    {
    lua_newtable(L);
    return 1;
    }

#endif

