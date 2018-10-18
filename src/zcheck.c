/* The MIT License (MIT)
 *
 * Copyright (c) 2018 Stefano Trettel
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

#if 0
/* Notes for my future self
 * ----------------------------------------------------------------------------
 * This module replaces the original structs.* module for handling of Vulkan
 * structs. A replacement was needed because the original module was not much
 * consistent and too hard to extend. This new module is more consinstent, but
 * less efficient because it dynamically allocates and deallocates every struct
 * at need (however, the original module did so as well for most structs: I'm
 * afraid that this is inevitable with the Vulkan extension system, if we don't
 * know in advance how and how many structs are needed...).
 *
 * Terminology used here:
 * - A 'typed struct' is a vulkan struct having the sType and pNext fields.
 * - A 'untyped struct' is one lacking those two fields (and thus not extendable
 *   nor chainable).
 * - A 'fresh struct' is a newly allocated struct, zeroed out, and (if typed)
 *   with its sType field properly initialized.
 * - A 'fresh chain' is a pNext-chain of fresh structs.
 *
 * The following functions are declared for the generic VkXxx vulkan struct
 * (either typed or untyped), and implemented where actually needed.
 * The functions that have a 'int *err' parameter set it with ERR_SUCCESS (0)
 * on success or with the relevant ERR_XXX code if an error occurrs (note that
 * all error codes are <0 except for ERR_NOTPRESENT which is > 0).
 *
 * VkXxx* znewVkXxx(lua_State *L, int *err);
 * VkXxx* znewarrayVkXxx(lua_State *L, uint32_t counter, int *err);
 *        Allocate a fresh struct (or array of contiguous structs).
 *        This allocates the base struct(s) only, with no pNext chain.
 *
 * VkXxx* znewchainVkXxx(lua_State *L, int *err);
 * VkXxx* znewchainarrayVkXxx(lua_State *L, uint32_t count, int *err);
 *        Allocate a fresh struct (or array of contiguous structs), together
 *        with its full pNext chain (also fresh).
 *        (see ZINIT for more details)
 *
 * VkXxx* zcheckVkXxx(lua_State *L, int arg, int *err);
 * VkXxx* zcheckarrayVkXxx(lua_State *L, int arg, uint32_t *count, int *err);
 *        Similar to the standard luaL_checkxxx() functions: checks the
 *        value on the Lua stack at index 'arg', allocates a fresh struct,
 *        initializes it with the value and returns it.
 *        (see ZCHECK for more details)
 *
 * int    zpushVkXxx(luaState *L, VkXxx *p);
 *        Similar to the standard lua_pushxxx() functions: pushes the value
 *        contained in the passed struct on top of the Lua stack.
 *        (see ZPUSH for more details)
 *
 * void   zfreeVkXxx(luaState *L, VkXxx *p, int base);
 * void   zfreearrayVkXxx(lua_State *L, VkXxx *p, uint32_t count, int base);
 *        Frees the passed struct (or array of contiguous structs), together
 *        with any nested allocated content (either fields or pNext chain).
 *    --> All structs returned by znew, znewchain, and zcheck and by their <--
 *    --> array versions must be freed using zfree or zfreearray, and this <--
 *    --> must be done also in case of error.                              <--
 *        The 'base' parameters controls the freeing of the base struct(s):
 *        - if base=0, the base struct is not freed (only its contents are)
 *        - if base!=0, the base struct is freed together with its contents.
 *        (Outside of this module always set base=1).
 *
 * For internal use only:
 * void   zclearVkXxx(luaState *L, VkXxx *p);           --> see ZCLEAR
 * int    zinitVkXxx(lua_State *L, VkXxx* p, int *err); --> see ZINIT
 */
#endif

/********************************************************************************
 * Helper functions                                                             *
 ********************************************************************************/

static int getfield(lua_State *L, int arg, const char *sname)
/* Pushes field 'sname' from the table at arg, and returns its type (LUA_TXXX) */
    { lua_pushstring(L, sname); return lua_rawget(L, arg); }

static int pushfield(lua_State *L, int arg, const char *sname)
/* Pushes field 'sname' from the table at arg, and returns its stack index */
    { lua_pushstring(L, sname); lua_rawget(L, arg); return lua_gettop(L); }

#define popfield    lua_remove
#define poperror()  lua_pop(L, 1)
#define pusherror() lua_pushstring(L, errstring(*err))
#define pushfielderror(name) lua_pushfstring(L, "%s: %s", (name), errstring(*err))
#define prependfield(name) \
    do { lua_pushfstring(L, "%s.%s", (name), lua_tostring(L, -1)); lua_remove(L, -2); } while(0)

/* ZCHECK - Must be implemented for structs that must be passed from Lua to C */
#define ZCHECK_BEGIN(VkXxx) VkXxx* zcheck##VkXxx(lua_State *L, int arg, int *err) { VkXxx *p; 
#define ZCHECK_END *err=0; return p; }

/* ZPUSH - Must be implemented for structs that must be passed from C to Lua */
#define ZPUSH_BEGIN(VkXxx) int zpush##VkXxx(lua_State *L, const VkXxx *p) {
#define ZPUSH_END return 1; }

/* ZCLEAR - Must be implemented for structs that have allocated nested fields,
 * other than the pNext chain. It must deallocate those fields, with any nested
 * chain, but must not free the base struct pointed by p, or parts of it.
 * Note that nested fields may appear as pointers or as fully embedded: if they
 * are pointers they must be zfree()'d with base=1, otherwise with base=0).
 * - The zclear function for a typed struct (if it need one) must be static and
 *   an entry for it must be added to the switch in the zfreeaux function.
 *-  The zclear function for an untyped struct must be either a global function
 *   or globally defined as a NULL pointer (in case no deallocation is needed).
 */
#define ZCLEAR_BEGIN(VkXxx) void zclear##VkXxx(lua_State *L, const void *p_) { VkXxx *p =(VkXxx*)p_; 
#define ZCLEAR_END }

/* ZINIT - Must be implemented for structs that must be passed 'fresh' to Vulkan
 * for parameters retrieval. Given a fresh struct, it must allocate fresh structs
 * for its required extensions and add them to its pNext chain.
 * The ZINIT_BEGIN macro also implements the znewchain functions, which are those
 * actually used outside this module (zinit should not be used directly). */
#define ZINIT_BEGIN(VkXxx)                                              \
VkXxx* znewchain##VkXxx(lua_State *L, int *err)                         \
    {                                                                   \
    VkXxx* p = znew##VkXxx(L, err);                                     \
    if(*err) return NULL;                                               \
    zinit##VkXxx(L, p, err);                                            \
    if(*err) { zfree##VkXxx(L, p, 1); return NULL; }                    \
    return p;                                                           \
    }                                                                   \
VkXxx* znewchainarray##VkXxx(lua_State *L, uint32_t count, int *err)    \
    {                                                                   \
    uint32_t i;                                                         \
    VkXxx* p = znewarray##VkXxx(L, count, err);                         \
    if(*err) return NULL;                                               \
    for(i=0; i<count; i++)                                              \
        {                                                               \
        zinit##VkXxx(L, &p[i], err);                                    \
        if(*err) { zfreearray##VkXxx(L, p, count, 1); return NULL; }    \
        }                                                               \
    return p;                                                           \
    }                                                                   \
int zinit##VkXxx(lua_State *L, VkXxx* p, int *err) { (void)L; (void)(p);
#define ZINIT_END *err=0; return *err;  }

/* Structs chaining via pNext -------------------------------------------------*/
#define pnextof(p_) (void**)&((p_)->pNext)
#define addtochain(chain_, p_) \
    do { if(p_) { *(chain_) = (p_); (chain_) = pnextof(p_); } } while(0)
/* addtochain() appends the struct pointed to by p_ to the chain.
 * const void **chain_ must contains the address of the pNext field of the
 * struct that is currently last in the chain. */
#define EXTENSIONS_BEGIN do { void **chain = pnextof(p);
#define EXTENSIONS_END } while(0);

/* These are for long chains of extensions like VkPhysicalDeviceFeatures2KHR,
 * where the extensions structs are all typed, do not need a zclear, and only
 * appear in these chains as extensions (so there is no need to define all the
 * functions for them and we can use znew() and zfree() without wrappers).
 * Notice that, in the Lua value, the fields of an extension struct are often
 * promoted to the base table, so there is no lua_newtable() call in the zpush
 * (or localpush) function, and no checktable() call in the zcheck function.
 */
#define ADDX(XXX, VkXxx) do {                                           \
    VkXxx* p1 = znew(L, VK_STRUCTURE_TYPE_##XXX, sizeof(VkXxx), err);   \
    if(*err) { zfree(L, p1, 1); return *err; }                          \
    addtochain(chain, p1);                                              \
} while(0)

/* defines a push function for local use only (localpushVkXxx). */
#define LOCALPUSH_BEGIN(VkXxx) static int localpush##VkXxx(lua_State *L, const VkXxx *p) {
#define LOCALPUSH_END return 1; }

#define XPUSH_BEGIN do {                                                \
    VkBaseOutStructure *pp_ = (VkBaseOutStructure*)p->pNext;            \
        while(pp_) { switch(pp_->sType) {
#define XCASE(XXX, VkXxx) case VK_STRUCTURE_TYPE_##XXX: localpush##VkXxx(L, (VkXxx*)pp_); break
#define XPUSH_END                                                       \
        default: unexpected(L); return 1; }                             \
            pp_ = pp_->pNext; }                                         \
} while(0);
/*--------------------------------------------------------------------------*/

static int checktable_(lua_State *L, int arg)
#define checktable(arg) if((*err = checktable_(L, (arg)))!=0) return NULL
    {
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); return ERR_NOTPRESENT; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); return ERR_TABLE; }
    return 0;
    }
#define newstruct(VkXxx) if((p = znew##VkXxx(L, err))==NULL) return NULL

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
#define SetStruct(name, sname, VkXxx) do { zpush##VkXxx(L, &(p->name)); lua_setfield(L, -2, sname); } while(0)
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
 | Get macros (for check functions)                                             |
 *------------------------------------------------------------------------------*/

static VkFlags GetFlags_(lua_State *L, int arg, const char *sname, int *err)
/* always opt., defaults to 0 */
    {
    int arg_ = pushfield(L, arg, sname);
    VkFlags flags = testflags(L, arg_, err);
    popfield(L, arg_);
    if(*err == ERR_NOTPRESENT) { *err=0; return 0; }
    if(*err) pushfielderror(sname);
    return flags;
    }

#define GetFlags(name, sname)  do {                             \
    p->name = GetFlags_(L, arg, sname, err);                    \
    if(*err) return p;                                          \
} while(0)

#define GetBits(name, sname, T) do {                            \
    /* same as GetFlags, but casts to T */                      \
    p->name = (T)GetFlags_(L, arg, sname, err);                 \
    if(*err) return p;                                          \
} while(0)

#define GetSamples(name, sname) do {                            \
/* VkSampleCountFlags may be passed either as string or number (eg 1 or "1" for 1 bit) */\
    int arg_ = pushfield(L, arg, sname);                        \
    p->name = (VkSampleCountFlagBits)testflags(L, arg_, err);   \
    popfield(L, arg_);                                          \
    if(*err == ERR_NOTPRESENT)                                  \
        { p->name = VK_SAMPLE_COUNT_1_BIT; *err = 0; }          \
    else if(*err)                                               \
        { pushfielderror(sname); return p; }                    \
} while(0)

/* Numbers -------------------------------------------------------------------*/

static lua_Number GetNumberDef_(lua_State *L, int arg, const char *sname, lua_Number defval, int *err)
    {
    lua_Number val = defval;
    int arg_ = pushfield(L, arg, sname);
    *err = 0;
    if(lua_isnumber(L, arg_)) val = lua_tonumber(L, arg_);
    else if(!lua_isnoneornil(L, arg_)) *err = ERR_TYPE;
    popfield(L, arg_);
    if(*err) pushfielderror(sname);
    return val;
    }

#define GetNumberDef(name, sname, defval) /* defaults to defval */ do { \
    p->name = GetNumberDef_(L, arg, sname, defval, err);                \
    if(*err) return p;                                                  \
} while(0)

#define GetNumber(name, sname) GetNumberDef(name, sname, 0) /* defaults to 0 */

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
        pushfielderror(sname);
    return val;
    }

#define GetIntegerOpt(name, sname, defval)  /* defaults to defval */ do {   \
    p->name = GetInteger_(L, arg, sname, defval, err);                      \
    if(*err) return p;                                                      \
} while(0)

#define GetInteger(name, sname) GetIntegerOpt(name, sname, 0) /* defaults to 0 */ 
#define GetHandle GetInteger /* uint64_t handle */

#define GetNumArray_(name, sname, n, towhatx) do {                          \
/* get a fixed-length n array of floats or integers */                      \
    int arg1_, arg2_, t_, i_, isnum_;                                       \
    *err = 0;                                                               \
    arg1_ = pushfield(L, arg, sname);                                       \
    t_ = lua_type(L, arg1_);                                                \
    if(t_ != LUA_TTABLE)                                                    \
        {                                                                   \
        popfield(L, arg1_);                                                 \
        if(t_ == LUA_TNIL || t_ == LUA_TNONE) { *err = ERR_NOTPRESENT; }    \
        else { *err=ERR_TABLE; pushfielderror(sname); return p; }           \
        }                                                                   \
    else {                                                                  \
        for(i_=0; i_ <(n); i_++)                                            \
            {                                                               \
            lua_rawgeti(L, arg1_, i_+1);                                    \
            arg2_ = lua_gettop(L);                                          \
            p->name[i_] = towhatx(L, arg2_, &isnum_);                       \
            popfield(L, arg2_);                                             \
            if(!isnum_) { *err=ERR_TYPE; break; }                           \
            }                                                               \
        popfield(L, arg1_);                                                 \
        if(*err < 0)                                                        \
            {                                                               \
            switch(*err)                                                    \
                {                                                           \
                case ERR_TABLE:                                             \
                case ERR_MEMORY:                                            \
                case ERR_EMPTY: pushfielderror(sname); return p;            \
                default:        prependfield(sname); return p;              \
                }                                                           \
            }                                                               \
        }                                                                   \
} while(0)

#define GetIntegerArray(name, sname, n) GetNumArray_(name, sname, n, lua_tointegerx)
#define GetNumberArray(name, sname, n) GetNumArray_(name, sname, n, lua_tonumberx)

/* Booleans ------------------------------------------------------------------*/

static int GetBoolean_(lua_State *L, int arg, const char *sname, int *err)
/* always opt., defaults to 0 (false) */
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
        pushfielderror(sname);
    return val;
    }

#define GetBoolean(name, sname) do {                \
    p->name = GetBoolean_(L, arg, sname, err);      \
    if(*err) return p;                              \
} while(0)

/* Strings -------------------------------------------------------------------*/

static const char *GetString_(lua_State *L, int arg, const char *sname, const char *defval, int *err, size_t *len)
/* The caller must Free() the returned string (if not NULL).
 * If len!=NULL, sets len with the string length or with 0 if defval is used. */
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
        pushfielderror(sname);
    return val;
    }

#define GetString(name, sname) do {                             \
    p->name =  GetString_(L, arg, sname, NULL, err, NULL);      \
    if(*err) return p;                                          \
} while(0)

#define GetStringOpt(name, sname) do {                          \
    p->name =  GetString_(L, arg, sname, NULL, err, NULL);      \
    if(*err < 0) return p;                                      \
    if(*err == ERR_NOTPRESENT) { *err = 0; poperror(); }        \
} while(0)

#define GetStringDef(name, sname, defval) do {                  \
    p->name =  GetString_(L, arg, sname, defval, err, NULL);    \
    if(*err) return p;                                          \
} while(0)

#define GetLString(name, sname, len) /* size_t* */ do {         \
    p->name =  GetString_(L, arg, sname, NULL, err, len);       \
    if(*err) return p;                                          \
} while(0)

/* Lightuserdata -------------------------------------------------------------*/

#define GetLightuserdataOpt(name, sname, TTT) do {  \
    int arg_ = pushfield(L, arg, sname);            \
    *err = 0;                                       \
    if(lua_isnoneornil(L, arg_))                    \
        p->name = NULL;                             \
    else                                            \
        {                                           \
        if(lua_type(L, arg_) != LUA_TLIGHTUSERDATA) \
            *err = ERR_TYPE;                        \
        else                                        \
            p->name = (TTT)lua_touserdata(L, arg_); \
        }                                           \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname); return p; } \
} while(0)

#define GetLightuserdata GetLightuserdataOpt

/* Enums ---------------------------------------------------------------------*/

#define GetEnum_(name, sname, testfunc, defval, opt) do {   \
    int arg_ = pushfield(L, arg, sname);                    \
    *err = 0;                                               \
    p->name = testfunc(L, arg_, err);                       \
    popfield(L, arg_);                                      \
    if(opt && (*err == ERR_NOTPRESENT))                     \
        { p->name = (defval); *err = 0; }                   \
    else if(*err)                                           \
        { pushfielderror(sname); return p; }       \
} while(0)

#define GetEnum(name, sname, testfunc) GetEnum_(name, sname, testfunc, 0,  0)
#define GetEnumOpt(name, sname, testfunc, defval) GetEnum_(name, sname, testfunc, defval, 1)

/* enums without default value (ie required) */
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

/* enums with default value (ie optional) */
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

#define GetStruct_(name, sname, VkXxx, opt) do {                    \
    int arg_ = pushfield(L, arg, sname);                            \
    VkXxx *tmp = zcheck##VkXxx(L, arg_, err);                       \
    if(tmp)                                                         \
        { memcpy(&(p->name), tmp, sizeof(VkXxx)); Free(L, tmp); }   \
    popfield(L, arg_);                                              \
    if(*err)                                                        \
        {                                                           \
        switch(*err)                                                \
            {                                                       \
            case ERR_NOTPRESENT: if(opt) /*@@ *err=0; */ break; /* else fallthrough */\
            case ERR_TABLE:                                         \
            case ERR_MEMORY:                                        \
            case ERR_EMPTY:                                         \
                { pushfielderror(sname); return p; }                \
            default:                                                \
                { prependfield(sname); return p; }                  \
            }                                                       \
        }                                                           \
} while(0)

#define GetStruct(name, sname, VkXxx) GetStruct_(name, sname, VkXxx, 0)
#define GetStructOpt(name, sname, VkXxx) GetStruct_(name, sname, VkXxx, 1)

#define GetStructArrayOpt(name, sname, n, VkXxx) do { /* arrays of fixed size */\
    VkXxx *tmp;                                                 \
    int arg1_, arg2_, t_, i_;                                   \
    arg1_ = pushfield(L, arg, sname);                           \
    t_ = lua_type(L, arg1_);                                    \
    *err = 0;                                                   \
    if(t_ != LUA_TTABLE)                                        \
        {                                                       \
        popfield(L, arg1_);                                     \
        if(t_ == LUA_TNIL || t_ == LUA_TNONE)                   \
            *err = ERR_NOTPRESENT;                              \
        else                                                    \
            { *err=ERR_TABLE; pushfielderror(sname); return p; }   \
        }                                                       \
    else {                                                      \
        for(i_=0; i_ <(n); i_++)                                \
            {                                                   \
            lua_rawgeti(L, arg1_, i_+1);                        \
            arg2_ = lua_gettop(L);                              \
            tmp = zcheck##VkXxx(L, arg2_, err);                 \
            if(tmp)                                             \
                { memcpy(&(p->name[i_]), tmp, sizeof(VkXxx)); Free(L, tmp); } \
            popfield(L, arg2_);                                 \
            if(*err < 0) break;                                 \
            }                                                   \
        popfield(L, arg1_);                                     \
        if(*err < 0)                                            \
            {                                                   \
            switch(*err)                                        \
                {                                               \
                case ERR_TABLE:                                 \
                case ERR_MEMORY:                                \
                case ERR_EMPTY:                                 \
                    pushfielderror(sname); return p;   \
                default:                                        \
                    prependfield(sname); return p;              \
                }                                               \
            }                                                   \
        }                                                       \
} while(0)

/* Objects -------------------------------------------------------------------*/

#define GetObject_(name, sname, TTT, ttt, opt) do { \
/* eg: TTT = VkRenderPass, ttt = render_pass */     \
    int arg_ = pushfield(L, arg, sname);            \
    *err = 0;                                       \
    if(lua_isnoneornil(L, arg_))                    \
        {                                           \
        p->name = 0;                                \
        if(!opt) *err = ERR_NOTPRESENT;             \
        }                                           \
    else                                            \
        {                                           \
        p->name = test##ttt(L, arg_, NULL);         \
        if(!p->name) *err = ERR_TYPE;               \
        }                                           \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname);  return p; }    \
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

/* Integers with special values ----------------------------------------------*/

#define GetSubpass(name, sname) /* integer or 'external' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    *err = 0;                                       \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "external") == 0)             \
            p->name = VK_SUBPASS_EXTERNAL;          \
        else                                        \
            *err = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = 0;                                \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        *err = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname); return p; }  \
} while(0)


/* 'remaining' stands for VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS, etc
 * (provided it has a value of ~0U) */
#define GetIntegerOrRemaining(name, sname, defval) /* integer or 'remaining' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    *err = 0;                                       \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "remaining") == 0)            \
            p->name = ~0U; /* VK_REMAINING_XXX */   \
        else                                        \
            *err = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = defval;                           \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        *err = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname); return p; }  \
} while(0)

#define GetIntegerOrWholeSize(name, sname) /* integer or 'whole size' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    *err = 0;                                       \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "whole size") == 0)           \
            p->name = VK_WHOLE_SIZE;                \
        else                                        \
            *err = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_WHOLE_SIZE;                    \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        *err = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname); return p; }  \
} while(0)

#define GetAttachment(name, sname) /* integer or 'unused', defval = 'unused' */ do {\
    const char *s_;                                 \
    int arg_ = pushfield(L, arg, sname);            \
    int t_ = lua_type(L, arg_);                     \
    *err = 0;                                       \
    if(t_ == LUA_TSTRING)                           \
        {                                           \
        s_ = lua_tostring(L, arg_);                 \
        if(strcmp(s_, "unused") == 0)               \
            p->name = VK_ATTACHMENT_UNUSED;         \
        else                                        \
            *err = ERR_VALUE;                       \
        }                                           \
    else if(t_ == LUA_TNONE || t_ == LUA_TNIL)      \
        p->name = VK_ATTACHMENT_UNUSED;             \
    else if(lua_isinteger(L, arg_))                 \
        p->name = lua_tointeger(L, arg_);           \
    else                                            \
        *err = ERR_TYPE;                            \
    popfield(L, arg_);                              \
    if(*err)                                        \
        { pushfielderror(sname); return p; }  \
} while(0)

/*------------------------------------------------------------------------------*
 | Arrays                                                                       |
 *------------------------------------------------------------------------------*
 * VkXxx* zcheckarrayVkXxx(lua_State *L, int arg, uint32_t *count, int *err)
 *
 * Checks if the variable at arg on the Lua stack is a list of VkXxx structures.
 * On success, returns an array of VkXxx structs and sets its length in *count.
 * The array must be released by the caller using zfreearray().
 *
 * On error, sets *err to ERR_XXX, *count to 0, leaves an error message on the
 * Lua stack and returns NULL. 
 *
 * This function espects the existence of the zcheckVkXxx function.
 * The returned array must be released with zfreearrayVkXxx().
 * Works only with structures having the sType and pNext fields.
 */

#define ZCHECKARRAY(VkXxx)                                                  \
VkXxx* zcheckarray##VkXxx(lua_State *L, int arg, uint32_t *count, int *err) \
    {                                                                       \
    int arg_;                                                               \
    VkXxx *list, *tmp;                                                      \
    uint32_t i;                                                             \
    *count = 0;                                                             \
    *err = 0;                                                               \
    if(lua_isnoneornil(L, arg))                                             \
        { *err = ERR_NOTPRESENT; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; } \
    if(lua_type(L, arg) != LUA_TTABLE)                                      \
        { *err = ERR_TABLE; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; } \
    *count = lua_rawlen(L, arg);                                            \
    if(*count == 0)                                                         \
        { *err = ERR_EMPTY; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; } \
    list = znewarray##VkXxx(L, *count, err);                                \
    if(!list)                                                               \
        { *count = 0; lua_pushfstring(L, ": %s", errstring(*err)); return NULL; }\
    for(i=0; i<*count; i++)                                                 \
        {                                                                   \
        lua_rawgeti(L, arg, i+1);                                           \
        arg_ = lua_gettop(L);                                               \
        tmp = zcheck##VkXxx(L, arg_, err);                                  \
        if(tmp)                                                             \
            { memcpy(&list[i], tmp, sizeof(VkXxx)); Free(L, tmp); }         \
        lua_remove(L, arg_);                                                \
        if(*err)                                                            \
            {                                                               \
            zfreearray##VkXxx(L, list, *count, 1);                          \
            *count = 0;                                                     \
            /* an error message has been already pushed by zcheckVkXxx() */ \
            lua_pushfstring(L, "%d.%s", i+1, lua_tostring(L, -1));          \
            lua_remove(L, -2);                                              \
            *err = ERR_GENERIC;                                             \
            return NULL;                                                    \
            }                                                               \
        }                                                                   \
    return list;                                                            \
    }

/********************************************************************************
 * Untyped structs                                                              *
 ********************************************************************************/

ZCHECK_BEGIN(VkViewport)
    checktable(arg);
    newstruct(VkViewport);
    GetNumber(x, "x");
    GetNumber(y, "y");
    GetNumber(width, "width");
    GetNumber(height, "height");
    GetNumber(minDepth, "min_depth");
    GetNumber(maxDepth, "max_depth");
ZCHECK_END
ZPUSH_BEGIN(VkViewport)
    lua_newtable(L);
    SetNumber(x, "x");
    SetNumber(y, "y");
    SetNumber(width, "width");
    SetNumber(height, "height");
    SetNumber(minDepth, "min_depth");
    SetNumber(maxDepth, "max_depth");
ZPUSH_END
ZCHECKARRAY(VkViewport)

ZCHECK_BEGIN(VkOffset2D)
    checktable(arg);
    newstruct(VkOffset2D);
    GetInteger(x, "x");
    GetInteger(y, "y");
ZCHECK_END
ZPUSH_BEGIN(VkOffset2D)
    lua_newtable(L);
    SetInteger(x, "x");
    SetInteger(y, "y");
ZPUSH_END

ZCHECK_BEGIN(VkOffset3D)
    checktable(arg);
    newstruct(VkOffset3D);
    GetInteger(x, "x");
    GetInteger(y, "y");
    GetInteger(z, "z");
ZCHECK_END
ZPUSH_BEGIN(VkOffset3D)
    lua_newtable(L);
    SetInteger(x, "x");
    SetInteger(y, "y");
    SetInteger(z, "z");
ZPUSH_END

ZCHECK_BEGIN(VkExtent2D)
    checktable(arg);
    newstruct(VkExtent2D);
    GetInteger(width, "width");
    GetInteger(height, "height");
ZCHECK_END
ZPUSH_BEGIN(VkExtent2D)
    lua_newtable(L);
    SetInteger(width, "width");
    SetInteger(height, "height");
ZPUSH_END

ZCHECK_BEGIN(VkExtent3D)
    checktable(arg);
    newstruct(VkExtent3D);
    GetInteger(width, "width");
    GetInteger(height, "height");
    GetInteger(depth, "depth");
ZCHECK_END
ZPUSH_BEGIN(VkExtent3D)
    lua_newtable(L);
    SetInteger(width, "width");
    SetInteger(height, "height");
    SetInteger(depth, "depth");
ZPUSH_END

ZCHECK_BEGIN(VkRect2D)
    checktable(arg);
    newstruct(VkRect2D);
    GetStructOpt(offset, "offset", VkOffset2D);
    GetStructOpt(extent, "extent", VkExtent2D);
ZCHECK_END
ZPUSH_BEGIN(VkRect2D)
    lua_newtable(L);
    SetStruct(offset, "offset", VkOffset2D);
    SetStruct(extent, "extent", VkExtent2D);
ZPUSH_END
ZCHECKARRAY(VkRect2D)

ZCHECK_BEGIN(VkXYColorEXT)
    checktable(arg);
    newstruct(VkXYColorEXT);
    GetNumber(x, "x");
    GetNumber(y, "y");
ZCHECK_END

ZCHECK_BEGIN(VkComponentMapping)
    checktable(arg);
    newstruct(VkComponentMapping);
    GetComponentSwizzle(r, "r");
    GetComponentSwizzle(g, "g");
    GetComponentSwizzle(b, "b");
    GetComponentSwizzle(a, "a");
ZCHECK_END
ZPUSH_BEGIN(VkComponentMapping)
    lua_newtable(L);
    SetEnum(r, "r", pushcomponentswizzle);
    SetEnum(g, "g", pushcomponentswizzle);
    SetEnum(b, "b", pushcomponentswizzle);
    SetEnum(a, "a", pushcomponentswizzle);
ZPUSH_END

ZCHECK_BEGIN(VkSampleLocationEXT)
    checktable(arg);
    newstruct(VkSampleLocationEXT);
    GetNumber(x, "x");
    GetNumber(y, "y");
ZCHECK_END
ZPUSH_BEGIN(VkSampleLocationEXT)
    lua_newtable(L);
    SetNumber(x, "x");
    SetNumber(y, "y");
ZPUSH_END
ZCHECKARRAY(VkSampleLocationEXT)

ZCHECK_BEGIN(VkImageSubresourceRange)
    checktable(arg);
    newstruct(VkImageSubresourceRange);
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(baseMipLevel, "base_mip_level");
    GetIntegerOrRemaining(levelCount, "level_count", 1);
    GetInteger(baseArrayLayer, "base_array_layer");
    GetIntegerOrRemaining(layerCount, "layer_count", 1);
ZCHECK_END
ZPUSH_BEGIN(VkImageSubresourceRange)
    lua_newtable(L);
    SetFlags(aspectMask, "aspect_mask");
    SetInteger(baseMipLevel, "base_mip_level");
    SetInteger(levelCount, "level_count");
    SetInteger(baseArrayLayer, "base_array_layer");
    SetInteger(layerCount, "layer_count");
ZPUSH_END
ZCHECKARRAY(VkImageSubresourceRange)

ZPUSH_BEGIN(VkExtensionProperties)
    lua_newtable(L);
    SetString(extensionName, "extension_name");
    SetInteger(specVersion, "spec_version");
ZPUSH_END

ZPUSH_BEGIN(VkLayerProperties)
    lua_newtable(L);
    SetString(layerName, "layer_name");
    SetInteger(specVersion, "spec_version");
    SetInteger(implementationVersion, "implementation_version");
    SetString(description, "description");
ZPUSH_END

ZCHECK_BEGIN(VkClearColorValue) 
    int i, t;
    const char* s;
    int colortype = -1;
    checktable(arg);
    newstruct(VkClearColorValue);
    getfield(L, arg, "t");
    s = luaL_optstring(L, -1, NULL);
    if(!s || (strcmp(s, "float32")==0)) colortype = 0;
    else if(strcmp(s, "int32")==0) colortype = 1;
    else if(strcmp(s, "uint32")==0) colortype = 2;
    else { lua_pop(L, 1); lua_pushstring(L, "t"); *err = ERR_VALUE; return p; }
    lua_pop(L, 1);
    if(colortype == 0) /* float32 */
        {
        for(i = 0; i < 4; i++)
            {
            t = lua_rawgeti(L, arg, i + 1);
            if(t != LUA_TNUMBER)
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1); *err = ERR_TYPE; return p; }
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
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1); *err = ERR_TYPE; return p; } 
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
                { lua_pop(L, 1); lua_pushfstring(L, "%d", i+1);  *err = ERR_TYPE; return p; } 
            p->uint32[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
            }
        }
ZCHECK_END

ZCHECK_BEGIN(VkClearValue)
    int t;
    checktable(arg);
    newstruct(VkClearValue);
    t = getfield(L, arg, "depth");
    lua_pop(L, 1);
    if(t==LUA_TNIL)
        {
        GetStruct(color, "color", VkClearColorValue);
        *err = 0;
        return p;
        }
    GetNumber(depthStencil.depth, "depth");
    GetInteger(depthStencil.stencil, "stencil");
ZCHECK_END
ZCHECKARRAY(VkClearValue)

ZCHECK_BEGIN(VkClearAttachment)
    checktable(arg);
    newstruct(VkClearAttachment);
    GetFlags(aspectMask, "aspect_mask");
    GetAttachment(colorAttachment, "color_attachment");
    GetStruct(clearValue, "clear_value", VkClearValue);
ZCHECK_END
ZCHECKARRAY(VkClearAttachment)

ZCHECK_BEGIN(VkClearRect)
    checktable(arg);
    newstruct(VkClearRect);
    GetStructOpt(rect, "rect", VkRect2D);
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
ZCHECK_END
ZCHECKARRAY(VkClearRect)

ZCHECK_BEGIN(VkImageSubresourceLayers)
    checktable(arg);
    newstruct(VkImageSubresourceLayers);
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(baseArrayLayer, "base_array_layer");
    GetInteger(layerCount, "layer_count");
ZCHECK_END

ZCHECK_BEGIN(VkImageCopy)
    checktable(arg);
    newstruct(VkImageCopy);
    GetStruct(srcSubresource, "src_subresource", VkImageSubresourceLayers);
    GetStructOpt(srcOffset, "src_offset", VkOffset3D);
    GetStruct(dstSubresource, "dst_subresource", VkImageSubresourceLayers);
    GetStructOpt(dstOffset, "dst_offset", VkOffset3D);
    GetStructOpt(extent, "extent", VkExtent3D);
ZCHECK_END
ZCHECKARRAY(VkImageCopy)

ZCHECK_BEGIN(VkImageBlit)
    checktable(arg);
    newstruct(VkImageBlit);
    GetStruct(srcSubresource, "src_subresource", VkImageSubresourceLayers);
    GetStructArrayOpt(srcOffsets, "src_offsets", 2, VkOffset3D);
    GetStruct(dstSubresource, "dst_subresource", VkImageSubresourceLayers);
    GetStructArrayOpt(dstOffsets, "dst_offsets", 2, VkOffset3D);
ZCHECK_END
ZCHECKARRAY(VkImageBlit)

ZCHECK_BEGIN(VkBufferImageCopy)
    checktable(arg);
    newstruct(VkBufferImageCopy);
    GetInteger(bufferOffset, "buffer_offset");
    GetInteger(bufferRowLength, "buffer_row_length");
    GetInteger(bufferImageHeight, "buffer_image_height");
    GetStruct(imageSubresource, "image_subresource", VkImageSubresourceLayers);
    GetStructOpt(imageOffset, "image_offset", VkOffset3D);
    GetStructOpt(imageExtent, "image_extent", VkExtent3D);
ZCHECK_END
ZCHECKARRAY(VkBufferImageCopy)

ZCHECK_BEGIN(VkImageResolve)
    checktable(arg);
    newstruct(VkImageResolve);
    GetStruct(srcSubresource, "src_subresource", VkImageSubresourceLayers);
    GetStructOpt(srcOffset, "src_offset", VkOffset3D);
    GetStruct(dstSubresource, "dst_subresource", VkImageSubresourceLayers);
    GetStructOpt(dstOffset, "dst_offset", VkOffset3D);
    GetStructOpt(extent, "extent", VkExtent3D);
ZCHECK_END
ZCHECKARRAY(VkImageResolve)

ZCHECK_BEGIN(VkBufferCopy)
    checktable(arg);
    newstruct(VkBufferCopy);
    GetInteger(srcOffset, "src_offset");
    GetInteger(dstOffset, "dst_offset");
    GetInteger(size, "size");
ZCHECK_END
ZCHECKARRAY(VkBufferCopy)

ZCHECK_BEGIN(VkSparseMemoryBind)
    checktable(arg);
    newstruct(VkSparseMemoryBind);
    GetInteger(resourceOffset, "resource_offset");
    GetInteger(size, "size");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "memory_offset");
    GetFlags(flags, "flags");
ZCHECK_END
ZCHECKARRAY(VkSparseMemoryBind)

ZCHECK_BEGIN(VkImageSubresource)
    checktable(arg);
    newstruct(VkImageSubresource);
    GetFlags(aspectMask, "aspect_mask");
    GetInteger(mipLevel, "mip_level");
    GetInteger(arrayLayer, "array_layer");
ZCHECK_END

ZCHECK_BEGIN(VkSparseImageMemoryBind)
    checktable(arg);
    newstruct(VkSparseImageMemoryBind);
    GetStruct(subresource, "subresource", VkImageSubresource);
    GetStructOpt(offset, "offset", VkOffset3D);
    GetStructOpt(extent, "extent", VkExtent3D);
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "memory_offset");
    GetFlags(flags, "flags");
ZCHECK_END
ZCHECKARRAY(VkSparseImageMemoryBind)

ZCLEAR_BEGIN(VkSparseBufferMemoryBindInfo)
    if(p->pBinds) zfreearrayVkSparseMemoryBind(L, p->pBinds, p->bindCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkSparseBufferMemoryBindInfo)
    int arg1;
    checktable(arg);
    newstruct(VkSparseBufferMemoryBindInfo);
    GetBuffer(buffer, "buffer");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = zcheckarrayVkSparseMemoryBind(L, arg1, &p->bindCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkSparseBufferMemoryBindInfo)

ZCLEAR_BEGIN(VkSparseImageOpaqueMemoryBindInfo)
    if(p->pBinds) zfreearrayVkSparseMemoryBind(L, p->pBinds, p->bindCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkSparseImageOpaqueMemoryBindInfo)
    int arg1;
    checktable(arg);
    newstruct(VkSparseImageOpaqueMemoryBindInfo);
    GetImage(image, "image");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = zcheckarrayVkSparseMemoryBind(L, arg1, &p->bindCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkSparseImageOpaqueMemoryBindInfo)

ZCLEAR_BEGIN(VkSparseImageMemoryBindInfo)
    if(p->pBinds) zfreearrayVkSparseImageMemoryBind(L, p->pBinds, p->bindCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkSparseImageMemoryBindInfo)
    int arg1;
    checktable(arg);
    newstruct(VkSparseImageMemoryBindInfo);
    GetImage(image, "image");
#define F "binds"
    arg1 = pushfield(L, arg, F);
    p->pBinds = zcheckarrayVkSparseImageMemoryBind(L, arg1, &p->bindCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkSparseImageMemoryBindInfo)

ZPUSH_BEGIN(VkSubresourceLayout) 
    lua_newtable(L);
    SetInteger(offset, "offset");
    SetInteger(size, "size");
    SetInteger(rowPitch, "row_pitch");
    SetInteger(arrayPitch, "array_pitch");
    SetInteger(depthPitch, "depth_pitch");
ZPUSH_END

ZCHECK_BEGIN(VkSpecializationMapEntry)
    checktable(arg);
    newstruct(VkSpecializationMapEntry);
    GetInteger(constantID, "constant_id");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
ZCHECK_END
ZCHECKARRAY(VkSpecializationMapEntry)

ZCLEAR_BEGIN(VkSpecializationInfo)
    if(p->pMapEntries)
        zfreearrayVkSpecializationMapEntry(L, p->pMapEntries, p->mapEntryCount, 1);
    if(p->pData) Free(L, (void*)p->pData);
ZCLEAR_END
ZCHECK_BEGIN(VkSpecializationInfo)
    size_t size;
    int arg1;
    const char* data;
    checktable(arg);
    newstruct(VkSpecializationInfo);
#define F "map_entries"
    arg1 = pushfield(L, arg, F);
    p->pMapEntries = zcheckarrayVkSpecializationMapEntry(L, arg1, &p->mapEntryCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "data"
    arg1 = pushfield(L, arg, F);
    data = lua_tolstring(L, arg1, &size);
    if(!data || size == 0)
        { popfield(L, arg1); *err=ERR_LENGTH; pushfielderror(F); return p; }
    p->pData = MallocNoErr(L, size);
    if(!p->pData)
        { popfield(L, arg1); *err=ERR_MEMORY; pushfielderror(F); return p; }
    memcpy((void*)p->pData, data, size);
    p->dataSize = size;
    popfield(L, arg1);
#undef F
ZCHECK_END

ZCHECK_BEGIN(VkVertexInputBindingDescription)
    checktable(arg);
    newstruct(VkVertexInputBindingDescription);
    GetInteger(binding, "binding");
    GetInteger(stride, "stride");
    GetVertexInputRate(inputRate, "input_rate");
ZCHECK_END
ZCHECKARRAY(VkVertexInputBindingDescription)

ZCHECK_BEGIN(VkVertexInputAttributeDescription)
    checktable(arg);
    newstruct(VkVertexInputAttributeDescription);
    GetInteger(location, "location");
    GetInteger(binding, "binding");
    GetFormat(format, "format");
    GetInteger(offset, "offset");
ZCHECK_END
ZCHECKARRAY(VkVertexInputAttributeDescription)

ZCHECK_BEGIN(VkStencilOpState)
    checktable(arg);
    newstruct(VkStencilOpState);
    GetStencilOp(failOp, "fail_op");
    GetStencilOp(passOp, "pass_op");
    GetStencilOp(depthFailOp, "depth_fail_op");
    GetCompareOp(compareOp, "compare_op");
    GetInteger(compareMask, "compare_mask");
    GetInteger(writeMask, "write_mask");
    GetInteger(reference, "reference");
ZCHECK_END

ZCHECK_BEGIN(VkPipelineColorBlendAttachmentState)
    checktable(arg);
    newstruct(VkPipelineColorBlendAttachmentState);
    GetBoolean(blendEnable, "blend_enable");
    GetBlendFactor(srcColorBlendFactor, "src_color_blend_factor");
    GetBlendFactor(dstColorBlendFactor, "dst_color_blend_factor");
    GetBlendOp(colorBlendOp, "color_blend_op");
    GetBlendFactor(srcAlphaBlendFactor, "src_alpha_blend_factor");
    GetBlendFactor(dstAlphaBlendFactor, "dst_alpha_blend_factor");
    GetBlendOp(alphaBlendOp, "alpha_blend_op");
    GetFlags(colorWriteMask, "color_write_mask");
ZCHECK_END
ZCHECKARRAY(VkPipelineColorBlendAttachmentState)

ZCHECK_BEGIN(VkRectLayerKHR)
    checktable(arg);
    newstruct(VkRectLayerKHR);
    GetStruct(offset, "offset", VkOffset2D);
    GetStruct(extent, "extent", VkExtent2D);
    GetInteger(layer, "layer");
ZCHECK_END
ZCHECKARRAY(VkRectLayerKHR)

ZCLEAR_BEGIN(VkPresentRegionKHR)
    if(p->pRectangles)
        zfreearrayVkRectLayerKHR(L, p->pRectangles, p->rectangleCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPresentRegionKHR)
    int arg1;
    checktable(arg);
    newstruct(VkPresentRegionKHR);
#define F "rectangles"
    arg1 = pushfield(L, arg, F);
    p->pRectangles = zcheckarrayVkRectLayerKHR(L, arg1, &p->rectangleCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END
ZCHECKARRAY(VkPresentRegionKHR)

ZCHECK_BEGIN(VkDescriptorImageInfo)
    checktable(arg);
    newstruct(VkDescriptorImageInfo);
    GetSampler(sampler, "sampler");
    GetImageView(imageView, "image_view");
    GetImageLayout(imageLayout, "image_layout");
ZCHECK_END
ZCHECKARRAY(VkDescriptorImageInfo)

ZCHECK_BEGIN(VkDescriptorBufferInfo)
    checktable(arg);
    newstruct(VkDescriptorBufferInfo);
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetIntegerOrWholeSize(range, "range");
ZCHECK_END
ZCHECKARRAY(VkDescriptorBufferInfo)

ZCHECK_BEGIN(VkDisplayModeParametersKHR)
    checktable(arg);
    newstruct(VkDisplayModeParametersKHR);
    GetStruct(visibleRegion, "visible_region", VkExtent2D);
    GetInteger(refreshRate, "refresh_rate");
ZCHECK_END
ZPUSH_BEGIN(VkDisplayModeParametersKHR)
    lua_newtable(L);
    SetStruct(visibleRegion, "visible_region", VkExtent2D);
    SetInteger(refreshRate, "refresh_rate");
ZPUSH_END

ZPUSH_BEGIN(VkDisplayModePropertiesKHR)
    lua_newtable(L);
/*  p->displayMode = set by caller */
    SetStruct(parameters, "parameters", VkDisplayModeParametersKHR);
    return 1;
ZPUSH_END

ZPUSH_BEGIN(VkDisplayPlaneCapabilitiesKHR)
    lua_newtable(L);
    SetFlags(supportedAlpha, "supported_alpha");
    SetStruct(minSrcPosition, "min_src_position", VkOffset2D);
    SetStruct(maxSrcPosition, "max_src_position", VkOffset2D);
    SetStruct(minSrcExtent, "min_src_extent", VkExtent2D);
    SetStruct(maxSrcExtent, "max_src_extent", VkExtent2D);
    SetStruct(minDstPosition, "min_dst_position", VkOffset2D);
    SetStruct(maxDstPosition, "max_dst_position", VkOffset2D);
    SetStruct(minDstExtent, "min_dst_extent", VkExtent2D);
    SetStruct(maxDstExtent, "max_dst_extent", VkExtent2D);
    return 1;
ZPUSH_END

ZCHECK_BEGIN(VkDescriptorUpdateTemplateEntry)
    checktable(arg);
    newstruct(VkDescriptorUpdateTemplateEntry);
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetInteger(descriptorCount, "descriptor_count");
    GetDescriptorType(descriptorType, "descriptor_type");
    GetInteger(offset, "offset");
    GetInteger(stride, "stride");
ZCHECK_END
ZCHECKARRAY(VkDescriptorUpdateTemplateEntry)

ZCHECK_BEGIN(VkAttachmentDescription)
    checktable(arg);
    newstruct(VkAttachmentDescription);
    GetFlags(flags, "flags");
    GetFormat(format, "format");
    GetSamples(samples, "samples");
    GetAttachmentLoadOp(loadOp, "load_op");
    GetAttachmentStoreOp(storeOp, "store_op");
    GetAttachmentLoadOp(stencilLoadOp, "stencil_load_op");
    GetAttachmentStoreOp(stencilStoreOp, "stencil_store_op");
    GetImageLayout(initialLayout, "initial_layout");
    GetImageLayout(finalLayout, "final_layout");
ZCHECK_END
ZCHECKARRAY(VkAttachmentDescription)

ZCHECK_BEGIN(VkSubpassDependency)
    checktable(arg);
    newstruct(VkSubpassDependency);
    GetSubpass(srcSubpass, "src_subpass");
    GetSubpass(dstSubpass, "dst_subpass");
    GetFlags(srcStageMask, "src_stage_mask");
    GetFlags(dstStageMask, "dst_stage_mask");
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetFlags(dependencyFlags, "dependency_flags");
ZCHECK_END
ZCHECKARRAY(VkSubpassDependency)

ZCHECK_BEGIN(VkAttachmentReference)
    checktable(arg);
    newstruct(VkAttachmentReference);
    GetAttachment(attachment, "attachment");
    GetImageLayout(layout, "layout");
ZCHECK_END
ZCHECKARRAY(VkAttachmentReference)

ZCHECK_BEGIN(VkDescriptorPoolSize)
    checktable(arg);
    newstruct(VkDescriptorPoolSize);
    GetDescriptorType(type, "type");
    GetIntegerOpt(descriptorCount, "descriptor_count", 1);
ZCHECK_END
ZCHECKARRAY(VkDescriptorPoolSize)

ZCHECK_BEGIN(VkPushConstantRange)
    checktable(arg);
    newstruct(VkPushConstantRange);
    GetFlags(stageFlags, "stage_flags");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
ZCHECK_END
ZCHECKARRAY(VkPushConstantRange)

ZCLEAR_BEGIN(VkSubpassDescription)
    if(p->pInputAttachments) 
        zfreearrayVkAttachmentReference(L, p->pInputAttachments, p->inputAttachmentCount, 1);
    if(p->pColorAttachments)
        zfreearrayVkAttachmentReference(L, p->pColorAttachments, p->colorAttachmentCount, 1);
    if(p->pResolveAttachments)
        zfreearrayVkAttachmentReference(L, p->pResolveAttachments, p->colorAttachmentCount, 1);
    if(p->pPreserveAttachments) Free(L, (void*)p->pPreserveAttachments);
    if(p->pDepthStencilAttachment ) 
        zfreeVkAttachmentReference(L, p->pDepthStencilAttachment, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkSubpassDescription)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkSubpassDescription);
    GetFlags(flags, "flags");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
#define F "input_attachments"
    arg1 = pushfield(L, arg, F);
    p->pInputAttachments = zcheckarrayVkAttachmentReference(L, arg1, &p->inputAttachmentCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "color_attachments"
    arg1 = pushfield(L, arg, F);
    p->pColorAttachments = zcheckarrayVkAttachmentReference(L, arg1, &p->colorAttachmentCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "resolve_attachments"
    arg1 = pushfield(L, arg, F);
    p->pResolveAttachments = zcheckarrayVkAttachmentReference(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
    else if(count != p->colorAttachmentCount)
        {
        if(p->pResolveAttachments)
            {
            zfreearrayVkAttachmentReference(L, p->pResolveAttachments, count, 1);
            p->pResolveAttachments = NULL;
            }
        *err = ERR_LENGTH; lua_pushstring(L, errstring(*err));
        return p;
        }
#undef F
#define F "depth_stencil_attachment"
    arg1 = pushfield(L, arg, F);
    p->pDepthStencilAttachment = zcheckVkAttachmentReference(L, arg1, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "preserve_attachments"
    arg1 = pushfield(L, arg, F);
    p->pPreserveAttachments = checkuint32list(L, arg1, &p->preserveAttachmentCount, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkSubpassDescription)

ZCHECK_BEGIN(VkInputAttachmentAspectReference)
    checktable(arg);
    newstruct(VkInputAttachmentAspectReference);
    GetInteger(subpass, "subpass");
    GetInteger(inputAttachmentIndex, "input_attachment_index");
    GetFlags(aspectMask, "aspect_mask");
ZCHECK_END
ZCHECKARRAY(VkInputAttachmentAspectReference)

ZCLEAR_BEGIN(VkAttachmentSampleLocationsEXT)
    zfreeVkSampleLocationsInfoEXT(L, &p->sampleLocationsInfo, 0);
ZCLEAR_END
ZCHECK_BEGIN(VkAttachmentSampleLocationsEXT)
    checktable(arg);
    newstruct(VkAttachmentSampleLocationsEXT);
    GetInteger(attachmentIndex, "attachment_index");
    GetStruct(sampleLocationsInfo, "sample_locations_info", VkSampleLocationsInfoEXT);
ZCHECK_END
ZCHECKARRAY(VkAttachmentSampleLocationsEXT)

ZCLEAR_BEGIN(VkSubpassSampleLocationsEXT)
    zfreeVkSampleLocationsInfoEXT(L, &p->sampleLocationsInfo, 0);
ZCLEAR_END
ZCHECK_BEGIN(VkSubpassSampleLocationsEXT)
    checktable(arg);
    newstruct(VkSubpassSampleLocationsEXT);
    GetInteger(subpassIndex, "subpass_index");
    GetStruct(sampleLocationsInfo, "sample_locations_info", VkSampleLocationsInfoEXT);
ZCHECK_END
ZCHECKARRAY(VkSubpassSampleLocationsEXT)

ZPUSH_BEGIN(VkExternalMemoryProperties)
    lua_newtable(L);
    SetFlags(externalMemoryFeatures, "external_memory_features");
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
ZPUSH_END

ZPUSH_BEGIN(VkDisplayPropertiesKHR)
    lua_newtable(L);
/*  p->display = set by caller */
    SetString(displayName, "display_name");
    SetStruct(physicalDimensions, "physical_dimensions", VkExtent2D);
    SetStruct(physicalResolution, "physical_resolution", VkExtent2D);
    SetFlags(supportedTransforms, "supported_transforms");
    SetBoolean(planeReorderPossible, "plane_reorder_possible");
    SetBoolean(persistentContent, "persistent_content");
ZPUSH_END

ZPUSH_BEGIN(VkDisplayPlanePropertiesKHR)
    lua_newtable(L);
/*  p->currentDisplay = set by caller */
    SetInteger(currentStackIndex, "current_stack_index");
ZPUSH_END
    
ZCLEAR_BEGIN(VkDescriptorSetLayoutBinding)
    if(p->pImmutableSamplers) Free(L, (void*)p->pImmutableSamplers);
ZCLEAR_END
ZCHECK_BEGIN(VkDescriptorSetLayoutBinding)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkDescriptorSetLayoutBinding);
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
        p->pImmutableSamplers = checksamplerlist(L, arg1, &count, err, NULL);
        popfield(L, arg1);
        if(*err < 0)
            { pushfielderror(F); return p; }
        if(*err != ERR_NOTPRESENT && count != p->descriptorCount)
            { *err=ERR_LENGTH; pushfielderror(F); return p; }
#undef F
        }
ZCHECK_END
ZCHECKARRAY(VkDescriptorSetLayoutBinding)

//££
/********************************************************************************
 * Typed structs                                                                *
 ********************************************************************************/

/*------------------------------------------------------------------------------*
 | Physical Device Features                                                     |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPhysicalDeviceFeatures)
    checktable(arg);
    newstruct(VkPhysicalDeviceFeatures);
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
ZCHECK_END
 
#define FUNC_BEGIN(XXX, VkXxx)                                                              \
static VkXxx* zcheck##VkXxx(lua_State *L, int arg, int *err)                                \
    { VkXxx* p; /* checktable(arg); */                                                      \
    if((p = znew(L, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##XXX, sizeof(VkXxx), err))==NULL)    \
        return NULL;
#define FUNC_END *err = 0; return p; }
FUNC_BEGIN(16BIT_STORAGE_FEATURES, VkPhysicalDevice16BitStorageFeatures)
    GetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    GetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    GetBoolean(storagePushConstant16, "storage_push_constant_16");
    GetBoolean(storageInputOutput16, "storage_input_output_16");
FUNC_END 
FUNC_BEGIN(VARIABLE_POINTER_FEATURES, VkPhysicalDeviceVariablePointerFeatures)
    GetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    GetBoolean(variablePointers, "variable_pointers");
FUNC_END 
FUNC_BEGIN(BLEND_OPERATION_ADVANCED_FEATURES_EXT, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT)
    GetBoolean(advancedBlendCoherentOperations, "advanced_blend_coherent_operations");
FUNC_END 
FUNC_BEGIN(SAMPLER_YCBCR_CONVERSION_FEATURES, VkPhysicalDeviceSamplerYcbcrConversionFeatures)
    GetBoolean(samplerYcbcrConversion, "sampler_ycbcr_conversion");
FUNC_END 
FUNC_BEGIN(CONDITIONAL_RENDERING_FEATURES_EXT, VkPhysicalDeviceConditionalRenderingFeaturesEXT)
    GetBoolean(conditionalRendering, "conditional_rendering");
    GetBoolean(inheritedConditionalRendering, "inherited_conditional_rendering");
FUNC_END 
FUNC_BEGIN(8BIT_STORAGE_FEATURES_KHR , VkPhysicalDevice8BitStorageFeaturesKHR)
    GetBoolean(storageBuffer8BitAccess, "storage_buffer_8bit_access");
    GetBoolean(uniformAndStorageBuffer8BitAccess, "uniform_and_storage_buffer_8bit_access");
    GetBoolean(storagePushConstant8, "storage_push_constant_8");
FUNC_END 
FUNC_BEGIN(PROTECTED_MEMORY_FEATURES, VkPhysicalDeviceProtectedMemoryFeatures)
    GetBoolean(protectedMemory, "protected_memory");
FUNC_END 
FUNC_BEGIN(SHADER_DRAW_PARAMETER_FEATURES, VkPhysicalDeviceShaderDrawParameterFeatures)
    GetBoolean(shaderDrawParameters, "shader_draw_parameters");
FUNC_END 
#undef FUNC_BEGIN
#undef FUNC_END

ZINIT_BEGIN(VkPhysicalDeviceFeatures2)
    EXTENSIONS_BEGIN
        ADDX(PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES, VkPhysicalDevice16BitStorageFeatures);
        ADDX(PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES, VkPhysicalDeviceVariablePointerFeatures);
        ADDX(PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT);
        ADDX(PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
                VkPhysicalDeviceSamplerYcbcrConversionFeatures);
        ADDX(PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
                VkPhysicalDeviceConditionalRenderingFeaturesEXT);
        ADDX(PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR , VkPhysicalDevice8BitStorageFeaturesKHR);
        ADDX(PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES, VkPhysicalDeviceProtectedMemoryFeatures);
        ADDX(PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES,
                VkPhysicalDeviceShaderDrawParameterFeatures);
    EXTENSIONS_END
ZINIT_END

ZCHECK_BEGIN(VkPhysicalDeviceFeatures2)
    VkPhysicalDeviceFeatures* features;
    //checktable(L); --> in zcheckVkPhysicalDeviceFeatures()
    newstruct(VkPhysicalDeviceFeatures2);
    features = zcheckVkPhysicalDeviceFeatures(L, arg, err);
    if(features)
        { memcpy(&p->features, features, sizeof(VkPhysicalDeviceFeatures)); Free(L, features); }
    if(*err < 0) return p;
    else if(*err == ERR_NOTPRESENT) poperror();
    EXTENSIONS_BEGIN
    #define ADD(VkXxx) do {                         \
        VkXxx* p1 = zcheck##VkXxx(L, arg, err);     \
        if(*err < 0) { zfree(L, p1, 1); return p; } \
        else if(*err == ERR_NOTPRESENT) poperror(); \
        else addtochain(chain, p1);                 \
    } while(0)
        ADD(VkPhysicalDevice16BitStorageFeatures);
        ADD(VkPhysicalDeviceVariablePointerFeatures);
        ADD(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT);
        ADD(VkPhysicalDeviceSamplerYcbcrConversionFeatures);
        ADD(VkPhysicalDeviceConditionalRenderingFeaturesEXT);
        ADD(VkPhysicalDevice8BitStorageFeaturesKHR);
        ADD(VkPhysicalDeviceProtectedMemoryFeatures);
        ADD(VkPhysicalDeviceShaderDrawParameterFeatures);
    #undef ADD
    EXTENSIONS_END
ZCHECK_END

LOCALPUSH_BEGIN(VkPhysicalDeviceFeatures)
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
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDevice16BitStorageFeatures)
    SetBoolean(storageBuffer16BitAccess, "storage_buffer_16bit_access");
    SetBoolean(uniformAndStorageBuffer16BitAccess, "uniform_and_storage_buffer_16bit_access");
    SetBoolean(storagePushConstant16, "storage_push_constant_16");
    SetBoolean(storageInputOutput16, "storage_input_output_16");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceVariablePointerFeatures)
    SetBoolean(variablePointersStorageBuffer, "variable_pointers_storage_buffer");
    SetBoolean(variablePointers, "variable_pointers");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT)
    SetBoolean(advancedBlendCoherentOperations, "advanced_blend_coherent_operations");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceSamplerYcbcrConversionFeatures)
    SetBoolean(samplerYcbcrConversion, "sampler_ycbcr_conversion");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceConditionalRenderingFeaturesEXT)
    SetBoolean(conditionalRendering, "conditional_rendering");
    SetBoolean(inheritedConditionalRendering, "inherited_conditional_rendering");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDevice8BitStorageFeaturesKHR)
    SetBoolean(storageBuffer8BitAccess, "storage_buffer_8bit_access");
    SetBoolean(uniformAndStorageBuffer8BitAccess, "uniform_and_storage_buffer_8bit_access");
    SetBoolean(storagePushConstant8, "storage_push_constant_8");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceProtectedMemoryFeatures)
    SetBoolean(protectedMemory, "protected_memory");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceShaderDrawParameterFeatures)
    SetBoolean(shaderDrawParameters, "shader_draw_parameters");
LOCALPUSH_END

ZPUSH_BEGIN(VkPhysicalDeviceFeatures)
    lua_newtable(L);
    localpushVkPhysicalDeviceFeatures(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkPhysicalDeviceFeatures2)
    lua_newtable(L);
    localpushVkPhysicalDeviceFeatures(L, &p->features);
    XPUSH_BEGIN
        XCASE(PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES, VkPhysicalDevice16BitStorageFeatures);
        XCASE(PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES, VkPhysicalDeviceVariablePointerFeatures);
        XCASE(PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT);
        XCASE(PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
                VkPhysicalDeviceSamplerYcbcrConversionFeatures);
        XCASE(PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
                VkPhysicalDeviceConditionalRenderingFeaturesEXT);
        XCASE(PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR , VkPhysicalDevice8BitStorageFeaturesKHR);
        XCASE(PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES, VkPhysicalDeviceProtectedMemoryFeatures);
        XCASE(PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES,
                VkPhysicalDeviceShaderDrawParameterFeatures);
    XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Physical Device Properties                                                   |
 *------------------------------------------------------------------------------*/

ZPUSH_BEGIN(VkPhysicalDeviceLimits)
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
ZPUSH_END

ZPUSH_BEGIN(VkPhysicalDeviceSparseProperties)
    lua_newtable(L);
    SetBoolean(residencyStandard2DBlockShape, "residency_standard_2d_block_shape");
    SetBoolean(residencyStandard2DMultisampleBlockShape, "residency_standard_2d_multisample_block_shape");
    SetBoolean(residencyStandard3DBlockShape, "residency_standard_3d_block_shape");
    SetBoolean(residencyAlignedMipSize, "residency_aligned_mip_size");
    SetBoolean(residencyNonResidentStrict, "residency_non_resident_strict");
ZPUSH_END

/*------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkPhysicalDeviceProperties)
    SetInteger(apiVersion, "api_version");
    SetInteger(driverVersion, "driver_version");
    SetInteger(vendorID, "vendor_id");
    SetInteger(deviceID, "device_id");
    SetEnum(deviceType, "device_type", pushphysicaldevicetype);
    SetString(deviceName, "device_name");
    SetUUID(pipelineCacheUUID, "pipeline_cache_uuid", VK_UUID_SIZE);
    SetStruct(limits, "limits", VkPhysicalDeviceLimits);
    SetStruct(sparseProperties, "sparse_properties", VkPhysicalDeviceSparseProperties);
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDevicePushDescriptorPropertiesKHR)
    SetInteger(maxPushDescriptors, "max_push_descriptors");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT)
    SetInteger(advancedBlendMaxColorAttachments, "advanced_blend_max_color_attachments");
    SetBoolean(advancedBlendIndependentBlend, "advanced_blend_independent_blend");
    SetBoolean(advancedBlendNonPremultipliedSrcColor, "advanced_blend_non_premultiplied_src_color");
    SetBoolean(advancedBlendNonPremultipliedDstColor, "advanced_blend_non_premultiplied_dst_color");
    SetBoolean(advancedBlendCorrelatedOverlap, "advanced_blend_correlated_overlap");
    SetBoolean(advancedBlendAllOperations, "advanced_blend_all_operations");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT)
    SetBoolean(filterMinmaxSingleComponentFormats, "filter_minmax_single_component_formats");
    SetBoolean(filterMinmaxImageComponentMapping, "filter_minmax_image_component_mapping");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceDiscardRectanglePropertiesEXT)
    SetInteger(maxDiscardRectangles, "max_discard_rectangles");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceIDProperties)
    SetUUID(deviceUUID, "device_uuid", VK_UUID_SIZE);
    SetUUID(driverUUID, "driver_uuid", VK_UUID_SIZE);
    SetUUID(deviceLUID, "device_luid", VK_LUID_SIZE);
    SetInteger(deviceNodeMask, "device_node_mask");
    SetBoolean(deviceLUIDValid, "device_luid_valid");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDevicePointClippingProperties)
    SetEnum(pointClippingBehavior, "point_clipping_behavior", pushpointclippingbehavior);
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceSampleLocationsPropertiesEXT)
    SetFlags(sampleLocationSampleCounts, "sample_location_sample_counts");
    SetStruct(maxSampleLocationGridSize, "max_sample_location_grid_size", VkExtent2D);
    SetNumberArray(sampleLocationCoordinateRange, "sample_location_coordinate_range", 2);
    SetInteger(sampleLocationSubPixelBits, "sample_location_sub_pixel_bits");
    SetBoolean(variableSampleLocations, "variable_sample_locations");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceMaintenance3Properties)
    SetInteger(maxPerSetDescriptors, "max_per_set_descriptors");
    SetInteger(maxMemoryAllocationSize, "max_memory_allocation_size");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceSubgroupProperties)
    SetFlags(supportedStages, "supported_stages");
    SetFlags(supportedOperations, "supported_operations");
    SetBoolean(quadOperationsInAllStages, "quad_operations_in_all_stages");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkPhysicalDeviceProtectedMemoryProperties)
    SetBoolean(protectedNoFault, "protected_no_fault");
LOCALPUSH_END

ZINIT_BEGIN(VkPhysicalDeviceProperties2)
    EXTENSIONS_BEGIN
        ADDX(PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
                VkPhysicalDevicePushDescriptorPropertiesKHR);
        ADDX(PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT,
                VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT);
        ADDX(PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT,
                VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT);
        ADDX(PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT,
                VkPhysicalDeviceDiscardRectanglePropertiesEXT);
        ADDX(PHYSICAL_DEVICE_ID_PROPERTIES, VkPhysicalDeviceIDProperties);
        ADDX(PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES, VkPhysicalDevicePointClippingProperties);
        ADDX(PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT,
                VkPhysicalDeviceSampleLocationsPropertiesEXT);
        ADDX(PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES, VkPhysicalDeviceMaintenance3Properties);
        ADDX(PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, VkPhysicalDeviceSubgroupProperties);
        ADDX(PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES, VkPhysicalDeviceProtectedMemoryProperties);
    EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkPhysicalDeviceProperties)
    lua_newtable(L);
    localpushVkPhysicalDeviceProperties(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkPhysicalDeviceProperties2)
    lua_newtable(L);
    localpushVkPhysicalDeviceProperties(L, &p->properties);
    XPUSH_BEGIN
        XCASE(PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
                 VkPhysicalDevicePushDescriptorPropertiesKHR);
        XCASE(PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT,
                 VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT);
        XCASE(PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT,
                 VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT);
        XCASE(PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT,
                 VkPhysicalDeviceDiscardRectanglePropertiesEXT);
        XCASE(PHYSICAL_DEVICE_ID_PROPERTIES, VkPhysicalDeviceIDProperties);
        XCASE(PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES, VkPhysicalDevicePointClippingProperties);
        XCASE(PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT,
                 VkPhysicalDeviceSampleLocationsPropertiesEXT);
        XCASE(PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES, VkPhysicalDeviceMaintenance3Properties);
        XCASE(PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, VkPhysicalDeviceSubgroupProperties);
        XCASE(PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES, VkPhysicalDeviceProtectedMemoryProperties);
    XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Format Properties                                                            |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkFormatProperties)
    SetFlags(linearTilingFeatures, "linear_tiling_features");
    SetFlags(optimalTilingFeatures, "optimal_tiling_features");
    SetFlags(bufferFeatures, "buffer_features");
LOCALPUSH_END

ZINIT_BEGIN(VkFormatProperties2)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkFormatProperties)
    lua_newtable(L);
    localpushVkFormatProperties(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkFormatProperties2)
    lua_newtable(L);
    localpushVkFormatProperties(L, &p->formatProperties);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Physical Device Memory Properties                                            |
 *------------------------------------------------------------------------------*/

static int localpushVkMemoryType(lua_State *L, const VkMemoryType *p, uint32_t index)
    {
    lua_newtable(L);
    lua_pushinteger(L, index); lua_setfield(L, -2, "memory_type_index");
    SetFlags(propertyFlags, "property_flags");
    SetInteger(heapIndex, "heap_index");
    return 1;
    }
static int localpushVkMemoryHeap(lua_State *L, const VkMemoryHeap *p, uint32_t index)
    {
    lua_newtable(L);
    lua_pushinteger(L, index); lua_setfield(L, -2, "memory_heap_index");
    SetInteger(size, "size");
    SetFlags(flags, "flags");
    return 1;
    }

LOCALPUSH_BEGIN(VkPhysicalDeviceMemoryProperties)
    uint32_t i;
    uint32_t tcount, hcount;
    tcount = (p->memoryTypeCount > VK_MAX_MEMORY_TYPES) ? VK_MAX_MEMORY_TYPES : p->memoryTypeCount;
    hcount = (p->memoryHeapCount > VK_MAX_MEMORY_HEAPS) ? VK_MAX_MEMORY_HEAPS : p->memoryHeapCount;
    lua_newtable(L);
    lua_newtable(L);
    for(i = 0; i < tcount; i++)
        {
        localpushVkMemoryType(L, &(p->memoryTypes[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "memory_types");
    lua_newtable(L);
    for(i = 0; i < hcount; i++)
        {
        localpushVkMemoryHeap(L, &(p->memoryHeaps[i]), i);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "memory_heaps");
LOCALPUSH_END

ZINIT_BEGIN(VkPhysicalDeviceMemoryProperties2)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkPhysicalDeviceMemoryProperties)
    lua_newtable(L);
    localpushVkPhysicalDeviceMemoryProperties(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkPhysicalDeviceMemoryProperties2)
    lua_newtable(L);
    localpushVkPhysicalDeviceMemoryProperties(L, &p->memoryProperties);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Image Format Properties                                                      |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkImageFormatProperties)
    SetStruct(maxExtent, "max_extent", VkExtent3D);
    SetInteger(maxMipLevels, "max_mip_levels");
    SetInteger(maxArrayLayers, "max_array_layers");
    SetInteger(sampleCounts, "sample_counts");
    SetInteger(maxResourceSize, "max_resource_size");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkSamplerYcbcrConversionImageFormatProperties)
    SetInteger(combinedImageSamplerDescriptorCount, "combined_image_sampler_descriptor_count");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkExternalImageFormatProperties)
    SetStruct(externalMemoryProperties, "external_memory_properties", VkExternalMemoryProperties);
LOCALPUSH_END

ZINIT_BEGIN(VkImageFormatProperties2)
    EXTENSIONS_BEGIN
        ADDX(SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES,
                    VkSamplerYcbcrConversionImageFormatProperties);
        ADDX(EXTERNAL_IMAGE_FORMAT_PROPERTIES, VkExternalImageFormatProperties);
    EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkImageFormatProperties)
    lua_newtable(L);
    localpushVkImageFormatProperties(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkImageFormatProperties2)
    lua_newtable(L);
    localpushVkImageFormatProperties(L, &p->imageFormatProperties);
    XPUSH_BEGIN
        XCASE(SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES,
                VkSamplerYcbcrConversionImageFormatProperties);
        XCASE(EXTERNAL_IMAGE_FORMAT_PROPERTIES, VkExternalImageFormatProperties);
    XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Sparse Image Format Properties                                               |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkSparseImageFormatProperties)
    SetFlags(aspectMask, "aspect_mask");
    SetStruct(imageGranularity, "image_granularity", VkExtent3D);
    SetFlags(flags, "flags");
LOCALPUSH_END

ZINIT_BEGIN(VkSparseImageFormatProperties2)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkSparseImageFormatProperties)
    lua_newtable(L);
    localpushVkSparseImageFormatProperties(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkSparseImageFormatProperties2)
    lua_newtable(L);
    localpushVkSparseImageFormatProperties(L, &p->properties);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END


/*------------------------------------------------------------------------------*
 | External Buffer/Fence/Semaphore Properties                                   |
 *------------------------------------------------------------------------------*/

ZPUSH_BEGIN(VkExternalBufferPropertiesKHR)
    lua_newtable(L);
    SetStruct(externalMemoryProperties, "external_memory_properties", VkExternalMemoryProperties);
ZPUSH_END

ZINIT_BEGIN(VkExternalBufferPropertiesKHR)
    (void)L; (void)p;
ZINIT_END

ZPUSH_BEGIN(VkExternalFencePropertiesKHR)
    lua_newtable(L);
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
    SetFlags(externalFenceFeatures, "external_fence_features");
ZPUSH_END

ZINIT_BEGIN(VkExternalFencePropertiesKHR)
    (void)L; (void)p;
ZINIT_END

ZPUSH_BEGIN(VkExternalSemaphorePropertiesKHR)
    lua_newtable(L);
    SetFlags(exportFromImportedHandleTypes, "export_from_imported_handle_types");
    SetFlags(compatibleHandleTypes, "compatible_handle_types");
    SetFlags(externalSemaphoreFeatures, "external_semaphore_features");
ZPUSH_END

ZINIT_BEGIN(VkExternalSemaphorePropertiesKHR)
    (void)L; (void)p;
ZINIT_END

ZPUSH_BEGIN(VkMultisamplePropertiesEXT)
    lua_newtable(L);
    SetStruct(maxSampleLocationGridSize, "max_sample_location_grid_size", VkExtent2D);
ZPUSH_END

ZINIT_BEGIN(VkMultisamplePropertiesEXT)
    (void)L; (void)p;
ZINIT_END

/*------------------------------------------------------------------------------*
 | Surface Capabilities                                                         |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkSurfaceCapabilitiesKHR)
    SetInteger(minImageCount, "min_image_count");
    SetInteger(maxImageCount, "max_image_count");
    if(p->currentExtent.width != (uint32_t)-1)
        SetStruct(currentExtent, "current_extent", VkExtent2D);
        /* width and height are either both -1 or ~=-1
         * the first case means 'not present', so instead of setting the width and height 
         * table fields with 0xffffffff or -1 (which are not the same, in lua_Integers),
         * we don't set current_extent at all
         */
    SetStruct(minImageExtent, "min_image_extent", VkExtent2D);
    SetStruct(maxImageExtent, "max_image_extent", VkExtent2D);
    SetInteger(maxImageArrayLayers, "max_image_array_layers");
    SetFlags(supportedTransforms, "supported_transforms");
    SetBits(currentTransform, "current_transform");
    SetFlags(supportedCompositeAlpha, "supported_composite_alpha");
    SetFlags(supportedUsageFlags, "supported_usage_flags");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkSharedPresentSurfaceCapabilitiesKHR)
    SetFlags(sharedPresentSupportedUsageFlags, "shared_present_supported_usage_flags");
LOCALPUSH_END

ZINIT_BEGIN(VkSurfaceCapabilities2KHR)
    EXTENSIONS_BEGIN
        ADDX(SHARED_PRESENT_SURFACE_CAPABILITIES_KHR, VkSharedPresentSurfaceCapabilitiesKHR);
    EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkSurfaceCapabilitiesKHR)
    lua_newtable(L);
    localpushVkSurfaceCapabilitiesKHR(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkSurfaceCapabilities2KHR)
    lua_newtable(L);
    localpushVkSurfaceCapabilitiesKHR(L, &p->surfaceCapabilities);
    XPUSH_BEGIN
        XCASE(SHARED_PRESENT_SURFACE_CAPABILITIES_KHR, VkSharedPresentSurfaceCapabilitiesKHR);
    XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Surface Format                                                               |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkSurfaceFormatKHR)
    SetEnum(format, "format", pushformat);
    SetEnum(colorSpace, "color_space", pushcolorspace);
LOCALPUSH_END

ZINIT_BEGIN(VkSurfaceFormat2KHR)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkSurfaceFormatKHR)
    lua_newtable(L);
    localpushVkSurfaceFormatKHR(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkSurfaceFormat2KHR)
    lua_newtable(L);
    localpushVkSurfaceFormatKHR(L, &p->surfaceFormat);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Surface Info                                                                 |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPhysicalDeviceSurfaceInfo2KHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceSurfaceInfo2KHR);
    GetSurface(surface, "surface");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Queue Family Properties                                                      |
 *------------------------------------------------------------------------------*/

//LOCALPUSH_BEGIN(VkQueueFamilyProperties)
static int localpushVkQueueFamilyProperties(lua_State *L, const VkQueueFamilyProperties *p, uint32_t index) {
    lua_pushinteger(L, index); lua_setfield(L, -2, "queue_family_index");
    SetFlags(queueFlags, "queue_flags");
    SetInteger(queueCount, "queue_count");
    SetInteger(timestampValidBits, "timestamp_valid_bits");
    SetStruct(minImageTransferGranularity, "min_image_transfer_granularity", VkExtent3D);
LOCALPUSH_END

ZINIT_BEGIN(VkQueueFamilyProperties2KHR)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

//ZPUSH_BEGIN(VkQueueFamilyProperties)
int zpushVkQueueFamilyProperties(lua_State *L, const VkQueueFamilyProperties *p, uint32_t index) {
    lua_newtable(L);
    localpushVkQueueFamilyProperties(L, p, index);
ZPUSH_END

//ZPUSH_BEGIN(VkQueueFamilyProperties2KHR)
int zpushVkQueueFamilyProperties2KHR(lua_State *L, const VkQueueFamilyProperties2KHR *p, uint32_t index) {
    lua_newtable(L);
    localpushVkQueueFamilyProperties(L, &p->queueFamilyProperties, index);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Memory Requirements                                                          |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkMemoryRequirements)
    SetInteger(size, "size");
    SetInteger(alignment, "alignment");
    SetInteger(memoryTypeBits, "memory_type_bits");
LOCALPUSH_END
LOCALPUSH_BEGIN(VkMemoryDedicatedRequirements)
    SetBoolean(prefersDedicatedAllocation, "prefers_dedicated_allocation");
    SetBoolean(requiresDedicatedAllocation, "requires_dedicated_allocation");
LOCALPUSH_END

ZINIT_BEGIN(VkMemoryRequirements2)
    EXTENSIONS_BEGIN
        ADDX(MEMORY_DEDICATED_REQUIREMENTS, VkMemoryDedicatedRequirements);
    EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkMemoryRequirements)
    lua_newtable(L);
    localpushVkMemoryRequirements(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkMemoryRequirements2)
    lua_newtable(L);
    localpushVkMemoryRequirements(L, &p->memoryRequirements);
    XPUSH_BEGIN
        XCASE(MEMORY_DEDICATED_REQUIREMENTS, VkMemoryDedicatedRequirements);
    XPUSH_END
ZPUSH_END


/*------------------------------------------------------------------------------*
 | Buffer Memory Requirements                                                   |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkBufferMemoryRequirementsInfo2KHR)
    checktable(arg);
    newstruct(VkBufferMemoryRequirementsInfo2KHR);
    /* p->buffer = set by caller */
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Image Memory Requirements                                                    |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkImagePlaneMemoryRequirementsInfoKHR)
    //checktable(arg);
    newstruct(VkImagePlaneMemoryRequirementsInfoKHR);
    GetFlags(planeAspect, "plane_aspect");
ZCHECK_END

ZCHECK_BEGIN(VkImageMemoryRequirementsInfo2KHR)
    checktable(arg);
    newstruct(VkImageMemoryRequirementsInfo2KHR);
    /* p->image = set by caller */
    EXTENSIONS_BEGIN
#define F "plane_aspect"
    if(ispresent(F))
        {
        VkImagePlaneMemoryRequirementsInfoKHR *p1
            = zcheckVkImagePlaneMemoryRequirementsInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Image Sparse Memory Requirements                                             |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkImageSparseMemoryRequirementsInfo2KHR)
    checktable(arg);
    newstruct(VkImageSparseMemoryRequirementsInfo2KHR);
    /* p->image = set by caller */
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Sparse Image Memory Requirements                                             |
 *------------------------------------------------------------------------------*/

LOCALPUSH_BEGIN(VkSparseImageMemoryRequirements)
    SetStruct(formatProperties, "format_properties", VkSparseImageFormatProperties);
    SetInteger(imageMipTailFirstLod, "image_mip_tail_first_lod");
    SetInteger(imageMipTailSize, "image_mip_tail_size");
    SetInteger(imageMipTailOffset, "image_mip_tail_offset");
    SetInteger(imageMipTailStride, "image_mip_tail_stride");
LOCALPUSH_END

ZINIT_BEGIN(VkSparseImageMemoryRequirements2)
    //EXTENSIONS_BEGIN
    //  ADDX(XXX, Xxx);
    //EXTENSIONS_END
ZINIT_END

ZPUSH_BEGIN(VkSparseImageMemoryRequirements)
    lua_newtable(L);
    localpushVkSparseImageMemoryRequirements(L, p);
ZPUSH_END

ZPUSH_BEGIN(VkSparseImageMemoryRequirements2)
    lua_newtable(L);
    localpushVkSparseImageMemoryRequirements(L, &p->memoryRequirements);
    //XPUSH_BEGIN
    //XPUSH_END
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Instance                                                                     |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkValidationFlagsEXT)
    if(p->pDisabledValidationChecks) freevalidationchecklist(L, p->pDisabledValidationChecks);
ZCLEAR_END
ZCHECK_BEGIN(VkValidationFlagsEXT)
    int arg1;
    newstruct(VkValidationFlagsEXT);
#define F "disabled_validation_checks"
    arg1 = pushfield(L, arg, F);
    p->pDisabledValidationChecks = 
        checkvalidationchecklist(L, arg1, &p->disabledValidationCheckCount, err);
    popfield(L, arg1);
    if(*err<0) { pushfielderror(F); return p; }
#undef F
ZCHECK_END

static ZCLEAR_BEGIN(VkApplicationInfo)
    if(p->pApplicationName) Free(L, (char*)p->pApplicationName);
    if(p->pEngineName) Free(L, (char*)p->pEngineName);
ZCLEAR_END
ZCHECK_BEGIN(VkApplicationInfo)
    checktable(arg);
    newstruct(VkApplicationInfo);
    GetStringOpt(pApplicationName, "application_name");
    GetInteger(applicationVersion, "application_version");
    GetStringOpt(pEngineName, "engine_name");
    GetInteger(engineVersion, "engine_version");
    GetInteger(apiVersion, "api_version");
ZCHECK_END

static ZCLEAR_BEGIN(VkInstanceCreateInfo)
    if(p->pApplicationInfo)
        zfreeVkApplicationInfo(L, p->pApplicationInfo, 1);
    if(p->ppEnabledLayerNames)
        freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames)
        freestringlist(L, (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
ZCLEAR_END

ZCHECK_BEGIN(VkInstanceCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkInstanceCreateInfo);
    GetFlags(flags, "flags");
#define F "application_info"
    arg1 = pushfield(L, arg, F);
    p->pApplicationInfo = zcheckVkApplicationInfo(L, arg1, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "enabled_layer_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledLayerNames =
        (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, err);
    popfield(L, arg1);
    if(*err < 0 && *err != ERR_EMPTY)
        { pushfielderror(F); return p; }
#undef F
#define F "enabled_extension_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledExtensionNames =
        (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, err);
    popfield(L, arg1);
    if(*err < 0 && *err != ERR_EMPTY)
        { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("disabled_validation_checks"))
        {
        VkValidationFlagsEXT *p1 = zcheckVkValidationFlagsEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Device                                                                       |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkDeviceQueueGlobalPriorityCreateInfoEXT)
    newstruct(VkDeviceQueueGlobalPriorityCreateInfoEXT);
    GetQueueGlobalPriority(globalPriority, "global_priority");
ZCHECK_END

static ZCLEAR_BEGIN(VkDeviceQueueCreateInfo)
    if(p->pQueuePriorities) Free(L, (void*)(p->pQueuePriorities));
ZCLEAR_END
ZCHECK_BEGIN(VkDeviceQueueCreateInfo)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkDeviceQueueCreateInfo);
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
#define F   "queue_priorities"
    arg1 = pushfield(L, arg, F);
    p->pQueuePriorities = (const float*)checkfloatlist(L, arg1, &count, err);
    p->queueCount = count;
    popfield(L, arg1);
    if(*err) { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
#define F "global_priority"
    if(ispresent(F))
        {
        VkDeviceQueueGlobalPriorityCreateInfoEXT *p1 =
            zcheckVkDeviceQueueGlobalPriorityCreateInfoEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END
ZCHECKARRAY(VkDeviceQueueCreateInfo) 

/*------------------------------------------------------------------------------*
 | Device Queue                                                                 |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDeviceCreateInfo)
    if(p->pQueueCreateInfos)
        zfreearrayVkDeviceQueueCreateInfo(L, p->pQueueCreateInfos, p->queueCreateInfoCount, 1);
    if(p->ppEnabledLayerNames)
        freestringlist(L, (char**)p->ppEnabledLayerNames, p->enabledLayerCount);
    if(p->ppEnabledExtensionNames)
        freestringlist(L,  (char**)p->ppEnabledExtensionNames, p->enabledExtensionCount);
    if(p->pEnabledFeatures) 
        zfreeVkPhysicalDeviceFeatures(L, p->pEnabledFeatures, 1);
ZCLEAR_END

//ZCHECK_BEGIN(VkDeviceCreateInfo)
VkDeviceCreateInfo* zcheckVkDeviceCreateInfo(lua_State *L, int arg, int *err, ud_t *ud) { //non-standard
    VkDeviceCreateInfo *p;
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkDeviceCreateInfo);
    GetFlags(flags, "flags");
#define F "queue_create_infos"
    arg1 = pushfield(L, arg, F);
    p->pQueueCreateInfos = zcheckarrayVkDeviceQueueCreateInfo(L, arg1, &count, err);
    p->queueCreateInfoCount = count;
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
#define F "enabled_layer_names" /* deprecated: aggiungere comunque */
    arg1 = pushfield(L, arg, F);
    p->ppEnabledLayerNames =
        (const char* const*)checkstringlist(L, arg1, &p->enabledLayerCount, err);
    popfield(L, arg1);
    if(*err < 0 && *err != ERR_EMPTY)
        { pushfielderror(F); return p; }
#undef F
#define F "enabled_extension_names"
    arg1 = pushfield(L, arg, F);
    p->ppEnabledExtensionNames =
        (const char* const*)checkstringlist(L, arg1, &p->enabledExtensionCount, err);
    popfield(L, arg1);
    if(*err < 0 && *err != ERR_EMPTY)
        { pushfielderror(F); return p; }
#undef F
#define F "enabled_features"
    arg1 = pushfield(L, arg, F);
    if(!ud->idt->GetPhysicalDeviceFeatures2KHR)
        {
        p->pEnabledFeatures = zcheckVkPhysicalDeviceFeatures(L, arg1, err);
        popfield(L, arg1);
        if(*err < 0) { prependfield(F); return p; }
        else if(*err == ERR_NOTPRESENT) poperror();
        }
#undef F
    EXTENSIONS_BEGIN
#define F "enabled_features"
    if(ud->idt->GetPhysicalDeviceFeatures2KHR)
        {
        VkPhysicalDeviceFeatures2 *p1 = zcheckVkPhysicalDeviceFeatures2(L, arg1, err);
        popfield(L, arg1);
        if(*err < 0) { prependfield(F); return p; }
        else if(*err == ERR_NOTPRESENT) poperror();
        else addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

ZCHECK_BEGIN(VkDeviceQueueInfo2)
    checktable(arg);
    newstruct(VkDeviceQueueInfo2);
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
    GetInteger(queueIndex, "queue_index");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Command Pool                                                                 |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkCommandPoolCreateInfo)
    checktable(arg);
    newstruct(VkCommandPoolCreateInfo);
    GetFlags(flags, "flags");
    GetInteger(queueFamilyIndex, "queue_family_index");
ZCHECK_END

ZCHECK_BEGIN(VkCommandBufferAllocateInfo)
    checktable(arg);
    newstruct(VkCommandBufferAllocateInfo);
    GetCommandBufferLevel(level, "level");
    GetInteger(commandBufferCount, "command_buffer_count");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Command Buffer                                                               |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkCommandBufferInheritanceConditionalRenderingInfoEXT)
    //checktable(arg);
    newstruct(VkCommandBufferInheritanceConditionalRenderingInfoEXT);
    GetBoolean(conditionalRenderingEnable, "conditional_rendering_enable");
ZCHECK_END

ZCHECK_BEGIN(VkCommandBufferInheritanceInfo)
    checktable(arg);
    newstruct(VkCommandBufferInheritanceInfo);
    GetRenderPassOpt(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetFramebufferOpt(framebuffer, "framebuffer");
    GetBoolean(occlusionQueryEnable, "occlusion_query_enable");
    GetFlags(queryFlags, "query_flags");
    GetFlags(pipelineStatistics, "pipeline_statistics");
    EXTENSIONS_BEGIN
    if(ispresent("conditional_rendering_enable"))
        {
        VkCommandBufferInheritanceConditionalRenderingInfoEXT* p1 =
            zcheckVkCommandBufferInheritanceConditionalRenderingInfoEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

ZCHECK_BEGIN(VkCommandBufferBeginInfo)
    int arg1;
    checktable(arg);
    newstruct(VkCommandBufferBeginInfo);
    GetFlags(flags, "flags");
#define F "inheritance_info"
    arg1 = pushfield(L, arg, F);
    p->pInheritanceInfo = zcheckVkCommandBufferInheritanceInfo(L, arg1, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Device Memory                                                                |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkMemoryDedicatedAllocateInfoKHR)
    //checktable(arg);
    newstruct(VkMemoryDedicatedAllocateInfoKHR);
    GetImageOpt(image, "image");
    GetBufferOpt(buffer, "buffer");
ZCHECK_END

ZCHECK_BEGIN(VkExportMemoryAllocateInfoKHR)
    //checktable(arg);
    newstruct(VkExportMemoryAllocateInfoKHR);
    GetFlags(handleTypes, "handle_types");
ZCHECK_END

ZCHECK_BEGIN(VkImportMemoryFdInfoKHR)
    //checktable(arg);
    newstruct(VkImportMemoryFdInfoKHR);
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
    GetInteger(fd, "fd");
ZCHECK_END

ZCHECK_BEGIN(VkMemoryAllocateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkMemoryAllocateInfo);
    GetInteger(allocationSize, "allocation_size");
    GetInteger(memoryTypeIndex, "memory_type_index");
    EXTENSIONS_BEGIN
    if(ispresent("image") || ispresent("buffer"))
        {
        VkMemoryDedicatedAllocateInfoKHR *p1 = zcheckVkMemoryDedicatedAllocateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    if(ispresent("handle_types"))
        {
        VkExportMemoryAllocateInfoKHR *p1 = zcheckVkExportMemoryAllocateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#define F "import_memory_fd_info"
        {
        VkImportMemoryFdInfoKHR *p1;
        arg1 = pushfield(L, arg, F);
        p1 = zcheckVkImportMemoryFdInfoKHR(L, arg1, err);
        popfield(L, arg1);
        if(*err<0) { prependfield(F); return p; }
        else if(*err == ERR_NOTPRESENT) poperror();
        else addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*/

ZPUSH_BEGIN(VkMemoryFdPropertiesKHR)
    lua_newtable(L);
    SetInteger(memoryTypeBits, "memory_type_bits");
ZPUSH_END

/*------------------------------------------------------------------------------*/
ZCHECK_BEGIN(VkMemoryGetFdInfoKHR)
    checktable(arg);
    newstruct(VkMemoryGetFdInfoKHR);
    /* p->memory = set by caller */
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Buffer                                                                       |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkExternalMemoryBufferCreateInfo)
    //checktable(arg);
    newstruct(VkExternalMemoryBufferCreateInfo);
    GetFlags(handleTypes, "handle_types");
ZCHECK_END

static ZCLEAR_BEGIN(VkBufferCreateInfo)
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
ZCLEAR_END

ZCHECK_BEGIN(VkBufferCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkBufferCreateInfo);
    GetFlags(flags, "flags");
    GetInteger(size, "size");
    GetFlags(usage, "usage");
    GetSharingMode(sharingMode, "sharing_mode");
#define F "queue_family_indices"
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("handle_types"))
        {
        VkExternalMemoryBufferCreateInfo *p1 = zcheckVkExternalMemoryBufferCreateInfo(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Buffer View                                                                  |
 *------------------------------------------------------------------------------*/
    
ZCHECK_BEGIN(VkBufferViewCreateInfo)
    checktable(arg);
    newstruct(VkBufferViewCreateInfo);
    GetFlags(flags, "flags");
/*  p->buffer = set by caller */
    GetFormat(format, "format");
    GetInteger(offset, "offset");
    GetInteger(range, "range");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Image                                                                        |
 *------------------------------------------------------------------------------*/
    
static ZCLEAR_BEGIN(VkImageFormatListCreateInfoKHR)
    if(p->pViewFormats)
        if(p->pViewFormats) freeformatlist(L, p->pViewFormats);
ZCLEAR_END
ZCHECK_BEGIN(VkImageFormatListCreateInfoKHR)
    //checktable(arg);
    newstruct(VkImageFormatListCreateInfoKHR);
    p->pViewFormats = checkformatlist(L, arg, &p->viewFormatCount, err);
    if(*err==ERR_NOTPRESENT) *err=ERR_GENERIC; //@@ mah
    if(*err<0) { pusherror(); return p; }
ZCHECK_END

ZCHECK_BEGIN(VkExternalMemoryImageCreateInfoKHR)
    //checktable(arg);
    newstruct(VkExternalMemoryImageCreateInfoKHR);
    GetFlags(handleTypes, "handle_types");
ZCHECK_END

static ZCLEAR_BEGIN(VkImageCreateInfo)
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
ZCLEAR_END
ZCHECK_BEGIN(VkImageCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkImageCreateInfo);
    GetFlags(flags, "flags");
    GetImageType(imageType, "image_type");
    GetFormat(format, "format");
    GetStructOpt(extent, "extent", VkExtent3D);
    GetIntegerOpt(mipLevels, "mip_levels", 1);
    GetIntegerOpt(arrayLayers, "array_layers", 1);
    GetSamples(samples, "samples");
    GetImageTiling(tiling, "tiling");
    GetFlags(usage, "usage");
    GetImageLayout(initialLayout, "initial_layout");
    GetSharingMode(sharingMode, "sharing_mode");
#define F "queue_family_indices"
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
#define F "handle_types"
    if(ispresent(F))
        {
        VkExternalMemoryImageCreateInfoKHR *p1 = zcheckVkExternalMemoryImageCreateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
#define F "view_formats"
    if(ispresent(F))
        {
        VkImageFormatListCreateInfoKHR *p1;
        arg1 = pushfield(L, arg, F);
        p1 = zcheckVkImageFormatListCreateInfoKHR(L, arg1, err);
        popfield(L, arg1);
        if(*err) { zfree(L, p1, 1);  pushfielderror(F); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Image View                                                                   |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkImageViewUsageCreateInfoKHR)
    //checktable(arg);
    newstruct(VkImageViewUsageCreateInfoKHR);
    GetFlags(usage, "usage");
ZCHECK_END

ZCHECK_BEGIN(VkImageViewCreateInfo)
    checktable(arg);
    newstruct(VkImageViewCreateInfo);
    GetFlags(flags, "flags");
/*  p->image = set by caller */
    GetImageViewType(viewType, "view_type");
    GetFormat(format, "format");
    GetStructOpt(components, "components", VkComponentMapping);
    GetStructOpt(subresourceRange, "subresource_range", VkImageSubresourceRange);
    EXTENSIONS_BEGIN
    if(ispresent("usage"))
        {
        VkImageViewUsageCreateInfoKHR *p1 = zcheckVkImageViewUsageCreateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Descriptor Pool                                                              |
 *------------------------------------------------------------------------------*/
    
static ZCLEAR_BEGIN(VkDescriptorPoolCreateInfo)
    if(p->pPoolSizes)
        zfreearrayVkDescriptorPoolSize(L, p->pPoolSizes, p->poolSizeCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkDescriptorPoolCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkDescriptorPoolCreateInfo);
    GetFlags(flags, "flags");
    GetInteger(maxSets, "max_sets");
#define F "pool_sizes"
    arg1 = pushfield(L, arg, F);
    p->pPoolSizes = zcheckarrayVkDescriptorPoolSize(L, arg1, &p->poolSizeCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Descriptor Set                                                               |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDescriptorSetAllocateInfo)
    if(p->pSetLayouts) Free(L, (void*)p->pSetLayouts);
ZCLEAR_END
ZCHECK_BEGIN(VkDescriptorSetAllocateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkDescriptorSetAllocateInfo);
    /* p->descriptorPool = set by caller */
#define F "set_layouts"
    arg1 = pushfield(L, arg, F);
    p->pSetLayouts = checkdescriptor_set_layoutlist(L, arg1, &p->descriptorSetCount, err, NULL);
    popfield(L, arg1);
    if(*err) { pushfielderror(F); return p; }
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Descriptor Set Layout                                                        |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDescriptorSetLayoutCreateInfo)
    if(p->pBindings)
        zfreearrayVkDescriptorSetLayoutBinding(L, p->pBindings, p->bindingCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkDescriptorSetLayoutCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkDescriptorSetLayoutCreateInfo);
    GetFlags(flags, "flags");
#define F "bindings"
    arg1 = pushfield(L, arg, F);
    p->pBindings = zcheckarrayVkDescriptorSetLayoutBinding(L, arg1, &p->bindingCount, err);
    popfield(L, arg1);
    if(*err<0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Pipeline Layout                                                              |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineLayoutCreateInfo)
    if(p->pSetLayouts) Free(L, (void*)p->pSetLayouts);
    if(p->pPushConstantRanges) 
        zfreearrayVkPushConstantRange(L, p->pPushConstantRanges, p->pushConstantRangeCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineLayoutCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineLayoutCreateInfo);
    GetFlags(flags, "flags");
#define F "set_layouts"
    arg1 = pushfield(L, arg, F);
    p->pSetLayouts = checkdescriptor_set_layoutlist(L, arg1, &p->setLayoutCount, err, NULL);
    popfield(L, arg1);
    if(*err<0) { pushfielderror(F); return p; }
#undef F
#define F "push_constant_ranges"
    arg1 = pushfield(L, arg, F);
    p->pPushConstantRanges = zcheckarrayVkPushConstantRange(L, arg1, &p->pushConstantRangeCount, err);
    popfield(L, arg1);
    if(*err<0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Query Pool                                                                   |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkQueryPoolCreateInfo)
    checktable(arg);
    newstruct(VkQueryPoolCreateInfo);
    GetFlags(flags, "flags");
    GetQueryType(queryType, "query_type");
    GetInteger(queryCount, "query_count");
    GetFlags(pipelineStatistics, "pipeline_statistics");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Render Pass                                                                  |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkRenderPassInputAttachmentAspectCreateInfoKHR)
    if(p->pAspectReferences)
        zfreearrayVkInputAttachmentAspectReference(L, p->pAspectReferences, p->aspectReferenceCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkRenderPassInputAttachmentAspectCreateInfoKHR)
    //checktable(arg);
    newstruct(VkRenderPassInputAttachmentAspectCreateInfoKHR);
    p->pAspectReferences = 
        zcheckarrayVkInputAttachmentAspectReference(L, arg, &p->aspectReferenceCount, err);
    if(*err) return p;
ZCHECK_END

static ZCLEAR_BEGIN(VkRenderPassCreateInfo)
    if(p->pAttachments)
        zfreearrayVkAttachmentDescription(L, p->pAttachments, p->attachmentCount, 1);
    if(p->pSubpasses)
        zfreearrayVkSubpassDescription(L, p->pSubpasses, p->subpassCount, 1);
    if(p->pDependencies) 
        zfreearrayVkSubpassDependency(L, p->pDependencies, p->dependencyCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkRenderPassCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkRenderPassCreateInfo);
    GetFlags(flags, "flags");
#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = zcheckarrayVkAttachmentDescription(L, arg1, &p->attachmentCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "subpasses"
    arg1 = pushfield(L, arg, F);
    p->pSubpasses = zcheckarrayVkSubpassDescription(L, arg1, &p->subpassCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
#undef F
#define F "dependencies"
    arg1 = pushfield(L, arg, F);
    p->pDependencies = zcheckarrayVkSubpassDependency(L, arg1, &p->dependencyCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
    EXTENSIONS_BEGIN
#define F "input_attachment_aspect_references"
    VkRenderPassInputAttachmentAspectCreateInfoKHR *p1;
    arg1 = pushfield(L, arg, F);
    p1 = zcheckVkRenderPassInputAttachmentAspectCreateInfoKHR(L, arg1, err);
    popfield(L, arg1);
    if(*err < 0) { zfree(L, p1, 1); prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
    else addtochain(chain, p1);
#undef F
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Framebuffer                                                                  |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkFramebufferCreateInfo)
    if(p->pAttachments) Free(L, (void*)p->pAttachments);
ZCLEAR_END
ZCHECK_BEGIN(VkFramebufferCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkFramebufferCreateInfo);
    GetFlags(flags, "flags");
    GetRenderPass(renderPass, "render_pass");
    GetInteger(width, "width");
    GetInteger(height, "height");
    GetIntegerOpt(layers, "layers", 1);
#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = checkimage_viewlist(L, arg1, &p->attachmentCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Shader Module                                                                |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkShaderModuleValidationCacheCreateInfoEXT)
    //checktable(arg);
    newstruct(VkShaderModuleValidationCacheCreateInfoEXT);
    GetValidationCache(validationCache, "validation_cache");
ZCHECK_END

ZCHECK_BEGIN(VkShaderModuleCreateInfo)
    checktable(arg);
    newstruct(VkShaderModuleCreateInfo);
    GetFlags(flags, "flags");
    /* p->pCode, p->codeSize: retrieved by the caller */
    EXTENSIONS_BEGIN
#define F "validation_cache"
    if(ispresent(F))
        {
        VkShaderModuleValidationCacheCreateInfoEXT *p1 =
            zcheckVkShaderModuleValidationCacheCreateInfoEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Swapchain                                                                    |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkSwapchainCounterCreateInfoEXT)
    //checktable(arg);
    newstruct(VkSwapchainCounterCreateInfoEXT);
    GetFlags(surfaceCounters, "surface_counters");
ZCHECK_END

static ZCLEAR_BEGIN(VkSwapchainCreateInfoKHR)
    if(p->pQueueFamilyIndices) Free(L, (void*)p->pQueueFamilyIndices);
ZCLEAR_END
ZCHECK_BEGIN(VkSwapchainCreateInfoKHR)
    int arg1;
    checktable(arg);
    newstruct(VkSwapchainCreateInfoKHR);
    GetFlags(flags, "flags");
    GetSurface(surface, "surface");
    GetInteger(minImageCount, "min_image_count");
    GetFormat(imageFormat, "image_format");
    GetColorSpace(imageColorSpace, "image_color_space");
    GetStructOpt(imageExtent, "image_extent", VkExtent2D);
    GetInteger(imageArrayLayers, "image_array_layers");
    GetFlags(imageUsage, "image_usage");
    GetSharingMode(imageSharingMode, "image_sharing_mode");
    GetBits(preTransform, "pre_transform", VkSurfaceTransformFlagBitsKHR);
    GetBits(compositeAlpha, "composite_alpha", VkCompositeAlphaFlagBitsKHR);
    GetPresentMode(presentMode, "present_mode");
    GetBoolean(clipped, "clipped");
    GetSwapchainOpt(oldSwapchain, "old_swapchain");
#define F "queue_family_indices"
    arg1 = pushfield(L, arg, F);
    p->pQueueFamilyIndices = checkuint32list(L, arg1, &p->queueFamilyIndexCount, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("surface_counters"))
        {
        VkSwapchainCounterCreateInfoEXT *p1 = zcheckVkSwapchainCounterCreateInfoEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END
ZCHECKARRAY(VkSwapchainCreateInfoKHR)

/*------------------------------------------------------------------------------*
 | Pipeline Cache                                                               |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineCacheCreateInfo)
    checktable(arg);
    newstruct(VkPipelineCacheCreateInfo);
    GetFlags(flags, "flags");
    /* p->pInitialData, p->initialDataSize: set (and free'd) by the caller */
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Validation Cache                                                             |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkValidationCacheCreateInfoEXT)
    checktable(arg);
    newstruct(VkValidationCacheCreateInfoEXT);
    GetFlags(flags, "flags");
    /* p->pInitialData, p->initialDataSize: set (and free'd) by the caller */
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Sampler                                                                      |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkSamplerReductionModeCreateInfoEXT)
    //checktable(arg);
    newstruct(VkSamplerReductionModeCreateInfoEXT);
    GetSamplerReductionMode(reductionMode, "reduction_mode");
ZCHECK_END

ZCHECK_BEGIN(VkSamplerYcbcrConversionInfoKHR)
    //checktable(arg);
    newstruct(VkSamplerYcbcrConversionInfoKHR);
    GetSamplerYcbcrConversion(conversion, "conversion");
ZCHECK_END

ZCHECK_BEGIN(VkSamplerCreateInfo)
    checktable(arg);
    newstruct(VkSamplerCreateInfo);
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
    EXTENSIONS_BEGIN
    if(ispresent("reduction_mode"))
        {
        VkSamplerReductionModeCreateInfoEXT *p1 =
            zcheckVkSamplerReductionModeCreateInfoEXT(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    if(ispresent("conversion"))
        {
        VkSamplerYcbcrConversionInfoKHR *p1 =
            zcheckVkSamplerYcbcrConversionInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Sampler Ycbcr                                                                |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkSamplerYcbcrConversionCreateInfoKHR)
    checktable(arg);
    newstruct(VkSamplerYcbcrConversionCreateInfoKHR);
    GetFormat(format, "format");
    GetSamplerYcbcrModelConversion(ycbcrModel, "ycbcr_model");
    GetSamplerYcbcrRange(ycbcrRange, "ycbcr_range");
    GetStructOpt(components, "components", VkComponentMapping);
    GetChromaLocation(xChromaOffset, "x_chroma_offset");
    GetChromaLocation(yChromaOffset, "y_chroma_offset");
    GetFilter(chromaFilter, "chroma_filter");
    GetBoolean(forceExplicitReconstruction, "force_explicit_reconstruction");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Event                                                                        |
 *------------------------------------------------------------------------------*/
    
ZCHECK_BEGIN(VkEventCreateInfo)
    checktable(arg);
    newstruct(VkEventCreateInfo);
    GetFlags(flags, "flags");
ZCHECK_END

ZCHECK_BEGIN(VkDeviceEventInfoEXT)
    checktable(arg);
    newstruct(VkDeviceEventInfoEXT);
    GetDeviceEventType(deviceEvent, "device_event");
ZCHECK_END

ZCHECK_BEGIN(VkDisplayEventInfoEXT)
    checktable(arg);
    newstruct(VkDisplayEventInfoEXT);
    GetDisplayEventType(displayEvent, "display_event");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Fence                                                                        |
 *------------------------------------------------------------------------------*/
    
ZCHECK_BEGIN(VkExportFenceCreateInfoKHR)
    //checktable(arg);
    newstruct(VkExportFenceCreateInfoKHR);
    GetFlags(handleTypes, "handle_types");
ZCHECK_END

ZCHECK_BEGIN(VkFenceCreateInfo)
    checktable(arg);
    newstruct(VkFenceCreateInfo);
    GetFlags(flags, "flags");
    EXTENSIONS_BEGIN
    if(ispresent("handle_types"))
        {
        VkExportFenceCreateInfoKHR *p1 = zcheckVkExportFenceCreateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

ZCHECK_BEGIN(VkImportFenceFdInfoKHR)
    checktable(arg);
    newstruct(VkImportFenceFdInfoKHR);
    /* p->fence is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
    GetInteger(fd, "fd");
ZCHECK_END

ZCHECK_BEGIN(VkFenceGetFdInfoKHR)
    checktable(arg);
    newstruct(VkFenceGetFdInfoKHR);
    /* p->fence is set by the caller */
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Semaphore                                                                    |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkExportSemaphoreCreateInfoKHR)
    //checktable(arg);
    newstruct(VkExportSemaphoreCreateInfoKHR);
    GetFlags(handleTypes, "handle_types");
ZCHECK_END

ZCHECK_BEGIN(VkSemaphoreCreateInfo)
    checktable(arg);
    newstruct(VkSemaphoreCreateInfo);
    GetFlags(flags, "flags");
    EXTENSIONS_BEGIN
    if(ispresent("handle_types"))
        {
        VkExportSemaphoreCreateInfoKHR *p1 = zcheckVkExportSemaphoreCreateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

ZCHECK_BEGIN(VkImportSemaphoreFdInfoKHR)
    checktable(arg);
    newstruct(VkImportSemaphoreFdInfoKHR);
    /* p->semaphore is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    GetInteger(fd, "fd");
ZCHECK_END

ZCHECK_BEGIN(VkSemaphoreGetFdInfoKHR)
    checktable(arg);
    newstruct(VkSemaphoreGetFdInfoKHR);
    /* p->semaphore is set by the caller */
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Memory Barrier                                                               |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkMemoryBarrier)
    checktable(arg);
    newstruct(VkMemoryBarrier);
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
ZCHECK_END
ZCHECKARRAY(VkMemoryBarrier)

/*------------------------------------------------------------------------------*
 | Buffer Memory Barrier                                                        |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkBufferMemoryBarrier)
    checktable(arg);
    newstruct(VkBufferMemoryBarrier);
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetIntegerOpt(srcQueueFamilyIndex, "src_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetIntegerOpt(dstQueueFamilyIndex, "dst_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetInteger(size, "size");
ZCHECK_END
ZCHECKARRAY(VkBufferMemoryBarrier)

/*------------------------------------------------------------------------------*
 | Image Memory Barrier                                                         |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkImageMemoryBarrier)
    checktable(arg);
    newstruct(VkImageMemoryBarrier);
    GetFlags(srcAccessMask, "src_access_mask");
    GetFlags(dstAccessMask, "dst_access_mask");
    GetImageLayout(oldLayout, "old_layout");
    GetImageLayout(newLayout, "new_layout");
    GetIntegerOpt(srcQueueFamilyIndex, "src_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetIntegerOpt(dstQueueFamilyIndex, "dst_queue_family_index", VK_QUEUE_FAMILY_IGNORED);
    GetImage(image, "image");
    GetStructOpt(subresourceRange, "subresource_range", VkImageSubresourceRange);
ZCHECK_END
ZCHECKARRAY(VkImageMemoryBarrier)

/*------------------------------------------------------------------------------*
 | Display Surface                                                              |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkDisplaySurfaceCreateInfoKHR)
    checktable(arg);
    newstruct(VkDisplaySurfaceCreateInfoKHR);
    GetFlags(flags, "flags");
/*  p->displayMode = set by caller */
    GetInteger(planeIndex, "plane_index");
    GetInteger(planeStackIndex, "plane_stack_index");
    GetBits(transform, "transform", VkSurfaceTransformFlagBitsKHR);
    GetNumber(globalAlpha, "global_alpha");
    GetBits(alphaMode, "alpha_mode", VkDisplayPlaneAlphaFlagBitsKHR);
    GetStruct(imageExtent, "image_extent", VkExtent2D);
ZCHECK_END
 
/*------------------------------------------------------------------------------*
 | Display Mode                                                                 |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDisplayModeCreateInfoKHR)
    zfreeVkDisplayModeParametersKHR(L, &p->parameters, 0);
ZCLEAR_END
ZCHECK_BEGIN(VkDisplayModeCreateInfoKHR)
    checktable(arg);
    newstruct(VkDisplayModeCreateInfoKHR);
    GetFlags(flags, "flags");
    GetStruct(parameters, "parameters", VkDisplayModeParametersKHR);
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Descriptor Update Template                                                   |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDescriptorUpdateTemplateCreateInfoKHR)
    if(p->pDescriptorUpdateEntries)
        zfreearrayVkDescriptorUpdateTemplateEntry(L, p->pDescriptorUpdateEntries, p->descriptorUpdateEntryCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkDescriptorUpdateTemplateCreateInfoKHR)
    int arg1;
    checktable(arg);
    newstruct(VkDescriptorUpdateTemplateCreateInfoKHR);
    GetFlags(flags, "flags");
    GetDescriptorUpdateTemplateType(templateType, "template_type");
    GetDescriptorSetLayoutOpt(descriptorSetLayout, "descriptor_set_layout");
    GetPipelineBindPoint(pipelineBindPoint, "pipeline_bind_point");
    GetPipelineLayoutOpt(pipelineLayout, "pipeline_layout");
    GetFlags(set, "set");
#define F "descriptor_update_entries"
    arg1 = pushfield(L, arg, F);
    p->pDescriptorUpdateEntries =
        zcheckarrayVkDescriptorUpdateTemplateEntry(L, arg1, &p->descriptorUpdateEntryCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Submit Info                                                                  |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkProtectedSubmitInfo)
	//checktable(arg);
    newstruct(VkProtectedSubmitInfo);
	GetBoolean(protectedSubmit, "protected_submit");
ZCHECK_END

static ZCLEAR_BEGIN(VkSubmitInfo)
    if(p->pWaitSemaphores) Free(L, (void*)p->pWaitSemaphores);
    if(p->pWaitDstStageMask) Free(L, (void*)p->pWaitDstStageMask);
    if(p->pCommandBuffers) Free(L, (void*)p->pCommandBuffers);
    if(p->pSignalSemaphores) Free(L, (void*)p->pSignalSemaphores);
ZCLEAR_END
ZCHECK_BEGIN(VkSubmitInfo)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkSubmitInfo);
#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &p->waitSemaphoreCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
#define F "wait_dst_stage_mask"
    arg1 = pushfield(L, arg, F);
    p->pWaitDstStageMask = checkflaglist(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
    if(count != p->waitSemaphoreCount) 
        { *err=ERR_LENGTH; pushfielderror(F); return p; }
#undef F
#define F "command_buffers"
    arg1 = pushfield(L, arg, F);
    p->pCommandBuffers = checkcommand_bufferlist(L, arg1, &p->commandBufferCount, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
#define F "signal_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &p->signalSemaphoreCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("protected_submit"))
        {
        VkProtectedSubmitInfo *p1 = zcheckVkProtectedSubmitInfo(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END
ZCHECKARRAY(VkSubmitInfo)

/*------------------------------------------------------------------------------*
 | Present Info                                                                 |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPresentRegionsKHR)
    if(p->pRegions)
        zfreearrayVkPresentRegionKHR(L, p->pRegions, p->swapchainCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPresentRegionsKHR)
    int arg1;
    checktable(arg);
    newstruct(VkPresentRegionsKHR);
#define F "regions"
    arg1 = pushfield(L, arg, F);
    p->pRegions = zcheckarrayVkPresentRegionKHR(L, arg1, &p->swapchainCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

ZCHECK_BEGIN(VkDisplayPresentInfoKHR)
    //checktable(arg);
    newstruct(VkDisplayPresentInfoKHR);
    GetStructOpt(srcRect, "src_rect", VkRect2D);
    GetStructOpt(dstRect, "dst_rect", VkRect2D);
    GetBoolean(persistent, "persistent");
ZCHECK_END

static ZCLEAR_BEGIN(VkPresentInfoKHR)
    if(p->pWaitSemaphores) Free(L, (void*)p->pWaitSemaphores);
    if(p->pSwapchains) Free(L, (void*)p->pSwapchains);
    if(p->pImageIndices) Free(L, (void*)p->pImageIndices);
    if(p->pResults) Free(L, (void*)p->pResults);
ZCLEAR_END
//ZCHECK_BEGIN(VkPresentInfoKHR)
VkPresentInfoKHR* zcheckVkPresentInfoKHR(lua_State *L, int arg, int *err, int results) { //non-standard
    VkPresentInfoKHR *p;
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkPresentInfoKHR);
#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &p->waitSemaphoreCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
#define F "swapchains"
    arg1 = pushfield(L, arg, F);
    p->pSwapchains = checkswapchainlist(L, arg1, &p->swapchainCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
    if(results) /* allocate memory for per-swapchain results */
        {
        p->pResults = (VkResult*)MallocNoErr(L, sizeof(VkResult)*(p->swapchainCount));
        if(!p->pResults) { *err=ERR_MEMORY; pusherror(); return p; }
        }
#undef F
#define F "image_indices"
    arg1 = pushfield(L, arg, F);
    p->pImageIndices = checkuint32list(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err)
        { pushfielderror(F); return p; }
    if(p->swapchainCount != count)
        { *err=ERR_LENGTH; pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("src_rect"))
        {
        VkDisplayPresentInfoKHR *p1 = zcheckVkDisplayPresentInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    if(ispresent("regions"))
        {
        VkPresentRegionsKHR *p1 = zcheckVkPresentRegionsKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END


/*------------------------------------------------------------------------------*
 | Render Pass Begin Info                                                       |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkRenderPassSampleLocationsBeginInfoEXT)
    if(p->pAttachmentInitialSampleLocations) zfreearrayVkAttachmentSampleLocationsEXT(L,
            p->pAttachmentInitialSampleLocations, p->attachmentInitialSampleLocationsCount, 1);
    if(p->pPostSubpassSampleLocations) zfreearrayVkSubpassSampleLocationsEXT(L,
            p->pPostSubpassSampleLocations, p->postSubpassSampleLocationsCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkRenderPassSampleLocationsBeginInfoEXT)
    int arg1;
    //checktable(arg);
    newstruct(VkRenderPassSampleLocationsBeginInfoEXT);
#define F "attachment_initial_sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pAttachmentInitialSampleLocations = zcheckarrayVkAttachmentSampleLocationsEXT(L,
            arg1, &p->attachmentInitialSampleLocationsCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err==ERR_NOTPRESENT) poperror();
#undef F
#define F "post_subpass_sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pPostSubpassSampleLocations = zcheckarrayVkSubpassSampleLocationsEXT(L,
            arg1, &p->postSubpassSampleLocationsCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    else if(*err==ERR_NOTPRESENT) poperror();
#undef F
#if 0 //no: validation layer should check this:
    if((p->attachmentInitialSampleLocationsCount + p->postSubpassSampleLocationsCount) == 0)
        { zfree(L, p, 1); *err=ERR_NOTPRESENT; pusherror(); return NULL; }
#endif
ZCHECK_END

static ZCLEAR_BEGIN(VkRenderPassBeginInfo)
    if(p->pClearValues) zfreearrayVkClearValue(L, p->pClearValues, p->clearValueCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkRenderPassBeginInfo)
    int arg1;
    checktable(arg);
    newstruct(VkRenderPassBeginInfo);
    GetRenderPass(renderPass, "render_pass");
    GetFramebuffer(framebuffer, "framebuffer");
    GetStructOpt(renderArea, "render_area", VkRect2D);
#define F "clear_values"
    arg1 = pushfield(L, arg, F);
    p->pClearValues = zcheckarrayVkClearValue(L, arg1, &p->clearValueCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
    EXTENSIONS_BEGIN
    VkRenderPassSampleLocationsBeginInfoEXT *p1 =
        zcheckVkRenderPassSampleLocationsBeginInfoEXT(L, arg, err);
    if(*err < 0) { zfree(L, p1, 1); return p; }
    else if(*err == ERR_NOTPRESENT) { zfree(L, p1, 1); poperror(); }
    else addtochain(chain, p1);
    EXTENSIONS_END
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Bind Sparse Info                                                             |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkBindSparseInfo)
    if(p->pWaitSemaphores)
        Free(L, (void*)p->pWaitSemaphores);
    if(p->pBufferBinds)
        zfreearrayVkSparseBufferMemoryBindInfo(L, p->pBufferBinds, p->bufferBindCount, 1);
    if(p->pImageOpaqueBinds)
        zfreearrayVkSparseImageOpaqueMemoryBindInfo(L, p->pImageOpaqueBinds, p->imageOpaqueBindCount, 1);
    if(p->pImageBinds) 
        zfreearrayVkSparseImageMemoryBindInfo(L, p->pImageBinds, p->imageBindCount, 1);
    if(p->pSignalSemaphores)
        Free(L, (void*)p->pSignalSemaphores);
ZCLEAR_END
ZCHECK_BEGIN(VkBindSparseInfo)
    int arg1;
    checktable(arg);
    newstruct(VkBindSparseInfo);
#define F "wait_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pWaitSemaphores = checksemaphorelist(L, arg1, &p->waitSemaphoreCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
#define F "buffer_binds"
    arg1 = pushfield(L, arg, F);
    p->pBufferBinds = zcheckarrayVkSparseBufferMemoryBindInfo(L, arg1, &p->bufferBindCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "image_opaque_binds"
    arg1 = pushfield(L, arg, F);
    p->pImageOpaqueBinds = 
        zcheckarrayVkSparseImageOpaqueMemoryBindInfo(L, arg1, &p->imageOpaqueBindCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "image_binds"
    arg1 = pushfield(L, arg, F);
    p->pImageBinds = zcheckarrayVkSparseImageMemoryBindInfo(L, arg1, &p->imageBindCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "signal_semaphores"
    arg1 = pushfield(L, arg, F);
    p->pSignalSemaphores = checksemaphorelist(L, arg1, &p->signalSemaphoreCount, err, NULL);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkBindSparseInfo)

/*------------------------------------------------------------------------------*
 | Write Descriptor Set                                                         |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkWriteDescriptorSet)
    if(!p->pImageInfo) 
        zfreearrayVkDescriptorImageInfo(L, p->pImageInfo, p->descriptorCount, 1);
    if(!p->pBufferInfo)
        zfreearrayVkDescriptorBufferInfo(L, p->pBufferInfo, p->descriptorCount, 1);
    if(!p->pTexelBufferView)
        Free(L, (void*)p->pTexelBufferView);
ZCLEAR_END
ZCHECK_BEGIN(VkWriteDescriptorSet)
    int arg1;
    checktable(arg);
    newstruct(VkWriteDescriptorSet);
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
            p->pImageInfo = zcheckarrayVkDescriptorImageInfo(L, arg1, &p->descriptorCount, err);
            popfield(L, arg1);
            if(*err) { prependfield(F); return p; }
            break;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
#define F "buffer_info"
            arg1 = pushfield(L, arg, F);
            p->pBufferInfo = zcheckarrayVkDescriptorBufferInfo(L, arg1, &p->descriptorCount, err);
            popfield(L, arg1);
            if(*err) { prependfield(F); return p; }
            break;
#undef F
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
#define F "texel_buffer_view"
            arg1 = pushfield(L, arg, F);
            p->pTexelBufferView = checkbuffer_viewlist(L, arg1, &p->descriptorCount, err, NULL);
            popfield(L, arg1);
            if(*err == ERR_NOTPRESENT) poperror();
            else if(*err) { pushfielderror(F); return p; }
            break;
#undef F
        default:
            unexpected(L); /* unhandled descriptorType ? */
        }
ZCHECK_END
ZCHECKARRAY(VkWriteDescriptorSet)

/*------------------------------------------------------------------------------*
 | Copy Descriptor Set                                                          |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkCopyDescriptorSet)
    checktable(arg);
    newstruct(VkCopyDescriptorSet);
    GetDescriptorSet(srcSet, "src_set");
    GetInteger(srcBinding, "src_binding");
    GetInteger(srcArrayElement, "src_array_element");
    GetDescriptorSet(dstSet, "dst_set");
    GetInteger(dstBinding, "dst_binding");
    GetInteger(dstArrayElement, "dst_array_element");
    GetInteger(descriptorCount, "descriptor_count");
ZCHECK_END
ZCHECKARRAY(VkCopyDescriptorSet)

/*------------------------------------------------------------------------------*
 | Bind Buffer Memory                                                           |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkBindBufferMemoryInfo)
    checktable(arg);
    newstruct(VkBindBufferMemoryInfo);
    GetBuffer(buffer, "buffer");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "offset");
ZCHECK_END
ZCHECKARRAY(VkBindBufferMemoryInfo)

/*------------------------------------------------------------------------------*
 | Bind Image Memory                                                            |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkBindImagePlaneMemoryInfoKHR)
    //checktable(arg);
    newstruct(VkBindImagePlaneMemoryInfoKHR);
    GetFlags(planeAspect, "plane_aspect");
ZCHECK_END

ZCHECK_BEGIN(VkBindImageMemoryInfo)
    checktable(arg);
    newstruct(VkBindImageMemoryInfo);
    GetImage(image, "image");
    GetDeviceMemory(memory, "memory");
    GetInteger(memoryOffset, "offset");
    EXTENSIONS_BEGIN
#define F "plane_aspect"
    if(ispresent(F))
        {
        VkBindImagePlaneMemoryInfoKHR *p1 = zcheckVkBindImagePlaneMemoryInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END
ZCHECKARRAY(VkBindImageMemoryInfo)

/*------------------------------------------------------------------------------*
 | Debug Utils                                                                  |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkDebugUtilsMessengerCreateInfoEXT)
    checktable(arg);
    newstruct(VkDebugUtilsMessengerCreateInfoEXT);
    GetFlags(flags, "flags");
    GetFlags(messageSeverity, "message_severity");
    GetFlags(messageType, "message_type");
    /* pfnUserCallback and pUserData are set by the caller */
ZCHECK_END

/*------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDebugUtilsObjectNameInfoEXT)
    if(p->pObjectName) Free(L, (char*)p->pObjectName);
ZCLEAR_END
ZCHECK_BEGIN(VkDebugUtilsObjectNameInfoEXT)
    checktable(arg);
    newstruct(VkDebugUtilsObjectNameInfoEXT);
    GetObjectType(objectType, "object_type");
    GetHandle(objectHandle, "object_handle");
    GetString(pObjectName, "object_name");
ZCHECK_END
ZPUSH_BEGIN(VkDebugUtilsObjectNameInfoEXT)
    lua_newtable(L);
    SetEnum(objectType, "object_type", pushobjecttype);
    SetHandle(objectHandle, "object_handle");
    SetString(pObjectName, "object_name");
ZPUSH_END
ZCHECKARRAY(VkDebugUtilsObjectNameInfoEXT)

/*------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDebugUtilsObjectTagInfoEXT)
    if(p->pTag) Free(L, (char*)p->pTag);
ZCLEAR_END
ZCHECK_BEGIN(VkDebugUtilsObjectTagInfoEXT)
    size_t len;
    checktable(arg);
    newstruct(VkDebugUtilsObjectTagInfoEXT);
    GetObjectType(objectType, "object_type");
    GetHandle(objectHandle, "object_handle");
    GetInteger(tagName, "tag_name");
    GetLString(pTag, "tag", &len);
    p->tagSize = len;
ZCHECK_END
ZPUSH_BEGIN(VkDebugUtilsObjectTagInfoEXT)
    lua_newtable(L);
    SetEnum(objectType, "object_type", pushobjecttype);
    SetHandle(objectHandle, "object_handle");
    SetInteger(tagName, "tag_name");
    SetLString(pTag, "tag", p->tagSize);
ZPUSH_END
ZCHECKARRAY(VkDebugUtilsObjectTagInfoEXT)

/*------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDebugUtilsLabelEXT)
    if(p->pLabelName) Free(L, (char*)p->pLabelName);
ZCLEAR_END
ZCHECK_BEGIN(VkDebugUtilsLabelEXT)
    checktable(arg);
    newstruct(VkDebugUtilsLabelEXT);
    GetString(pLabelName, "label_name");
    GetNumberArray(color, "color", 4);
ZCHECK_END
ZPUSH_BEGIN(VkDebugUtilsLabelEXT)
    lua_newtable(L);
    SetString(pLabelName, "label_name");
    SetNumberArray(color, "color", 4);
ZPUSH_END
ZCHECKARRAY(VkDebugUtilsLabelEXT)

/*------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkDebugUtilsMessengerCallbackDataEXT)
    if(p->pMessageIdName) Free(L, (char*)p->pMessageIdName);
    if(p->pMessage) Free(L, (char*)p->pMessage);
    if(p->pQueueLabels)
        zfreearrayVkDebugUtilsLabelEXT(L, p->pQueueLabels, p->queueLabelCount, 1);
    if(p->pCmdBufLabels)
        zfreearrayVkDebugUtilsLabelEXT(L, p->pCmdBufLabels, p->cmdBufLabelCount, 1);
    if(p->pObjects)
        zfreearrayVkDebugUtilsObjectNameInfoEXT(L, p->pObjects, p->objectCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkDebugUtilsMessengerCallbackDataEXT)
    int arg1;
    checktable(arg);
    newstruct(VkDebugUtilsMessengerCallbackDataEXT);
    GetFlags(flags, "flags");
    GetInteger(messageIdNumber, "message_id_number");
#define F "message_id_name"
    arg1 = pushfield(L, arg, F);
    p->pMessageIdName =  GetString_(L, arg1, F, NULL, err, NULL);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
#define F "message"
    arg1 = pushfield(L, arg, F);
    p->pMessage =  GetString_(L, arg1, F, NULL, err, NULL);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
#define F "queue_labels"
    arg1 = pushfield(L, arg, F);
    p->pQueueLabels = zcheckarrayVkDebugUtilsLabelEXT(L, arg1, &p->queueLabelCount, err);
    popfield(L, arg1);
    if(*err == ERR_NOTPRESENT || *err == ERR_EMPTY) poperror();
    else if(*err) { prependfield(F); return p; }
#undef F
#define F "cmd_buf_labels"
    arg1 = pushfield(L, arg, F);
    p->pCmdBufLabels = zcheckarrayVkDebugUtilsLabelEXT(L, arg1, &p->cmdBufLabelCount, err);
    popfield(L, arg1);
    if(*err == ERR_NOTPRESENT || *err == ERR_EMPTY) poperror();
    else if(*err) { prependfield(F); return p; }
#undef F
#define F "objects"
    arg1 = pushfield(L, arg, F);
    p->pObjects = zcheckarrayVkDebugUtilsObjectNameInfoEXT(L, arg1, &p->objectCount, err);
    popfield(L, arg1);
    if(*err == ERR_NOTPRESENT || *err == ERR_EMPTY) poperror();
    else if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END
ZPUSH_BEGIN(VkDebugUtilsMessengerCallbackDataEXT)
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
            zpushVkDebugUtilsLabelEXT(L, &p->pQueueLabels[i]);
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
            zpushVkDebugUtilsLabelEXT(L, &p->pCmdBufLabels[i]);
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
            zpushVkDebugUtilsObjectNameInfoEXT(L, &p->pObjects[i]);
            lua_rawseti(L, -2, i+1);
            }
        }
    lua_setfield(L, -1, F);
#undef F
ZPUSH_END

/*------------------------------------------------------------------------------*
 | Miscellanea                                                                  |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkDisplayPowerInfoEXT)
    checktable(arg);
    newstruct(VkDisplayPowerInfoEXT);
    GetDisplayPowerState(powerState, "power_state");
ZCHECK_END

/*------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkConditionalRenderingBeginInfoEXT)
    checktable(arg);
    newstruct(VkConditionalRenderingBeginInfoEXT);
    GetBuffer(buffer, "buffer");
    GetInteger(offset, "offset");
    GetFlags(flags, "flags");
ZCHECK_END

/*------------------------------------------------------------------------------*/

ZPUSH_BEGIN(VkDescriptorSetLayoutSupportKHR)
    lua_newtable(L);
    SetBoolean(supported, "supported");
ZPUSH_END

/*------------------------------------------------------------------------------*/
    
ZCHECK_BEGIN(VkMappedMemoryRange)
    checktable(arg);
    newstruct(VkMappedMemoryRange);
    GetDeviceMemory(memory, "memory");
    GetInteger(offset, "offset");
    GetIntegerOrWholeSize(size, "size");
ZCHECK_END
ZCHECKARRAY(VkMappedMemoryRange)

/*------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkHdrMetadataEXT)
    checktable(arg);
    newstruct(VkHdrMetadataEXT);
    GetStructOpt(displayPrimaryRed, "display_primary_red", VkXYColorEXT);
    GetStructOpt(displayPrimaryGreen, "display_primary_green", VkXYColorEXT);
    GetStructOpt(displayPrimaryBlue, "display_primary_blue", VkXYColorEXT);
    GetStructOpt(whitePoint, "white_point", VkXYColorEXT);
    GetNumber(maxLuminance, "max_luminance");
    GetNumber(minLuminance, "min_luminance");
    GetNumber(maxContentLightLevel, "max_content_light_level");
    GetNumber(maxFrameAverageLightLevel, "max_frame_average_light_level");
ZCHECK_END

ZCHECK_BEGIN(VkPhysicalDeviceExternalBufferInfoKHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceExternalBufferInfoKHR);
    GetFlags(flags, "flags");
    GetFlags(usage, "usage");
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*/


ZCHECK_BEGIN(VkPhysicalDeviceExternalFenceInfoKHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceExternalFenceInfoKHR);
    GetBits(handleType, "handle_type", VkExternalFenceHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPhysicalDeviceExternalSemaphoreInfoKHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceExternalSemaphoreInfoKHR);
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
ZCHECK_END

/*------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPhysicalDeviceExternalImageFormatInfoKHR)
    //checktable(arg);
    newstruct(VkPhysicalDeviceExternalImageFormatInfoKHR);
    GetBits(handleType, "handle_type", VkExternalMemoryHandleTypeFlagBits);
ZCHECK_END

ZCHECK_BEGIN(VkPhysicalDeviceImageFormatInfo2KHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceImageFormatInfo2KHR);
    GetFormat(format, "format");
    GetImageType(type, "type");
    GetImageTiling(tiling, "tiling");
    GetFlags(usage, "usage");
    GetFlags(flags, "flags");
    EXTENSIONS_BEGIN
    if(ispresent("handle_type"))
        {
        VkPhysicalDeviceExternalImageFormatInfoKHR *p1 =
            zcheckVkPhysicalDeviceExternalImageFormatInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, *err); return p; }
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

ZCHECK_BEGIN(VkPhysicalDeviceSparseImageFormatInfo2KHR)
    checktable(arg);
    newstruct(VkPhysicalDeviceSparseImageFormatInfo2KHR);
    GetFormat(format, "format");
    GetImageType(type, "type");
    GetSamples(samples, "samples");
    GetFlags(usage, "usage");
    GetImageTiling(tiling, "tiling");
ZCHECK_END

/*------------------------------------------------------------------------------*
 | Win32                                                                        |
 *------------------------------------------------------------------------------*/

#ifdef VK_USE_PLATFORM_WIN32_KHR

#if 0 //@@todo VK_KHR_external_semaphore_win32
typedef struct VkExportSemaphoreWin32HandleInfoKHR {
    VkStructureType               sType;
    const void*                   pNext;
    const SECURITY_ATTRIBUTES*    pAttributes;
    DWORD                         dwAccess;
    LPCWSTR                       name;
} VkExportSemaphoreWin32HandleInfoKHR;
#endif

#if 0 //@@todo VK_KHR_external_semaphore_win32
typedef struct VkD3D12FenceSubmitInfoKHR {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           waitSemaphoreValuesCount;
    const uint64_t*    pWaitSemaphoreValues;
    uint32_t           signalSemaphoreValuesCount;
    const uint64_t*    pSignalSemaphoreValues;
} VkD3D12FenceSubmitInfoKHR;
#endif

ZCHECK_BEGIN(VkImportSemaphoreWin32HandleInfoKHR) //@@DOC VK_KHR_external_semaphore_win32
    checktable(arg);
    newstruct(VkImportSemaphoreWin32HandleInfoKHR);
    /* p->semaphore is set by the caller */
    GetFlags(flags, "flags");
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
    GetLightuserdata(handle, "handle", HANDLE);
    p->name = NULL; //@@ LPCWSTR name;
ZCHECK_END

ZCHECK_BEGIN(VkSemaphoreGetWin32HandleInfoKHR) //@@DOC VK_KHR_external_semaphore_win32
    checktable(arg);
    newstruct(VkSemaphoreGetWin32HandleInfoKHR);
    p->sType = VK_STRUCTURE_TYPE_;
    /* p->semaphore is set by the caller */
    GetBits(handleType, "handle_type", VkExternalSemaphoreHandleTypeFlagBits);
ZCHECK_END

#endif /* VK_USE_PLATFORM_WIN32_KHR */

/*------------------------------------------------------------------------------*
 | Pipeline Shader Stage                                                        |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineShaderStageCreateInfo)
    if(p->pName) Free(L, (void*)p->pName);
    if(p->pSpecializationInfo)
        zfreeVkSpecializationInfo(L, p->pSpecializationInfo, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineShaderStageCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineShaderStageCreateInfo);
    GetFlags(flags, "flags");
    GetBits(stage, "stage", VkShaderStageFlagBits);
    GetShaderModule(module, "module");
    GetStringDef(pName, "name", "main");
#define F "specialization_info"
    arg1 = pushfield(L, arg, F);
    p->pSpecializationInfo = zcheckVkSpecializationInfo(L, arg1, err);
    popfield(L, arg1);
    if(*err == ERR_NOTPRESENT) poperror();
    else if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END
ZCHECKARRAY(VkPipelineShaderStageCreateInfo)

/*------------------------------------------------------------------------------*
 | Compute Pipeline                                                             |
 *------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkComputePipelineCreateInfo)
    zfreeVkPipelineShaderStageCreateInfo(L, &p->stage, 0);
ZCLEAR_END
ZCHECK_BEGIN(VkComputePipelineCreateInfo)
    checktable(arg);
    newstruct(VkComputePipelineCreateInfo);
    GetFlags(flags, "flags");
    GetPipelineLayout(layout, "layout");
    GetPipelineOpt(basePipelineHandle, "base_pipeline_handle");
    GetIntegerOpt(basePipelineIndex, "base_pipeline_index", -1);
    GetStruct(stage, "stage", VkPipelineShaderStageCreateInfo);
ZCHECK_END
ZCHECKARRAY(VkComputePipelineCreateInfo)

/*------------------------------------------------------------------------------*
 | Graphics Pipeline                                                            |
 *------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineInputAssemblyStateCreateInfo)
    checktable(arg);
    newstruct(VkPipelineInputAssemblyStateCreateInfo);
    GetFlags(flags, "flags");
    GetTopology(topology, "topology");
    GetBoolean(primitiveRestartEnable, "primitive_restart_enable");
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineTessellationDomainOriginStateCreateInfoKHR)
    //checktable(arg);
    newstruct(VkPipelineTessellationDomainOriginStateCreateInfoKHR);
    GetTessellationDomainOrigin(domainOrigin, "domain_origin");
ZCHECK_END

ZCHECK_BEGIN(VkPipelineTessellationStateCreateInfo)
    checktable(arg);
    newstruct(VkPipelineTessellationStateCreateInfo);
    GetFlags(flags, "flags");
    GetInteger(patchControlPoints, "patch_control_points");
    EXTENSIONS_BEGIN
#define F   "domain_origin"
    if(ispresent(F))
        {
        VkPipelineTessellationDomainOriginStateCreateInfoKHR *p1 =
            zcheckVkPipelineTessellationDomainOriginStateCreateInfoKHR(L, arg, err);
        if(*err) { zfree(L, p1, 1); return p; }
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineViewportStateCreateInfo)
    if(p->pViewports) zfreearrayVkViewport(L, p->pViewports, p->viewportCount, 1);
    if(p->pScissors) zfreearrayVkRect2D(L, p->pScissors, p->scissorCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineViewportStateCreateInfo)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkPipelineViewportStateCreateInfo);
    GetFlags(flags, "flags");
    /* scissorCount and viewportCount must be equal, and they
     * may be > 0 even if scissors and/or viewports lists are not given */
    GetIntegerOpt(viewportCount, "viewport_count", 1);
    GetIntegerOpt(scissorCount, "scissor_count", 1);
#define F "viewports"
    arg1 = pushfield(L, arg, F);
    p->pViewports = zcheckarrayVkViewport(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err < 0) { p->viewportCount = count; prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
    else p->viewportCount = count;
#undef F
#define F "scissors"
    arg1 = pushfield(L, arg, F);
    p->pScissors = zcheckarrayVkRect2D(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err < 0) { p->scissorCount = count; prependfield(F); return p; }
    else if(*err == ERR_NOTPRESENT) poperror();
    else p->scissorCount = count;
#undef F
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineRasterizationStateCreateInfo)
    checktable(arg);
    newstruct(VkPipelineRasterizationStateCreateInfo);
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
    GetNumberDef(lineWidth, "line_width", 1.0);
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkSampleLocationsInfoEXT)
    if(p->pSampleLocations)
        zfreearrayVkSampleLocationEXT(L, p->pSampleLocations, p->sampleLocationsCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkSampleLocationsInfoEXT)
    int arg1;
    checktable(arg);
    newstruct(VkSampleLocationsInfoEXT);
    GetBits(sampleLocationsPerPixel, "sample_locations_per_pixel", VkSampleCountFlagBits);
    GetStruct(sampleLocationGridSize, "sample_location_grid_size", VkExtent2D);
#define F "sample_locations"
    arg1 = pushfield(L, arg, F);
    p->pSampleLocations = zcheckarrayVkSampleLocationEXT(L, arg1, &p->sampleLocationsCount, err);
    popfield(L, arg1);
    if(*err) { prependfield(F); return p; }
#undef F
ZCHECK_END

static ZCLEAR_BEGIN(VkPipelineSampleLocationsStateCreateInfoEXT)
    zfreeVkSampleLocationsInfoEXT(L, &p->sampleLocationsInfo, 0);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineSampleLocationsStateCreateInfoEXT)
//  checktable(arg);
    newstruct(VkPipelineSampleLocationsStateCreateInfoEXT);
    GetBoolean(sampleLocationsEnable, "sample_locations_enable");
    GetStruct(sampleLocationsInfo, "sample_locations_info", VkSampleLocationsInfoEXT);
ZCHECK_END

static ZCLEAR_BEGIN(VkPipelineMultisampleStateCreateInfo)
    if(p->pSampleMask) Free(L, (void*)p->pSampleMask);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineMultisampleStateCreateInfo)
    int arg1;
    uint32_t count;
    checktable(arg);
    newstruct(VkPipelineMultisampleStateCreateInfo);
    GetFlags(flags, "flags");
    GetSamples(rasterizationSamples, "rasterization_samples");
    GetBoolean(sampleShadingEnable, "sample_shading_enable");
    GetNumber(minSampleShading, "min_sample_shading");
    GetBoolean(alphaToCoverageEnable, "alpha_to_coverage_enable");
    GetBoolean(alphaToOneEnable, "alpha_to_one_enable");
#define F "sample_mask"
    arg1 = pushfield(L, arg, F);
    p->pSampleMask = (VkSampleMask*)checkuint32list(L, arg1, &count, err);
    popfield(L, arg1);
    if(*err < 0) { pushfielderror(F); return p; }
    if((count > 0) && (count != p->rasterizationSamples / 32))
        { *err=ERR_LENGTH; pushfielderror(F); return p; }
#undef F
    EXTENSIONS_BEGIN
#define F   "sample_locations_info"
    if(ispresent(F))
        {
        VkPipelineSampleLocationsStateCreateInfoEXT *p1 =
            zcheckVkPipelineSampleLocationsStateCreateInfoEXT(L, arg, err);
        if(*err < 0) { zfree(L, p1, 1); return p; }
        else if(*err == ERR_NOTPRESENT) poperror();
        addtochain(chain, p1);
        }
#undef F
    EXTENSIONS_END
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineVertexInputStateCreateInfo)
    if(p->pVertexBindingDescriptions) zfreearrayVkVertexInputBindingDescription(L,
            p->pVertexBindingDescriptions, p->vertexBindingDescriptionCount, 1);
    if(p->pVertexAttributeDescriptions) zfreearrayVkVertexInputAttributeDescription(L,
            p->pVertexAttributeDescriptions, p->vertexAttributeDescriptionCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineVertexInputStateCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineVertexInputStateCreateInfo);
    GetFlags(flags, "flags");
#define F "vertex_binding_descriptions"
    arg1 = pushfield(L, arg, F);
    p->pVertexBindingDescriptions = zcheckarrayVkVertexInputBindingDescription(L,
        arg1, &p->vertexBindingDescriptionCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
#define F "vertex_attribute_descriptions"
    arg1 = pushfield(L, arg, F);
    p->pVertexAttributeDescriptions = zcheckarrayVkVertexInputAttributeDescription(L,
        arg1, &p->vertexAttributeDescriptionCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineDepthStencilStateCreateInfo)
    checktable(arg);
    newstruct(VkPipelineDepthStencilStateCreateInfo);
    GetFlags(flags, "flags");
    GetBoolean(depthTestEnable, "depth_test_enable");
    GetBoolean(depthWriteEnable, "depth_write_enable");
    GetCompareOp(depthCompareOp, "depth_compare_op");
    GetBoolean(depthBoundsTestEnable, "depth_bounds_test_enable");
    GetBoolean(stencilTestEnable, "stencil_test_enable");
    GetStructOpt(front, "front", VkStencilOpState);
    GetStructOpt(back, "back", VkStencilOpState);
    GetNumber(minDepthBounds, "min_depth_bounds");
    GetNumber(maxDepthBounds, "max_depth_bounds");
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

ZCHECK_BEGIN(VkPipelineColorBlendAdvancedStateCreateInfoEXT)
    //checktable(arg);
    newstruct(VkPipelineColorBlendAdvancedStateCreateInfoEXT);
    GetBoolean(srcPremultiplied, "src_premultiplied");
    GetBoolean(dstPremultiplied, "dst_premultiplied");
    GetBlendOverlap(blendOverlap, "blend_overlap");
ZCHECK_END

static ZCLEAR_BEGIN(VkPipelineColorBlendStateCreateInfo)
    if(p->pAttachments)
        zfreearrayVkPipelineColorBlendAttachmentState(L, p->pAttachments, p->attachmentCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineColorBlendStateCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineColorBlendStateCreateInfo);
    GetFlags(flags, "flags");
    GetBoolean(logicOpEnable, "logic_op_enable");
    GetLogicOp(logicOp, "logic_op");
    GetNumberArray(blendConstants, "blend_constants", 4);
#define F "attachments"
    arg1 = pushfield(L, arg, F);
    p->pAttachments = zcheckarrayVkPipelineColorBlendAttachmentState(L, arg1, &p->attachmentCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p;}
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
    EXTENSIONS_BEGIN
    if(ispresent("src_premultiplied") || ispresent("dst_premultiplied") || ispresent("blend_overlap"))
        {
        VkPipelineColorBlendAdvancedStateCreateInfoEXT *p1 =
            zcheckVkPipelineColorBlendAdvancedStateCreateInfoEXT(L, arg, err);
        if(*err < 0) { zfree(L, p1, 1); return p; }
        else if(*err == ERR_NOTPRESENT) poperror();
        addtochain(chain, p1);
        }
    EXTENSIONS_END
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineDynamicStateCreateInfo)
    if(p->pDynamicStates) freedynamicstatelist(L, (VkDynamicState*)p->pDynamicStates);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineDynamicStateCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineDynamicStateCreateInfo);
    GetFlags(flags, "flags");
#define F "dynamic_states"
    arg1 = pushfield(L, arg, F);
    p->pDynamicStates = checkdynamicstatelist(L, arg1, &p->dynamicStateCount, err);
    popfield(L, arg1);
    if(*err) { pushfielderror(F); return p; }
#undef F
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkPipelineDiscardRectangleStateCreateInfoEXT)
    if(p->pDiscardRectangles)
        zfreearrayVkRect2D(L, p->pDiscardRectangles, p->discardRectangleCount, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkPipelineDiscardRectangleStateCreateInfoEXT)
    int arg1;
    checktable(arg);
    newstruct(VkPipelineDiscardRectangleStateCreateInfoEXT);
    GetFlags(flags, "flags");
    GetDiscardRectangleMode(discardRectangleMode, "discard_rectangle_mode");
#define F "discard_rectangles"
    arg1 = pushfield(L, arg, F);
    p->pDiscardRectangles = zcheckarrayVkRect2D(L, arg1, &p->discardRectangleCount, err);
    popfield(L, arg1);
    if(*err < 0) { prependfield(F); return p; }
    if(*err == ERR_NOTPRESENT) poperror();
#undef F
ZCHECK_END

/*-------------------------------------------------------------------------------------*/

static ZCLEAR_BEGIN(VkGraphicsPipelineCreateInfo)
    if(p->pStages) zfreearrayVkPipelineShaderStageCreateInfo(L, p->pStages, p->stageCount, 1);
    if(p->pVertexInputState) zfreeVkPipelineVertexInputStateCreateInfo(L, p->pVertexInputState, 1);
    if(p->pInputAssemblyState) zfreeVkPipelineInputAssemblyStateCreateInfo(L, p->pInputAssemblyState, 1);
    if(p->pTessellationState) zfreeVkPipelineTessellationStateCreateInfo(L, p->pTessellationState, 1);
    if(p->pViewportState) zfreeVkPipelineViewportStateCreateInfo(L, p->pViewportState, 1);
    if(p->pRasterizationState) zfreeVkPipelineRasterizationStateCreateInfo(L, p->pRasterizationState, 1);
    if(p->pMultisampleState) zfreeVkPipelineMultisampleStateCreateInfo(L, p->pMultisampleState, 1);
    if(p->pDepthStencilState) zfreeVkPipelineDepthStencilStateCreateInfo(L, p->pDepthStencilState, 1);
    if(p->pColorBlendState) zfreeVkPipelineColorBlendStateCreateInfo(L, p->pColorBlendState, 1);
    if(p->pDynamicState) zfreeVkPipelineDynamicStateCreateInfo(L, p->pDynamicState, 1);
ZCLEAR_END
ZCHECK_BEGIN(VkGraphicsPipelineCreateInfo)
    int arg1;
    checktable(arg);
    newstruct(VkGraphicsPipelineCreateInfo);
    GetFlags(flags, "flags");
    GetPipelineLayout(layout, "layout");
    GetRenderPass(renderPass, "render_pass");
    GetInteger(subpass, "subpass");
    GetPipelineOpt(basePipelineHandle, "base_pipeline_handle");
    GetIntegerOpt(basePipelineIndex, "base_pipeline_index", -1);
#define F "stages"
    arg1 = pushfield(L, arg, F);
    p->pStages = zcheckarrayVkPipelineShaderStageCreateInfo(L, arg1, &p->stageCount, err);
    popfield(L, arg1);
    if(*err<0) { prependfield(F); return p; }
#undef F
#define GET(name, sname, VkXxx, mandatory)  do {            \
    arg1 = pushfield(L, arg, sname);                        \
    p->name = zcheck##VkXxx(L, arg1, err);                  \
    popfield(L, arg1);                                      \
    if(*err<0) { prependfield(sname); return p; }           \
    else if((mandatory) && *err==ERR_NOTPRESENT) poperror();\
} while(0)
    GET(pVertexInputState, "vertex_input_state", VkPipelineVertexInputStateCreateInfo, 1);
    GET(pInputAssemblyState, "input_assembly_state", VkPipelineInputAssemblyStateCreateInfo, 1);
    GET(pTessellationState, "tessellation_state", VkPipelineTessellationStateCreateInfo, 0);
    GET(pViewportState, "viewport_state", VkPipelineViewportStateCreateInfo, 0);
    GET(pRasterizationState, "rasterization_state", VkPipelineRasterizationStateCreateInfo, 1);
    GET(pMultisampleState, "multisample_state", VkPipelineMultisampleStateCreateInfo, 0);
    GET(pDepthStencilState, "depth_stencil_state", VkPipelineDepthStencilStateCreateInfo, 0);
    GET(pColorBlendState, "color_blend_state", VkPipelineColorBlendStateCreateInfo, 0);
    GET(pDynamicState, "dynamic_state", VkPipelineDynamicStateCreateInfo, 0);
#undef GET
    EXTENSIONS_BEGIN
#define F "discard_rectangle_state"
    {
    VkPipelineDiscardRectangleStateCreateInfoEXT *p1;
    arg1 = pushfield(L, arg, F);
    p1 = zcheckVkPipelineDiscardRectangleStateCreateInfoEXT(L, arg1, err);
    popfield(L, arg1);
    if(*err<0) { zfree(L, p1, 1); prependfield(F); return p; }
    else if(*err==ERR_NOTPRESENT) poperror();
    else addtochain(chain, p1);
    }
#undef F
    EXTENSIONS_END
ZCHECK_END
ZCHECKARRAY(VkGraphicsPipelineCreateInfo)


/********************************************************************************
 * znew and zfree                                                               *
 ********************************************************************************/
/* The following functions are not meant to be used directly outside this module.
 * The specialized wrappers should be used instead (znewVkXxx() etc.)
 */

void* znew(lua_State *L, VkStructureType sType, size_t sz, int *err)
/* Allocate and initialize a structure of the given size and sType.
 * sType = -1 for structures that do not have the sType and pNext fields
 */
    {
    void *p = MallocNoErr(L, sz);
    if(p==NULL) { *err = ERR_MEMORY; return NULL; }
    if(sType != (VkStructureType)-1)
        ((VkBaseOutStructure*)p)->sType = sType;
    *err = 0;
    return p;
    }

void* znewarray(lua_State *L, VkStructureType sType, size_t sz, uint32_t count, int *err)
/* Same as znew(), but for an array of contiguous structures. */
    {
    void *p = MallocNoErr(L, sz*count);
    if(p==NULL) { *err = ERR_MEMORY; return NULL; }
    if(sType != (VkStructureType)-1)
        {
        uint32_t i;
        uintptr_t pp = (uintptr_t)p;
        for(i=0; i < count; i++)
            {
            ((VkBaseOutStructure*)pp)->sType = sType;
            pp = pp + sz;
            }
        }
    *err = 0;
    return p;
    }

void zfree_untyped(lua_State *L, const void *p, int base, void (*clearfunc)(lua_State *L, const void *p))
    { 
    if(!p) return;
    if(clearfunc) clearfunc(L, (void*)p);
    if(base) Free(L, (void*)p);
    }

void zfreearray_untyped(lua_State *L, const void *p, size_t sz, uint32_t count, int base, void (*clearfunc)(lua_State *L, const void *p))
    {
    size_t i;
    uintptr_t pp = (uintptr_t)p;
    if(!p) return;
    if(clearfunc)
        {
        for(i=0; i<count; i++)
            {
            clearfunc(L, (void*)pp);
            pp = pp + sz;
            }
        }
    if(base) Free(L, (void*)p);
    }

static void zfreeaux(lua_State *L, void *pp); /* forward declaration */

void zfree(lua_State *L, const void *p, int base)
/* Free the structure pointed by p, with all its contents (pNext chain, etc). */
    {
    if(!p) return;
    zfreeaux(L, (void*)p);
    if(base) Free(L, (void*)p);
    }

void zfreearray(lua_State *L, const void *p, size_t sz, uint32_t count, int base)
/* Same as zfree(), but for an array of contiguous structures. */
    {
    size_t i;
    uintptr_t pp = (uintptr_t)p;
    if(!p) return;
    for(i=0; i<count; i++)
        {
        zfreeaux(L, (void*)pp);
        pp = pp + sz;
        }
    if(base) Free(L, (void*)p);
    }

static void zfreeaux(lua_State *L, void *pp)
    {
    VkBaseOutStructure*p = (VkBaseOutStructure*)pp;
    if(p->pNext)
        zfree(L, (void*)p->pNext, 1);
    switch(p->sType)
        {
        /* call the zclear function for structures that need clearing outside the pNext chain */
#define CASE(XXX, VkXxx) case VK_STRUCTURE_TYPE_##XXX: zclear##VkXxx(L, (VkXxx*)pp); break
        CASE(VALIDATION_FLAGS_EXT, VkValidationFlagsEXT);
        CASE(INSTANCE_CREATE_INFO, VkInstanceCreateInfo);
        CASE(APPLICATION_INFO, VkApplicationInfo);
        CASE(BUFFER_CREATE_INFO, VkBufferCreateInfo);
        CASE(DEVICE_CREATE_INFO, VkDeviceCreateInfo);
        CASE(DEVICE_QUEUE_CREATE_INFO, VkDeviceQueueCreateInfo);
        CASE(IMAGE_FORMAT_LIST_CREATE_INFO_KHR, VkImageFormatListCreateInfoKHR);
        CASE(IMAGE_CREATE_INFO, VkImageCreateInfo);
        CASE(DESCRIPTOR_POOL_CREATE_INFO, VkDescriptorPoolCreateInfo);
        CASE(DESCRIPTOR_SET_ALLOCATE_INFO, VkDescriptorSetAllocateInfo);
        CASE(DESCRIPTOR_SET_LAYOUT_CREATE_INFO, VkDescriptorSetLayoutCreateInfo);
        CASE(PIPELINE_LAYOUT_CREATE_INFO, VkPipelineLayoutCreateInfo);
        CASE(RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO_KHR, VkRenderPassInputAttachmentAspectCreateInfoKHR);
        CASE(RENDER_PASS_CREATE_INFO, VkRenderPassCreateInfo);
        CASE(FRAMEBUFFER_CREATE_INFO, VkFramebufferCreateInfo);
        CASE(SWAPCHAIN_CREATE_INFO_KHR, VkSwapchainCreateInfoKHR);
        CASE(DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_KHR, VkDescriptorUpdateTemplateCreateInfoKHR);
        CASE(SUBMIT_INFO, VkSubmitInfo);
        CASE(PRESENT_REGIONS_KHR, VkPresentRegionsKHR);
        CASE(PRESENT_INFO_KHR, VkPresentInfoKHR);
        CASE(BIND_SPARSE_INFO, VkBindSparseInfo);
        CASE(WRITE_DESCRIPTOR_SET, VkWriteDescriptorSet);
        CASE(DEBUG_UTILS_OBJECT_NAME_INFO_EXT, VkDebugUtilsObjectNameInfoEXT);
        CASE(DEBUG_UTILS_OBJECT_TAG_INFO_EXT, VkDebugUtilsObjectTagInfoEXT);
        CASE(DEBUG_UTILS_LABEL_EXT, VkDebugUtilsLabelEXT);
        CASE(DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT, VkDebugUtilsMessengerCallbackDataEXT);
        CASE(PIPELINE_SHADER_STAGE_CREATE_INFO, VkPipelineShaderStageCreateInfo);
        CASE(COMPUTE_PIPELINE_CREATE_INFO, VkComputePipelineCreateInfo);
        CASE(PIPELINE_VIEWPORT_STATE_CREATE_INFO, VkPipelineViewportStateCreateInfo);
        CASE(SAMPLE_LOCATIONS_INFO_EXT, VkSampleLocationsInfoEXT);
        CASE(PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT, VkPipelineSampleLocationsStateCreateInfoEXT);
        CASE(PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, VkPipelineMultisampleStateCreateInfo);
        CASE(PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, VkPipelineVertexInputStateCreateInfo);
        CASE(PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, VkPipelineColorBlendStateCreateInfo);
        CASE(PIPELINE_DYNAMIC_STATE_CREATE_INFO, VkPipelineDynamicStateCreateInfo);
        CASE(PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT, VkPipelineDiscardRectangleStateCreateInfoEXT);
        CASE(GRAPHICS_PIPELINE_CREATE_INFO, VkGraphicsPipelineCreateInfo);
        CASE(RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT, VkRenderPassSampleLocationsBeginInfoEXT);
        CASE(RENDER_PASS_BEGIN_INFO, VkRenderPassBeginInfo);
        CASE(DISPLAY_MODE_CREATE_INFO_KHR, VkDisplayModeCreateInfoKHR);
#undef CASE
        default: 
            return;
        }
    }

#if 0 //@@scaffolding 10yy
static ZCLEAR_BEGIN()
ZCLEAR_END
ZCHECK_BEGIN()
    checktable(arg);
    newstruct();
ZCHECK_END
ZCHECKARRAY()
   { "", "" },
        CASE(, );

#endif


