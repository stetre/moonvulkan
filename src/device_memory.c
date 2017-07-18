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

typedef struct {
    char *memp;    /* start of mapped area (=NULL if not mapped) */
    size_t memsz;  /* size of mapped area (=0 if not mapped) */
    size_t maxsz; /* max size (allocationSize) */
} ud_info_t;


static int freedevice_memory(lua_State *L, ud_t *ud)
    {
    VkDeviceMemory device_memory = (VkDeviceMemory)ud->handle;
    VkDevice device = ud->device;
    const VkAllocationCallbacks *allocator = ud->allocator;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(device_memory, "device_memory");
    UD(device)->ddt->FreeMemory(device, device_memory, allocator);
    return 0;
    }

static int Allocate(lua_State *L)
    {
    VkResult ec;
    ud_info_t *ud_info;
    ud_t *ud, *device_ud;
    VkDeviceMemory device_memory;
    VkMemoryAllocateInfo_CHAIN info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        if(echeckmemoryallocateinfo(L, 2, &info)) return argerror(L, 2);
        }
    else
        {
        info.p1.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.p1.pNext = NULL;
        info.p1.allocationSize = luaL_checkinteger(L, 2);
        info.p1.memoryTypeIndex = luaL_checkinteger(L, 3);
        }

    ud_info = (ud_info_t*)MallocNoErr(L, sizeof(ud_info_t));
    if(!ud_info) return errmemory(L);

    ec = device_ud->ddt->AllocateMemory(device, &info.p1, allocator, &device_memory);
    if(ec)
        {
        Free(L, ud_info);
        CheckError(L, ec);
        }
    TRACE_CREATE(device_memory, "device_memory");
    ud = newuserdata_nondispatchable(L, device_memory, DEVICE_MEMORY_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->destructor = freedevice_memory;
    ud->allocator = allocator;
    ud->ddt = device_ud->ddt;
    ud->info = ud_info;
    ud_info->maxsz = info.p1.allocationSize;
    return 1;
    }


static int GetDeviceMemoryCommitment(lua_State *L)
    {
    VkDeviceSize bytes;
    ud_t *ud;
    VkDeviceMemory memory = checkdevice_memory(L, 1, &ud);
    VkDevice device = ud->device;
    ud->ddt->GetDeviceMemoryCommitment(device, memory, &bytes);
    lua_pushinteger(L, bytes);
    return 1;
    }


/*--------------------------------------------------------------------------------*/

static int MapMemory(lua_State *L)
    {
    VkResult ec;
    void *data;
    ud_t *ud;
    VkDeviceMemory memory = checkdevice_memory(L, 1, &ud);
    ud_info_t *ud_info = (ud_info_t*)ud->info;
    VkDevice device = ud->device;
    VkDeviceSize offset = luaL_checkinteger(L, 2);
    VkDeviceSize size = checksizeorwholesize(L, 3);
    VkMemoryMapFlags flags = optflags(L, 4, 0);

    if(ud_info->memp)
        return luaL_error(L, "memory is already mapped");

    ec = ud->ddt->MapMemory(device, memory, offset, size, flags, &data);
    CheckError(L, ec);
    ud_info->memp = (char*)data;
    ud_info->memsz = (size == VK_WHOLE_SIZE) ? ud_info->maxsz - offset : size;
    return 0;
    }

static int UnmapMemory(lua_State *L)
    {
    ud_t *ud;
    VkDeviceMemory memory = checkdevice_memory(L, 1, &ud);
    ud_info_t *ud_info = (ud_info_t*)ud->info;
    VkDevice device = ud->device;

    if(ud_info->memp == NULL)
        return luaL_error(L, "memory is not mapped");

    ud->ddt->UnmapMemory(device, memory);
    ud_info->memp = NULL;
    ud_info->memsz = 0;
    return 0;
    }

static int Write(lua_State *L) /* NONVK */
    {
    size_t size;
    ud_t *ud;
    ud_info_t *ud_info;
    VkDeviceSize offset;
    const char *data;

    (void)checkdevice_memory(L, 1, &ud);
    ud_info = (ud_info_t*)ud->info;
    offset = luaL_checkinteger(L, 2);
    data = luaL_tolstring(L, 3, &size);
    if(!ud_info->memp)
        return luaL_error(L, "memory is not mapped");
    /* boundary checks */
    if(size > (ud_info->memsz - offset))
        return argerrorc(L, 3, ERR_LENGTH);
    memcpy(ud_info->memp + offset, data, size);
    return 0;
    }


static int Read(lua_State *L) /* NONVK */
    {
    ud_t *ud;
    ud_info_t *ud_info;
    VkDeviceSize offset, size;

    (void)checkdevice_memory(L, 1, &ud);
    ud_info = (ud_info_t*)ud->info;
    offset = luaL_checkinteger(L, 2);
    size = checksizeorwholesize(L, 3);
    if(!ud_info->memp)
        return luaL_error(L, "memory is not mapped");
    /* boundary checks */
    if(size == VK_WHOLE_SIZE)
        size = ud_info->memsz - offset;
    else if(size > (ud_info->memsz - offset))
        return argerrorc(L, 3, ERR_LENGTH);
    lua_pushlstring(L, ud_info->memp + offset, size);
    return 1;
    }

static int FlushMappedMemoryRanges(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkDevice device = checkdevice(L, 1, &ud);
    VkMappedMemoryRange* ranges = echeckmappedmemoryrangelist(L, 2, &count, &err);
    if(err) return argerrorc(L, 2, err);
    
    ec = ud->ddt->FlushMappedMemoryRanges(device, count, ranges);
    Free(L, ranges);
    CheckError(L, ec);
    return 0;
    }

static int InvalidateMappedMemoryRanges(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkDevice device = checkdevice(L, 1, &ud);
    VkMappedMemoryRange* ranges = echeckmappedmemoryrangelist(L, 2, &count, &err);
    if(err) return argerrorc(L, 2, err);
    
    ec = ud->ddt->InvalidateMappedMemoryRanges(device, count, ranges);
    Free(L, ranges);
    CheckError(L, ec);
    return 0;
    }

/*--------------------------------------------------------------------------------*/

static int GetBufferMemoryRequirements2(lua_State *L, VkBuffer buffer, ud_t *ud)
    {
    VkMemoryRequirements2KHR_CHAIN req;
    VkBufferMemoryRequirementsInfo2KHR info;
    VkDevice device = ud->device;

    info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
    info.pNext = NULL;
    info.buffer = buffer;

    initmemoryrequirements2(L, &req);
    ud->ddt->GetBufferMemoryRequirements2KHR(device, &info, &req.p1);
    pushmemoryrequirements2(L, &req);
    return 1;
    }

static int GetBufferMemoryRequirements(lua_State *L)
    {
    ud_t *ud;
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    VkDevice device = ud->device;
    VkMemoryRequirements req;

    if(ud->ddt->GetBufferMemoryRequirements2KHR)
        return GetBufferMemoryRequirements2(L, buffer, ud);

    ud->ddt->GetBufferMemoryRequirements(device, buffer, &req);
    pushmemoryrequirements(L, &req);
    return 1;
    }

static int GetImageMemoryRequirements2(lua_State *L, VkImage image, ud_t *ud)
    {
    VkMemoryRequirements2KHR_CHAIN req;
    VkImageMemoryRequirementsInfo2KHR info;
    VkDevice device = ud->device;

    info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR;
    info.pNext = NULL;
    info.image = image;

    initmemoryrequirements2(L, &req);
    ud->ddt->GetImageMemoryRequirements2KHR(device, &info, &req.p1);
    pushmemoryrequirements2(L, &req);
    return 1;
    }

static int GetImageMemoryRequirements(lua_State *L)
    {
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;
    VkMemoryRequirements req;
    if(ud->ddt->GetImageMemoryRequirements2KHR)
        return GetImageMemoryRequirements2(L, image, ud);

    ud->ddt->GetImageMemoryRequirements(device, image, &req);
    pushmemoryrequirements(L, &req);
    return 1;
    }


static int GetImageSparseMemoryRequirements2(lua_State *L, VkImage image, ud_t *ud)
    {
    uint32_t count, i;
    VkSparseImageMemoryRequirements2KHR *requirements;
    VkImageSparseMemoryRequirementsInfo2KHR info;
    VkDevice device = ud->device;

    info.sType = VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2_KHR;
    info.pNext = NULL;
    info.image = image;

    lua_newtable(L);
    ud->ddt->GetImageSparseMemoryRequirements2KHR(device, &info, &count, NULL);

    if(count == 0)
        return 1;

    requirements = newsparseimagememoryrequirements2(L, count);

    ud->ddt->GetImageSparseMemoryRequirements2KHR(device, &info, &count, requirements);
    for(i = 0; i <count; i++)
        {
        pushsparseimagememoryrequirements2(L, &requirements[i]);
        lua_rawseti(L, -2, i+1);
        }

    freesparseimagememoryrequirements2(L, requirements, count);
    return 1;
    }


static int GetImageSparseMemoryRequirements(lua_State *L)
    {
    uint32_t count, i;
    VkSparseImageMemoryRequirements *requirements;
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;

    if(ud->ddt->GetImageSparseMemoryRequirements2KHR)
        return GetImageSparseMemoryRequirements2(L, image, ud);

    lua_newtable(L);
    ud->ddt->GetImageSparseMemoryRequirements(device, image, &count, NULL);

    if(count == 0)
        return 1;

    requirements =
        (VkSparseImageMemoryRequirements*)Malloc(L, sizeof(VkSparseImageMemoryRequirements)*count);

    ud->ddt->GetImageSparseMemoryRequirements(device, image, &count, requirements);
    for(i = 0; i <count; i++)
        {
        pushsparseimagememoryrequirements(L, &requirements[i]);
        lua_rawseti(L, -2, i+1);
        }

    Free(L, requirements);
    return 1;
    }


/*--------------------------------------------------------------------------------*/

static int BindBufferMemory(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    VkDevice device = ud->device;
    VkDeviceMemory memory = checkdevice_memory(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    ec = ud->ddt->BindBufferMemory(device, buffer, memory, offset);
    CheckError(L, ec);
    return 0;
    }

static int BindImageMemory(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;
    VkDeviceMemory memory = checkdevice_memory(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    ec = ud->ddt->BindImageMemory(device, image, memory, offset);
    CheckError(L, ec);
    return 0;
    }


RAW_FUNC(device_memory)
TYPE_FUNC(device_memory)
INSTANCE_FUNC(device_memory)
DEVICE_FUNC(device_memory)
PARENT_FUNC(device_memory)
DELETE_FUNC(device_memory)
DESTROY_FUNC(device_memory)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "instance", Instance },
        { "device", Device },
        { "parent", Parent },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "allocate_memory",  Allocate },
        { "free_memory",  Destroy },
        { "map_memory", MapMemory },
        { "unmap_memory", UnmapMemory },
        { "get_device_memory_commitment", GetDeviceMemoryCommitment },
        { "flush_mapped_memory_ranges", FlushMappedMemoryRanges },
        { "invalidate_mapped_memory_ranges", InvalidateMappedMemoryRanges },
        { "write_memory", Write },
        { "read_memory", Read },
        { "get_buffer_memory_requirements", GetBufferMemoryRequirements },
        { "get_image_memory_requirements", GetImageMemoryRequirements },
        { "get_image_sparse_memory_requirements", GetImageSparseMemoryRequirements },
        { "bind_image_memory", BindImageMemory },
        { "bind_buffer_memory", BindBufferMemory },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_device_memory(lua_State *L)
    {
    udata_define(L, DEVICE_MEMORY_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

