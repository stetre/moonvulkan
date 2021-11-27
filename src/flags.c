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
#include "nosuffix.h"

#define ADD(c) do { lua_pushinteger(L, VK_##c); lua_setfield(L, -2, #c); } while(0)

/* checkkkflags: accepts a list of strings starting from index=arg
 * pushxxxflags -> pushes a list of strings 
 */
/*----------------------------------------------------------------------*
 | Reserved
 *----------------------------------------------------------------------*/

static VkFlags checkreservedflags(lua_State *L, int arg)
    {
    const char *s;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        }

    return 0;
    }

static int pushreservedflags(lua_State *L, VkFlags flags)
    {
    (void)L; (void)flags;
    return 0;
    }


static int ReservedFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushreservedflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkreservedflags(L, 1));
    return 1;
    }


/*----------------------------------------------------------------------*
 | VkStencilFaceFlags
 *----------------------------------------------------------------------*/

static VkFlags checkstencilfaceflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_STENCIL_FACE_FRONT_BIT, "front");
    CASE(VK_STENCIL_FACE_BACK_BIT, "back");
    CASE(VK_STENCIL_FACE_FRONT_AND_BACK, "front and back");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushstencilfaceflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_STENCIL_FACE_FRONT_BIT, "front");
    CASE(VK_STENCIL_FACE_BACK_BIT, "back");
    CASE(VK_STENCIL_FACE_FRONT_AND_BACK, "front and back");
#undef CASE

    return n;
    }

static int StencilFaceFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushstencilfaceflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkstencilfaceflags(L, 1));
    return 1;
    }

#define Add_StencilFaceFlags(L) \
    ADD(STENCIL_FACE_FRONT_BIT);\
    ADD(STENCIL_FACE_BACK_BIT);\
    ADD(STENCIL_FRONT_AND_BACK);\



/*----------------------------------------------------------------------*
 | VkCommandBufferResetFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcommandbufferresetflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT, "release resources");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcommandbufferresetflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT, "release resources");
#undef CASE

    return n;
    }

static int CommandBufferResetFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcommandbufferresetflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcommandbufferresetflags(L, 1));
    return 1;
    }

#define Add_CommandBufferResetFlags(L)  \
    ADD(COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);\



/*----------------------------------------------------------------------*
 | VkQueryControlFlags
 *----------------------------------------------------------------------*/

static VkFlags checkquerycontrolflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_QUERY_CONTROL_PRECISE_BIT, "precise");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushquerycontrolflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_QUERY_CONTROL_PRECISE_BIT, "precise");
#undef CASE

    return n;
    }

static int QueryControlFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushquerycontrolflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkquerycontrolflags(L, 1));
    return 1;
    }

#define Add_QueryControlFlags(L)    \
    ADD(QUERY_CONTROL_PRECISE_BIT);\


/*----------------------------------------------------------------------*
 | VkCommandBufferUsageFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcommandbufferusageflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, "one time submit");
    CASE(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, "render pass continue");
    CASE(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, "simultaneous use");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcommandbufferusageflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, "one time submit");
    CASE(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, "render pass continue");
    CASE(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, "simultaneous use");
#undef CASE

    return n;
    }

static int CommandBufferUsageFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcommandbufferusageflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcommandbufferusageflags(L, 1));
    return 1;
    }

#define Add_CommandBufferUsageFlags(L)  \
    ADD(COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);\
    ADD(COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);\
    ADD(COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);\



/*----------------------------------------------------------------------*
 | VkCommandPoolResetFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcommandpoolresetflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT, "release resources");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcommandpoolresetflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT, "release resources");
#undef CASE

    return n;
    }

static int CommandPoolResetFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcommandpoolresetflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcommandpoolresetflags(L, 1));
    return 1;
    }

#define Add_CommandPoolResetFlags(L)    \
    ADD(COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);\



/*----------------------------------------------------------------------*
 | VkCommandPoolCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcommandpoolcreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, "transient");
    CASE(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "reset command buffer");
    CASE(VK_COMMAND_POOL_CREATE_PROTECTED_BIT, "protected");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcommandpoolcreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, "transient");
    CASE(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "reset command buffer");
    CASE(VK_COMMAND_POOL_CREATE_PROTECTED_BIT, "protected");
#undef CASE

    return n;
    }

static int CommandPoolCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcommandpoolcreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcommandpoolcreateflags(L, 1));
    return 1;
    }

#define Add_CommandPoolCreateFlags(L)   \
    ADD(COMMAND_POOL_CREATE_TRANSIENT_BIT);\
    ADD(COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);\
    ADD(COMMAND_POOL_CREATE_PROTECTED_BIT);\

/*----------------------------------------------------------------------*
 | VkDependencyFlags
 *----------------------------------------------------------------------*/

static VkFlags checkdependencyflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_DEPENDENCY_BY_REGION_BIT, "by region");
    CASE(VK_DEPENDENCY_DEVICE_GROUP_BIT, "device group");
    CASE(VK_DEPENDENCY_VIEW_LOCAL_BIT, "view local");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdependencyflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_DEPENDENCY_BY_REGION_BIT, "by region");
    CASE(VK_DEPENDENCY_DEVICE_GROUP_BIT, "device group");
    CASE(VK_DEPENDENCY_VIEW_LOCAL_BIT, "view local");
#undef CASE

    return n;
    }

static int DependencyFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdependencyflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdependencyflags(L, 1));
    return 1;
    }

#define Add_DependencyFlags(L)  \
    ADD(DEPENDENCY_BY_REGION_BIT);\
    ADD(DEPENDENCY_DEVICE_GROUP_BIT);\
    ADD(DEPENDENCY_VIEW_LOCAL_BIT);\


/*----------------------------------------------------------------------*
 | VkAccessFlags
 *----------------------------------------------------------------------*/

static VkFlags64 checkaccessflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags64 flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_ACCESS_INDIRECT_COMMAND_READ_BIT, "indirect command read");
    CASE(VK_ACCESS_INDEX_READ_BIT, "index read");
    CASE(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, "vertex attribute read");
    CASE(VK_ACCESS_UNIFORM_READ_BIT, "uniform read");
    CASE(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, "input attachment read");
    CASE(VK_ACCESS_SHADER_READ_BIT, "shader read");
    CASE(VK_ACCESS_SHADER_WRITE_BIT, "shader write");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, "color attachment read");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, "color attachment write");
    CASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, "depth stencil attachment read");
    CASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, "depth stencil attachment write");
    CASE(VK_ACCESS_TRANSFER_READ_BIT, "transfer read");
    CASE(VK_ACCESS_TRANSFER_WRITE_BIT, "transfer write");
    CASE(VK_ACCESS_HOST_READ_BIT, "host read");
    CASE(VK_ACCESS_HOST_WRITE_BIT, "host write");
    CASE(VK_ACCESS_MEMORY_READ_BIT, "memory read");
    CASE(VK_ACCESS_MEMORY_WRITE_BIT, "memory write");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT, "color attachment read noncoherent");
    CASE(VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT, "conditional rendering read");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT, "transform feedback write");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT, "transform feedback counter read");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT, "transform feedback counter write");
    CASE(VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT, "fragment density map read");
    CASE(VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT, "acceleration structure read");
    CASE(VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT, "acceleration structure write");
    CASE(VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT, "fragment shading rate attachment read");
    CASE(VK_ACCESS_NONE, "none");
    CASE(VK_ACCESS_SHADER_SAMPLED_READ_BIT, "shader sampled read");
    CASE(VK_ACCESS_SHADER_STORAGE_READ_BIT, "shader storage read");
    CASE(VK_ACCESS_SHADER_STORAGE_WRITE_BIT, "shader storage write");
#undef CASE
        return (VkFlags64)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushaccessflags(lua_State *L, VkFlags64 flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_ACCESS_INDIRECT_COMMAND_READ_BIT, "indirect command read");
    CASE(VK_ACCESS_INDEX_READ_BIT, "index read");
    CASE(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, "vertex attribute read");
    CASE(VK_ACCESS_UNIFORM_READ_BIT, "uniform read");
    CASE(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, "input attachment read");
    CASE(VK_ACCESS_SHADER_READ_BIT, "shader read");
    CASE(VK_ACCESS_SHADER_WRITE_BIT, "shader write");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, "color attachment read");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, "color attachment write");
    CASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, "depth stencil attachment read");
    CASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, "depth stencil attachment write");
    CASE(VK_ACCESS_TRANSFER_READ_BIT, "transfer read");
    CASE(VK_ACCESS_TRANSFER_WRITE_BIT, "transfer write");
    CASE(VK_ACCESS_HOST_READ_BIT, "host read");
    CASE(VK_ACCESS_HOST_WRITE_BIT, "host write");
    CASE(VK_ACCESS_MEMORY_READ_BIT, "memory read");
    CASE(VK_ACCESS_MEMORY_WRITE_BIT, "memory write");
    CASE(VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT, "color attachment read noncoherent");
    CASE(VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT, "conditional rendering read");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT, "transform feedback write");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT, "transform feedback counter read");
    CASE(VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT, "transform feedback counter write");
    CASE(VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT, "fragment density map read");
    CASE(VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT, "acceleration structure read");
    CASE(VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT, "acceleration structure write");
    CASE(VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT, "fragment shading rate attachment read");
    CASE(VK_ACCESS_NONE, "none");
    CASE(VK_ACCESS_SHADER_SAMPLED_READ_BIT, "shader sampled read");
    CASE(VK_ACCESS_SHADER_STORAGE_READ_BIT, "shader storage read");
    CASE(VK_ACCESS_SHADER_STORAGE_WRITE_BIT, "shader storage write");
#undef CASE
    return n;
    }

static int AccessFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushaccessflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkaccessflags(L, 1));
    return 1;
    }

#define Add_AccessFlags(L)  \
    ADD(ACCESS_INDIRECT_COMMAND_READ_BIT);\
    ADD(ACCESS_INDEX_READ_BIT);\
    ADD(ACCESS_VERTEX_ATTRIBUTE_READ_BIT);\
    ADD(ACCESS_UNIFORM_READ_BIT);\
    ADD(ACCESS_INPUT_ATTACHMENT_READ_BIT);\
    ADD(ACCESS_SHADER_READ_BIT);\
    ADD(ACCESS_SHADER_WRITE_BIT);\
    ADD(ACCESS_COLOR_ATTACHMENT_READ_BIT);\
    ADD(ACCESS_COLOR_ATTACHMENT_WRITE_BIT);\
    ADD(ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);\
    ADD(ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);\
    ADD(ACCESS_TRANSFER_READ_BIT);\
    ADD(ACCESS_TRANSFER_WRITE_BIT);\
    ADD(ACCESS_HOST_READ_BIT);\
    ADD(ACCESS_HOST_WRITE_BIT);\
    ADD(ACCESS_MEMORY_READ_BIT);\
    ADD(ACCESS_MEMORY_WRITE_BIT);\
    ADD(ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT);\
    ADD(ACCESS_CONDITIONAL_RENDERING_READ_BIT);\
    ADD(ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT);\
    ADD(ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT);\
    ADD(ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT);\
    ADD(ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT);\
    ADD(ACCESS_ACCELERATION_STRUCTURE_READ_BIT);\
    ADD(ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT);\
    ADD(ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT);\
    ADD(ACCESS_NONE);\
    ADD(ACCESS_SHADER_SAMPLED_READ_BIT);\
    ADD(ACCESS_SHADER_STORAGE_READ_BIT);\
    ADD(ACCESS_SHADER_STORAGE_WRITE_BIT);\


/*----------------------------------------------------------------------*
 | VkAttachmentDescriptionFlags
 *----------------------------------------------------------------------*/

static VkFlags checkattachmentdescriptionflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT, "may alias");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushattachmentdescriptionflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT, "may alias");
#undef CASE

    return n;
    }

static int AttachmentDescriptionFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushattachmentdescriptionflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkattachmentdescriptionflags(L, 1));
    return 1;
    }

#define Add_AttachmentDescriptionFlags(L)   \
    ADD(ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT);\

/*----------------------------------------------------------------------*
 | VkDescriptorPoolCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkdescriptorpoolcreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, "free descriptor set");
    CASE(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, "update after bind");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdescriptorpoolcreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, "free descriptor set");
#undef CASE

    return n;
    }

static int DescriptorPoolCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdescriptorpoolcreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdescriptorpoolcreateflags(L, 1));
    return 1;
    }

#define Add_DescriptorPoolCreateFlags(L)    \
    ADD(DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);\


/*----------------------------------------------------------------------*
 | VkDescriptorSetLayoutCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkdescriptorsetlayoutcreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT, "push descriptor");
        CASE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, "update after bind pool");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdescriptorsetlayoutcreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT, "push descriptor");
#undef CASE

    return n;
    }

static int DescriptorSetLayoutCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdescriptorsetlayoutcreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdescriptorsetlayoutcreateflags(L, 1));
    return 1;
    }

#define Add_DescriptorSetLayoutCreateFlags(L)   \
    ADD(DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);\

/*----------------------------------------------------------------------*
 | VkColorComponentFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcolorcomponentflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COLOR_COMPONENT_R_BIT, "r");
    CASE(VK_COLOR_COMPONENT_G_BIT, "g");
    CASE(VK_COLOR_COMPONENT_B_BIT, "b");
    CASE(VK_COLOR_COMPONENT_A_BIT, "a");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcolorcomponentflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COLOR_COMPONENT_R_BIT, "r");
    CASE(VK_COLOR_COMPONENT_G_BIT, "g");
    CASE(VK_COLOR_COMPONENT_B_BIT, "b");
    CASE(VK_COLOR_COMPONENT_A_BIT, "a");
#undef CASE

    return n;
    }

static int ColorComponentFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcolorcomponentflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcolorcomponentflags(L, 1));
    return 1;
    }

#define Add_ColorComponentFlags(L)  \
    ADD(COLOR_COMPONENT_R_BIT);\
    ADD(COLOR_COMPONENT_G_BIT);\
    ADD(COLOR_COMPONENT_B_BIT);\
    ADD(COLOR_COMPONENT_A_BIT);\



/*----------------------------------------------------------------------*
 | VkCullModeFlags
 *----------------------------------------------------------------------*/

static VkFlags checkcullmodeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_CULL_MODE_NONE, "none");
    CASE(VK_CULL_MODE_FRONT_BIT, "front");
    CASE(VK_CULL_MODE_BACK_BIT, "back");
    CASE(VK_CULL_MODE_FRONT_AND_BACK, "front and back");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcullmodeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

//    if(flags==0) { lua_pushstring(L, "none"); return 1; }

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
//  CASE(VK_CULL_MODE_NONE, "none");
    CASE(VK_CULL_MODE_FRONT_BIT, "front");
    CASE(VK_CULL_MODE_BACK_BIT, "back");
//  CASE(VK_CULL_MODE_FRONT_AND_BACK, "front and back");
#undef CASE

    return n;
    }

static int CullModeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcullmodeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcullmodeflags(L, 1));
    return 1;
    }

#define Add_CullModeFlags(L)    \
    ADD(CULL_MODE_NONE);\
    ADD(CULL_MODE_FRONT_BIT);\
    ADD(CULL_MODE_BACK_BIT);\
    ADD(CULL_MODE_FRONT_AND_BACK);\



/*----------------------------------------------------------------------*
 | VkShaderStageFlags
 *----------------------------------------------------------------------*/

static VkFlags checkshaderstageflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SHADER_STAGE_VERTEX_BIT, "vertex");
    CASE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tessellation control");
    CASE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tessellation evaluation");
    CASE(VK_SHADER_STAGE_GEOMETRY_BIT, "geometry");
    CASE(VK_SHADER_STAGE_FRAGMENT_BIT, "fragment");
    CASE(VK_SHADER_STAGE_COMPUTE_BIT, "compute");
    CASE(VK_SHADER_STAGE_RAYGEN_BIT, "raygen");
    CASE(VK_SHADER_STAGE_ANY_HIT_BIT, "any hit");
    CASE(VK_SHADER_STAGE_CLOSEST_HIT_BIT, "closest hit");
    CASE(VK_SHADER_STAGE_MISS_BIT, "miss");
    CASE(VK_SHADER_STAGE_INTERSECTION_BIT, "intersection");
    CASE(VK_SHADER_STAGE_CALLABLE_BIT, "callable");
    // These are not individual bits:
    CASE(VK_SHADER_STAGE_ALL_GRAPHICS, "all graphics");
    CASE(VK_SHADER_STAGE_ALL, "all");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushshaderstageflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SHADER_STAGE_VERTEX_BIT, "vertex");
    CASE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tessellation control");
    CASE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tessellation evaluation");
    CASE(VK_SHADER_STAGE_GEOMETRY_BIT, "geometry");
    CASE(VK_SHADER_STAGE_FRAGMENT_BIT, "fragment");
    CASE(VK_SHADER_STAGE_COMPUTE_BIT, "compute");
    CASE(VK_SHADER_STAGE_RAYGEN_BIT, "raygen");
    CASE(VK_SHADER_STAGE_ANY_HIT_BIT, "any hit");
    CASE(VK_SHADER_STAGE_CLOSEST_HIT_BIT, "closest hit");
    CASE(VK_SHADER_STAGE_MISS_BIT, "miss");
    CASE(VK_SHADER_STAGE_INTERSECTION_BIT, "intersection");
    CASE(VK_SHADER_STAGE_CALLABLE_BIT, "callable");
    // These are not individual bits:
    CASE(VK_SHADER_STAGE_ALL_GRAPHICS, "all graphics"); 
    CASE(VK_SHADER_STAGE_ALL, "all");
#undef CASE

    return n;
    }

static int ShaderStageFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushshaderstageflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkshaderstageflags(L, 1));
    return 1;
    }

#define Add_ShaderStageFlags(L) \
    ADD(SHADER_STAGE_VERTEX_BIT);\
    ADD(SHADER_STAGE_TESSELLATION_CONTROL_BIT);\
    ADD(SHADER_STAGE_TESSELLATION_EVALUATION_BIT);\
    ADD(SHADER_STAGE_GEOMETRY_BIT);\
    ADD(SHADER_STAGE_FRAGMENT_BIT);\
    ADD(SHADER_STAGE_COMPUTE_BIT);\
    ADD(SHADER_STAGE_ALL_GRAPHICS);\
    ADD(SHADER_STAGE_ALL);\
    ADD(SHADER_STAGE_RAYGEN_BIT);\
    ADD(SHADER_STAGE_ANY_HIT_BIT);\
    ADD(SHADER_STAGE_CLOSEST_HIT_BIT);\
    ADD(SHADER_STAGE_MISS_BIT);\
    ADD(SHADER_STAGE_INTERSECTION_BIT);\
    ADD(SHADER_STAGE_CALLABLE_BIT);\


/*----------------------------------------------------------------------*
 | VkPipelineCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkpipelinecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT, "disable optimization");
    CASE(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT, "allow derivatives");
    CASE(VK_PIPELINE_CREATE_DERIVATIVE_BIT, "derivative");
    CASE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT, "view index from device index");
    CASE(VK_PIPELINE_CREATE_DISPATCH_BASE, "dispatch base");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT, "ray tracing no null any hit shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT, "ray tracing no null closest hit shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT, "ray tracing no null miss shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT, "ray tracing no null intersection shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT, "ray tracing skip triangles");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT, "ray tracing skip aabbs");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT, "ray tracing shader group handle capture replay");
    CASE(VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT, "capture statistics");
    CASE(VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT, "capture internal representations");
    CASE(VK_PIPELINE_CREATE_LIBRARY_BIT, "library");
    CASE(VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT, "fail on pipeline compile required");
    CASE(VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT, "early return on failure");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushpipelinecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT, "disable optimization");
    CASE(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT, "allow derivatives");
    CASE(VK_PIPELINE_CREATE_DERIVATIVE_BIT, "derivative");
    CASE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT, "view index from device index");
    CASE(VK_PIPELINE_CREATE_DISPATCH_BASE, "dispatch base");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT, "ray tracing no null any hit shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT, "ray tracing no null closest hit shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT, "ray tracing no null miss shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT, "ray tracing no null intersection shaders");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT, "ray tracing skip triangles");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT, "ray tracing skip aabbs");
    CASE(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT, "ray tracing shader group handle capture replay");
    CASE(VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT, "capture statistics");
    CASE(VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT, "capture internal representations");
    CASE(VK_PIPELINE_CREATE_LIBRARY_BIT, "library");
    CASE(VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT, "fail on pipeline compile required");
    CASE(VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT, "early return on failure");
#undef CASE

    return n;
    }

static int PipelineCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpipelinecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpipelinecreateflags(L, 1));
    return 1;
    }

#define VK_PIPELINE_CREATE_DISPATCH_BASE_BIT VK_PIPELINE_CREATE_DISPATCH_BASE /*@@ spec bug ? */
#define Add_PipelineCreateFlags(L)  \
    ADD(PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT);\
    ADD(PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT);\
    ADD(PIPELINE_CREATE_DERIVATIVE_BIT);\
    ADD(PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT);\
    ADD(PIPELINE_CREATE_DISPATCH_BASE_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT);\
    ADD(PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT);\
    ADD(PIPELINE_CREATE_CAPTURE_STATISTICS_BIT);\
    ADD(PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT);\
    ADD(PIPELINE_CREATE_LIBRARY_BIT);\
    ADD(PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT);\
    ADD(PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT);\

/*----------------------------------------------------------------------*
 | VkBufferUsageFlags
 *----------------------------------------------------------------------*/

static VkFlags checkbufferusageflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "transfer src");
    CASE(VK_BUFFER_USAGE_TRANSFER_DST_BIT, "transfer dst");
    CASE(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "uniform texel buffer");
    CASE(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "storage texel buffer");
    CASE(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "uniform buffer");
    CASE(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "storage buffer");
    CASE(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "index buffer");
    CASE(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "vertex buffer");
    CASE(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, "indirect buffer");
    CASE(VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT, "conditional rendering");
    CASE(VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT, "transform feedback buffer");
    CASE(VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT, "transform feedback counter buffer");
    CASE(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, "shader device address");
    CASE(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT, "acceleration structure build input read only");
    CASE(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT, "acceleration structure storage");
    CASE(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT, "shader binding table");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushbufferusageflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "transfer src");
    CASE(VK_BUFFER_USAGE_TRANSFER_DST_BIT, "transfer dst");
    CASE(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "uniform texel buffer");
    CASE(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "storage texel buffer");
    CASE(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "uniform buffer");
    CASE(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "storage buffer");
    CASE(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "index buffer");
    CASE(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "vertex buffer");
    CASE(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, "indirect buffer");
    CASE(VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT, "conditional rendering");
    CASE(VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT, "transform feedback buffer");
    CASE(VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT, "transform feedback counter buffer");
    CASE(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, "shader device address");
    CASE(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT, "acceleration structure build input read only");
    CASE(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT, "acceleration structure storage");
    CASE(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT, "shader binding table");
#undef CASE

    return n;
    }

static int BufferUsageFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushbufferusageflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkbufferusageflags(L, 1));
    return 1;
    }

#define Add_BufferUsageFlags(L) \
    ADD(BUFFER_USAGE_TRANSFER_SRC_BIT);\
    ADD(BUFFER_USAGE_TRANSFER_DST_BIT);\
    ADD(BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);\
    ADD(BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);\
    ADD(BUFFER_USAGE_UNIFORM_BUFFER_BIT);\
    ADD(BUFFER_USAGE_STORAGE_BUFFER_BIT);\
    ADD(BUFFER_USAGE_INDEX_BUFFER_BIT);\
    ADD(BUFFER_USAGE_VERTEX_BUFFER_BIT);\
    ADD(BUFFER_USAGE_INDIRECT_BUFFER_BIT);\
    ADD(BUFFER_USAGE_CONDITIONAL_RENDERING_BIT);\
    ADD(BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT);\
    ADD(BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT);\
    ADD(BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);\
    ADD(BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT);\
    ADD(BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT);\
    ADD(BUFFER_USAGE_SHADER_BINDING_TABLE_BIT);\


/*----------------------------------------------------------------------*
 | VkBufferCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkbuffercreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_BUFFER_CREATE_SPARSE_BINDING_BIT, "sparse binding");
    CASE(VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, "sparse residency");
    CASE(VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, "sparse aliased");
    CASE(VK_BUFFER_CREATE_PROTECTED_BIT, "protected");
    CASE(VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushbuffercreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_BUFFER_CREATE_SPARSE_BINDING_BIT, "sparse binding");
    CASE(VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, "sparse residency");
    CASE(VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, "sparse aliased");
    CASE(VK_BUFFER_CREATE_PROTECTED_BIT, "protected");
    CASE(VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE

    return n;
    }

static int BufferCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushbuffercreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkbuffercreateflags(L, 1));
    return 1;
    }

#define Add_BufferCreateFlags(L)    \
    ADD(BUFFER_CREATE_SPARSE_BINDING_BIT);\
    ADD(BUFFER_CREATE_SPARSE_RESIDENCY_BIT);\
    ADD(BUFFER_CREATE_SPARSE_ALIASED_BIT);\
    ADD(BUFFER_CREATE_PROTECTED_BIT);\
    ADD(BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT);\

/*----------------------------------------------------------------------*
 | VkQueryResultFlags
 *----------------------------------------------------------------------*/

static VkFlags checkqueryresultflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_QUERY_RESULT_64_BIT, "64");
    CASE(VK_QUERY_RESULT_WAIT_BIT, "wait");
    CASE(VK_QUERY_RESULT_WITH_AVAILABILITY_BIT, "with availability");
    CASE(VK_QUERY_RESULT_PARTIAL_BIT, "partial");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushqueryresultflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_QUERY_RESULT_64_BIT, "64");
    CASE(VK_QUERY_RESULT_WAIT_BIT, "wait");
    CASE(VK_QUERY_RESULT_WITH_AVAILABILITY_BIT, "with availability");
    CASE(VK_QUERY_RESULT_PARTIAL_BIT, "partial");
#undef CASE

    return n;
    }

static int QueryResultFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushqueryresultflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkqueryresultflags(L, 1));
    return 1;
    }

#define Add_QueryResultFlags(L) \
    ADD(QUERY_RESULT_64_BIT);\
    ADD(QUERY_RESULT_WAIT_BIT);\
    ADD(QUERY_RESULT_WITH_AVAILABILITY_BIT);\
    ADD(QUERY_RESULT_PARTIAL_BIT);\



/*----------------------------------------------------------------------*
 | VkQueryPipelineStatisticFlags
 *----------------------------------------------------------------------*/

static VkFlags checkquerypipelinestatisticflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, "input assembly vertices");
    CASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, "input assembly primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, "vertex shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, "geometry shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, "geometry shader primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, "clipping invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, "clipping primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, "fragment shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, "tessellation control shader patches");
    CASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, "tessellation evaluation shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, "compute shader invocations");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushquerypipelinestatisticflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, "input assembly vertices");
    CASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, "input assembly primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, "vertex shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, "geometry shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, "geometry shader primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, "clipping invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, "clipping primitives");
    CASE(VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, "fragment shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, "tessellation control shader patches");
    CASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, "tessellation evaluation shader invocations");
    CASE(VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, "compute shader invocations");
#undef CASE

    return n;
    }

static int QueryPipelineStatisticFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushquerypipelinestatisticflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkquerypipelinestatisticflags(L, 1));
    return 1;
    }

#define Add_QueryPipelineStatisticFlags(L)  \
    ADD(QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT);\
    ADD(QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT);\



/*----------------------------------------------------------------------*
 | VkFenceCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkfencecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_FENCE_CREATE_SIGNALED_BIT, "signaled");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushfencecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_FENCE_CREATE_SIGNALED_BIT, "signaled");
#undef CASE

    return n;
    }

static int FenceCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushfencecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkfencecreateflags(L, 1));
    return 1;
    }

#define Add_FenceCreateFlags(L) \
    ADD(FENCE_CREATE_SIGNALED_BIT);\




/*----------------------------------------------------------------------*
 | VkSparseMemoryBindFlags
 *----------------------------------------------------------------------*/

static VkFlags checksparsememorybindflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SPARSE_MEMORY_BIND_METADATA_BIT, "metadata");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsparsememorybindflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SPARSE_MEMORY_BIND_METADATA_BIT, "metadata");
#undef CASE

    return n;
    }

static int SparseMemoryBindFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsparsememorybindflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksparsememorybindflags(L, 1));
    return 1;
    }

#define Add_SparseMemoryBindFlags(L)    \
    ADD(SPARSE_MEMORY_BIND_METADATA_BIT);\



/*----------------------------------------------------------------------*
 | VkSparseImageFormatFlags
 *----------------------------------------------------------------------*/

static VkFlags checksparseimageformatflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT, "single miptail");
    CASE(VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT, "aligned mip size");
    CASE(VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT, "nonstandard block size");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsparseimageformatflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT, "single miptail");
    CASE(VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT, "aligned mip size");
    CASE(VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT, "nonstandard block size");
#undef CASE

    return n;
    }

static int SparseImageFormatFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsparseimageformatflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksparseimageformatflags(L, 1));
    return 1;
    }

#define Add_SparseImageFormatFlags(L)   \
    ADD(SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);\
    ADD(SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT);\
    ADD(SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT);\



/*----------------------------------------------------------------------*
 | VkImageAspectFlags
 *----------------------------------------------------------------------*/

static VkFlags checkimageaspectflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_IMAGE_ASPECT_COLOR_BIT, "color");
    CASE(VK_IMAGE_ASPECT_DEPTH_BIT, "depth");
    CASE(VK_IMAGE_ASPECT_STENCIL_BIT, "stencil");
    CASE(VK_IMAGE_ASPECT_METADATA_BIT, "metadata");
    CASE(VK_IMAGE_ASPECT_PLANE_0_BIT, "plane 0");
    CASE(VK_IMAGE_ASPECT_PLANE_1_BIT, "plane 1");
    CASE(VK_IMAGE_ASPECT_PLANE_2_BIT, "plane 2");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT, "memory plane 0");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT, "memory plane 1");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT, "memory plane 2");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT, "memory plane 3");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushimageaspectflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_IMAGE_ASPECT_COLOR_BIT, "color");
    CASE(VK_IMAGE_ASPECT_DEPTH_BIT, "depth");
    CASE(VK_IMAGE_ASPECT_STENCIL_BIT, "stencil");
    CASE(VK_IMAGE_ASPECT_METADATA_BIT, "metadata");
    CASE(VK_IMAGE_ASPECT_PLANE_0_BIT, "plane 0");
    CASE(VK_IMAGE_ASPECT_PLANE_1_BIT, "plane 1");
    CASE(VK_IMAGE_ASPECT_PLANE_2_BIT, "plane 2");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT, "memory plane 0");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT, "memory plane 1");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT, "memory plane 2");
    CASE(VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT, "memory plane 3");
#undef CASE

    return n;
    }

static int ImageAspectFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushimageaspectflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkimageaspectflags(L, 1));
    return 1;
    }

#define Add_ImageAspectFlags(L) \
    ADD(IMAGE_ASPECT_COLOR_BIT);\
    ADD(IMAGE_ASPECT_DEPTH_BIT);\
    ADD(IMAGE_ASPECT_STENCIL_BIT);\
    ADD(IMAGE_ASPECT_METADATA_BIT);\
    ADD(IMAGE_ASPECT_PLANE_0_BIT);\
    ADD(IMAGE_ASPECT_PLANE_1_BIT);\
    ADD(IMAGE_ASPECT_PLANE_2_BIT);\
    ADD(IMAGE_ASPECT_MEMORY_PLANE_0_BIT);\
    ADD(IMAGE_ASPECT_MEMORY_PLANE_1_BIT);\
    ADD(IMAGE_ASPECT_MEMORY_PLANE_2_BIT);\
    ADD(IMAGE_ASPECT_MEMORY_PLANE_3_BIT);\

/*----------------------------------------------------------------------*
 | VkPipelineStageFlags
 *----------------------------------------------------------------------*/

static VkFlags64 checkpipelinestageflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags64 flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "top of pipe");
        CASE(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, "draw indirect");
        CASE(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, "vertex input");
        CASE(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, "vertex shader");
        CASE(VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, "tessellation control shader");
        CASE(VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, "tessellation evaluation shader");
        CASE(VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, "geometry shader");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, "fragment shader");
        CASE(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, "early fragment tests");
        CASE(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, "late fragment tests");
        CASE(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, "color attachment output");
        CASE(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, "compute shader");
        CASE(VK_PIPELINE_STAGE_TRANSFER_BIT, "transfer");
        CASE(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "bottom of pipe");
        CASE(VK_PIPELINE_STAGE_HOST_BIT, "host");
        CASE(VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT, "conditional rendering");
        CASE(VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT, "transform feedback");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT, "fragment density process");
        CASE(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT, "acceleration structure build");
        CASE(VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT, "ray tracing shader");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT, "fragment shading rate attachment");
        CASE(VK_PIPELINE_STAGE_NONE, "none");
        CASE(VK_PIPELINE_STAGE_COPY_BIT, "copy");
        CASE(VK_PIPELINE_STAGE_RESOLVE_BIT, "resolve");
        CASE(VK_PIPELINE_STAGE_BLIT_BIT, "blit");
        CASE(VK_PIPELINE_STAGE_CLEAR_BIT, "clear");
        CASE(VK_PIPELINE_STAGE_INDEX_INPUT_BIT, "index input");
        CASE(VK_PIPELINE_STAGE_VERTEX_ATTRIBUTE_INPUT_BIT, "vertex attribute input");
        CASE(VK_PIPELINE_STAGE_PRE_RASTERIZATION_SHADERS_BIT, "pre rasterization shaders");
    // These are not individual bits:
        CASE(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, "all graphics");
        CASE(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, "all commands");
#undef CASE
        return (VkFlags64)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushpipelinestageflags(lua_State *L, VkFlags64 flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "top of pipe");
        CASE(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, "draw indirect");
        CASE(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, "vertex input");
        CASE(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, "vertex shader");
        CASE(VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, "tessellation control shader");
        CASE(VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, "tessellation evaluation shader");
        CASE(VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, "geometry shader");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, "fragment shader");
        CASE(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, "early fragment tests");
        CASE(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, "late fragment tests");
        CASE(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, "color attachment output");
        CASE(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, "compute shader");
        CASE(VK_PIPELINE_STAGE_TRANSFER_BIT, "transfer");
        CASE(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "bottom of pipe");
        CASE(VK_PIPELINE_STAGE_HOST_BIT, "host");
        CASE(VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT, "conditional rendering");
        CASE(VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT, "transform feedback");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT, "fragment density process");
        CASE(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT, "acceleration structure build");
        CASE(VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT, "ray tracing shader");
        CASE(VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT, "fragment shading rate attachment");
        CASE(VK_PIPELINE_STAGE_NONE, "none");
        CASE(VK_PIPELINE_STAGE_COPY_BIT, "copy");
        CASE(VK_PIPELINE_STAGE_RESOLVE_BIT, "resolve");
        CASE(VK_PIPELINE_STAGE_BLIT_BIT, "blit");
        CASE(VK_PIPELINE_STAGE_CLEAR_BIT, "clear");
        CASE(VK_PIPELINE_STAGE_INDEX_INPUT_BIT, "index input");
        CASE(VK_PIPELINE_STAGE_VERTEX_ATTRIBUTE_INPUT_BIT, "vertex attribute input");
        CASE(VK_PIPELINE_STAGE_PRE_RASTERIZATION_SHADERS_BIT, "pre rasterization shaders");
    // These are not individual bits:
        CASE(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, "all graphics");
        CASE(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, "all commands");
#undef CASE

    return n;
    }

static int PipelineStageFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpipelinestageflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpipelinestageflags(L, 1));
    return 1;
    }

#define Add_PipelineStageFlags(L)   \
    ADD(PIPELINE_STAGE_TOP_OF_PIPE_BIT);\
    ADD(PIPELINE_STAGE_DRAW_INDIRECT_BIT);\
    ADD(PIPELINE_STAGE_VERTEX_INPUT_BIT);\
    ADD(PIPELINE_STAGE_VERTEX_SHADER_BIT);\
    ADD(PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);\
    ADD(PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT);\
    ADD(PIPELINE_STAGE_GEOMETRY_SHADER_BIT);\
    ADD(PIPELINE_STAGE_FRAGMENT_SHADER_BIT);\
    ADD(PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);\
    ADD(PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);\
    ADD(PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);\
    ADD(PIPELINE_STAGE_COMPUTE_SHADER_BIT);\
    ADD(PIPELINE_STAGE_TRANSFER_BIT);\
    ADD(PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);\
    ADD(PIPELINE_STAGE_HOST_BIT);\
    ADD(PIPELINE_STAGE_ALL_GRAPHICS_BIT);\
    ADD(PIPELINE_STAGE_ALL_COMMANDS_BIT);\
    ADD(PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT);\
    ADD(PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT);\
    ADD(PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT);\
    ADD(PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT);\
    ADD(PIPELINE_STAGE_RAY_TRACING_SHADER_BIT);\
    ADD(PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT);\
    ADD(PIPELINE_STAGE_NONE);\
    ADD(PIPELINE_STAGE_COPY_BIT);\
    ADD(PIPELINE_STAGE_RESOLVE_BIT);\
    ADD(PIPELINE_STAGE_BLIT_BIT);\
    ADD(PIPELINE_STAGE_CLEAR_BIT);\
    ADD(PIPELINE_STAGE_INDEX_INPUT_BIT);\
    ADD(PIPELINE_STAGE_VERTEX_ATTRIBUTE_INPUT_BIT);\
    ADD(PIPELINE_STAGE_PRE_RASTERIZATION_SHADERS_BIT);\

/*----------------------------------------------------------------------*
 | VkMemoryHeapFlag
 *----------------------------------------------------------------------*/

static VkFlags checkmemoryheapflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, "device local");
        CASE(VK_MEMORY_HEAP_MULTI_INSTANCE_BIT, "multi instance");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmemoryheapflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, "device local");
        CASE(VK_MEMORY_HEAP_MULTI_INSTANCE_BIT, "multi instance");
#undef CASE

    return n;
    }

static int MemoryHeapFlag(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmemoryheapflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmemoryheapflags(L, 1));
    return 1;
    }

#define Add_MemoryHeapFlag(L)   \
        ADD(MEMORY_HEAP_DEVICE_LOCAL_BIT);\
        ADD(MEMORY_HEAP_MULTI_INSTANCE_BIT);\


/*----------------------------------------------------------------------*
 | VkMemoryPropertyFlags
 *----------------------------------------------------------------------*/

static VkFlags checkmemorypropertyflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "device local");
        CASE(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, "host visible");
        CASE(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "host coherent");
        CASE(VK_MEMORY_PROPERTY_HOST_CACHED_BIT, "host cached");
        CASE(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, "lazily allocated");
        CASE(VK_MEMORY_PROPERTY_PROTECTED_BIT, "protected");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmemorypropertyflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "device local");
        CASE(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, "host visible");
        CASE(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "host coherent");
        CASE(VK_MEMORY_PROPERTY_HOST_CACHED_BIT, "host cached");
        CASE(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, "lazily allocated");
        CASE(VK_MEMORY_PROPERTY_PROTECTED_BIT, "protected");
#undef CASE

    return n;
    }

static int MemoryPropertyFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmemorypropertyflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmemorypropertyflags(L, 1));
    return 1;
    }

#define Add_MemoryPropertyFlags(L)  \
    ADD(MEMORY_PROPERTY_DEVICE_LOCAL_BIT);\
    ADD(MEMORY_PROPERTY_HOST_VISIBLE_BIT);\
    ADD(MEMORY_PROPERTY_HOST_COHERENT_BIT);\
    ADD(MEMORY_PROPERTY_HOST_CACHED_BIT);\
    ADD(MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);\
    ADD(MEMORY_PROPERTY_PROTECTED_BIT);\


/*----------------------------------------------------------------------*
 | VkQueueFlags
 *----------------------------------------------------------------------*/

static VkFlags checkqueueflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_QUEUE_GRAPHICS_BIT, "graphics");
        CASE(VK_QUEUE_COMPUTE_BIT, "compute");
        CASE(VK_QUEUE_TRANSFER_BIT, "transfer");
        CASE(VK_QUEUE_SPARSE_BINDING_BIT, "sparse binding");
        CASE(VK_QUEUE_PROTECTED_BIT, "protected");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushqueueflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_QUEUE_GRAPHICS_BIT, "graphics");
        CASE(VK_QUEUE_COMPUTE_BIT, "compute");
        CASE(VK_QUEUE_TRANSFER_BIT, "transfer");
        CASE(VK_QUEUE_SPARSE_BINDING_BIT, "sparse binding");
        CASE(VK_QUEUE_PROTECTED_BIT, "protected");
#undef CASE

    return n;
    }

static int QueueFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushqueueflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkqueueflags(L, 1));
    return 1;
    }

#define Add_QueueFlags(L)   \
    ADD(QUEUE_GRAPHICS_BIT);\
    ADD(QUEUE_COMPUTE_BIT);\
    ADD(QUEUE_TRANSFER_BIT);\
    ADD(QUEUE_SPARSE_BINDING_BIT);\
    ADD(QUEUE_PROTECTED_BIT);\


/*----------------------------------------------------------------------*
 | VkSampleCountFlags
 *----------------------------------------------------------------------*/

static VkFlags checksamplecountflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_SAMPLE_COUNT_1_BIT, "1");
        CASE(VK_SAMPLE_COUNT_2_BIT, "2");
        CASE(VK_SAMPLE_COUNT_4_BIT, "4");
        CASE(VK_SAMPLE_COUNT_8_BIT, "8");
        CASE(VK_SAMPLE_COUNT_16_BIT, "16");
        CASE(VK_SAMPLE_COUNT_32_BIT, "32");
        CASE(VK_SAMPLE_COUNT_64_BIT, "64");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsamplecountflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_SAMPLE_COUNT_1_BIT, "1");
        CASE(VK_SAMPLE_COUNT_2_BIT, "2");
        CASE(VK_SAMPLE_COUNT_4_BIT, "4");
        CASE(VK_SAMPLE_COUNT_8_BIT, "8");
        CASE(VK_SAMPLE_COUNT_16_BIT, "16");
        CASE(VK_SAMPLE_COUNT_32_BIT, "32");
        CASE(VK_SAMPLE_COUNT_64_BIT, "64");
#undef CASE

    return n;
    }

static int SampleCountFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsamplecountflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksamplecountflags(L, 1));
    return 1;
    }

#define Add_SampleCountFlags(L) \
    ADD(SAMPLE_COUNT_1_BIT);\
    ADD(SAMPLE_COUNT_2_BIT);\
    ADD(SAMPLE_COUNT_4_BIT);\
    ADD(SAMPLE_COUNT_8_BIT);\
    ADD(SAMPLE_COUNT_16_BIT);\
    ADD(SAMPLE_COUNT_32_BIT);\
    ADD(SAMPLE_COUNT_64_BIT);\



/*----------------------------------------------------------------------*
 | VkImageCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkimagecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_IMAGE_CREATE_SPARSE_BINDING_BIT, "sparse binding");
        CASE(VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, "sparse residency");
        CASE(VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, "sparse aliased");
        CASE(VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, "mutable format");
        CASE(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, "cube compatible");
        CASE(VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "block texel view compatible");
        CASE(VK_IMAGE_CREATE_EXTENDED_USAGE_BIT, "extended usage");
        CASE(VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT, "sample locations compatible depth");
        CASE(VK_IMAGE_CREATE_DISJOINT_BIT, "disjoint");
        CASE(VK_IMAGE_CREATE_ALIAS_BIT, "alias");
        CASE(VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,"split instance bind regions");
        CASE(VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,"2d array compatible");
        CASE(VK_IMAGE_CREATE_PROTECTED_BIT, "protected");
        CASE(VK_IMAGE_CREATE_SUBSAMPLED_BIT, "subsampled");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushimagecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_IMAGE_CREATE_SPARSE_BINDING_BIT, "sparse binding");
        CASE(VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, "sparse residency");
        CASE(VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, "sparse aliased");
        CASE(VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, "mutable format");
        CASE(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, "cube compatible");
        CASE(VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "block texel view compatible");
        CASE(VK_IMAGE_CREATE_EXTENDED_USAGE_BIT, "extended usage");
        CASE(VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT, "sample locations compatible depth");
        CASE(VK_IMAGE_CREATE_DISJOINT_BIT, "disjoint");
        CASE(VK_IMAGE_CREATE_ALIAS_BIT, "alias");
        CASE(VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,"split instance bind regions");
        CASE(VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,"2d array compatible");
        CASE(VK_IMAGE_CREATE_PROTECTED_BIT, "protected");
        CASE(VK_IMAGE_CREATE_SUBSAMPLED_BIT, "subsampled");
#undef CASE

    return n;
    }

static int ImageCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushimagecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkimagecreateflags(L, 1));
    return 1;
    }

#define Add_ImageCreateFlags(L) \
    ADD(IMAGE_CREATE_SPARSE_BINDING_BIT);\
    ADD(IMAGE_CREATE_SPARSE_RESIDENCY_BIT);\
    ADD(IMAGE_CREATE_SPARSE_ALIASED_BIT);\
    ADD(IMAGE_CREATE_MUTABLE_FORMAT_BIT);\
    ADD(IMAGE_CREATE_CUBE_COMPATIBLE_BIT);\
    ADD(IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT);\
    ADD(IMAGE_CREATE_EXTENDED_USAGE_BIT);\
    ADD(IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT);\
    ADD(IMAGE_CREATE_DISJOINT_BIT);\
    ADD(IMAGE_CREATE_ALIAS_BIT);\
    ADD(IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT);\
    ADD(IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT);\
    ADD(IMAGE_CREATE_PROTECTED_BIT);\
    ADD(IMAGE_CREATE_SUBSAMPLED_BIT);\

/*----------------------------------------------------------------------*
 | VkImageUsageFlags
 *----------------------------------------------------------------------*/

static VkFlags checkimageusageflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "transfer src");
        CASE(VK_IMAGE_USAGE_TRANSFER_DST_BIT, "transfer dst");
        CASE(VK_IMAGE_USAGE_SAMPLED_BIT, "sampled");
        CASE(VK_IMAGE_USAGE_STORAGE_BIT, "storage");
        CASE(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "color attachment");
        CASE(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "depth stencil attachment");
        CASE(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, "transient attachment");
        CASE(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "input attachment");
        CASE(VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT, "fragment density map");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushimageusageflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "transfer src");
        CASE(VK_IMAGE_USAGE_TRANSFER_DST_BIT, "transfer dst");
        CASE(VK_IMAGE_USAGE_SAMPLED_BIT, "sampled");
        CASE(VK_IMAGE_USAGE_STORAGE_BIT, "storage");
        CASE(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "color attachment");
        CASE(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "depth stencil attachment");
        CASE(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, "transient attachment");
        CASE(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "input attachment");
        CASE(VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT, "fragment density map");
#undef CASE

    return n;
    }

static int ImageUsageFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushimageusageflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkimageusageflags(L, 1));
    return 1;
    }

#define Add_ImageUsageFlags(L)  \
    ADD(IMAGE_USAGE_TRANSFER_SRC_BIT);\
    ADD(IMAGE_USAGE_TRANSFER_DST_BIT);\
    ADD(IMAGE_USAGE_SAMPLED_BIT);\
    ADD(IMAGE_USAGE_STORAGE_BIT);\
    ADD(IMAGE_USAGE_COLOR_ATTACHMENT_BIT);\
    ADD(IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);\
    ADD(IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);\
    ADD(IMAGE_USAGE_INPUT_ATTACHMENT_BIT);\
    ADD(IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT);\


/*----------------------------------------------------------------------*
 | VkFormatFeatureFlags                                                 |
 *----------------------------------------------------------------------*/

static VkFlags64 checkformatfeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags64 flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, "sampled image");
        CASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, "storage image");
        CASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT, "storage image atomic");
        CASE(VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT, "uniform texel buffer");
        CASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT, "storage texel buffer");
        CASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, "storage texel buffer atomic");
        CASE(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT, "vertex buffer");
        CASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, "color attachment");
        CASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, "color attachment blend");
        CASE(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, "depth stencil attachment");
        CASE(VK_FORMAT_FEATURE_BLIT_SRC_BIT, "blit src");
        CASE(VK_FORMAT_FEATURE_BLIT_DST_BIT, "blit dst");
        CASE(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, "transfer src");
        CASE(VK_FORMAT_FEATURE_TRANSFER_DST_BIT, "transfer dst");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "sampled image filter linear");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT, "sampled image filter minmax");
        CASE(VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,"midpoint chroma samples");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,"sampled image ycbcr conversion linear filter");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,"sampled image ycbcr conversion separate reconstruction filter");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,"sampled image ycbcr conversion chroma reconstruction explicit");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,"sampled image ycbcr conversion chroma reconstruction explicit forceable");
        CASE(VK_FORMAT_FEATURE_DISJOINT_BIT,"disjoint");
        CASE(VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,"cosited chroma samples");
        CASE(VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT, "fragment density map");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT, "sampled image filter cubic");
        CASE(VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT, "acceleration structure vertex buffer");
        CASE(VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT, "fragment shading rate attachment");
        CASE(VK_FORMAT_FEATURE_STORAGE_READ_WITHOUT_FORMAT_BIT, "storage read without format");
        CASE(VK_FORMAT_FEATURE_STORAGE_WRITE_WITHOUT_FORMAT_BIT, "storage write without format");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT, "sampled image depth comparison");
#undef CASE
        return (VkFlags64)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }


static int pushformatfeatureflags(lua_State *L, VkFlags64 flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, "sampled image");
        CASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, "storage image");
        CASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT, "storage image atomic");
        CASE(VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT, "uniform texel buffer");
        CASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT, "storage texel buffer");
        CASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, "storage texel buffer atomic");
        CASE(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT, "vertex buffer");
        CASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, "color attachment");
        CASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, "color attachment blend");
        CASE(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, "depth stencil attachment");
        CASE(VK_FORMAT_FEATURE_BLIT_SRC_BIT, "blit src");
        CASE(VK_FORMAT_FEATURE_BLIT_DST_BIT, "blit dst");
        CASE(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, "transfer src");
        CASE(VK_FORMAT_FEATURE_TRANSFER_DST_BIT, "transfer dst");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "sampled image filter linear");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT, "sampled image filter minmax");
        CASE(VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,"midpoint chroma samples");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,"sampled image ycbcr conversion linear filter");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,"sampled image ycbcr conversion separate reconstruction filter");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,"sampled image ycbcr conversion chroma reconstruction explicit");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,"sampled image ycbcr conversion chroma reconstruction explicit forceable");
        CASE(VK_FORMAT_FEATURE_DISJOINT_BIT,"disjoint");
        CASE(VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,"cosited chroma samples");
        CASE(VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT, "fragment density map");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT, "sampled image filter cubic");
        CASE(VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT, "acceleration structure vertex buffer");
        CASE(VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT, "fragment shading rate attachment");
        CASE(VK_FORMAT_FEATURE_STORAGE_READ_WITHOUT_FORMAT_BIT, "storage read without format");
        CASE(VK_FORMAT_FEATURE_STORAGE_WRITE_WITHOUT_FORMAT_BIT, "storage write without format");
        CASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT, "sampled image depth comparison");
#undef CASE

    return n;
    }

static int FormatFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushformatfeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkformatfeatureflags(L, 1));
    return 1;
    }

#define Add_FormatFeatureFlags(L)   \
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_IMAGE_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT);\
    ADD(FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT);\
    ADD(FORMAT_FEATURE_VERTEX_BUFFER_BIT);\
    ADD(FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);\
    ADD(FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT);\
    ADD(FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);\
    ADD(FORMAT_FEATURE_BLIT_SRC_BIT);\
    ADD(FORMAT_FEATURE_BLIT_DST_BIT);\
    ADD(FORMAT_FEATURE_TRANSFER_SRC_BIT);\
    ADD(FORMAT_FEATURE_TRANSFER_DST_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT);\
    ADD(FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);\
    ADD(FORMAT_FEATURE_DISJOINT_BIT);\
    ADD(FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);\
    ADD(FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT);\
    ADD(FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT);\
    ADD(FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_READ_WITHOUT_FORMAT_BIT);\
    ADD(FORMAT_FEATURE_STORAGE_WRITE_WITHOUT_FORMAT_BIT);\
    ADD(FORMAT_FEATURE_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT);\

/*----------------------------------------------------------------------*
 | VkSurfaceTransformFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checksurfacetransformflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SURFACE_TRANSFORM_IDENTITY_BIT, "identity");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_90_BIT, "rotate 90");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_180_BIT, "rotate 180");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_270_BIT, "rotate 270");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT, "horizontal mirror");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT, "horizontal mirror rotate 90");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT, "horizontal mirror rotate 180");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT, "horizontal mirror rotate 270");
    CASE(VK_SURFACE_TRANSFORM_INHERIT_BIT, "inherit");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsurfacetransformflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SURFACE_TRANSFORM_IDENTITY_BIT, "identity");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_90_BIT, "rotate 90");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_180_BIT, "rotate 180");
    CASE(VK_SURFACE_TRANSFORM_ROTATE_270_BIT, "rotate 270");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT, "horizontal mirror");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT, "horizontal mirror rotate 90");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT, "horizontal mirror rotate 180");
    CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT, "horizontal mirror rotate 270");
    CASE(VK_SURFACE_TRANSFORM_INHERIT_BIT, "inherit");
#undef CASE

    return n;
    }

static int SurfaceTransformFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsurfacetransformflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksurfacetransformflags(L, 1));
    return 1;
    }

#define Add_SurfaceTransformFlagsKHR(L) \
    ADD(SURFACE_TRANSFORM_IDENTITY_BIT); \
    ADD(SURFACE_TRANSFORM_ROTATE_90_BIT); \
    ADD(SURFACE_TRANSFORM_ROTATE_180_BIT); \
    ADD(SURFACE_TRANSFORM_ROTATE_270_BIT); \
    ADD(SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT);\
    ADD(SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT);\
    ADD(SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT);\
    ADD(SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT);\
    ADD(SURFACE_TRANSFORM_INHERIT_BIT);\


/*----------------------------------------------------------------------*
 | VkCompositeAlphaFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkcompositealphaflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_COMPOSITE_ALPHA_OPAQUE_BIT, "opaque");
    CASE(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT, "pre multiplied");
    CASE(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT, "post multiplied");
    CASE(VK_COMPOSITE_ALPHA_INHERIT_BIT, "inherit");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcompositealphaflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_COMPOSITE_ALPHA_OPAQUE_BIT, "opaque");
    CASE(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT, "pre multiplied");
    CASE(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT, "post multiplied");
    CASE(VK_COMPOSITE_ALPHA_INHERIT_BIT, "inherit");
#undef CASE

    return n;
    }

static int CompositeAlphaFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcompositealphaflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcompositealphaflags(L, 1));
    return 1;
    }

#define Add_CompositeAlphaFlagKHR(L)    \
    ADD(COMPOSITE_ALPHA_OPAQUE_BIT);\
    ADD(COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT);\
    ADD(COMPOSITE_ALPHA_POST_MULTIPLIED_BIT);\
    ADD(COMPOSITE_ALPHA_INHERIT_BIT);\

/*----------------------------------------------------------------------*
 | VkDisplayPlaneAlphaFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkdisplayplanealphaflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT, "opaque");
    CASE(VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT, "global");
    CASE(VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT, "per pixel");
    CASE(VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT, "per pixel premultiplied");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdisplayplanealphaflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT, "opaque");
    CASE(VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT, "global");
    CASE(VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT, "per pixel");
    CASE(VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT, "per pixel premultiplied");
#undef CASE

    return n;
    }

static int DisplayPlaneAlphaFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdisplayplanealphaflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdisplayplanealphaflags(L, 1));
    return 1;
    }

#define Add_DisplayPlaneAlphaFlagsKHR(L)    \
    ADD(DISPLAY_PLANE_ALPHA_OPAQUE_BIT);\
    ADD(DISPLAY_PLANE_ALPHA_GLOBAL_BIT);\
    ADD(DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT);\
    ADD(DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT);\



/*----------------------------------------------------------------------*
 | VkDebugReportFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checkdebugreportflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_DEBUG_REPORT_INFORMATION_BIT, "information");
    CASE(VK_DEBUG_REPORT_WARNING_BIT, "warning");
    CASE(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT, "performance warning");
    CASE(VK_DEBUG_REPORT_ERROR_BIT, "error");
    CASE(VK_DEBUG_REPORT_DEBUG_BIT, "debug");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdebugreportflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_DEBUG_REPORT_INFORMATION_BIT, "information");
    CASE(VK_DEBUG_REPORT_WARNING_BIT, "warning");
    CASE(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT, "performance warning");
    CASE(VK_DEBUG_REPORT_ERROR_BIT, "error");
    CASE(VK_DEBUG_REPORT_DEBUG_BIT, "debug");
#undef CASE

    return n;
    }

static int DebugReportFlagsEXT(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdebugreportflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdebugreportflags(L, 1));
    return 1;
    }

#define Add_DebugReportFlagsEXT(L)  \
    ADD(DEBUG_REPORT_INFORMATION_BIT);\
    ADD(DEBUG_REPORT_WARNING_BIT);\
    ADD(DEBUG_REPORT_PERFORMANCE_WARNING_BIT);\
    ADD(DEBUG_REPORT_ERROR_BIT);\
    ADD(DEBUG_REPORT_DEBUG_BIT);\

/*----------------------------------------------------------------------*
 | VkSurfaceCounterFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checksurfacecounterflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SURFACE_COUNTER_VBLANK_BIT, "vblank");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsurfacecounterflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SURFACE_COUNTER_VBLANK_BIT, "vblank");
#undef CASE

    return n;
    }

static int SurfaceCounterFlagsEXT(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsurfacecounterflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksurfacecounterflags(L, 1));
    return 1;
    }

#define Add_SurfaceCounterFlagsEXT(L)   \
    ADD(SURFACE_COUNTER_VBLANK);\


/*----------------------------------------------------------------------*
 | VkExternalMemoryHandleTypeFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalmemoryhandletypeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, "d3d11 texture");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT, "d3d11 texture kmt");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT, "d3d12 heap");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT, "d3d12 resource");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalmemoryhandletypeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, "d3d11 texture");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT, "d3d11 texture kmt");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT, "d3d12 heap");
    CASE(VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT, "d3d12 resource");
#undef CASE

    return n;
    }

static int ExternalMemoryHandleTypeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalmemoryhandletypeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalmemoryhandletypeflags(L, 1));
    return 1;
    }

#define Add_ExternalMemoryHandleTypeFlags(L) \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT); \
    ADD(EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT); \

/*----------------------------------------------------------------------*
 | VkExternalMemoryFeatureFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalmemoryfeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT, "dedicated only");
    CASE(VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalmemoryfeatureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT, "dedicated only");
    CASE(VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE

    return n;
    }

static int ExternalMemoryFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalmemoryfeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalmemoryfeatureflags(L, 1));
    return 1;
    }

#define Add_ExternalMemoryFeatureFlags(L) \
    ADD(EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT); \
    ADD(EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT); \
    ADD(EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT); \


/*----------------------------------------------------------------------*
 | VkExternalSemaphoreHandleTypeFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalsemaphorehandletypeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT, "d3d12 fence");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT, "sync fd");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalsemaphorehandletypeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT, "d3d12 fence");
    CASE(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT, "sync fd");
#undef CASE

    return n;
    }

static int ExternalSemaphoreHandleTypeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalsemaphorehandletypeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalsemaphorehandletypeflags(L, 1));
    return 1;
    }

#define Add_ExternalSemaphoreHandleTypeFlags(L) \
    ADD(EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT); \
    ADD(EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT); \
    ADD(EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT); \
    ADD(EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT); \
    ADD(EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT); \



/*----------------------------------------------------------------------*
 | VkExternalSemaphoreFeatureFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalsemaphorefeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalsemaphorefeatureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE

    return n;
    }

static int ExternalSemaphoreFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalsemaphorefeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalsemaphorefeatureflags(L, 1));
    return 1;
    }

#define Add_ExternalSemaphoreFeatureFlags(L) \
    ADD(EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT); \
    ADD(EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT); \


/*----------------------------------------------------------------------*
 | VkSemaphoreImportFlags
 *----------------------------------------------------------------------*/

static VkFlags checksemaphoreimportflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SEMAPHORE_IMPORT_TEMPORARY_BIT, "temporary");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsemaphoreimportflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SEMAPHORE_IMPORT_TEMPORARY_BIT, "temporary");
#undef CASE

    return n;
    }

static int SemaphoreImportFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsemaphoreimportflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksemaphoreimportflags(L, 1));
    return 1;
    }

#define Add_SemaphoreImportFlags(L) \
    ADD(SEMAPHORE_IMPORT_TEMPORARY_BIT); \


/*----------------------------------------------------------------------*
 | VkExternalFenceHandleTypeFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalfencehandletypeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT, "sync fd");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalfencehandletypeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT, "opaque fd");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT, "opaque win32");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT, "opaque win32 kmt");
    CASE(VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT, "sync fd");
#undef CASE

    return n;
    }

static int ExternalFenceHandleTypeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalfencehandletypeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalfencehandletypeflags(L, 1));
    return 1;
    }

#define Add_ExternalFenceHandleTypeFlags(L) \
    ADD(EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT); \
    ADD(EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT); \
    ADD(EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT); \
    ADD(EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT); \

/*----------------------------------------------------------------------*
 | VkExternalFenceFeatureFlags
 *----------------------------------------------------------------------*/

static VkFlags checkexternalfencefeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushexternalfencefeatureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT, "exportable");
    CASE(VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT, "importable");
#undef CASE

    return n;
    }

static int ExternalFenceFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushexternalfencefeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkexternalfencefeatureflags(L, 1));
    return 1;
    }

#define Add_ExternalFenceFeatureFlags(L) \
    ADD(EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT); \
    ADD(EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT); \

/*----------------------------------------------------------------------*
 | VkFenceImportFlags
 *----------------------------------------------------------------------*/

static VkFlags checkfenceimportflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_FENCE_IMPORT_TEMPORARY_BIT, "temporary");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushfenceimportflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_FENCE_IMPORT_TEMPORARY_BIT, "temporary");
#undef CASE

    return n;
    }

static int FenceImportFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushfenceimportflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkfenceimportflags(L, 1));
    return 1;
    }

#define Add_FenceImportFlags(L) \
    ADD(FENCE_IMPORT_TEMPORARY_BIT); \


/*----------------------------------------------------------------------*
 | VkDeviceQueueCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkdevicequeuecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT, "protected");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdevicequeuecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT, "protected");
#undef CASE

    return n;
    }

static int DeviceQueueCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdevicequeuecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdevicequeuecreateflags(L, 1));
    return 1;
    }

#define Add_DeviceQueueCreateFlags(L) \
    ADD(DEVICE_QUEUE_CREATE_PROTECTED_BIT);\

/*----------------------------------------------------------------------*
 | VkSubgroupFeatureFlags
 *----------------------------------------------------------------------*/

static VkFlags checksubgroupfeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_SUBGROUP_FEATURE_BASIC_BIT, "basic");
        CASE(VK_SUBGROUP_FEATURE_VOTE_BIT, "vote");
        CASE(VK_SUBGROUP_FEATURE_ARITHMETIC_BIT, "arithmetic");
        CASE(VK_SUBGROUP_FEATURE_BALLOT_BIT, "ballot");
        CASE(VK_SUBGROUP_FEATURE_SHUFFLE_BIT, "shuffle");
        CASE(VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT, "shuffle relative");
        CASE(VK_SUBGROUP_FEATURE_CLUSTERED_BIT, "clustered");
        CASE(VK_SUBGROUP_FEATURE_QUAD_BIT, "quad");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsubgroupfeatureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_SUBGROUP_FEATURE_BASIC_BIT, "basic");
        CASE(VK_SUBGROUP_FEATURE_VOTE_BIT, "vote");
        CASE(VK_SUBGROUP_FEATURE_ARITHMETIC_BIT, "arithmetic");
        CASE(VK_SUBGROUP_FEATURE_BALLOT_BIT, "ballot");
        CASE(VK_SUBGROUP_FEATURE_SHUFFLE_BIT, "shuffle");
        CASE(VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT, "shuffle relative");
        CASE(VK_SUBGROUP_FEATURE_CLUSTERED_BIT, "clustered");
        CASE(VK_SUBGROUP_FEATURE_QUAD_BIT, "quad");
#undef CASE

    return n;
    }

static int SubgroupFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsubgroupfeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksubgroupfeatureflags(L, 1));
    return 1;
    }

#define Add_SubgroupFeatureFlags(L) \
    ADD(SUBGROUP_FEATURE_BASIC_BIT);\
    ADD(SUBGROUP_FEATURE_VOTE_BIT);\
    ADD(SUBGROUP_FEATURE_ARITHMETIC_BIT);\
    ADD(SUBGROUP_FEATURE_BALLOT_BIT);\
    ADD(SUBGROUP_FEATURE_SHUFFLE_BIT);\
    ADD(SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT);\
    ADD(SUBGROUP_FEATURE_CLUSTERED_BIT);\
    ADD(SUBGROUP_FEATURE_QUAD_BIT);\

/*----------------------------------------------------------------------*
 | VkPeerMemoryFeatureFlags
 *----------------------------------------------------------------------*/

static VkFlags checkpeermemoryfeatureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_PEER_MEMORY_FEATURE_COPY_SRC_BIT, "copy src");
        CASE(VK_PEER_MEMORY_FEATURE_COPY_DST_BIT, "copy dst");
        CASE(VK_PEER_MEMORY_FEATURE_GENERIC_SRC_BIT, "generic src");
        CASE(VK_PEER_MEMORY_FEATURE_GENERIC_DST_BIT, "generic dst");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushpeermemoryfeatureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_PEER_MEMORY_FEATURE_COPY_SRC_BIT, "copy src");
        CASE(VK_PEER_MEMORY_FEATURE_COPY_DST_BIT, "copy dst");
        CASE(VK_PEER_MEMORY_FEATURE_GENERIC_SRC_BIT, "generic src");
        CASE(VK_PEER_MEMORY_FEATURE_GENERIC_DST_BIT, "generic dst");
#undef CASE

    return n;
    }

static int PeerMemoryFeatureFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpeermemoryfeatureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpeermemoryfeatureflags(L, 1));
    return 1;
    }

#define Add_PeerMemoryFeatureFlags(L) \
    ADD(PEER_MEMORY_FEATURE_COPY_SRC_BIT);\
    ADD(PEER_MEMORY_FEATURE_COPY_DST_BIT);\
    ADD(PEER_MEMORY_FEATURE_GENERIC_SRC_BIT);\
    ADD(PEER_MEMORY_FEATURE_GENERIC_DST_BIT);\

/*----------------------------------------------------------------------*
 | VkMemoryAllocateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkmemoryallocateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT, "device mask");
        CASE(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, "device address");
        CASE(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmemoryallocateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT, "device mask");
        CASE(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, "device address");
        CASE(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE

    return n;
    }

static int MemoryAllocateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmemoryallocateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmemoryallocateflags(L, 1));
    return 1;
    }

#define Add_MemoryAllocateFlags(L) \
    ADD(MEMORY_ALLOCATE_DEVICE_MASK_BIT);\
    ADD(MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);\
    ADD(MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT);\

/*----------------------------------------------------------------------*
 | VkDebugUtilsMessageSeverityFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checkdebugutilsmessageseverityflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT, "verbose");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT, "info");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT, "warning");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT, "error");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdebugutilsmessageseverityflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT, "verbose");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT, "info");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT, "warning");
        CASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT, "error");
#undef CASE

    return n;
    }

static int DebugUtilsMessageSeverityFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdebugutilsmessageseverityflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdebugutilsmessageseverityflags(L, 1));
    return 1;
    }

#define Add_DebugUtilsMessageSeverityFlags(L) \
    ADD(DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT);\
    ADD(DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT);\
    ADD(DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT);\
    ADD(DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT);\

/*----------------------------------------------------------------------*
 | VkDebugUtilsMessageTypeFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checkdebugutilsmessagetypeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT, "general");
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT, "validation");
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT, "performance");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdebugutilsmessagetypeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT, "general");
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT, "validation");
        CASE(VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT, "performance");
#undef CASE

    return n;
    }

static int DebugUtilsMessageTypeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdebugutilsmessagetypeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdebugutilsmessagetypeflags(L, 1));
    return 1;
    }

#define Add_DebugUtilsMessageTypeFlags(L) \
    ADD(DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT);\
    ADD(DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT);\
    ADD(DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT);\

/*----------------------------------------------------------------------*
 | VkDeviceGroupPresentModeFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkdevicegrouppresentmodeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT, "local");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT, "remote");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_SUM_BIT, "sum");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT, "local multi device");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdevicegrouppresentmodeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT, "local");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT, "remote");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_SUM_BIT, "sum");
        CASE(VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT, "local multi device");
#undef CASE

    return n;
    }

static int DeviceGroupPresentModeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdevicegrouppresentmodeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdevicegrouppresentmodeflags(L, 1));
    return 1;
    }

#define Add_DeviceGroupPresentModeFlags(L) \
    ADD(DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT);\
    ADD(DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT);\
    ADD(DEVICE_GROUP_PRESENT_MODE_SUM_BIT);\
    ADD(DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT);\


/*----------------------------------------------------------------------*
 | VkSwapchainCreateFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkswapchaincreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, "split instance bind regions");
        CASE(VK_SWAPCHAIN_CREATE_PROTECTED_BIT, "protected");
        CASE(VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT, "mutable format");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushswapchaincreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, "split instance bind regions");
        CASE(VK_SWAPCHAIN_CREATE_PROTECTED_BIT, "protected");
        CASE(VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT, "mutable format");
#undef CASE

    return n;
    }

static int SwapchainCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushswapchaincreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkswapchaincreateflags(L, 1));
    return 1;
    }

#define Add_SwapchainCreateFlags(L) \
    ADD(SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT);\
    ADD(SWAPCHAIN_CREATE_PROTECTED_BIT);\
    ADD(SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT);\

/*----------------------------------------------------------------------*
 | VkDescriptorBindingFlags
 *----------------------------------------------------------------------*/

static VkFlags checkdescriptorbindingflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, "update after bind");
        CASE(VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT, "update unused while pending");
        CASE(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, "partially bound");
        CASE(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, "variable descriptor count");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdescriptorbindingflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, "update after bind");
        CASE(VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT, "update unused while pending");
        CASE(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, "partially bound");
        CASE(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, "variable descriptor count");
#undef CASE

    return n;
    }

static int DescriptorBindingFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdescriptorbindingflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdescriptorbindingflags(L, 1));
    return 1;
    }

#define Add_DescriptorBindingFlags(L) \
    ADD(DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);\
    ADD(DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);\
    ADD(DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);\
    ADD(DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);\

/*----------------------------------------------------------------------*
 | VkConditionalRenderingFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checkconditionalrenderingflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_CONDITIONAL_RENDERING_INVERTED_BIT, "inverted");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushconditionalrenderingflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_CONDITIONAL_RENDERING_INVERTED_BIT, "inverted");
#undef CASE

    return n;
    }

static int ConditionalRenderingFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushconditionalrenderingflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkconditionalrenderingflags(L, 1));
    return 1;
    }

#define Add_ConditionalRenderingFlags(L) \
    ADD(CONDITIONAL_RENDERING_INVERTED_BIT);\

/*----------------------------------------------------------------------*
 | VkImageViewCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkimageviewcreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT, "fragment density map dynamic");
    CASE(VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT, "fragment density map deferred");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }
    return flags;
    }

static int pushimageviewcreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;
#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT, "fragment density map dynamic");
    CASE(VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT, "fragment density map deferred");
#undef CASE
    return n;
    }

static int ImageViewCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushimageviewcreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkimageviewcreateflags(L, 1));
    return 1;
    }

#define Add_ImageViewCreateFlags(L) \
    ADD(IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT);\
    ADD(IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT);\


/*----------------------------------------------------------------------*
 | VkSamplerCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checksamplercreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_SAMPLER_CREATE_SUBSAMPLED_BIT, "subsampled");
    CASE(VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT, "subsampled coarse reconstruction");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }
    return flags;
    }

static int pushsamplercreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;
#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_SAMPLER_CREATE_SUBSAMPLED_BIT, "subsampled");
    CASE(VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT, "subsampled coarse reconstruction");
#undef CASE
    return n;
    }

static int SamplerCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsamplercreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksamplercreateflags(L, 1));
    return 1;
    }

#define Add_SamplerCreateFlags(L) \
    ADD(SAMPLER_CREATE_SUBSAMPLED_BIT);\
    ADD(SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT);\


/*----------------------------------------------------------------------*
 | VkFramebufferCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkframebuffercreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, "imageless");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }
    return flags;
    }

static int pushframebuffercreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;
#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, "imageless");
#undef CASE
    return n;
    }

static int FramebufferCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushframebuffercreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkframebuffercreateflags(L, 1));
    return 1;
    }

#define Add_FramebufferCreateFlags(L) \
    ADD(FRAMEBUFFER_CREATE_IMAGELESS_BIT);\

/*----------------------------------------------------------------------*
 | VkResolveModeFlags
 *----------------------------------------------------------------------*/

static VkFlags checkresolvemodeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_RESOLVE_MODE_NONE, "none");
    CASE(VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, "sample zero");
    CASE(VK_RESOLVE_MODE_AVERAGE_BIT, "average");
    CASE(VK_RESOLVE_MODE_MIN_BIT, "min");
    CASE(VK_RESOLVE_MODE_MAX_BIT, "max");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }
    return flags;
    }

static int pushresolvemodeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;
#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_RESOLVE_MODE_NONE, "none");
    CASE(VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, "sample zero");
    CASE(VK_RESOLVE_MODE_AVERAGE_BIT, "average");
    CASE(VK_RESOLVE_MODE_MIN_BIT, "min");
    CASE(VK_RESOLVE_MODE_MAX_BIT, "max");
#undef CASE
    return n;
    }

static int ResolveModeFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushresolvemodeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkresolvemodeflags(L, 1));
    return 1;
    }

#define Add_ResolveModeFlags(L) \
    ADD(RESOLVE_MODE_NONE);\
    ADD(RESOLVE_MODE_SAMPLE_ZERO_BIT);\
    ADD(RESOLVE_MODE_AVERAGE_BIT);\
    ADD(RESOLVE_MODE_MIN_BIT);\
    ADD(RESOLVE_MODE_MAX_BIT);\


/*----------------------------------------------------------------------*
 | VkPipelineCreationFeedbackFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checkpipelinecreationfeedbackflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT, "valid");
    CASE(VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT, "application pipeline cache hit");
    CASE(VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT, "base pipeline acceleration");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }
    return flags;
    }

static int pushpipelinecreationfeedbackflags(lua_State *L, VkFlags flags)
    {
    int n = 0;
#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT, "valid");
    CASE(VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT, "application pipeline cache hit");
    CASE(VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT, "base pipeline acceleration");
#undef CASE
    return n;
    }

static int PipelineCreationFeedbackFlagsEXT(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpipelinecreationfeedbackflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpipelinecreationfeedbackflags(L, 1));
    return 1;
    }

#define Add_PipelineCreationFeedbackFlags(L) \
    ADD(PIPELINE_CREATION_FEEDBACK_VALID_BIT);\
    ADD(PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT);\
    ADD(PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT);\

/*----------------------------------------------------------------------*
 | VkEventCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkeventcreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_EVENT_CREATE_DEVICE_ONLY_BIT, "device only");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pusheventcreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_EVENT_CREATE_DEVICE_ONLY_BIT, "device only");
#undef CASE

    return n;
    }

static int EventCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pusheventcreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkeventcreateflags(L, 1));
    return 1;
    }

#define Add_EventCreateFlags(L) \
    ADD(EVENT_CREATE_DEVICE_ONLY_BIT);\

/*----------------------------------------------------------------------*
 | VkPipelineCacheCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkpipelinecachecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT, "externally synchronized");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushpipelinecachecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT, "externally synchronized");
#undef CASE

    return n;
    }

static int PipelineCacheCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpipelinecachecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpipelinecachecreateflags(L, 1));
    return 1;
    }

#define Add_PipelineCacheCreateFlags(L) \
    ADD(PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT);\


/*----------------------------------------------------------------------*
 | VkPipelineShaderStageCreateFlags
 *----------------------------------------------------------------------*/

static VkFlags checkpipelineshaderstagecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT, "allow varying subgroup size");
        CASE(VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT, "require full subgroups");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushpipelineshaderstagecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT, "allow varying subgroup size");
        CASE(VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT, "require full subgroups");
#undef CASE

    return n;
    }

static int PipelineShaderStageCreateFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushpipelineshaderstagecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkpipelineshaderstagecreateflags(L, 1));
    return 1;
    }

#define Add_PipelineShaderStageCreateFlags(L) \
    ADD(PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT);\
    ADD(PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT);\


/*----------------------------------------------------------------------*
 | VkSemaphoreWaitFlags
 *----------------------------------------------------------------------*/

static VkFlags checksemaphorewaitflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_SEMAPHORE_WAIT_ANY_BIT, "any");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsemaphorewaitflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_SEMAPHORE_WAIT_ANY_BIT, "any");
#undef CASE

    return n;
    }

static int SemaphoreWaitFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsemaphorewaitflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksemaphorewaitflags(L, 1));
    return 1;
    }

#define Add_SemaphoreWaitFlags(L) \
    ADD(SEMAPHORE_WAIT_ANY_BIT);\

/*----------------------------------------------------------------------*
 | VkPerformanceCounterDescriptionFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkperformancecounterdescriptionflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_PERFORMANCE_COUNTER_DESCRIPTION_PERFORMANCE_IMPACTING_BIT, "performance impacting");
        CASE(VK_PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_BIT, "concurrently impacted");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushperformancecounterdescriptionflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_PERFORMANCE_COUNTER_DESCRIPTION_PERFORMANCE_IMPACTING_BIT, "performance impacting");
        CASE(VK_PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_BIT, "concurrently impacted");
#undef CASE

    return n;
    }

static int PerformanceCounterDescriptionFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushperformancecounterdescriptionflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkperformancecounterdescriptionflags(L, 1));
    return 1;
    }

#define Add_PerformanceCounterDescriptionFlagsKHR(L) \
    ADD(PERFORMANCE_COUNTER_DESCRIPTION_PERFORMANCE_IMPACTING_BIT);\
    ADD(PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_BIT);\

/*----------------------------------------------------------------------*
 | VkSubmitFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checksubmitflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_SUBMIT_PROTECTED_BIT, "protected");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushsubmitflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_SUBMIT_PROTECTED_BIT, "protected");
#undef CASE

    return n;
    }

static int SubmitFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushsubmitflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checksubmitflags(L, 1));
    return 1;
    }

#define Add_SubmitFlagsKHR(L) \
    ADD(SUBMIT_PROTECTED_BIT);\

/*----------------------------------------------------------------------*
 | VkGeometryFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkgeometryflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_GEOMETRY_OPAQUE_BIT, "opaque");
        CASE(VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT, "no duplicate any hit invocation");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushgeometryflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_GEOMETRY_OPAQUE_BIT, "opaque");
        CASE(VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT, "no duplicate any hit invocation");
#undef CASE

    return n;
    }

static int GeometryFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushgeometryflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkgeometryflags(L, 1));
    return 1;
    }

#define Add_GeometryFlagsKHR(L) \
    ADD(GEOMETRY_OPAQUE_BIT);\
    ADD(GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT);\

/*----------------------------------------------------------------------*
 | VkGeometryInstanceFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkgeometryinstanceflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT, "triangle facing cull disable");
        CASE(VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT, "triangle flip facing");
        CASE(VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT, "force opaque");
        CASE(VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT, "force no opaque");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushgeometryinstanceflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT, "triangle facing cull disable");
        CASE(VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT, "triangle flip facing");
        CASE(VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT, "force opaque");
        CASE(VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT, "force no opaque");
#undef CASE

    return n;
    }

static int GeometryInstanceFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushgeometryinstanceflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkgeometryinstanceflags(L, 1));
    return 1;
    }

#define Add_GeometryInstanceFlagsKHR(L) \
    ADD(GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT);\
    ADD(GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT);\
    ADD(GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT);\
    ADD(GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT);\

/*----------------------------------------------------------------------*
 | VkBuildAccelerationStructureFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkbuildaccelerationstructureflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT, "allow update");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT, "allow compaction");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT, "prefer fast trace");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT, "prefer fast build");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT, "low memory");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushbuildaccelerationstructureflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT, "allow update");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT, "allow compaction");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT, "prefer fast trace");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT, "prefer fast build");
        CASE(VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT, "low memory");
#undef CASE

    return n;
    }

static int BuildAccelerationStructureFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushbuildaccelerationstructureflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkbuildaccelerationstructureflags(L, 1));
    return 1;
    }

#define Add_BuildAccelerationStructureFlagsKHR(L) \
    ADD(BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT);\
    ADD(BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT);\
    ADD(BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT);\
    ADD(BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT);\
    ADD(BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT);\
 
/*----------------------------------------------------------------------*
 | VkToolPurposeFlagsEXT
 *----------------------------------------------------------------------*/

static VkFlags checktoolpurposeflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_TOOL_PURPOSE_VALIDATION_BIT, "validation");
        CASE(VK_TOOL_PURPOSE_PROFILING_BIT, "profiling");
        CASE(VK_TOOL_PURPOSE_TRACING_BIT, "tracing");
        CASE(VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT, "additional features");
        CASE(VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT, "modifying features");
        CASE(VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT, "debug reporting");
        CASE(VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT, "debug markers");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushtoolpurposeflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_TOOL_PURPOSE_VALIDATION_BIT, "validation");
        CASE(VK_TOOL_PURPOSE_PROFILING_BIT, "profiling");
        CASE(VK_TOOL_PURPOSE_TRACING_BIT, "tracing");
        CASE(VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT, "additional features");
        CASE(VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT, "modifying features");
        CASE(VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT, "debug reporting");
        CASE(VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT, "debug markers");
#undef CASE

    return n;
    }

static int ToolPurposeFlagsEXT(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushtoolpurposeflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checktoolpurposeflags(L, 1));
    return 1;
    }

#define Add_ToolPurposeFlagsEXT(L) \
    ADD(TOOL_PURPOSE_VALIDATION_BIT);\
    ADD(TOOL_PURPOSE_PROFILING_BIT);\
    ADD(TOOL_PURPOSE_TRACING_BIT);\
    ADD(TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT);\
    ADD(TOOL_PURPOSE_MODIFYING_FEATURES_BIT);\
    ADD(TOOL_PURPOSE_DEBUG_REPORTING_BIT);\
    ADD(TOOL_PURPOSE_DEBUG_MARKERS_BIT);\

/*----------------------------------------------------------------------*
 | VkAccelerationStructureCreateFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkaccelerationstructurecreateflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushaccelerationstructurecreateflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "device address capture replay");
#undef CASE

    return n;
    }

static int AccelerationStructureCreateFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushaccelerationstructurecreateflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkaccelerationstructurecreateflags(L, 1));
    return 1;
    }

#define Add_AccelerationStructureCreateFlagsKHR(L) \
    ADD(ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT);\

/*----------------------------------------------------------------------*
 | VkRenderingFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkrenderingflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT, "contents secondary command buffers");
    CASE(VK_RENDERING_SUSPENDING_BIT, "suspending");
    CASE(VK_RENDERING_RESUMING_BIT, "resuming");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushrenderingflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT, "contents secondary command buffers");
    CASE(VK_RENDERING_SUSPENDING_BIT, "suspending");
    CASE(VK_RENDERING_RESUMING_BIT, "resuming");
#undef CASE

    return n;
    }

static int RenderingFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushrenderingflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkrenderingflags(L, 1));
    return 1;
    }

#define Add_RenderingFlagsKHR(L) \
    ADD(RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT);\
    ADD(RENDERING_SUSPENDING_BIT);\
    ADD(RENDERING_RESUMING_BIT);\


/*------------------------------------------------------------------------------*
 | Additional utilities                                                         |
 *------------------------------------------------------------------------------*/

static int AddConstants(lua_State *L) /* vk.XXX constants for VK_XXX values */
    {
    Add_StencilFaceFlags(L);
    Add_CommandBufferResetFlags(L);
    Add_QueryControlFlags(L);
    Add_CommandBufferUsageFlags(L);
    Add_CommandPoolResetFlags(L);
    Add_CommandPoolCreateFlags(L);
    Add_DependencyFlags(L);
    Add_AccessFlags(L);
    Add_AttachmentDescriptionFlags(L);
    Add_DescriptorPoolCreateFlags(L);
    Add_DescriptorSetLayoutCreateFlags(L);
    Add_ColorComponentFlags(L);
    Add_CullModeFlags(L);
    Add_ShaderStageFlags(L);
    Add_PipelineCreateFlags(L);
    Add_BufferUsageFlags(L);
    Add_BufferCreateFlags(L);
    Add_QueryResultFlags(L);
    Add_QueryPipelineStatisticFlags(L);
    Add_FenceCreateFlags(L);
    Add_SparseMemoryBindFlags(L);
    Add_SparseImageFormatFlags(L);
    Add_ImageAspectFlags(L);
    Add_PipelineStageFlags(L);
    Add_MemoryHeapFlag(L);
    Add_MemoryPropertyFlags(L);
    Add_QueueFlags(L);
    Add_SampleCountFlags(L);
    Add_ImageCreateFlags(L);
    Add_ImageUsageFlags(L);
    Add_FormatFeatureFlags(L);
    Add_DeviceQueueCreateFlags(L);
    Add_SubgroupFeatureFlags(L);
    Add_PeerMemoryFeatureFlags(L);
    Add_MemoryAllocateFlags(L);
    Add_ImageViewCreateFlags(L);
    Add_SamplerCreateFlags(L);
    Add_FramebufferCreateFlags(L);
    Add_ResolveModeFlags(L);
    Add_PipelineCreationFeedbackFlags(L);
    Add_EventCreateFlags(L);
    Add_PipelineCacheCreateFlags(L);
    Add_PipelineShaderStageCreateFlags(L);
    Add_SemaphoreWaitFlags(L);
    Add_PerformanceCounterDescriptionFlagsKHR(L);
    Add_SubmitFlagsKHR(L);
    Add_GeometryFlagsKHR(L);
    Add_GeometryInstanceFlagsKHR(L);
    Add_BuildAccelerationStructureFlagsKHR(L);
    Add_ToolPurposeFlagsEXT(L);
    Add_AccelerationStructureCreateFlagsKHR(L);
    Add_SurfaceTransformFlagsKHR(L);
    Add_CompositeAlphaFlagKHR(L);
    Add_DisplayPlaneAlphaFlagsKHR(L);
    Add_DebugReportFlagsEXT(L);
    Add_SurfaceCounterFlagsEXT(L);
    Add_ExternalMemoryHandleTypeFlags(L);
    Add_ExternalMemoryFeatureFlags(L);
    Add_ExternalSemaphoreHandleTypeFlags(L);
    Add_ExternalSemaphoreFeatureFlags(L);
    Add_SemaphoreImportFlags(L);
    Add_ExternalFenceHandleTypeFlags(L);
    Add_ExternalFenceFeatureFlags(L);
    Add_FenceImportFlags(L);
    Add_DebugUtilsMessageSeverityFlags(L);
    Add_DebugUtilsMessageTypeFlags(L);
    Add_DeviceGroupPresentModeFlags(L);
    Add_SwapchainCreateFlags(L);
    Add_DescriptorBindingFlags(L);
    Add_ConditionalRenderingFlags(L);
    Add_RenderingFlagsKHR(L);
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "stencilfaceflags", StencilFaceFlags },
        { "commandbufferresetflags", CommandBufferResetFlags },
        { "querycontrolflags", QueryControlFlags },
        { "commandbufferusageflags", CommandBufferUsageFlags },
        { "commandpoolresetflags", CommandPoolResetFlags },
        { "commandpoolcreateflags", CommandPoolCreateFlags },
        { "dependencyflags", DependencyFlags },
        { "accessflags", AccessFlags },
        { "attachmentdescriptionflags", AttachmentDescriptionFlags },
        { "descriptorpoolcreateflags", DescriptorPoolCreateFlags },
        { "descriptorsetlayoutcreateflags", DescriptorSetLayoutCreateFlags },
        { "colorcomponentflags", ColorComponentFlags },
        { "cullmodeflags", CullModeFlags },
        { "shaderstageflags", ShaderStageFlags },
        { "pipelinecreateflags", PipelineCreateFlags },
        { "bufferusageflags", BufferUsageFlags },
        { "buffercreateflags", BufferCreateFlags },
        { "queryresultflags", QueryResultFlags },
        { "querypipelinestatisticflags", QueryPipelineStatisticFlags },
        { "fencecreateflags", FenceCreateFlags },
        { "sparsememorybindflags", SparseMemoryBindFlags },
        { "sparseimageformatflags", SparseImageFormatFlags },
        { "imageaspectflags", ImageAspectFlags },
        { "pipelinestageflags", PipelineStageFlags },
        { "memoryheapflags", MemoryHeapFlag},
        { "memorypropertyflags", MemoryPropertyFlags },
        { "queueflags", QueueFlags },
        { "samplecountflags", SampleCountFlags },
        { "imagecreateflags", ImageCreateFlags },
        { "imageusageflags", ImageUsageFlags },
        { "formatfeatureflags", FormatFeatureFlags },
        { "devicequeuecreateflags", DeviceQueueCreateFlags },
        { "subgroupfeatureflags", SubgroupFeatureFlags },
        { "peermemoryfeatureflags", PeerMemoryFeatureFlags },
        { "memoryallocateflags", MemoryAllocateFlags },
        { "imageviewcreateflags", ImageViewCreateFlags },
        { "samplercreateflags", SamplerCreateFlags },
        { "framebuffercreateflags", FramebufferCreateFlags },
        { "resolvemodeflags", ResolveModeFlags },
        { "pipelinecreationfeedbackflags", PipelineCreationFeedbackFlagsEXT },
        { "eventcreateflags", EventCreateFlags },
        { "pipelinecachecreateflags", PipelineCacheCreateFlags },
        { "pipelineshaderstagecreateflags", PipelineShaderStageCreateFlags },
        { "semaphorewaitflags", SemaphoreWaitFlags },
        { "performancecounterdescriptionflags", PerformanceCounterDescriptionFlagsKHR },
        { "submitflags", SubmitFlagsKHR },
        { "geometryflags", GeometryFlagsKHR },
        { "geometryinstanceflags", GeometryInstanceFlagsKHR },
        { "buildaccelerationstructureflags", BuildAccelerationStructureFlagsKHR },
        { "toolpurposeflags", ToolPurposeFlagsEXT },
        { "surfacetransformflags", SurfaceTransformFlagsKHR },
        { "compositealphaflags", CompositeAlphaFlagsKHR },
        { "displayplanealphaflags", DisplayPlaneAlphaFlagsKHR },
        { "debugreportflags", DebugReportFlagsEXT },
        { "surfacecounterflags", SurfaceCounterFlagsEXT },
        { "externalmemoryhandletypeflags", ExternalMemoryHandleTypeFlags },
        { "externalmemoryfeatureflags", ExternalMemoryFeatureFlags },
        { "externalsemaphorehandletypeflags", ExternalSemaphoreHandleTypeFlags },
        { "externalsemaphorefeatureflags", ExternalSemaphoreFeatureFlags },
        { "semaphoreimportflags", SemaphoreImportFlags },
        { "externalfencehandletypeflags", ExternalFenceHandleTypeFlags },
        { "externalfencefeatureflags", ExternalFenceFeatureFlags },
        { "fenceimportflags", FenceImportFlags },
        { "debugutilsmessageseverityflags", DebugUtilsMessageSeverityFlags },
        { "debugutilsmessagetypeflags", DebugUtilsMessageTypeFlags },
        { "devicegrouppresentmodeflags", DeviceGroupPresentModeFlags },
        { "swapchaincreateflags", SwapchainCreateFlags },
        { "descriptorbindingflags", DescriptorBindingFlags },
        { "conditionalrenderingflags", ConditionalRenderingFlags },
        { "accelerationstructurecreateflags", AccelerationStructureCreateFlagsKHR },
        { "renderingflags", RenderingFlagsKHR },
        /* Reserved flags */
        { "instancecreateflags", ReservedFlags },
        { "devicecreateflags", ReservedFlags },
        { "memorymapflags", ReservedFlags },
        { "semaphorecreateflags", ReservedFlags },
        { "querypoolcreateflags", ReservedFlags },
        { "bufferviewcreateflags", ReservedFlags },
        { "shadermodulecreateflags", ReservedFlags },
        { "pipelinevertexinputstatecreateflags", ReservedFlags },
        { "pipelineinputassemblystatecreateflags", ReservedFlags },
        { "pipelinetessellationstatecreateflags", ReservedFlags },
        { "pipelineviewportstatecreateflags", ReservedFlags },
        { "pipelinerasterizationstatecreateflags", ReservedFlags },
        { "pipelinemultisamplestatecreateflags", ReservedFlags },
        { "pipelinedepthstencilstatecreateflags", ReservedFlags },
        { "pipelinecolorblendstatecreateflags", ReservedFlags },
        { "pipelinedynamicstatecreateflags", ReservedFlags },
        { "pipelinelayoutcreateflags", ReservedFlags },
        { "descriptorpoolresetflags", ReservedFlags },
        { "renderpasscreateflags", ReservedFlags },
        { "subpassdescriptionflags", ReservedFlags },
        { "displaymodecreateflags", ReservedFlags },
        { "displaysurfacecreateflags", ReservedFlags },
        { "xlibsurfacecreateflags", ReservedFlags },
        { "xcbsurfacecreateflags", ReservedFlags },
        { "waylandsurfacecreateflags", ReservedFlags },
        { "mirsurfacecreateflags", ReservedFlags },
        { "androidsurfacecreateflags", ReservedFlags },
        { "win32surfacecreateflags", ReservedFlags },
        { "commandpooltrimflags", ReservedFlags },
        { "descriptorupdatetemplatecreateflags", ReservedFlags },
        { "pipelinediscardrectanglestatecreateflags", ReservedFlags },
        { "validationcachecreateflags", ReservedFlags },
        { "debugutilsmessengercallbackdataflags", ReservedFlags },
        { "debugutilsmessengercreateflags", ReservedFlags },
        { "pipelinerasterizationconservativestatecreateflags", ReservedFlags },
        { "pipelinerasterizationstatestreamcreateflags", ReservedFlags },
        { "pipelinerasterizationdepthclipstatecreateflags", ReservedFlags },
        { "acquireprofilinglockflags", ReservedFlags },
        { "privatedataslotcreateflags", ReservedFlags },
        { "headlesssurfacecreateflags", ReservedFlags },
//      { "", ReservedFlags }, /*  */
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_flags(lua_State *L)
    {
    AddConstants(L);
    luaL_setfuncs(L, Functions, 0);
    }


#if 0 // scaffolding

/*----------------------------------------------------------------------*
 | VkZzzFlagsKHR
 *----------------------------------------------------------------------*/

static VkFlags checkzzzflags(lua_State *L, int arg)
    {
    const char *s;
    VkFlags flags = 0;

    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(VK_ZZZ_, "");
#undef CASE
        return (VkFlags)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushzzzflags(lua_State *L, VkFlags flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(VK_ZZZ_, "");
#undef CASE

    return n;
    }

static int ZzzFlagsKHR(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushzzzflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkzzzflags(L, 1));
    return 1;
    }

    Add_ZzzFlagsKHR(L);
        { "zzzflags", ZzzFlagsKHR },
#define Add_ZzzFlagsKHR(L) \
    ADD(ZZZ_);\

[[zzzflags]]
[small]#*zzzflags*: vk.ZZZ_XXX_BIT+
Values:
Rfr: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkZzzFlagBitsKHR.html[VkZzzFlagBitsKHR].#

#endif


