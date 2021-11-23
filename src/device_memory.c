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
    int err;
    VkResult ec;
    ud_info_t *ud_info;
    ud_t *ud, *device_ud;
    VkDeviceMemory device_memory;
    VkMemoryAllocateInfo* info;
    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

#define CLEANUP zfreeVkMemoryAllocateInfo(L, info, 1)
    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        info = zcheckVkMemoryAllocateInfo(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        VkDeviceSize allocationSize = luaL_checkinteger(L, 2);
        uint32_t memoryTypeIndex = luaL_checkinteger(L, 3);
        info = znewVkMemoryAllocateInfo(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        info->allocationSize = allocationSize;
        info->memoryTypeIndex = memoryTypeIndex;
        }

    ud_info = (ud_info_t*)MallocNoErr(L, sizeof(ud_info_t));
    if(!ud_info) { CLEANUP; return errmemory(L); }

    ec = device_ud->ddt->AllocateMemory(device, info, allocator, &device_memory);
    
    if(ec) { CLEANUP; Free(L, ud_info); CheckError(L, ec); return 0; }
    TRACE_CREATE(device_memory, "device_memory");
    ud = newuserdata_nondispatchable(L, device_memory, DEVICE_MEMORY_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->destructor = freedevice_memory;
    ud->allocator = allocator;
    ud->ddt = device_ud->ddt;
    ud->info = ud_info;
    ud_info->maxsz = info->allocationSize;
    CLEANUP;
#undef CLEANUP
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
    lua_pushlightuserdata(L, data);
    return 1;
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
#define CLEANUP zfreearrayVkMappedMemoryRange(L, ranges, count, 1)
    VkMappedMemoryRange* ranges = zcheckarrayVkMappedMemoryRange(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->FlushMappedMemoryRanges(device, count, ranges);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int InvalidateMappedMemoryRanges(lua_State *L)
    {
    int err;
    uint32_t count;
    VkResult ec;
    ud_t *ud;
    VkDevice device = checkdevice(L, 1, &ud);
#define CLEANUP zfreearrayVkMappedMemoryRange(L, ranges, count, 1)
    VkMappedMemoryRange* ranges = zcheckarrayVkMappedMemoryRange(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->InvalidateMappedMemoryRanges(device, count, ranges);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

/*--------------------------------------------------------------------------------*/

static int GetBufferMemoryRequirements2(lua_State *L, VkBuffer buffer, ud_t *ud)
    {
    int err;
    VkMemoryRequirements2* req=NULL;
    VkBufferMemoryRequirementsInfo2KHR* info=NULL;
    VkDevice device = ud->device;
#define CLEANUP do {                                        \
    zfreeVkBufferMemoryRequirementsInfo2KHR(L, info, 1);    \
    zfreeVkMemoryRequirements2(L, req, 1);                  \
} while(0)
    if(lua_istable(L, 2))
        {
        info = zcheckVkBufferMemoryRequirementsInfo2KHR(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        info = znewVkBufferMemoryRequirementsInfo2KHR(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        }
    info->buffer = buffer;

    req = znewchainVkMemoryRequirements2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }

    ud->ddt->GetBufferMemoryRequirements2(device, info, req);
    zpushVkMemoryRequirements2(L, req);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetBufferMemoryRequirements(lua_State *L)
    {
    ud_t *ud;
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    VkDevice device = ud->device;
    VkMemoryRequirements req;

    if(ud->ddt->GetBufferMemoryRequirements2)
        return GetBufferMemoryRequirements2(L, buffer, ud);

    ud->ddt->GetBufferMemoryRequirements(device, buffer, &req);
    zpushVkMemoryRequirements(L, &req);
    return 1;
    }

static int GetImageMemoryRequirements2(lua_State *L, VkImage image, ud_t *ud)
    {
    int err;
    VkMemoryRequirements2* req = NULL;
    VkImageMemoryRequirementsInfo2* info = NULL;
    VkDevice device = ud->device;
#define CLEANUP do {                                        \
    zfreeVkImageMemoryRequirementsInfo2KHR(L, info, 1);     \
    zfreeVkMemoryRequirements2(L, req, 1);                  \
} while(0)
    if(lua_istable(L, 2))
        {
        info = zcheckVkImageMemoryRequirementsInfo2KHR(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        info = znewVkImageMemoryRequirementsInfo2KHR(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        }
    info->image = image;
    req = znewchainVkMemoryRequirements2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetImageMemoryRequirements2(device, info, req);
    zpushVkMemoryRequirements2(L, req);
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetImageMemoryRequirements(lua_State *L)
    {
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;
    VkMemoryRequirements req;
    if(ud->ddt->GetImageMemoryRequirements2)
        return GetImageMemoryRequirements2(L, image, ud);
    ud->ddt->GetImageMemoryRequirements(device, image, &req);
    zpushVkMemoryRequirements(L, &req);
    return 1;
    }


static int GetImageSparseMemoryRequirements2(lua_State *L, VkImage image, ud_t *ud)
    {
    int err;
    uint32_t count, i;
    VkSparseImageMemoryRequirements2 *req=NULL;
    VkImageSparseMemoryRequirementsInfo2KHR* info=NULL;
    VkDevice device = ud->device;
#define CLEANUP do {                                            \
    zfreeVkImageSparseMemoryRequirementsInfo2KHR(L, info, 1);   \
    zfreearrayVkSparseImageMemoryRequirements2(L, req, count, 1);   \
} while(0)
    if(lua_istable(L, 2))
        {
        info = zcheckVkImageSparseMemoryRequirementsInfo2KHR(L, 2, &err);
        if(err) { CLEANUP; return argerror(L, 2); }
        }
    else
        {
        info = znewVkImageSparseMemoryRequirementsInfo2KHR(L, &err);
        if(err) { CLEANUP; return lua_error(L); }
        }
    info->image = image;
    lua_newtable(L);
    ud->ddt->GetImageSparseMemoryRequirements2(device, info, &count, NULL);
    if(count == 0) { CLEANUP; return 1; }
    req = znewchainarrayVkSparseImageMemoryRequirements2(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetImageSparseMemoryRequirements2(device, info, &count, req);
    for(i = 0; i <count; i++)
        {
        zpushVkSparseImageMemoryRequirements2(L, &req[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }

static int GetImageSparseMemoryRequirements(lua_State *L)
    {
    uint32_t count, i;
    VkSparseImageMemoryRequirements *req;
    ud_t *ud;
    VkImage image = checkimage(L, 1, &ud);
    VkDevice device = ud->device;
    if(ud->ddt->GetImageSparseMemoryRequirements2)
        return GetImageSparseMemoryRequirements2(L, image, ud);
    lua_newtable(L);
    ud->ddt->GetImageSparseMemoryRequirements(device, image, &count, NULL);
    if(count == 0)
        return 1;
    req = (VkSparseImageMemoryRequirements*)Malloc(L, sizeof(VkSparseImageMemoryRequirements)*count);
    ud->ddt->GetImageSparseMemoryRequirements(device, image, &count, req);
    for(i = 0; i <count; i++)
        {
        zpushVkSparseImageMemoryRequirements(L, &req[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, req);
    return 1;
    }

/*--------------------------------------------------------------------------------*/

static int GetDeviceBufferMemoryRequirements(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkDeviceBufferMemoryRequirementsKHR *info = NULL;
    VkMemoryRequirements2 *req = NULL;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, GetDeviceBufferMemoryRequirementsKHR);
#define CLEANUP do {                                            \
    zfreeVkDeviceBufferMemoryRequirementsKHR(L, info, 1);       \
    zfreeVkMemoryRequirements2(L, req, 1);                      \
} while(0)
    info = zcheckVkDeviceBufferMemoryRequirementsKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    req = znewchainVkMemoryRequirements2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetDeviceBufferMemoryRequirementsKHR(device, info, req);
    zpushVkMemoryRequirements2(L, req);
#undef CLEANUP
    return 1;
    }

static int GetDeviceImageMemoryRequirements(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkDeviceImageMemoryRequirementsKHR *info = NULL;
    VkMemoryRequirements2 *req = NULL;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, GetDeviceImageMemoryRequirementsKHR);
#define CLEANUP do {                                            \
    zfreeVkDeviceImageMemoryRequirementsKHR(L, info, 1);        \
    zfreeVkMemoryRequirements2(L, req, 1);                      \
} while(0)
    info = zcheckVkDeviceImageMemoryRequirementsKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    req = znewchainVkMemoryRequirements2(L, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetDeviceImageMemoryRequirementsKHR(device, info, req);
    zpushVkMemoryRequirements2(L, req);
#undef CLEANUP
    return 1;
    }

static int GetDeviceImageSparseMemoryRequirements(lua_State *L)
    {
    int err;
    uint32_t count, i;
    ud_t *ud;
    VkDeviceImageMemoryRequirementsKHR *info = NULL;
    VkSparseImageMemoryRequirements2 *req = NULL;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, GetDeviceImageSparseMemoryRequirementsKHR);
#define CLEANUP do {                                                \
    zfreeVkDeviceImageMemoryRequirementsKHR(L, info, 1);            \
    zfreearrayVkSparseImageMemoryRequirements2(L, req, count, 1);   \
} while(0)
    info = zcheckVkDeviceImageMemoryRequirementsKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    lua_newtable(L);
    ud->ddt->GetDeviceImageSparseMemoryRequirementsKHR(device, info, &count, NULL);
    if(count == 0) { CLEANUP; return 1; }
    req = znewchainarrayVkSparseImageMemoryRequirements2(L, count, &err);
    if(err) { CLEANUP; return lua_error(L); }
    ud->ddt->GetDeviceImageSparseMemoryRequirementsKHR(device, info, &count, req);
    for(i = 0; i <count; i++)
        {
        zpushVkSparseImageMemoryRequirements2(L, &req[i]);
        lua_rawseti(L, -2, i+1);
        }
    CLEANUP;
#undef CLEANUP
    return 1;
    }

/*--------------------------------------------------------------------------------*/

static int BindBufferMemory1(lua_State *L)
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

static int BindImageMemory1(lua_State *L)
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

static int BindBufferMemory2(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    uint32_t count;
    VkBindBufferMemoryInfo* bind_infos;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, BindBufferMemory2);
#define CLEANUP zfreearrayVkBindBufferMemoryInfo(L, bind_infos, count, 1)
    bind_infos = zcheckarrayVkBindBufferMemoryInfo(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->BindBufferMemory2(device, count, bind_infos);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int BindImageMemory2(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    uint32_t count;
    VkBindImageMemoryInfo* bind_infos;
    VkDevice device = checkdevice(L, 1, &ud);
    CheckDevicePfn(L, ud, BindImageMemory2);
#define CLEANUP zfreearrayVkBindImageMemoryInfo(L, bind_infos, count, 1)
    bind_infos = zcheckarrayVkBindImageMemoryInfo(L, 2, &count, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    ec = ud->ddt->BindImageMemory2(device, count, bind_infos);
    CLEANUP;
    CheckError(L, ec);
#undef CLEANUP
    return 0;
    }

static int BindBufferMemory(lua_State *L)
    {
    if(testbuffer(L, 1, NULL))
        return BindBufferMemory1(L);
    return BindBufferMemory2(L);
    }

static int BindImageMemory(lua_State *L)
    {
    if(testimage(L, 1, NULL))
        return BindImageMemory1(L);
    return BindImageMemory2(L);
    }

/*--------------------------------------------------------------------------------*/

static int GetMemoryFd(lua_State *L)
    {
    int err;
    VkResult ec;
    ud_t *ud;
    int fd;
    VkMemoryGetFdInfoKHR* info;
    VkDeviceMemory memory = checkdevice_memory(L, 1, &ud);
    CheckDevicePfn(L, ud, GetMemoryFdKHR);
#define CLEANUP zfreeVkMemoryGetFdInfoKHR(L, info, 1)
    info = zcheckVkMemoryGetFdInfoKHR(L, 2, &err);
    if(err) { CLEANUP; return argerror(L, 2); }
    info->memory = memory;
    ec = ud->ddt->GetMemoryFdKHR(ud->device, info, &fd);
    CLEANUP;
#undef CLEANUP
    CheckError(L, ec);
    lua_pushinteger(L, fd);
    return 1;
    }

static int GetMemoryFdProperties(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkMemoryFdPropertiesKHR properties;
    VkDevice device = checkdevice(L, 1, &ud);
    VkExternalMemoryHandleTypeFlagBits handleType = checkflags(L, 2);
    int fd = luaL_checkinteger(L, 3);

    CheckDevicePfn(L, ud, GetMemoryFdPropertiesKHR);
    ec = ud->ddt->GetMemoryFdPropertiesKHR(device, handleType, fd, &properties);
    CheckError(L, ec);
    zpushVkMemoryFdPropertiesKHR(L, &properties);
    return 1;
    }

static int GetMemoryHostPointerProperties(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkMemoryHostPointerPropertiesEXT properties;
    VkDevice device = checkdevice(L, 1, &ud);
    VkExternalMemoryHandleTypeFlagBits handleType = checkflags(L, 2);
    void* ptr = checklightuserdata(L, 3);
    CheckDevicePfn(L, ud, GetMemoryHostPointerPropertiesEXT);
    ec = ud->ddt->GetMemoryHostPointerPropertiesEXT(device, handleType, ptr, &properties);
    CheckError(L, ec);
    zpushVkMemoryHostPointerPropertiesEXT(L, &properties);
    return 1;
    }


static int GetDeviceGroupPeerMemoryFeatures(lua_State *L)
    {
    ud_t *ud;
    VkPeerMemoryFeatureFlags flags;
    VkDevice device = checkdevice(L, 1, &ud);
    uint32_t heapIndex = luaL_checkinteger(L, 2);
    uint32_t localDeviceIndex = luaL_checkinteger(L, 3);
    uint32_t remoteDeviceIndex = luaL_checkinteger(L, 4);
    CheckDevicePfn(L, ud, GetDeviceGroupPeerMemoryFeatures);
    ud->ddt->GetDeviceGroupPeerMemoryFeatures(device,
            heapIndex, localDeviceIndex, remoteDeviceIndex, &flags);
    pushflags(L, flags);
    return 1;
    }

static int GetBufferDeviceAddress(lua_State *L)
    {
    VkBufferDeviceAddressInfo info;
    VkDeviceAddress address;
    ud_t *ud;
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    CheckDevicePfn(L, ud, GetBufferDeviceAddress);
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.pNext = NULL;
    info.buffer = buffer;
    address = ud->ddt->GetBufferDeviceAddress(ud->device, &info);
    lua_pushinteger(L, address);
    return 1;
    }

static int GetBufferOpaqueCaptureAddress(lua_State *L)
    {
    VkBufferDeviceAddressInfo info;
    uint64_t address;
    ud_t *ud;
    VkBuffer buffer = checkbuffer(L, 1, &ud);
    CheckDevicePfn(L, ud, GetBufferOpaqueCaptureAddress);
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.pNext = NULL;
    info.buffer = buffer;
    address = ud->ddt->GetBufferOpaqueCaptureAddress(ud->device, &info);
    lua_pushinteger(L, address);
    return 1;
    }

static int GetDeviceMemoryOpaqueCaptureAddress(lua_State *L)
    {
    VkDeviceMemoryOpaqueCaptureAddressInfo info;
    uint64_t address;
    ud_t *ud;
    VkDeviceMemory device_memory = checkdevice_memory(L, 1, &ud);
    CheckDevicePfn(L, ud, GetDeviceMemoryOpaqueCaptureAddress);
    info.sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO;
    info.pNext = NULL;
    info.memory = device_memory;
    address = ud->ddt->GetDeviceMemoryOpaqueCaptureAddress(ud->device, &info);
    lua_pushinteger(L, address);
    return 1;
    }

static int SetDeviceMemoryPriority(lua_State *L)
    {
    ud_t *ud;
    VkDeviceMemory device_memory = checkdevice_memory(L, 1, &ud);
    float priority = luaL_checknumber(L, 2);
    CheckDevicePfn(L, ud, SetDeviceMemoryPriorityEXT);
    ud->ddt->SetDeviceMemoryPriorityEXT(ud->device, device_memory, priority);
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
        { "get_device_buffer_memory_requirements", GetDeviceBufferMemoryRequirements },
        { "get_device_image_memory_requirements", GetDeviceImageMemoryRequirements },
        { "get_device_image_sparse_memory_requirements", GetDeviceImageSparseMemoryRequirements },
        { "bind_image_memory", BindImageMemory },
        { "bind_buffer_memory", BindBufferMemory },
        { "get_memory_fd", GetMemoryFd },
        { "get_memory_fd_properties", GetMemoryFdProperties },
        { "get_memory_host_pointer_properties", GetMemoryHostPointerProperties },
        { "get_device_group_peer_memory_features", GetDeviceGroupPeerMemoryFeatures },
        { "get_buffer_device_address", GetBufferDeviceAddress },
        { "get_buffer_opaque_capture_address", GetBufferOpaqueCaptureAddress },
        { "get_device_memory_opaque_capture_address", GetDeviceMemoryOpaqueCaptureAddress },
        { "set_device_memory_priority", SetDeviceMemoryPriority },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_device_memory(lua_State *L)
    {
    udata_define(L, DEVICE_MEMORY_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

