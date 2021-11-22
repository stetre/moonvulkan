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

/*------------------------------------------------------------------------------*
 | Misc utilities                                                               |
 *------------------------------------------------------------------------------*/

int noprintf(const char *fmt, ...) 
    { (void)fmt; return 0; }

int notavailable(lua_State *L, ...) 
    { 
    return luaL_error(L, "function not available in this Vulkan version");
    }

/*------------------------------------------------------------------------------*
 | Time utilities                                                               |
 *------------------------------------------------------------------------------*/

#if defined(LINUX)

#if 0
static double tstosec(const struct timespec *ts)
    {
    return ts->tv_sec*1.0+ts->tv_nsec*1.0e-9;
    }
#endif

static void sectots(struct timespec *ts, double seconds)
    {
    ts->tv_sec=(time_t)seconds;
    ts->tv_nsec=(long)((seconds-((double)ts->tv_sec))*1.0e9);
    }

double now(void)
    {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0)
        { printf("clock_gettime error\n"); return -1; }
    return ts.tv_sec + ts.tv_nsec*1.0e-9;
#else
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
        { printf("gettimeofday error\n"); return -1; }
    return tv.tv_sec + tv.tv_usec*1.0e-6;
#endif
    }

void sleeep(double seconds)
    {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts, ts1;
    struct timespec *req, *rem, *tmp;
    sectots(&ts, seconds);
    req = &ts;
    rem = &ts1;
    while(1)
        {
        if(nanosleep(req, rem) == 0)
            return;
        tmp = req;
        req = rem;
        rem = tmp;
        }
#else
    usleep((useconds_t)(seconds*1.0e6));
#endif
    }

#define time_init(L) do { /* do nothing */ } while(0)

#elif defined(MINGW)

#include "damnwindows.h"

static LARGE_INTEGER Frequency;
double now(void)
    {
    LARGE_INTEGER ts;
    QueryPerformanceCounter(&ts);
    return ((double)(ts.QuadPart))/Frequency.QuadPart;
    }

void sleeep(double seconds)
    {
    DWORD msec = (DWORD)seconds * 1000;
    //if(msec < 0) return;  DWORD seems to be unsigned
    Sleep(msec);
    }

static void time_init(lua_State *L)
    {
    (void)L;
    QueryPerformanceFrequency(&Frequency);
    }

#endif



/*------------------------------------------------------------------------------*
 | Malloc                                                                       |
 *------------------------------------------------------------------------------*/

/* We do not use malloc(), free() etc directly. Instead, we inherit the memory 
 * allocator from the main Lua state instead (see lua_getallocf in the Lua manual)
 * and use that.
 *
 * By doing so, we can use an alternative malloc() implementation without recompiling
 * this library (we have needs to recompile lua only, or execute it with LD_PRELOAD
 * set to the path to the malloc library we want to use).
 */
static lua_Alloc Alloc = NULL;
static void* AllocUd = NULL;

static void malloc_init(lua_State *L)
    {
    if(Alloc) unexpected(L);
    Alloc = lua_getallocf(L, &AllocUd);
    }

static void* Malloc_(size_t size)
    { return Alloc ? Alloc(AllocUd, NULL, 0, size) : NULL; }

static void Free_(void *ptr)
    { if(Alloc) Alloc(AllocUd, ptr, 0, 0); }

void *Malloc(lua_State *L, size_t size)
    {
    void *ptr = Malloc_(size);
    if(ptr==NULL)
        { luaL_error(L, errstring(ERR_MEMORY)); return NULL; }
    memset(ptr, 0, size);
    //DBG("Malloc %p\n", ptr);
    return ptr;
    }

void *MallocNoErr(lua_State *L, size_t size) /* do not raise errors (check the retval) */
    {
    void *ptr = Malloc_(size);
    (void)L;
    if(ptr==NULL)
        return NULL;
    memset(ptr, 0, size);
    //DBG("MallocNoErr %p\n", ptr);
    return ptr;
    }

char *Strdup(lua_State *L, const char *s)
    {
    size_t len = strnlen(s, 256);
    char *ptr = (char*)Malloc(L, len + 1);
    if(len>0)
        memcpy(ptr, s, len);
    ptr[len]='\0';
    return ptr;
    }


void Free(lua_State *L, void *ptr)
    {
    (void)L;
    //DBG("Free %p\n", ptr);
    if(ptr) Free_(ptr);
    }

/*------------------------------------------------------------------------------*
 | Handles                                                                      |
 *------------------------------------------------------------------------------*/

/* NOTE: we cannot use lightuserdata to pass Vulkan handles between C and Lua, 
 *       because handles are not (in general) pointers. 
 *       In particular, handles of non-dispatchable objects are defined as uint64_t
 *       also on 32-bit platforms, so they do not fit into a void*.
 *       So we pass them as lua_Integer, which is hopefully large enough.
 */

uint64_t checkhandle(lua_State *L, int arg)
    {
    return (uint64_t)luaL_checkinteger(L, arg);
    }   
    
uint64_t opthandle(lua_State *L, int arg)
    {
    if(lua_isnoneornil(L, arg)) return 0;
    return checkhandle(L, arg);
    }

int pushhandle(lua_State *L, uint64_t handle)
    {
    lua_pushinteger(L, handle);
    return 1;
    }


/*------------------------------------------------------------------------------*
 | Light userdata                                                               |
 *------------------------------------------------------------------------------*/

void *checklightuserdata(lua_State *L, int arg)
    {
    if(lua_type(L, arg) != LUA_TLIGHTUSERDATA)
        { luaL_argerror(L, arg, "expected lightuserdata"); return NULL; }
    return lua_touserdata(L, arg);
    }
    
void *optlightuserdata(lua_State *L, int arg)
    {
    if(lua_isnoneornil(L, arg))
        return NULL;
    return checklightuserdata(L, arg);
    }


/*------------------------------------------------------------------------------*
 | String List                                                                  |
 *------------------------------------------------------------------------------*/

char** checkstringlist(lua_State *L, int arg, uint32_t *count, int *err)
    {
    int t;
    char** list;
    const char* s;
    uint32_t i;
    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (char**)MallocNoErr(L, sizeof(char*) * (*count + 1));
    if(!list)
        { *err = ERR_MEMORY; return NULL; }
    for(i=0; i<*count; i++)
        {
        t = lua_rawgeti(L, arg, i+1);
        if(t != LUA_TSTRING)
            {
            lua_pop(L, 1);
            freestringlist(L, list, *count);
            *count = 0;
            *err = ERR_TYPE;
            return NULL;
            }
        s = lua_tostring(L, -1);
        list[i] = Strdup(L, s);
        lua_pop(L, 1);
        }
    /* list[*count]=NULL; */
    return list;
    }

void freestringlist(lua_State *L, char** list, uint32_t count)
    {
    uint32_t i;
    if(!list)
        return;
    for(i=0; i<count; i++)
        Free(L, list[i]);
    Free(L, list);
    }

void pushstringlist(lua_State *L, char** list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushstring(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | uint32_t List                                                                |
 *------------------------------------------------------------------------------*/

uint32_t* checkuint32list(lua_State *L, int arg, uint32_t *count, int *err)
    {
    uint32_t* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (uint32_t*)MallocNoErr(L, sizeof(uint32_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(!lua_isinteger(L, -1))
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_tointeger(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushuint32list(lua_State *L, uint32_t *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }

/*------------------------------------------------------------------------------*
 | VkBool32 List                                                                |
 *------------------------------------------------------------------------------*/

VkBool32* checkbooleanlist(lua_State *L, int arg, uint32_t *count, int *err)
    {
    VkBool32* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (VkBool32*)MallocNoErr(L, sizeof(VkBool32) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(!lua_isboolean(L, -1))
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_toboolean(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushbooleanlist(lua_State *L, VkBool32 *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushboolean(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | int32_t List                                                                 |
 *------------------------------------------------------------------------------*/

int32_t* checkint32list(lua_State *L, int arg, uint32_t *count, int *err)
    {
    int32_t* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (int32_t*)MallocNoErr(L, sizeof(int32_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(!lua_isinteger(L, -1))
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_tointeger(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushint32list(lua_State *L, int32_t *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | uint64_t List                                                                |
 *------------------------------------------------------------------------------*/

uint64_t* checkuint64list(lua_State *L, int arg, uint32_t *count, int *err)
    {
    uint64_t* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (uint64_t*)MallocNoErr(L, sizeof(uint64_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(!lua_isinteger(L, -1))
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_tointeger(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushuint64list(lua_State *L, uint64_t *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }

/*------------------------------------------------------------------------------*
 | VkDeviceSize List                                                            |
 *------------------------------------------------------------------------------*/

VkDeviceSize checkdevicesize(lua_State *L, int arg)
    {
    const char *s;
    if(lua_type(L, arg) == LUA_TSTRING)
        {
        s = lua_tostring(L, arg);
        if(strcmp(s, "whole size") == 0)
            return VK_WHOLE_SIZE;
        else
            return argerrorc(L, arg, ERR_VALUE);
        }
    return (VkDeviceSize)luaL_checkinteger(L, arg);
    }

VkDeviceSize* checkdevicesizelist(lua_State *L, int arg, uint32_t *count, int *err)
    {
    const char *s;
    VkDeviceSize* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (VkDeviceSize*)MallocNoErr(L, sizeof(VkDeviceSize) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(lua_type(L, -1) == LUA_TSTRING)
            {
            s = lua_tostring(L, -1);
            if(strcmp(s, "whole size") == 0)
                list[i] = VK_WHOLE_SIZE;
            else *err = ERR_VALUE;
            }
        else if(lua_isinteger(L, -1))
            list[i] = lua_tointeger(L, -1);
        else
            *err = ERR_TYPE;
        lua_pop(L, 1);
        if(*err)
            { Free(L, list); *count = 0; return NULL; }
        }
    return list;
    }

void pushdevicesizelist(lua_State *L, VkDeviceSize *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | float List                                                                   |
 *------------------------------------------------------------------------------*/

float* checkfloatlist(lua_State *L, int arg, uint32_t *count, int *err)
    {
    float* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (float*)MallocNoErr(L, sizeof(float) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(!lua_isnumber(L, -1))
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushfloatlist(lua_State *L, float *list, uint32_t count)
    {
    uint32_t i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushnumber(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }

/*------------------------------------------------------------------------------*
 | VkFlags                                                                      |
 *------------------------------------------------------------------------------*/

VkFlags64 testflags(lua_State *L, int arg, int *err)
    {
    if(lua_isinteger(L, arg))
        { *err = 0; return (VkFlags64)lua_tointeger(L, arg); }
    *err = lua_isnoneornil(L, arg) ? ERR_NOTPRESENT : ERR_TYPE;
    return (VkFlags64)0;
    }

VkFlags64 optflags(lua_State *L, int arg, VkFlags64 defval)
    {
    return (VkFlags64)luaL_optinteger(L, arg, defval);
    }

VkFlags64 checkflags(lua_State *L, int arg)
    {
    return (VkFlags64)luaL_checkinteger(L, arg);
    }

int pushflags(lua_State *L, VkFlags64 value)
    {
    lua_pushinteger(L, value);
    return 1;
    }

// 32 bit flags only:
VkFlags* checkflagslist(lua_State *L, int arg, uint32_t *count, int *err)
    {
    return (VkFlags*)checkuint32list(L, arg, count, err);
    }

void pushflagslist(lua_State *L, VkFlags *list, uint32_t count)
    {
    pushuint32list(L, (uint32_t*)list, count);
    }

/*------------------------------------------------------------------------------*
 | Custom luaL_checkxxx() style functions                                       |
 *------------------------------------------------------------------------------*/

int checkboolean(lua_State *L, int arg)
    {
    if(!lua_isboolean(L, arg))
        return (int)luaL_argerror(L, arg, "boolean expected");
    return lua_toboolean(L, arg); // ? VK_TRUE : VK_FALSE;
    }


int testboolean(lua_State *L, int arg, int *err)
    {
    if(!lua_isboolean(L, arg))
        { *err = ERR_TYPE; return 0; }
    *err = 0;
    return lua_toboolean(L, arg); // ? VK_TRUE : VK_FALSE;
    }


int optboolean(lua_State *L, int arg, int d)
    {
    if(!lua_isboolean(L, arg))
        return d;
    return lua_toboolean(L, arg);
    }

#if 0
/* 1-based index to 0-based ------------------------------------------*/

int testindex(lua_State *L, int arg, int *err)
    {
    int val;
    if(!lua_isinteger(L, arg))
        { *err = ERR_TYPE; return 0; }
    val = lua_tointeger(L, arg);
    if(val < 1)
        { *err = ERR_GENERIC; return 0; }
    *err = 0;
    return val - 1;
    }

int checkindex(lua_State *L, int arg)
    {
    int val = luaL_checkinteger(L, arg);
    if(val < 1)
        return luaL_argerror(L, arg, "positive integer expected");
    return val - 1;
    }

int optindex(lua_State *L, int arg, int optval /* 0-based */)
    {
    int val = luaL_optinteger(L, arg, optval + 1);
    if(val < 1)
        return luaL_argerror(L, arg, "positive integer expected");
    return val - 1;
    }

void pushindex(lua_State *L, int val)
    { lua_pushinteger((L), (val) + 1); }
#endif


VkDeviceSize checksizeorwholesize(lua_State *L, int arg)
    {
    const char* s;
    if(lua_type(L, arg) == LUA_TSTRING)
        {
        s = lua_tostring(L, arg);
        if(strcmp(s, "whole size") != 0)
            return luaL_argerror(L, arg, badvalue(L, s));
        return VK_WHOLE_SIZE;
        }
    return (VkDeviceSize)luaL_checkinteger(L, arg);
    }


uint64_t testtimeout(lua_State *L, int arg, int *err)
    {
    const char* s;
    uint64_t val;
    int isnum;
    int t = lua_type(L, arg);
    *err = 0;
    switch(t)
        {
        case LUA_TNONE:
        case LUA_TNIL:
                return UINT64_MAX;
        case LUA_TSTRING:   
                s = lua_tostring(L, arg);
                if(strcmp(s, "blocking") == 0) return UINT64_MAX;
                break;
        case LUA_TNUMBER:
                val = lua_tointegerx(L, arg, &isnum);
                if(isnum) return val;
        default:
            break;
        }
    *err = ERR_VALUE;
    return 0;
    }


uint64_t checktimeout(lua_State *L, int arg)
    {
    const char* s;
    int t = lua_type(L, arg);
    switch(t)
        {
        case LUA_TNONE:
        case LUA_TNIL:
                return UINT64_MAX;
        case LUA_TSTRING:   
                s = lua_tostring(L, arg);
                if(strcmp(s, "blocking") == 0) return UINT64_MAX;
                return luaL_argerror(L, arg, badvalue(L, s));
        default:
            break;
        }
    return (uint64_t)luaL_checkinteger(L, arg);
    }

/*------------------------------------------------------------------------------*
 | Internal error codes                                                         |
 *------------------------------------------------------------------------------*/

const char* errstring(int err)
    {
    switch(err)
        {
        case 0: return "success";
        case ERR_GENERIC: return "generic error";
        case ERR_TABLE: return "not a table";
        case ERR_EMPTY: return "empty list";
        case ERR_TYPE: return "invalid type";
        case ERR_VALUE: return "invalid value";
        case ERR_NOTPRESENT: return "missing";
        case ERR_MEMORY: return "out of memory";
        case ERR_LENGTH: return "invalid length";
        case ERR_POOL: return "elements are not from the same pool";
        case ERR_UNKNOWN: return "unknown field name";
        default:
            return "???";
        }
    return NULL; /* unreachable */
    }

/*------------------------------------------------------------------------------*
 | Inits                                                                        |
 *------------------------------------------------------------------------------*/

void moonvulkan_utils_init(lua_State *L)
    {
    malloc_init(L);
    time_init(L);
    }

