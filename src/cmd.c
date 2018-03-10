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
    
static int CmdBindPipeline(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineBindPoint pipelineBindPoint = checkpipelinebindpoint(L, 2);
    VkPipeline pipeline = checkpipeline(L, 3, NULL);
    ud->ddt->CmdBindPipeline(cb, pipelineBindPoint, pipeline);
    return 0;
    }

static int CmdSetViewport(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t firstViewport = luaL_checkinteger(L, 2);
    VkViewport *viewports = echeckviewportlist(L, 3, &count, &err);
    if(err) return argerror(L, 3);
        
    ud->ddt->CmdSetViewport(cb, firstViewport, count, viewports);
    Free(L, viewports);
    return 0;
    }

static int CmdSetScissor(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t first = luaL_checkinteger(L, 2);
    VkRect2D *scissors = echeckrect2dlist(L, 3, &count, &err);
    if(err) return argerror(L, 3);
        
    ud->ddt->CmdSetScissor(cb, first, count, scissors);
    Free(L, scissors);
    return 0;
    }

static int CmdSetLineWidth(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    float line_width = luaL_optnumber(L, 2, 1.0);
    ud->ddt->CmdSetLineWidth(cb, line_width);
    return 0;
    }

static int CmdSetDepthBias(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    float depthBiasConstantFactor = luaL_checknumber(L, 2);
    float depthBiasClamp = luaL_checknumber(L, 3); 
    float depthBiasSlopeFactor = luaL_checknumber(L, 4);
    ud->ddt->CmdSetDepthBias(cb, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    return 0;
    }

static int CmdSetBlendConstants(lua_State *L)
    {
    float blendConstants[4];
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    blendConstants[0] = luaL_checknumber(L, 2);
    blendConstants[1] = luaL_checknumber(L, 3);
    blendConstants[2] = luaL_checknumber(L, 4);
    blendConstants[3] = luaL_checknumber(L, 5);
    ud->ddt->CmdSetBlendConstants(cb, blendConstants);
    return 0;
    }

static int CmdSetDepthBounds(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    float minDepthBounds = luaL_checknumber(L, 2);
    float maxDepthBounds = luaL_checknumber(L, 3);
    ud->ddt->CmdSetDepthBounds(cb, minDepthBounds, maxDepthBounds);
    return 0;
    }

static int CmdSetStencilCompareMask(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkStencilFaceFlags faceMask = checkflags(L, 2);
    uint32_t compareMask = luaL_checkinteger(L, 3);
    ud->ddt->CmdSetStencilCompareMask(cb, faceMask, compareMask);
    return 0;
    }

static int CmdSetStencilWriteMask(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkStencilFaceFlags faceMask = checkflags(L, 2);
    uint32_t writeMask = luaL_checkinteger(L, 3);
    ud->ddt->CmdSetStencilWriteMask(cb, faceMask, writeMask);
    return 0;
    }

static int CmdSetStencilReference(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkStencilFaceFlags faceMask = checkflags(L, 2);
    uint32_t reference = luaL_checkinteger(L, 3);
    ud->ddt->CmdSetStencilReference(cb, faceMask, reference);
    return 0;
    }

static int CmdBindDescriptorSets(lua_State *L)
    {
    int err; 
    uint32_t sets_count, offsets_count = 0;
    VkDescriptorSet* sets; 
    uint32_t* offsets;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineBindPoint bindpoint = checkpipelinebindpoint(L, 2);
    VkPipelineLayout layout = checkpipeline_layout(L, 3, NULL);
    uint32_t firstSet = luaL_checkinteger(L, 4);

    sets = checkdescriptor_setlist(L, 5, &sets_count, &err, NULL);
    if(err) return argerrorc(L, 5, err);
    
    offsets = checkuint32list(L, 6, &offsets_count, &err);
    if(err < 0) { Free(L, sets); return argerrorc(L, 6, err); }

    ud->ddt->CmdBindDescriptorSets(cb, bindpoint, layout, firstSet, sets_count, sets, offsets_count, offsets);
    Free(L, sets); 
    if(offsets) Free(L, offsets);
    return 0;
    }

static int CmdBindIndexBuffer(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer buffer = checkbuffer(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    VkIndexType indexType = checkindextype(L, 4);
    ud->ddt->CmdBindIndexBuffer(cb, buffer, offset, indexType);
    return 0;
    }

static int CmdBindVertexBuffers(lua_State *L)
    {
    int err;
    uint32_t count, offsets_count;
    VkBuffer* buffers;
    VkDeviceSize* offsets;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t first = luaL_checkinteger(L, 2);

    buffers = checkbufferlist(L, 3, &count, &err);
    if(err) return argerrorc(L, 3, err);
    
    offsets = checkdevicesizelist(L, 4, &offsets_count, &err);
    if(err)
        { 
        Free(L, buffers); 
        return argerrorc(L, 4, err);
        }
    if(offsets_count != count)
        { 
        Free(L, buffers); 
        Free(L, offsets); 
        return argerrorc(L, 4, ERR_LENGTH);
        }

    ud->ddt->CmdBindVertexBuffers(cb, first, count, buffers, offsets);
    Free(L, buffers);
    Free(L, offsets);
    return 0;
    }

static int CmdDraw(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t vertexCount = luaL_checkinteger(L, 2);
    uint32_t instanceCount = luaL_checkinteger(L, 3);
    uint32_t firstVertex = luaL_checkinteger(L, 4);
    uint32_t firstInstance = luaL_checkinteger(L, 5);
    ud->ddt->CmdDraw(cb, vertexCount, instanceCount, firstVertex, firstInstance);
    return 0;
    }

static int CmdDrawIndexed(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t indexCount = luaL_checkinteger(L, 2);
    uint32_t instanceCount = luaL_checkinteger(L, 3);
    uint32_t firstIndex = luaL_checkinteger(L, 4);
    int32_t vertexOffset = luaL_checkinteger(L, 5);
    uint32_t firstInstance = luaL_checkinteger(L, 6);
    ud->ddt->CmdDrawIndexed(cb, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return 0;
    }

static int CmdDrawIndirect(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer buffer = checkbuffer(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    uint32_t drawCount = luaL_checkinteger(L, 4);
    uint32_t stride = luaL_checkinteger(L, 5);
    ud->ddt->CmdDrawIndirect(cb, buffer, offset, drawCount, stride);
    return 0;
    }

static int CmdDrawIndexedIndirect(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer buffer = checkbuffer(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    uint32_t drawCount = luaL_checkinteger(L, 4);
    uint32_t stride = luaL_checkinteger(L, 5);
    ud->ddt->CmdDrawIndexedIndirect(cb, buffer, offset, drawCount, stride);
    return 0;
    }

static int CmdDispatch(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t x = luaL_checkinteger(L, 2);
    uint32_t y = luaL_checkinteger(L, 3);
    uint32_t z = luaL_checkinteger(L, 4);
    ud->ddt->CmdDispatch(cb, x, y, z);
    return 0;
    }

static int CmdDispatchIndirect(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer buffer = checkbuffer(L, 2, NULL);
    VkDeviceSize offset = luaL_checkinteger(L, 3);
    ud->ddt->CmdDispatchIndirect(cb, buffer, offset);
    return 0;
    }

static int CmdCopyBuffer(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer srcBuffer = checkbuffer(L, 2, NULL);
    VkBuffer dstBuffer = checkbuffer(L, 3, NULL);
    VkBufferCopy* regions = echeckbuffercopylist(L, 4, &count, &err);
    if(err) return argerror(L, 4);

    ud->ddt->CmdCopyBuffer(cb, srcBuffer, dstBuffer, count, regions);
    Free(L, regions);
    return 0;
    }

static int CmdCopyImage(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage srcImage = checkimage(L, 2, NULL);
    VkImageLayout srcImageLayout = checkimagelayout(L, 3);
    VkImage dstImage = checkimage(L, 4, NULL);
    VkImageLayout dstImageLayout = checkimagelayout(L, 5);
    VkImageCopy* regions = echeckimagecopylist(L, 6, &count, &err);
    if(err) return argerror(L, 6);

    ud->ddt->CmdCopyImage(cb, srcImage, srcImageLayout, dstImage, dstImageLayout, count, regions);
    Free(L, regions);
    return 0;
    }

static int CmdBlitImage(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage srcImage = checkimage(L, 2, NULL);
    VkImageLayout srcImageLayout = checkimagelayout(L, 3);
    VkImage dstImage = checkimage(L, 4, NULL);
    VkImageLayout dstImageLayout = checkimagelayout(L, 5);
    VkFilter filter = checkfilter(L, 7); /* before regions so there is no need to free them on error */
    VkImageBlit* regions = echeckimageblitlist(L, 6, &count, &err);
    if(err) return argerror(L, 6);

    ud->ddt->CmdBlitImage(cb, srcImage, srcImageLayout, dstImage, dstImageLayout, count, regions, filter);
    Free(L, regions);
    return 0;
    }

static int CmdCopyBufferToImage(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer srcBuffer = checkbuffer(L, 2, NULL);
    VkImage dstImage = checkimage(L, 3, NULL);
    VkImageLayout dstImageLayout = checkimagelayout(L, 4);
    VkBufferImageCopy* regions = echeckbufferimagecopylist(L, 5, &count, &err);
    if(err) return argerror(L, 5);
    ud->ddt->CmdCopyBufferToImage(cb, srcBuffer, dstImage, dstImageLayout, count, regions);
    Free(L, regions);
    return 0;
    }

static int CmdCopyImageToBuffer(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage srcImage = checkimage(L, 2, NULL);
    VkImageLayout srcImageLayout = checkimagelayout(L, 3);
    VkBuffer dstBuffer = checkbuffer(L, 4, NULL);
    VkBufferImageCopy* regions = echeckbufferimagecopylist(L, 5, &count, &err);
    if(err) return argerror(L, 5);

    ud->ddt->CmdCopyImageToBuffer(cb, srcImage, srcImageLayout, dstBuffer, count, regions);
    Free(L, regions);
    return 0;
    }


static int CmdUpdateBuffer(lua_State *L)
    {
    size_t size;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer dstBuffer = checkbuffer(L, 2, NULL);
    VkDeviceSize dstOffset = luaL_checkinteger(L, 3);
    const char *data = luaL_checklstring(L, 4, &size);
    if((size==0) || (size % 4)!=0)
        return argerrorc(L, 4, ERR_LENGTH);
    ud->ddt->CmdUpdateBuffer(cb, dstBuffer, dstOffset, (VkDeviceSize)size, data);
    return 0;
    }

static int CmdFillBuffer(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkBuffer dstBuffer = checkbuffer(L, 2, NULL);
    VkDeviceSize dstOffset = luaL_checkinteger(L, 3);
    VkDeviceSize size = checksizeorwholesize(L, 4);
    uint32_t data = luaL_checkinteger(L, 5);
    ud->ddt->CmdFillBuffer(cb, dstBuffer, dstOffset, size, data);
    return 0;
    }

static int CmdClearColorImage(lua_State *L)
    {
    int err;
    uint32_t count;
    VkImageSubresourceRange* ranges;
    VkClearColorValue color;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage image = checkimage(L, 2, NULL);
    VkImageLayout imageLayout = checkimagelayout(L, 3);
    if(echeckclearcolorvalue(L, 4, &color)) return argerror(L, 4);
    ranges = echeckimagesubresourcerangelist(L, 5, &count, &err);
    if(err) return argerror(L, 5);
    ud->ddt->CmdClearColorImage(cb, image, imageLayout, &color, count, ranges);
    Free(L, ranges);
    return 0;
    }

static int CmdClearDepthStencilImage(lua_State *L)
    {
    int err;
    uint32_t count;
    VkClearDepthStencilValue depthstencil;
    VkImageSubresourceRange* ranges;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage image = checkimage(L, 2, NULL);
    VkImageLayout imageLayout = checkimagelayout(L, 3);
    depthstencil.depth = luaL_checknumber(L, 4);
    depthstencil.stencil = luaL_checkinteger(L, 5);
    ranges = echeckimagesubresourcerangelist(L, 6, &count, &err);
    if(err) return argerror(L, 6);
    ud->ddt->CmdClearDepthStencilImage(cb, image, imageLayout, &depthstencil, count, ranges);
    Free(L, ranges);
    return 0;
    }

static int CmdClearAttachments(lua_State *L)
    {
    int err;
    uint32_t att_count, rect_count;
    VkClearAttachment* attachments;
    VkClearRect* rects;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);

    attachments = echeckclearattachmentlist(L, 2, &att_count, &err);
    if(err) return argerror(L, 2);

    rects = echeckclearrectlist(L, 3, &rect_count, &err);
    if(err) 
        {
        Free(L, attachments);
        return argerror(L, 3);
        }

    ud->ddt->CmdClearAttachments(cb, att_count, attachments, rect_count, rects);
    Free(L, attachments);
    Free(L, rects);
    return 0;
    }


static int CmdResolveImage(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkImage srcImage = checkimage(L, 2, NULL);
    VkImageLayout srcImageLayout = checkimagelayout(L, 3);
    VkImage dstImage = checkimage(L, 4, NULL);
    VkImageLayout dstImageLayout = checkimagelayout(L, 5);
    VkImageResolve* regions = echeckimageresolvelist(L, 6, &count, &err);
    if(err) return argerror(L, 6);
    ud->ddt->CmdResolveImage(cb, srcImage, srcImageLayout, dstImage, dstImageLayout, count, regions);
    Free(L, regions);
    return 0;
    }


static int CmdSetEvent(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkEvent event = checkevent(L, 2, NULL);
    VkPipelineStageFlags stageMask = checkflags(L, 3);
    ud->ddt->CmdSetEvent(cb, event, stageMask);
    return 0;
    }

static int CmdResetEvent(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkEvent event = checkevent(L, 2, NULL);
    VkPipelineStageFlags stageMask = checkflags(L, 3);
    ud->ddt->CmdResetEvent(cb, event, stageMask);
    return 0;
    }

static int CmdWaitEvents(lua_State *L)
    {
    int err, errarg = 0;
    uint32_t eventCount, memoryBarrierCount, bufferMemoryBarrierCount, imageMemoryBarrierCount;
    VkEvent* pEvents = NULL;
    VkMemoryBarrier* pMemoryBarriers = NULL;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = NULL;
    VkImageMemoryBarrier* pImageMemoryBarriers = NULL;

    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineStageFlags srcStageMask = checkflags(L, 2);
    VkPipelineStageFlags dstStageMask = checkflags(L, 3);

    pEvents = checkeventlist(L, 4, &eventCount, &err, NULL);
    if(err) { errarg = 4; lua_pushstring(L, errstring(err)); goto done; }

    pMemoryBarriers = echeckmemorybarrierlist(L, 5, &memoryBarrierCount, &err);
    if(err < 0) { errarg = 5; goto done; }
    if(err) lua_pop(L, 1);

    pBufferMemoryBarriers = echeckbuffermemorybarrierlist(L, 6, &bufferMemoryBarrierCount, &err);
    if(err < 0) { errarg = 6; goto done; }
    if(err) lua_pop(L, 1);

    pImageMemoryBarriers = echeckimagememorybarrierlist(L, 7, &imageMemoryBarrierCount, &err);
    if(err < 0) { errarg = 7; goto done; }
    if(err) lua_pop(L, 1);

    ud->ddt->CmdWaitEvents(cb, eventCount, pEvents, srcStageMask, dstStageMask, 
            memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, 
            imageMemoryBarrierCount, pImageMemoryBarriers);

done: /* I'm italian, I love spaghetti */
    if(pEvents) Free(L, pEvents);
    if(pMemoryBarriers) Free(L, pMemoryBarriers);
    if(pBufferMemoryBarriers) Free(L, pBufferMemoryBarriers);
    if(pImageMemoryBarriers) Free(L, pImageMemoryBarriers);
    if(errarg)
        return argerror(L, errarg);
    return 0;
    }

static int CmdPipelineBarrier(lua_State *L)
    {
    int err, errarg = 0;
    uint32_t memoryBarrierCount, bufferMemoryBarrierCount, imageMemoryBarrierCount;
    VkMemoryBarrier* pMemoryBarriers = NULL;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = NULL;
    VkImageMemoryBarrier* pImageMemoryBarriers = NULL;

    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineStageFlags srcStageMask = checkflags(L, 2);
    VkPipelineStageFlags dstStageMask = checkflags(L, 3);
    VkDependencyFlags dependencyFlags = checkflags(L, 4);

    pMemoryBarriers = echeckmemorybarrierlist(L, 5, &memoryBarrierCount, &err);
    if(err < 0) { errarg = 5; goto done; }
    if(err) lua_pop(L, 1);

    pBufferMemoryBarriers = echeckbuffermemorybarrierlist(L, 6, &bufferMemoryBarrierCount, &err);
    if(err < 0) { errarg = 6; goto done; }
    if(err) lua_pop(L, 1);

    pImageMemoryBarriers = echeckimagememorybarrierlist(L, 7, &imageMemoryBarrierCount, &err);
    if(err < 0) { errarg = 7; goto done; }
    if(err) lua_pop(L, 1);


    ud->ddt->CmdPipelineBarrier(cb, srcStageMask, dstStageMask, dependencyFlags, 
        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, 
        imageMemoryBarrierCount, pImageMemoryBarriers);

done: /* I'm italian, I love spaghetti */
    if(pMemoryBarriers) Free(L, pMemoryBarriers);
    if(pBufferMemoryBarriers) Free(L, pBufferMemoryBarriers);
    if(pImageMemoryBarriers) Free(L, pImageMemoryBarriers);
    if(errarg)
        return argerror(L, errarg);
    return 0;
    }

static int CmdBeginQuery(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkQueryPool queryPool = checkquery_pool(L, 2, NULL);
    uint32_t query = luaL_checkinteger(L, 3);
    VkQueryControlFlags flags = checkflags(L, 4);
    ud->ddt->CmdBeginQuery(cb, queryPool, query, flags);
    return 0;
    }

static int CmdEndQuery(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkQueryPool queryPool = checkquery_pool(L, 2, NULL);
    uint32_t query = luaL_checkinteger(L, 3);
    ud->ddt->CmdEndQuery(cb, queryPool, query);
    return 0;
    }

static int CmdResetQueryPool(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkQueryPool queryPool = checkquery_pool(L, 2, NULL);
    uint32_t firstQuery = luaL_checkinteger(L, 3);
    uint32_t queryCount = luaL_checkinteger(L, 4);
    ud->ddt->CmdResetQueryPool(cb, queryPool, firstQuery, queryCount);
    return 0;
    }

static int CmdWriteTimestamp(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineStageFlagBits pipelineStage = (VkPipelineStageFlagBits)checkflags(L, 2);
    VkQueryPool queryPool = checkquery_pool(L, 3, NULL);
    uint32_t query = luaL_checkinteger(L, 4);
    ud->ddt->CmdWriteTimestamp(cb, pipelineStage, queryPool, query);
    return 0;
    }

static int CmdCopyQueryPoolResults(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkQueryPool queryPool = checkquery_pool(L, 2, NULL);
    uint32_t firstQuery = luaL_checkinteger(L, 3);
    uint32_t queryCount = luaL_checkinteger(L, 4);
    VkBuffer dstBuffer = checkbuffer(L, 5, NULL);
    VkDeviceSize dstOffset  = luaL_checkinteger(L, 6);
    VkDeviceSize stride = luaL_checkinteger(L, 7);
    VkQueryResultFlags flags = checkflags(L, 8);
    ud->ddt->CmdCopyQueryPoolResults(cb, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    return 0;
    }

static int CmdPushConstants(lua_State *L)
    {
    size_t size;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineLayout layout = checkpipeline_layout(L, 2, NULL);
    VkShaderStageFlags stageFlags = checkflags(L, 3);
    uint32_t offset = luaL_checkinteger(L, 4);
    const char* values = luaL_checklstring(L, 5, &size);
    ud->ddt->CmdPushConstants(cb, layout, stageFlags, offset, (uint32_t)size, values);
    return 0;
    }

static int CmdBeginRenderPass(lua_State *L)
    {
    VkSubpassContents contents;
    VkRenderPassBeginInfo_CHAIN info;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    if(echeckrenderpassbegininfo(L, 2, &info) != 0)
        {
        freerenderpassbegininfo(L, &info);
        return argerror(L, 2);
        }
    
    contents = checksubpasscontents(L, 3);
    ud->ddt->CmdBeginRenderPass(cb, &info.p1, contents);
    freerenderpassbegininfo(L, &info);
    return 0;
    }

static int CmdNextSubpass(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkSubpassContents contents = checksubpasscontents(L, 2);
    ud->ddt->CmdNextSubpass(cb, contents);
    return 0;
    }

static int CmdEndRenderPass(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    ud->ddt->CmdEndRenderPass(cb);
    return 0;
    }

static int CmdExecuteCommands(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkCommandBuffer *buffers = checkcommand_bufferlist(L, 2, &count, &err);
    if(err) return argerrorc(L, 2, err);

    ud->ddt->CmdExecuteCommands(cb, count, buffers);
    Free(L, buffers);
    return 0;
    }


/*-------- Extensions ------------------------------------------------------------ */

static int checkdebugmarkermarkerinfo(lua_State *L, int arg, VkDebugMarkerMarkerInfoEXT *info)
    {
    int i, argcolor = arg+1;
    info->sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    info->pNext = NULL;
    info->pMarkerName = luaL_checkstring(L, arg);
    if(lua_isnoneornil(L, argcolor))
        {
        memset(info->color, 0, 4*sizeof(float));
        return 0;
        }
    if(!lua_istable(L, argcolor))
        return argerrorc(L, arg, ERR_TABLE);
    for(i = 0; i < 4; i++)
        {
        if(lua_rawgeti(L, argcolor, i+1) != LUA_TNUMBER)
            {
            lua_pop(L, 1);
            return argerrorc(L, argcolor, ERR_TYPE);
            }
        info->color[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        }
    return 0;
    }

static int CmdDebugMarkerBegin(lua_State *L) 

    {
    ud_t *ud;
    VkDebugMarkerMarkerInfoEXT info;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    checkdebugmarkermarkerinfo(L, 2, &info);
    CheckDevicePfn(L, ud, CmdDebugMarkerBeginEXT);
    ud->ddt->CmdDebugMarkerBeginEXT(cb, &info);
    return 0;
    }

static int CmdDebugMarkerEnd(lua_State *L)
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    CheckDevicePfn(L, ud, CmdDebugMarkerEndEXT);
    ud->ddt->CmdDebugMarkerEndEXT(cb);
    return 0;
    }

static int CmdDebugMarkerInsert(lua_State *L)
    {
    ud_t *ud;
    VkDebugMarkerMarkerInfoEXT info;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    CheckDevicePfn(L, ud, CmdDebugMarkerInsertEXT);
    checkdebugmarkermarkerinfo(L, 2, &info);
    ud->ddt->CmdDebugMarkerInsertEXT(cb, &info);
    return 0;
    }

static int CmdPushDescriptorSet(lua_State *L)
    {
    int err;
    uint32_t count;
    VkWriteDescriptorSet* writes;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkPipelineBindPoint pipelineBindPoint = checkpipelinebindpoint(L, 2);
    VkPipelineLayout layout = checkpipeline_layout(L, 3, NULL);
    uint32_t set = luaL_checkinteger(L, 4);
    CheckDevicePfn(L, ud, CmdPushDescriptorSetKHR);
    writes = echeckwritedescriptorsetlist(L, 5, &count, &err);
    if(err) return argerrorc(L, 5, err);
    ud->ddt->CmdPushDescriptorSetKHR(cb, pipelineBindPoint, layout, set, count, writes);
    freewritedescriptorsetlist(L, writes, count);
    return 0;
    }

static int CmdPushDescriptorSetWithTemplate(lua_State *L)
    {
    size_t len;
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    VkDescriptorUpdateTemplate desc_template = checkdescriptor_update_template(L, 2, NULL);
    VkPipelineLayout layout = checkpipeline_layout(L, 3, NULL);
    uint32_t set = luaL_checkinteger(L, 4);
    const void* data = luaL_checklstring(L, 5, &len);
    CheckDevicePfn(L, ud, CmdPushDescriptorSetWithTemplateKHR);
    ud->ddt->CmdPushDescriptorSetWithTemplateKHR(cb, desc_template, layout, set, data);
    return 0;
    }

static int CmdSetDiscardRectangle(lua_State *L)
    {
    int err;
    uint32_t count;
    ud_t *ud;
    VkRect2D *rects;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    uint32_t first = luaL_checkinteger(L, 2);
    CheckDevicePfn(L, ud, CmdSetDiscardRectangleEXT);
    rects = echeckrect2dlist(L, 3, &count, &err);
    if(err) return argerror(L, 3);
    ud->ddt->CmdSetDiscardRectangleEXT(cb, first, count, rects);
    Free(L, rects);
    return 0;
    }

static int CmdSetSampleLocations(lua_State *L)
    {
    int err;
    ud_t *ud;
    VkSampleLocationsInfoEXT info;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    CheckDevicePfn(L, ud, CmdSetSampleLocationsEXT);
    err = echecksamplelocationsinfo(L, 2, &info);
    if(err) return argerror(L, 2);
    ud->ddt->CmdSetSampleLocationsEXT(cb, &info);
    freesamplelocationsinfo(L, &info);
    return 0;
    }


#if 0 // 10yy
        { "",  },
static int (lua_State *L) //@@ scaffolding
    {
    ud_t *ud;
    VkCommandBuffer cb = checkcommand_buffer(L, 1, &ud);
    CheckDevicePfn(L, ud, );
    ud->ddt->(cb, );
    return 0;
    }

#endif



static const struct luaL_Reg Functions[] = 
    {
        { "cmd_bind_pipeline", CmdBindPipeline },
        { "cmd_set_viewport", CmdSetViewport },
        { "cmd_set_scissor", CmdSetScissor },
        { "cmd_set_line_width", CmdSetLineWidth },
        { "cmd_set_depth_bias", CmdSetDepthBias },
        { "cmd_set_blend_constants", CmdSetBlendConstants },
        { "cmd_set_depth_bounds", CmdSetDepthBounds },
        { "cmd_set_stencil_compare_mask", CmdSetStencilCompareMask },
        { "cmd_set_stencil_write_mask", CmdSetStencilWriteMask },
        { "cmd_set_stencil_reference", CmdSetStencilReference },
        { "cmd_bind_descriptor_sets", CmdBindDescriptorSets },
        { "cmd_bind_index_buffer", CmdBindIndexBuffer },
        { "cmd_bind_vertex_buffers", CmdBindVertexBuffers },
        { "cmd_draw", CmdDraw },
        { "cmd_draw_indexed", CmdDrawIndexed },
        { "cmd_draw_indirect", CmdDrawIndirect },
        { "cmd_draw_indexed_indirect", CmdDrawIndexedIndirect },
        { "cmd_dispatch", CmdDispatch },
        { "cmd_dispatch_indirect", CmdDispatchIndirect },
        { "cmd_copy_buffer", CmdCopyBuffer },
        { "cmd_copy_image", CmdCopyImage },
        { "cmd_blit_image", CmdBlitImage },
        { "cmd_copy_buffer_to_image", CmdCopyBufferToImage },
        { "cmd_copy_image_to_buffer", CmdCopyImageToBuffer },
        { "cmd_update_buffer", CmdUpdateBuffer },
        { "cmd_fill_buffer", CmdFillBuffer },
        { "cmd_clear_color_image", CmdClearColorImage },
        { "cmd_clear_depth_stencil_image", CmdClearDepthStencilImage },
        { "cmd_clear_attachments", CmdClearAttachments },
        { "cmd_resolve_image", CmdResolveImage },
        { "cmd_set_event", CmdSetEvent },
        { "cmd_reset_event", CmdResetEvent },
        { "cmd_wait_events", CmdWaitEvents },
        { "cmd_pipeline_barrier", CmdPipelineBarrier },
        { "cmd_begin_query", CmdBeginQuery },
        { "cmd_end_query", CmdEndQuery },
        { "cmd_reset_query_pool", CmdResetQueryPool },
        { "cmd_write_timestamp", CmdWriteTimestamp },
        { "cmd_copy_query_pool_results", CmdCopyQueryPoolResults },
        { "cmd_push_constants", CmdPushConstants },
        { "cmd_begin_render_pass", CmdBeginRenderPass },
        { "cmd_next_subpass", CmdNextSubpass },
        { "cmd_end_render_pass", CmdEndRenderPass },
        { "cmd_execute_commands", CmdExecuteCommands },
        { "cmd_debug_marker_begin", CmdDebugMarkerBegin },
        { "cmd_debug_marker_end", CmdDebugMarkerEnd },
        { "cmd_debug_marker_insert", CmdDebugMarkerInsert },
        { "cmd_push_descriptor_set", CmdPushDescriptorSet },
        { "cmd_push_descriptor_set_with_template", CmdPushDescriptorSetWithTemplate },
        { "cmd_set_discard_rectangle", CmdSetDiscardRectangle },
        { "cmd_set_sample_locations", CmdSetSampleLocations },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_cmd(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }

