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
    
static size_t Sizeoftype(lua_State *L, int type)
    {
    switch(type)
        {
        case NONVK_TYPE_BYTE:
        case NONVK_TYPE_UBYTE: 
        case NONVK_TYPE_INT8:
        case NONVK_TYPE_UINT8:  return 1;
        case NONVK_TYPE_SHORT:
        case NONVK_TYPE_USHORT: 
        case NONVK_TYPE_INT16:
        case NONVK_TYPE_UINT16: return 2;
        case NONVK_TYPE_INT:
        case NONVK_TYPE_UINT: 
        case NONVK_TYPE_INT32:
        case NONVK_TYPE_UINT32: return 4;
        case NONVK_TYPE_LONG:
        case NONVK_TYPE_ULONG: 
        case NONVK_TYPE_INT64:
        case NONVK_TYPE_UINT64: return 8;
        case NONVK_TYPE_FLOAT: return 4;
        case NONVK_TYPE_DOUBLE: return 8;
        default:
            return unexpected(L);
        }
    return 0;
    }

static int Sizeof(lua_State *L)
/* size = sizeof(type) */
    {
    int type = checktype(L, 1);
    lua_pushinteger(L, Sizeoftype(L, type));
    return 1;
    }


static int Flatten_(lua_State *L, int arg)
    {
    int len, i, top, n=0;

    if(lua_type(L, arg) != LUA_TTABLE)
        return luaL_error(L, "table expected");

    lua_len(L, arg);
    len = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if(len==0) return n;

    luaL_checkstack(L, len, NULL);
    for(i=1; i<=len; i++)
        {
        lua_geti(L, arg, i);
        top = lua_gettop(L);
        if(lua_type(L, top) == LUA_TTABLE)
            {
            n += Flatten_(L, top);
            lua_remove(L, top);
            }
        else n++;
        }
    return n;
    }

static int Flatten(lua_State *L)
    {
    return Flatten_(L, 1);
    }

static size_t CheckValues(lua_State *L, size_t first, int integral)
#define CheckNumbers(L, arg) CheckValues((L), (arg), 0) 
#define CheckIntegers(L, arg) CheckValues((L), (arg), 1) 
    {
    size_t n, arg;

    if(lua_istable(L, first))
        {
        n = (size_t)Flatten_(L, first);
        lua_remove(L, first); /* remove table */
        if(n == 0)
            return luaL_argerror(L, first, errstring(ERR_EMPTY));
        }
    else if(integral)
        {
        arg = first;
        while(!lua_isnoneornil(L, arg))
            luaL_checkinteger(L, arg++);
        if(arg == first)
            luaL_checkinteger(L, arg); /* raise an error */
        n = arg - first;
        }
    else
        {
        arg = first;
        while(!lua_isnoneornil(L, arg))
            luaL_checknumber(L, arg++);
        if(arg == first)
            luaL_checknumber(L, arg); /* raise an error */
        n = arg - first;
        }
    return n;
    }


#define PACK_NUMBERS(T)                     \
static int Pack##T(lua_State *L)            \
    {                                       \
    size_t n, i, arg, len;                  \
    T *data;                                \
    n = CheckNumbers(L, 2);                 \
    len = n * sizeof(T);                    \
    data = (T*)Malloc(L, len);              \
    arg = 2;                                \
    for(i = 0; i < n; i++)                  \
        data[i] = lua_tonumber(L, arg++);   \
    lua_pushlstring(L, (char*)data, len);   \
    Free(L, data);                          \
    return 1;                               \
    }

#define PACK_INTEGERS(T)                    \
static int Pack##T(lua_State *L)            \
    {                                       \
    size_t n, i, arg, len;                  \
    T *data;                                \
    n = CheckIntegers(L, 2);                \
    len = n * sizeof(T);                    \
    data = (T*)Malloc(L, len);              \
    arg = 2;                                \
    for(i = 0; i < n; i++)                  \
        data[i] = lua_tointeger(L, arg++);  \
    lua_pushlstring(L, (char*)data, len);   \
    Free(L, data);                          \
    return 1;                               \
    }


PACK_NUMBERS(float)
PACK_NUMBERS(double)
PACK_INTEGERS(int8_t)
PACK_INTEGERS(uint8_t)
PACK_INTEGERS(int16_t)
PACK_INTEGERS(uint16_t)
PACK_INTEGERS(int32_t)
PACK_INTEGERS(uint32_t)
PACK_INTEGERS(int64_t)
PACK_INTEGERS(uint64_t)

static int Pack(lua_State *L)
    {
    int type = checktype(L, 1);
    switch(type)
        {
        case NONVK_TYPE_BYTE: 
        case NONVK_TYPE_INT8:   return Packint8_t(L);
        case NONVK_TYPE_UBYTE: 
        case NONVK_TYPE_UINT8:  return Packuint8_t(L);
        case NONVK_TYPE_SHORT: 
        case NONVK_TYPE_INT16:  return Packint16_t(L);
        case NONVK_TYPE_USHORT: 
        case NONVK_TYPE_UINT16: return Packuint16_t(L);
        case NONVK_TYPE_INT: 
        case NONVK_TYPE_INT32:  return Packint32_t(L);
        case NONVK_TYPE_UINT: 
        case NONVK_TYPE_UINT32: return Packuint32_t(L);
        case NONVK_TYPE_LONG: 
        case NONVK_TYPE_INT64:  return Packint64_t(L);
        case NONVK_TYPE_ULONG: 
        case NONVK_TYPE_UINT64: return Packuint64_t(L);
        case NONVK_TYPE_FLOAT:  return Packfloat(L);
        case NONVK_TYPE_DOUBLE: return Packdouble(L);
        default:
            return unexpected(L);
        }
    return 0;
    }

#define UNPACK_NUMBERS(T)                   \
static int Unpack##T(lua_State *L, const void* data, size_t len) \
    {                                       \
    size_t n;                               \
    size_t i=0;                             \
    if((len < sizeof(T)) || (len % sizeof(T)) != 0) \
        return luaL_argerror(L, 2, "invalid length");   \
    n = len / sizeof(T);                    \
    for(i = 0; i < n; i++)                  \
        lua_pushnumber(L, ((T*)data)[i]);   \
    return n;                               \
    }

#define UNPACK_INTEGERS(T)                  \
static int Unpack##T(lua_State *L, const void* data, size_t len)            \
    {                                       \
    size_t n;                               \
    size_t i=0;                             \
    if((len < sizeof(T)) || (len % sizeof(T)) != 0) \
        return luaL_argerror(L, 2, "invalid length");   \
    n = len / sizeof(T);                    \
    for(i = 0; i < n; i++)                  \
        lua_pushinteger(L, ((T*)data)[i]);   \
    return n;                               \
    }


UNPACK_NUMBERS(float)
UNPACK_NUMBERS(double)
UNPACK_INTEGERS(int8_t)
UNPACK_INTEGERS(uint8_t)
UNPACK_INTEGERS(int16_t)
UNPACK_INTEGERS(uint16_t)
UNPACK_INTEGERS(int32_t)
UNPACK_INTEGERS(uint32_t)
UNPACK_INTEGERS(int64_t)
UNPACK_INTEGERS(uint64_t)

static int Unpack(lua_State *L)
    {
    size_t len;
    int type = checktype(L, 1);
    const void *data = luaL_checklstring(L, 2, &len);
    switch(type)
        {
        case NONVK_TYPE_BYTE: 
        case NONVK_TYPE_INT8:   return Unpackint8_t(L, data, len);
        case NONVK_TYPE_UBYTE: 
        case NONVK_TYPE_UINT8:  return Unpackuint8_t(L, data, len);
        case NONVK_TYPE_SHORT: 
        case NONVK_TYPE_INT16:  return Unpackint16_t(L, data, len);
        case NONVK_TYPE_USHORT: 
        case NONVK_TYPE_UINT16: return Unpackuint16_t(L, data, len);
        case NONVK_TYPE_INT: 
        case NONVK_TYPE_INT32:  return Unpackint32_t(L, data, len);
        case NONVK_TYPE_UINT: 
        case NONVK_TYPE_UINT32: return Unpackuint32_t(L, data, len);
        case NONVK_TYPE_LONG: 
        case NONVK_TYPE_INT64:  return Unpackint64_t(L, data, len);
        case NONVK_TYPE_ULONG: 
        case NONVK_TYPE_UINT64: return Unpackuint64_t(L, data, len);
        case NONVK_TYPE_FLOAT:  return Unpackfloat(L, data, len);
        case NONVK_TYPE_DOUBLE: return Unpackdouble(L, data, len);
        default:
            return unexpected(L);
        }

    return 0;
    }

static int PackDescriptorImageInfo(lua_State *L)
    {
    VkDescriptorImageInfo info;
    info.sampler = checksampler(L, 1, NULL);
    info.imageView = checkimage_view(L, 2, NULL);
    info.imageLayout = checkimagelayout(L, 3);
    lua_pushlstring(L, (char*)&info, sizeof(info));
    return 1;
    }

static int PackDescriptorBufferInfo(lua_State *L)
    {
    VkDescriptorBufferInfo info;
    info.buffer = checkbuffer(L, 1, NULL);
    info.offset = luaL_checkinteger(L, 2);
    info.range = checksizeorwholesize(L, 3);
    lua_pushlstring(L, (char*)&info, sizeof(info));
    return 1;
    }

static int PackBufferView(lua_State *L)
    {
    size_t len;
    VkBufferView buffer_view = checkbuffer_view(L, 1, NULL);
    const char* data = luaL_checklstring(L, 2, &len);
    lua_pushlstring(L, (char*)&buffer_view, sizeof(buffer_view));
    if(data)
        {
        lua_pushvalue(L, 2);
        lua_concat(L, 2);
        }
    return 1;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "flatten", Flatten },
        { "sizeof", Sizeof },
        { "pack", Pack },
        { "unpack", Unpack },
        { "pack_descriptorimageinfo", PackDescriptorImageInfo },
        { "pack_descriptorbufferinfo", PackDescriptorBufferInfo },
        { "pack_bufferview", PackBufferView },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_datahandling(lua_State *L)
    {
    //@@ TODO: check that sizeof(float) = 4 and sizeof(double) = 8
    luaL_setfuncs(L, Functions, 0);
    }


