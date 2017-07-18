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

#if 0
static int TableSize_(lua_State *L, int arg)
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
            n += TableSize_(L, top);
            lua_remove(L, top);
            }
        else {
            n++;
            lua_remove(L, top);
            }
        }
    return n;
    }

static int TableSize(lua_State *L)
    {
    TableSize_(L, 1);
    return 1;
    }
#endif

static int Flatten1_(lua_State *L, int table_index, int cur_index, int arg)
    {
    int len, i, top, m, n=0;

    if(lua_type(L, arg) != LUA_TTABLE)
        return luaL_error(L, "table expected");

    lua_len(L, arg);
    len = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if(len==0) return n;

    for(i=1; i<=len; i++)
        {
        lua_geti(L, arg, i);
        top = lua_gettop(L);
        if(lua_type(L, top) == LUA_TTABLE)
            {
            m = Flatten1_(L, table_index, cur_index, top);
            n += m;
            cur_index += m;
            lua_remove(L, top);
            }
        else
            {
            n++;
            cur_index++;
#if 0 /* no, we'll check this later */
            if(!lua_isnumber(L, lua_gettop(L)))
                return luaL_argerror(L, cur_index, "number expected");
#endif
            lua_rawseti(L, table_index, cur_index);
            }
        }
    return n;
    }

static int Flatten_(lua_State *L, int arg)
    {
    int table_index, last_arg, i, n;
    //DBG("top = %d\n", lua_gettop(L));
    if(lua_type(L, arg) == LUA_TTABLE)
        {
        lua_newtable(L);
        n = Flatten1_(L, lua_gettop(L), 0, arg);
        }
    else
        {
        /* create a table with all the arguments, and flatten it */
        last_arg = lua_gettop(L);
        lua_newtable(L);
        table_index = lua_gettop(L);
        for(i=arg; i <= last_arg; i++)
            {
            lua_pushvalue(L, i);
            lua_rawseti(L, table_index, i-arg+1);
            }
        lua_newtable(L);
        n = Flatten1_(L, lua_gettop(L), 0, table_index);
        }
    //DBG("top = %d n = %d\n", lua_gettop(L), n);
    return n;
    }

static int Flatten(lua_State *L)
    {
    int n, i, table_index;
    n = Flatten_(L, 1);
    table_index = lua_gettop(L);
    luaL_checkstack(L, n, "too many elements, cannot grow Lua stack");
    for(i = 0; i < n; i++)
        lua_rawgeti(L, table_index, i+1);
    return n;
    }

static int FlattenTable(lua_State *L)
    {
    Flatten_(L, 1);
    return 1;
    }


#define PACK(T, what) /* what= number or integer */ \
static int Pack##T(lua_State *L)            \
    {                                       \
    int isnum;                              \
    size_t n, i, len;                       \
    T *data;                                \
    n = Flatten_(L, 2);                     \
    len = n * sizeof(T);                    \
    data = (T*)Malloc(L, len);              \
    for(i = 0; i < n; i++)                  \
        {                                   \
        lua_rawgeti(L, -1, i+1);            \
        data[i] = lua_to##what##x(L, -1, &isnum); \
        if(!isnum)                          \
            {                               \
            Free(L, data);                  \
            return luaL_error(L, "element %d is not a "#what, i+1); \
            }                               \
        lua_pop(L, 1);                      \
        }                                   \
    lua_pushlstring(L, (char*)data, len);   \
    Free(L, data);                          \
    return 1;                               \
    }

#define PACK_NUMBERS(T)     PACK(T, number)
#define PACK_INTEGERS(T)    PACK(T, integer)

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

#define UNPACK(T, what) /* what= number or integer */ \
static int Unpack##T(lua_State *L, const void* data, size_t len) \
    {                                       \
    size_t n;                               \
    size_t i=0;                             \
    if((len < sizeof(T)) || (len % sizeof(T)) != 0) \
        return luaL_argerror(L, 2, "invalid length");   \
    n = len / sizeof(T);                    \
    lua_newtable(L);                        \
    for(i = 0; i < n; i++)                  \
        {                                   \
        lua_push##what(L, ((T*)data)[i]);   \
        lua_rawseti(L, -2, i+1);            \
        }                                   \
    return 1;                               \
    }

#define UNPACK_NUMBERS(T)   UNPACK(T, number)
#define UNPACK_INTEGERS(T)  UNPACK(T, integer)


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
        { "flatten_table", FlattenTable },
        { "sizeof", Sizeof },
//      { "table_size", TableSize },
        { "pack", Pack },
        { "unpack", Unpack },
        { "pack_descriptorimageinfo", PackDescriptorImageInfo },
        { "pack_descriptorbufferinfo", PackDescriptorBufferInfo },
        { "pack_bufferview", PackBufferView },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_datahandling(lua_State *L)
    {
    if(sizeof(float)!=4) luaL_error(L, "MoonVulkan expects sizeof(float)==4");
    if(sizeof(double)!=8) luaL_error(L, "MoonVulkan expects sizeof(double)==8");
    luaL_setfuncs(L, Functions, 0);
    }


