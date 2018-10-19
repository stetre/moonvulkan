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

/********************************************************************************
 * Internal common header                                                       *
 ********************************************************************************/

#ifndef internalDEFINED
#define internalDEFINED

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "moonvulkan.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include "damnwindows.h"
#endif

#define TOSTR_(x) #x
#define TOSTR(x) TOSTR_(x)

#include "getproc.h"
#include "objects.h"
#include "zcheck.h"
#include "enums.h"


/* Note: all the dynamic symbols of this library (should) start with 'moonvulkan_' .
 * The only exception is the luaopen_moonvulkan() function, which is searched for
 * with that name by Lua.
 * MoonVulkan's string references on the Lua registry also start with 'moonvulkan_'.
 */
#if 0
/* .c */
#define  moonvulkan_
#endif

/* utils.c */
#define noprintf moonvulkan_noprintf
int noprintf(const char *fmt, ...); 
#define now moonvulkan_now
double now(void);
#define since(t) (now() - (t))
#define sleeep moonvulkan_sleeep
void sleeep(double seconds);
#define notavailable moonvulkan_notavailable
int notavailable(lua_State *L, ...);
#define Malloc moonvulkan_Malloc
void *Malloc(lua_State *L, size_t size);
#define MallocNoErr moonvulkan_MallocNoErr
void *MallocNoErr(lua_State *L, size_t size);
#define MALLOC(L, TTT)          (TTT*)Malloc((L), sizeof(TTT))
#define NMALLOC(L, TTT, n)      (TTT*)Malloc((L), (n)*sizeof(TTT))
#define MALLOC_NOERR(L, TTT)    (TTT*)MallocNoErr((L), sizeof(TTT))
#define NMALLOC_NOERR(L, TTT, n)    (TTT*)MallocNoErr((L), (n)*sizeof(TTT))
#define Strdup moonvulkan_Strdup
char *Strdup(lua_State *L, const char *s);
#define Free moonvulkan_Free
void Free(lua_State *L, void *ptr);
#define checkboolean moonvulkan_checkboolean
int checkboolean(lua_State *L, int arg);
#define testboolean moonvulkan_testboolean
int testboolean(lua_State *L, int arg, int *err);
#define optboolean moonvulkan_optboolean
int optboolean(lua_State *L, int arg, int d);
#if 0
#define testindex moonvulkan_testindex
int testindex(lua_State *L, int arg, int *err);
#define checkindex moonvulkan_checkindex
int checkindex(lua_State *L, int arg);
#define optindex moonvulkan_optindex
int optindex(lua_State *L, int arg, int optval);
#define pushindex moonvulkan_pushindex
void pushindex(lua_State *L, int val);
#endif
#define checkhandle moonvulkan_checkhandle
uint64_t checkhandle(lua_State *L, int arg);
#define opthandle moonvulkan_opthandle
uint64_t opthandle(lua_State *L, int arg);
#define pushhandle moonvulkan_pushhandle
int pushhandle(lua_State *L, uint64_t handle);
#define checklightuserdata moonvulkan_checklightuserdata
void *checklightuserdata(lua_State *L, int arg);
#define optlightuserdata moonvulkan_optlightuserdata
void *optlightuserdata(lua_State *L, int arg);
#define checkstringlist moonvulkan_checkstringlist
char** checkstringlist(lua_State *L, int arg, uint32_t *count, int *err);
#define freestringlist moonvulkan_freestringlist
void freestringlist(lua_State *L, char** list, uint32_t count);
#define pushstringlist moonvulkan_pushstringlist
void pushstringlist(lua_State *L, char** list, uint32_t count);
#define checkuint32list moonvulkan_checkuint32list
uint32_t* checkuint32list(lua_State *L, int arg, uint32_t *count, int *err);
#define pushuint32list moonvulkan_pushuint32list
void pushuint32list(lua_State *L, uint32_t *list, uint32_t count);
#define checkint32list moonvulkan_checkint32list
int32_t* checkint32list(lua_State *L, int arg, uint32_t *count, int *err);
#define pushint32list moonvulkan_pushint32list
void pushint32list(lua_State *L, int32_t *list, uint32_t count);
#define checkdevicesizelist moonvulkan_checkdevicesizelist
VkDeviceSize* checkdevicesizelist(lua_State *L, int arg, uint32_t *count, int *err);
#define pushdevicesizelist moonvulkan_pushdevicesizelist
void pushdevicesizelist(lua_State *L, VkDeviceSize *list, uint32_t count);
#define checkfloatlist moonvulkan_checkfloatlist
float* checkfloatlist(lua_State *L, int arg, uint32_t *count, int *err);
#define pushfloatlist moonvulkan_pushfloatlist
void pushfloatlist(lua_State *L, float *list, uint32_t count);
#define testflags moonvulkan_testflags
VkFlags testflags(lua_State *L, int arg, int *err);
#define optflags moonvulkan_optflags
VkFlags optflags(lua_State *L, int arg, VkFlags defval);
#define checkflags moonvulkan_checkflags
VkFlags checkflags(lua_State *L, int arg);
#define pushflags moonvulkan_pushflags
int pushflags(lua_State *L, VkFlags value);
#define checkflagslist moonvulkan_checkflagslist
VkFlags* checkflagslist(lua_State *L, int arg, uint32_t *count, int *err);
#define pushflagslist moonvulkan_pushflagslist
void pushflagslist(lua_State *L, VkFlags *list, uint32_t count);

#define checksizeorwholesize moonvulkan_checksizeorwholesize
VkDeviceSize checksizeorwholesize(lua_State *L, int arg);
#define checktimeout moonvulkan_checktimeout
uint64_t checktimeout(lua_State *L, int arg);

#define optallocator(L, arg) (VkAllocationCallbacks*)optlightuserdata((L), (arg))

/* Internal error codes */
#define ERR_NOTPRESENT       1
#define ERR_SUCCESS          0
#define ERR_GENERIC         -1
#define ERR_TYPE            -2
#define ERR_VALUE           -3
#define ERR_TABLE           -4
#define ERR_EMPTY           -5
#define ERR_MEMORY          -6
#define ERR_LENGTH          -7
#define ERR_POOL            -8
#define ERR_UNKNOWN         -9
#define errstring moonvulkan_errstring
const char* errstring(int err);

/* tracing.c */
#define trace_objects moonvulkan_trace_objects
extern int trace_objects;

/* main.c */
int luaopen_moonvulkan(lua_State *L);
void moonvulkan_utils_init(lua_State *L);
int moonvulkan_open_getproc(lua_State *L);
void moonvulkan_atexit_getproc(void);
void moonvulkan_open_enums(lua_State *L);
void moonvulkan_open_flags(lua_State *L);
void moonvulkan_open_cmd(lua_State *L);
void moonvulkan_open_versions(lua_State *L);
void moonvulkan_open_tracing(lua_State *L);
void moonvulkan_open_datahandling(lua_State *L);


/*------------------------------------------------------------------------------*
 | Debug and other utilities                                                    |
 *------------------------------------------------------------------------------*/

#if defined(MINGW) /*@@FIXME: find a way to print uint64_t on f**ing Windows without warnings */
#define TRACE_CREATE(p, ttt) do {                                   \
    if(trace_objects) { printf("create "ttt" %p\n", (void*)(uintptr_t)(p)); }   \
} while(0)

#define TRACE_DELETE(p, ttt) do {                                   \
    if(trace_objects) { printf("delete "ttt" %p\n", (void*)(uintptr_t)(p)); }   \
} while(0)
#else
#define TRACE_CREATE(p, ttt) do {                                   \
    if(trace_objects) { printf("create "ttt" 0x%.16lx\n", (uint64_t)(uintptr_t)(p)); }   \
} while(0)

#define TRACE_DELETE(p, ttt) do {                                   \
    if(trace_objects) { printf("delete "ttt" 0x%.16lx\n", (uint64_t)(uintptr_t)(p)); }   \
} while(0)
#endif


#define CheckError(L, ec) \
    do { if(ec != VK_SUCCESS) { pushresult((L), (ec)); return lua_error(L); } } while(0)

/* If this is printed, it denotes a suspect bug: */
#define UNEXPECTED_ERROR "unexpected error (%s, %d)", __FILE__, __LINE__
#define unexpected(L) luaL_error((L), UNEXPECTED_ERROR)

#define notsupported(L) luaL_error((L), "operation not supported")
#define errmemory(L) luaL_error((L), errstring(ERR_MEMORY))
#define argerror(L, arg) luaL_argerror((L), (arg), lua_tostring((L), -1))
#define argerrorc(L, arg, err_code) luaL_argerror((L), (arg), errstring(err_code))

#define badvalue(L,s)   lua_pushfstring((L), "invalid value '%s'", (s))

/* DEBUG -------------------------------------------------------- */
#if defined(DEBUG)

#define DBG printf
#define TR() do { printf("trace %s %d\n",__FILE__,__LINE__); } while(0)
#define BK() do { printf("break %s %d\n",__FILE__,__LINE__); getchar(); } while(0)
#define TSTART double ts = now();
#define TSTOP do {                                          \
    ts = since(ts); ts = ts*1e6;                            \
    printf("%s %d %.3f us\n", __FILE__, __LINE__, ts);      \
    ts = now();                                             \
} while(0);

#else 

#define DBG noprintf
#define TR()
#define BK()
#define TSTART do {} while(0) 
#define TSTOP do {} while(0)    

#endif /* DEBUG ------------------------------------------------- */


#endif /* internalDEFINED */
