-- The MIT License (MIT)
--
-- Copyright (c) 2017 Stefano Trettel
--
-- Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
-- 

-- *********************************************************************
-- DO NOT require() THIS MODULE (it is loaded automatically by MoonVulkan)
-- *********************************************************************

local vk = moonvulkan -- require("moonvulkan")

function vk.offset2d(x, y) return { x=x, y=y } end
function vk.offset3d(x, y, z) return { x=x, y=y, z=z } end

function vk.extent2d(width, height) return { width=width, height=height } end
function vk.extent3d(width, height, depth) return { width=width, height=height, depth=depth } end

function vk.rect2d(x, y, width, height)
		return { offset = { x=x, y=y }, extent = { width=width, height=height } }
end

function vk.componentmapping(r, g, b, a) return { r=r, g=g, b=b, a=a } end

function vk.viewport(x, y, width, height, min_depth, max_depth)
	return { x=x, y=y, width=width, height=height, min_depth=min_depth, max_depth=max_depth }
end

function vk.imagesubresource(aspect_mask, mip_level, array_layer)
	return { 
			aspect_mask = type(aspect_mask) == 'table' and 
					vk.imageaspectflags(table.unpack(aspect_mask)) or aspect_mask,
			mip_level = mip_level, 
			array_layer = array_layer, 
			}
end

function vk.imagesubresourcelayers(aspect_mask, mip_level, base_array_layer, layer_count)
	return { 
			aspect_mask = type(aspect_mask) == 'table' and 
					vk.imageaspectflags(table.unpack(aspect_mask)) or aspect_mask,
			mip_level = mip_level, 
			base_array_layer = base_array_layer,
			layer_count = layer_count,
			}
end


function vk.imagesubresourcerange(aspect_mask, base_mip_level, level_count, base_array_layer, layer_count)
	return { 
			aspect_mask = type(aspect_mask) == 'table' and 
					vk.imageaspectflags(table.unpack(aspect_mask)) or aspect_mask,
			base_mip_level = base_mip_level, 
			level_count = level_count, 
			base_array_layer = base_array_layer, 
			layer_count = layer_count	
			}
end

function vk.pushconstantrange(stage_flags, offset, size)
	return { 
			stage_flags = type(stage_flags) == 'table' and 
					vk.shaderstageflags(table.unpack(stage_flags)) or stage_flags,
			offset = offset,
			size = size,
			}
end

function vk.descriptorsetlayoutbinding(binding, descriptor_type, descriptor_count, stage_flags, immutable_samplers)
	return { 
			binding = binding, 
			descriptor_type= descriptor_type, 
			descriptor_count = descriptor_count, 
			stage_flags = type(stage_flags) == 'table' and 
					vk.shaderstageflags(table.unpack(stage_flags)) or stage_flags,
			immutable_samplers = immutable_samplers,
			}
end


--@@ TODO? 
-- clearcolorvalue
-- clearvalue
-- stencilopstate
--
