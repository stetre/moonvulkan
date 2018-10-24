#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")
local glmath = require("moonglmath")
local mi = require("moonimage")
local ai = require("moonassimp")

local clamp = glmath.clamp
local vec3, mat4 = glmath.vec3, glmath.mat4
local rotate, translate, scale = glmath.rotate, glmath.translate, glmath.scale
local look_at, perspective = glmath.look_at, glmath.perspective
local transpose = glmath.transpose

local FLOATSZ = vk.sizeof('float') -- or simply 4...

local FPS = 60
local WIDTH, HEIGHT = 800, 600
local LAYERS, EXTENSIONS = {}, {}

local MODEL_PATH = "resources/chalet.obj"
local TEXTURE_PATH = "resources/chalet.jpg"

local DEBUG = arg[1] == "--debug"
if not DEBUG then 
   print("To enable debug, run as: \n$ "..arg[0].." --debug")
end

-- Init window ----------------------------------------------------------------
glfw.window_hint('client api', 'no api')
glfw.window_hint('resizable', true)
local window = glfw.create_window(WIDTH, HEIGHT, "Vulkan")

-- Init Vulkan ----------------------------------------------------------------

-- Add the extensions required by GLFW
for _, ext in ipairs(glfw.get_required_instance_extensions()) do
   table.insert(EXTENSIONS, ext)
end

if DEBUG then -- add layers and extensions required for debug
   table.insert(LAYERS, "VK_LAYER_LUNARG_standard_validation")
   table.insert(EXTENSIONS, "VK_EXT_debug_report")
end

do -- Check that the requested layers are available
   local available_layers = vk.enumerate_instance_layer_properties(true)
   for _, layer in ipairs(LAYERS) do
      assert(available_layers[layer], "layer '"..layer.."' is not available")
   end
end

-- Create instance
local instance = vk.create_instance({
   application_info = {
      application_name = "Hello Triangle",
      application_version = vk.make_version(1, 0, 0),
      engine_name = "No Engine",
      engine_version = vk.make_version(1, 0, 0),
      api_version = vk.API_VERSION_1_0,
   },
   enabled_layer_names = LAYERS,
   enabled_extension_names = EXTENSIONS,
})

local debug_callback 
if DEBUG then -- Set debug callback
   debug_callback = vk.create_debug_report_callback(
      instance,
      vk.DEBUG_REPORT_ERROR_BIT_EXT | vk.DEBUG_REPORT_WARNING_BIT_EXT,
      function(...) print("DEBUG", ...) end
   )
end

-- Create surface
local surface = vk.created_surface(instance, glfw.create_window_surface(window, instance:raw()))

-- Select a physical device
local physdev, graphics_family, present_family
local surface_capabilities, surface_formats, surface_present_modes
for _, pd in ipairs(vk.enumerate_physical_devices(instance)) do
   -- Search for a physical device having both a graphics capable queue family
   -- and a present capable one (they may or may not be the same family).
   graphics_family, present_family = nil, nil
   for i, qf in ipairs(vk.get_physical_device_queue_family_properties(pd)) do
      if qf.queue_count > 0 and (qf.queue_flags & vk.QUEUE_GRAPHICS_BIT) ~= 0 then
         graphics_family = graphics_family or i-1
      end
      if vk.get_physical_device_surface_support(pd, i-1, surface) then
         present_family = present_family or i-1
      end
      if graphics_family and present_family then physdev = pd; break; end
   end
   if physdev then -- retrieve some details and check if they suit us.
      surface_capabilities = vk.get_physical_device_surface_capabilities(pd, surface)
      surface_formats = vk.get_physical_device_surface_formats(pd, surface)
      surface_present_modes = vk.get_physical_device_surface_present_modes(pd, surface)
      local features = vk.get_physical_device_features(physdev)
      if #surface_formats > 0 and #surface_present_modes > 0 and features.sampler_anisotropy then 
         break -- found
      end
      physdev = nil -- not suitable, try with the next one (if any)
   end
end
assert(physdev, "cannot find a suitable physical device")

-- Get max usable sample count
local msaa_samples = 1
do
   local props = vk.get_physical_device_properties(physdev)
   local counts = math.min(props.limits.framebuffer_color_sample_counts,
                           props.limits.framebuffer_depth_sample_counts)
-- print(props.limits.framebuffer_color_sample_counts, props.limits.framebuffer_depth_sample_counts)
   if counts & 64 == 64 then msaa_samples = 64
   elseif counts & 32 == 32 then msaa_samples = 32
   elseif counts & 16 == 16 then msaa_samples = 16
   elseif counts & 8 == 8 then msaa_samples = 8
   elseif counts & 4 == 4 then msaa_samples = 4
   elseif counts & 2 == 2 then msaa_samples = 2
-- else msaa_samples = 1
   end
-- print(msaa_samples)
end

-- Create a logical device
local device = vk.create_device(physdev, {
   -- If the two queue families are different we must create a queue from
   -- each of them, otherwise we must create only one queue.
   queue_create_infos = graphics_family ~= present_family
      and {{queue_family_index=graphics_family, queue_priorities={1.0}},
           {queue_family_index=present_family,  queue_priorities={1.0}}}
      or  {{queue_family_index=graphics_family, queue_priorities={1.0}}},
   enabled_features = { 
      sampler_anisotropy = true,
   },
   enabled_extension_names = { "VK_KHR_swapchain" },
})

-- Get handles to the queues created with the device
local graphics_queue = vk.get_device_queue(device, graphics_family, 0)
local present_queue = vk.get_device_queue(device, present_family, 0)
-- print(graphics_queue == present_queue) -- they may be the same one


-- Compile the shaders and create the shader modules

local function glsl_to_spirv(filename, options)
-- This utility compiles the given GLSL file using glslangValidator, and returns 
-- the SPIR-V code as a binary string.
-- For this to work, glslangValidator must be in the executables PATH.
-- The shader stage is deduced by the filename extension (eg. '.vert' -> vertex stage).
-- options: additional glslangValidator options (eg. "-s" to silence error reporting).
   local options = options and options.." " or " "
   local tmpfile = 'tmp.spv' -- temporary output spv file
   assert(os.execute("glslangValidator -V "..options..filename.." -o "..tmpfile))
   local f = assert(io.open(tmpfile, 'rb'))
   local code = f:read('a')
   f:close()
   os.execute('rm -f '..tmpfile)
   return code
end

local vert_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/depth.vert"))
local frag_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/depth.frag"))

-- Vertex data and element indices

local scene = assert(ai.import_file(MODEL_PATH, "join identical vertices", "triangulate", "flip uvs"))
local mesh = scene:mesh(1)

local positions = mesh:all_positions()
local texcoords = mesh:all_texture_coords()[1]
print(#positions, #texcoords, #positions[1], #texcoords[1])

local vertices = {}
for i=1,#positions do 
   vertices[i] = { positions[i], {1.0, 1.0, 1.0}, texcoords[i]}
end
local vertexdata = vk.pack('float', vertices)
local vertex_binding_descriptions = {
   { binding=0, stride=8*FLOATSZ, input_rate='vertex' },
}
local vertex_attribute_descriptions = {
   { binding=0, location=0, format='r32g32b32 sfloat', offset=0 },         -- position
   { binding=0, location=1, format='r32g32b32 sfloat', offset=2*FLOATSZ }, -- color
   { binding=0, location=2, format='r32g32 sfloat', offset=6*FLOATSZ },    -- texcoord 
}

local indices = vk.flatten_table(mesh:all_indices(true))
local indexdata = vk.pack('uint32', indices)

-- The clip matrix flips the y-axis and scales the z axis to account for how the Vulkan
-- coordinate system is defined wrt OpenGL:
local clip_matrix = mat4(1, 0, 0, 0,
                         0,-1, 0, 0,
                         0, 0,.5,.5,
                         0, 0, 0, 1)
-- Note that it also changes the orientation of the triangles, so we need to set
-- front_face='counter clockwise' in the rasterization_state:
--     0__1             3__2
--     |\ | --flip y--> | /|  triangles = 0-1-2, 2-3-0  (counter clockwise)
--     |_\|             |/_|
--     3  2             0  1

-- Create command pool
local command_pool = vk.create_command_pool(device, 0, graphics_family)

-- Utilities ------------------------------------------------------------------

local function find_memory_type(physdev, reqs, flags)
-- Searches for a suitable memory type. Returns the memory_type_index or raises an error.
-- reqs: memoryrequirements
-- flags: additional memorypropertyflags
   local memory_types = vk.get_physical_device_memory_properties(physdev).memory_types
   local typebits = reqs.memory_type_bits
   for i = 0, #memory_types-1 do
      if (typebits & (1<<i)) ~= 0 and (memory_types[i+1].property_flags & flags) == flags then
         return i
      end
   end
   error("failed to find a suitable memory type")
end

local function find_supported_format(candidates, tiling, features)
-- candidates:{format}, tiling:imagetiling, features:formatfeatureflags
   for _, format in ipairs(candidates) do
      local props = vk.get_physical_device_format_properties(physdev, format)
      if tiling == 'linear' and (props.linear_tiling_features & features) == features then
         return format
      elseif tiling == 'optimal' and (props.optimal_tiling_features & features) == features then
         return format
      end
   end
   error("failed to find supported format")
end

local function find_depth_format()
   return find_supported_format({'d32 sfloat', 'd32 sfloat s8 uint', 'd24 unorm s8 uint'},
             'optimal', vk.FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
end

local function has_stencil_component(format)
   return format == 'd32 sfloat s8 uint' or format == 'd24 unorm s8 uint'
end

local function create_buffer(size, usage, memflags)
-- Creates a buffer, allocates device memory for it, and binds them.
-- usage: bufferusageflags
-- memflags: additional memorypropertyflags
   -- 1) Create buffer
   local buffer = vk.create_buffer(device, {
      size = size,
      usage = usage,
      sharing_mode = 'exclusive',
   })

   -- 2) Allocate memory and bind it to the buffer
   local reqs = vk.get_buffer_memory_requirements(buffer)
   local mti = find_memory_type(physdev, reqs, memflags)
   local devmem = vk.allocate_memory(device, reqs.size, mti)
   vk.bind_buffer_memory(buffer, devmem, 0)

   return buffer, devmem
end

local function begin_single_time_command()
   local cb = vk.allocate_command_buffers(command_pool, 'primary', 1)[1]
   vk.begin_command_buffer(cb, vk.COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
   return cb
end

local function end_single_time_command(cb)
   vk.end_command_buffer(cb)
   vk.queue_submit(graphics_queue, {{command_buffers = {cb}}})
   vk.queue_wait_idle(graphics_queue)
   vk.free_command_buffers({cb})
end

local function copy_buffer(src_buffer, dst_buffer, size)
   local cb = begin_single_time_command()
   vk.cmd_copy_buffer(cb, src_buffer, dst_buffer, {{size=size}})
   end_single_time_command(cb)
end

local function create_image(w, h, mip_levels, num_samples, format, tiling, usage, memflags)
-- format:format, tiling:imagetiling, usage:imageusageflags , memflags:memorypropertyflags
   local image = vk.create_image(device, {
      image_type = '2d',
      extent = {width=w, height=h, depth=1},
      mip_levels = mip_levels,
      array_layers = 1,
      format = format,
      tiling = tiling,
      samples = num_samples,
      sharing_mode = 'exclusive',
      usage = usage,
   })

   local reqs = vk.get_image_memory_requirements(image)
   local mti = find_memory_type(physdev, reqs, memflags)
   local devmem = vk.allocate_memory(device, reqs.size, mti)
   vk.bind_image_memory(image, devmem, 0)

   return image, devmem
end
 
local function transition_image_layout(image, format, mip_levels, old_layout, new_layout)
   local aspect_mask = vk.IMAGE_ASPECT_COLOR_BIT
   if new_layout == 'depth stencil attachment optimal' then
      aspect_mask = vk.IMAGE_ASPECT_DEPTH_BIT | (has_stencil_component(format) and vk.IMAGE_STENCIL_DEPTH_BIT or 0)
   end

   local barrier = {
      src_access_mask = 0,
      dst_access_mask = 0,
      old_layout = old_layout,
      new_layout = new_layout,
      image = image,
      subresource_range = vk.imagesubresourcerange(aspect_mask, 0, mip_levels, 0, 1),
   }

   local src_stage, dst_stage = 0, 0

   if old_layout == 'undefined' and new_layout == 'transfer dst optimal' then
      barrier.src_access_mask = 0
      barrier.dst_access_mask = vk.ACCESS_TRANSFER_WRITE_BIT
      src_stage = vk.PIPELINE_STAGE_TOP_OF_PIPE_BIT
      dst_stage = vk.PIPELINE_STAGE_TRANSFER_BIT
   elseif old_layout == 'transfer dst optimal' and new_layout == 'shader read only optimal' then
      barrier.src_access_mask = vk.ACCESS_TRANSFER_WRITE_BIT
      barrier.dst_access_mask = vk.ACCESS_SHADER_READ_BIT
      src_stage = vk.PIPELINE_STAGE_TRANSFER_BIT
      dst_stage = vk.PIPELINE_STAGE_FRAGMENT_SHADER_BIT
   elseif old_layout == 'undefined' and new_layout == 'depth stencil attachment optimal' then
      barrier.src_access_mask = 0
      barrier.dst_access_mask = vk.ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | vk.ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
      src_stage = vk.PIPELINE_STAGE_TOP_OF_PIPE_BIT
      dst_stage = vk.PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
   elseif old_layout == 'undefined' and new_layout == 'color attachment optimal' then
      barrier.src_access_mask = 0
      barrier.dst_access_mask = vk.ACCESS_COLOR_ATTACHMENT_READ_BIT | vk.ACCESS_COLOR_ATTACHMENT_WRITE_BIT
      src_stage = vk.PIPELINE_STAGE_TOP_OF_PIPE_BIT
      dst_stage = vk.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
   else
      error("unsupported layout transition!")
   end

   local cb = begin_single_time_command()
   vk.cmd_pipeline_barrier(cb, src_stage, dst_stage, 0, nil, nil, {barrier})
   end_single_time_command(cb)
end

local function copy_buffer_to_image(buffer, image, w, h)
   local region = {
      buffer_offset = 0,
      buffer_row_length = 0,
      buffer_image_height = 0,
      image_subresource = vk.imagesubresourcelayers({'color'}, 0, 0, 1),
      image_offset = {x=0, y=0, z=0},
      image_extent = {width=w, height=h, depth=1},
   }

   local cb = begin_single_time_command()
   vk.cmd_copy_buffer_to_image(cb, buffer, image, 'transfer dst optimal', {region})
   end_single_time_command(cb)
end

local function generate_mipmaps(image, w, h, mip_levels)
   local cb = begin_single_time_command()

   for i = 1, mip_levels-1 do
      vk.cmd_pipeline_barrier(cb,
         vk.PIPELINE_STAGE_TRANSFER_BIT, vk.PIPELINE_STAGE_TRANSFER_BIT, 0, nil, nil,
         {{
         image = image,
         old_layout = 'transfer dst optimal',
         new_layout = 'transfer src optimal',
         src_access_mask = vk.ACCESS_TRANSFER_WRITE_BIT,
         dst_access_mask = vk.ACCESS_TRANSFER_READ_BIT,
         subresource_range = vk.imagesubresourcerange({'color'}, i-1, 1, 0, 1),
         }}
      )

      local w2 = w > 1 and math.floor(w/2) or w
      local h2 = h > 1 and math.floor(h/2) or h

      local blit = {
         src_offsets = {{x=0,y=0,z=0}, {x=w,y=h,z=1}},
         src_subresource = vk.imagesubresourcelayers({'color'}, i-1, 0, 1),
         dst_offsets = {{x=0,y=0,z=0}, {x=w2,y=h2,z=1}},
         dst_subresource = vk.imagesubresourcelayers({'color'}, i, 0, 1),
      }

      vk.cmd_blit_image(cb, image, 'transfer src optimal', image, 'transfer dst optimal', {blit}, 'linear')

      vk.cmd_pipeline_barrier(cb,
         vk.PIPELINE_STAGE_TRANSFER_BIT, vk.PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, nil, nil,
         {{
         image = image,
         old_layout = 'transfer src optimal',
         new_layout = 'transfer dst optimal',
         src_access_mask = vk.ACCESS_TRANSFER_READ_BIT,
         dst_access_mask = vk.ACCESS_SHADER_READ_BIT,
         subresource_range = vk.imagesubresourcerange({'color'}, i-1, 1, 0, 1),
         }}
      )

      w, h = w2, h2
   end
   end_single_time_command(cb)
end

-------------------------------------------------------------------------------

-- Create texture image
local texture_image, texture_memory, texture_image_view, texture_mip_levels
do
   local format = 'r8g8b8a8 unorm'
   local data, w, h = mi.load(TEXTURE_PATH, 'rgba')
   local staging_buffer, staging_memory = create_buffer(#data, 
               vk.BUFFER_USAGE_TRANSFER_SRC_BIT,
               vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT | vk.MEMORY_PROPERTY_HOST_COHERENT_BIT)

   vk.map_memory(staging_memory, 0, #data)
   vk.write_memory(staging_memory, 0, data)
   vk.unmap_memory(staging_memory)

   texture_mip_levels = math.floor(math.log(math.max(w,h))) + 1

   texture_image, texture_memory = create_image(w, h, texture_mip_levels, 1, format, 'optimal',
            vk.IMAGE_USAGE_TRANSFER_SRC_BIT|vk.IMAGE_USAGE_TRANSFER_DST_BIT|vk.IMAGE_USAGE_SAMPLED_BIT, 
            vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

   transition_image_layout(texture_image, format, texture_mip_levels, 'undefined', 'transfer dst optimal')
   copy_buffer_to_image(staging_buffer, texture_image, w, h)
   generate_mipmaps(texture_image, w, h, texture_mip_levels) -- also transitions to 'shader read only optimal'

   vk.destroy_buffer(staging_buffer)
   vk.free_memory(staging_memory)

   -- Create image view
   texture_image_view = vk.create_image_view(texture_image, {
      view_type = '2d',
      format = format, 
      components = {},
      subresource_range = vk.imagesubresourcerange({'color'}, 0, texture_mip_levels, 0, 1),
   })
end

-- Create sampler
local texture_sampler = vk.create_sampler(device, {
   mag_filter = 'linear',
   min_filter = 'linear',
   address_mode_u = 'repeat',
   address_mode_v = 'repeat',
   address_mode_w = 'repeat',
   anisotropy_enable = true,
   max_anisotropy = 16,
   border_color = 'int opaque black',
   unnormalized_coordinates = false,
   compare_enable = false,
   compare_op = 'always',
   mipmap_mode = 'linear',
   min_lod = 0,
   max_lod = texture_mip_levels,
   mip_lod_bias = 0,
})

-- Create vertex buffer
local vertex_buffer, vertex_buffer_memory
do 
   -- 1) Create a staging buffer backed with host visible memory, so that we
   --    can map it and write data to it
   local staging_buffer, staging_memory = create_buffer(#vertexdata, 
         vk.BUFFER_USAGE_TRANSFER_SRC_BIT, 
         vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT|vk.MEMORY_PROPERTY_HOST_COHERENT_BIT)
   -- 2) Copy the vertex data to the staging buffer
   vk.map_memory(staging_memory, 0, #vertexdata)
   vk.write_memory(staging_memory, 0, vertexdata)
   vk.unmap_memory(staging_memory)
   -- 3) Create a vertex buffer backed with device local memory, which is faster
   --    to access from the device but cannot be mapped and written to directly 
   --    from the host (that's why we need a host visible staging buffer)
   vertex_buffer, vertex_buffer_memory = create_buffer(#vertexdata,
         vk.BUFFER_USAGE_TRANSFER_DST_BIT|vk.BUFFER_USAGE_VERTEX_BUFFER_BIT,
         vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
   -- 4) Transfer the vertex data from the staging buffer to the vertex buffer
   copy_buffer(staging_buffer, vertex_buffer, #vertexdata)
   -- Release staging resources (we don't need them any more)
   vk.destroy_buffer(staging_buffer)
   vk.free_memory(staging_memory)
end
 
-- Create index buffer
local index_buffer, index_buffer_memory
do 
   -- 1) Create a staging buffer
   local staging_buffer, staging_memory = create_buffer(#indexdata, 
         vk.BUFFER_USAGE_TRANSFER_SRC_BIT, 
         vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT|vk.MEMORY_PROPERTY_HOST_COHERENT_BIT)
   -- 2) Copy the index data to the staging buffer
   vk.map_memory(staging_memory, 0, #indexdata)
   vk.write_memory(staging_memory, 0, indexdata)
   vk.unmap_memory(staging_memory)
   -- 3) Create a index buffer
   index_buffer, index_buffer_memory = create_buffer(#indexdata,
         vk.BUFFER_USAGE_TRANSFER_DST_BIT|vk.BUFFER_USAGE_INDEX_BUFFER_BIT,
         vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
   -- 4) Transfer the index data from the staging buffer to the index buffer
   copy_buffer(staging_buffer, index_buffer, #indexdata)
   vk.destroy_buffer(staging_buffer)
   vk.free_memory(staging_memory)
end
 
-- Uniform variables

local ubodata = vk.pack('float', { mat4(), mat4(), mat4() }) -- { model, view, proj }

local uniform_staging_buffer, uniform_staging_memory = create_buffer(#ubodata,
   vk.BUFFER_USAGE_TRANSFER_SRC_BIT, 
   vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT|vk.MEMORY_PROPERTY_HOST_COHERENT_BIT)
   
local uniform_buffer, uniform_memory = create_buffer(#ubodata,
   vk.BUFFER_USAGE_TRANSFER_DST_BIT|vk.BUFFER_USAGE_UNIFORM_BUFFER_BIT,
   vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

local function update_ubo(t, aspect_ratio)
   -- Update the new transformation matrices and upload them to the GPU
   local model = rotate(t*math.rad(90), 0, 0, 1)
   local view = look_at(vec3(2, 2, 2), vec3(0, 0, 0), vec3(0, 0, 1))
   local proj = perspective(math.rad(45), aspect_ratio, 0.1, 10.0)
   -- Note that we transpose the matrices because they are in row-major order,
   -- while GLSL expect them to be in column-major order:
   ubodata = vk.pack('float', {transpose(model), transpose(view), transpose(clip_matrix*proj)})
   vk.map_memory(uniform_staging_memory, 0, #ubodata)
   vk.write_memory(uniform_staging_memory, 0, ubodata)
   vk.unmap_memory(uniform_staging_memory)
   copy_buffer(uniform_staging_buffer, uniform_buffer, #ubodata)
end

-- Create descriptor set layout
local descriptor_set_layout = vk.create_descriptor_set_layout(device, 0, {
   {
   binding = 0,
   descriptor_type = 'uniform buffer',
   descriptor_count = 1,
   stage_flags = vk.shaderstageflags('vertex')
   },
   {
   binding = 1,
   descriptor_type = 'combined image sampler',
   descriptor_count = 1,
   stage_flags = vk.shaderstageflags('fragment')
   }
})

-- Create descriptor pool and allocate descriptor set 
local descriptor_pool = vk.create_descriptor_pool(device, 0, 1, {
   { type='uniform buffer', descriptor_count=1 },
   { type='combined image sampler', descriptor_count=1 },
})

local descriptor_set = vk.allocate_descriptor_sets(descriptor_pool, {descriptor_set_layout})[1]

-- Update descriptor set
vk.update_descriptor_sets(device, {
   {
   dst_set = descriptor_set,
   dst_binding = 0,
   dst_array_element = 0,
   descriptor_type = 'uniform buffer',
   buffer_info = {{ buffer=uniform_buffer, offset=0, range=#ubodata}},
   },
   {
   dst_set = descriptor_set,
   dst_binding = 1,
   dst_array_element = 0,
   descriptor_type = 'combined image sampler',
   image_info = {{sampler=texture_sampler, image_view=texture_image_view, image_layout='shader read only optimal'}},
   }
}, nil)

-- Swapchain (re)creation

local swapchain, swapchain_images, swapchain_image_views, swapchain_format, swapchain_extent
local render_pass, pipeline_layout, graphics_pipeline, swapchain_framebuffers, command_buffers
local depth_format, depth_image, depth_image_mem, depth_image_view
local color_format, color_image, color_image_mem, color_image_view

local function create_swapchain(window, width, height)
   WIDTH, HEIGHT = width, height

   if swapchain then -- swapchain is being recreated, cleanup old
      vk.device_wait_idle(device)
      vk.destroy_image_view(depth_image_view)
      vk.destroy_image(depth_image)
      vk.free_memory(depth_image_mem)
      vk.free_memory(color_image_mem)
      for _, fb in ipairs(swapchain_framebuffers) do vk.destroy_framebuffer(fb) end
      vk.free_command_buffers(command_buffers)
      vk.destroy_pipeline(graphics_pipeline)
      vk.destroy_pipeline_layout(pipeline_layout)
      vk.destroy_render_pass(render_pass)
      for _, v in ipairs(swapchain_image_views) do vk.destroy_image_view(v) end
      vk.destroy_swapchain(swapchain)
   end
   -- Create the swapchain and get the swapchain images
   -- Before creating the swapchain, we have to choose a few things depending on
   -- our requirements and on the surface details we retrieved previously.
   
   -- 1) Determine the minimum number of swapchain images to ask for.
   -- We would like to have at least 2 of them, for double buffering.
   -- By the specs, nmin>=1 and if nmax==0 there is no theoretical upper limit.
   local nmin, nmax = surface_capabilities.min_image_count, surface_capabilities.max_image_count
   local image_count = nmax==0 and nmin+1 or math.min(nmax, nmin+1) 

   -- 2) Choose the swapchain format. Our desired one is:
   local surface_format = {format='b8g8r8a8 unorm', color_space='srgb nonlinear'}
   if not (#surface_formats==1 and surface_formats[1]=='undefined') then -- must choose a reported one
      for _, fmt in ipairs(surface_formats) do
         if fmt.format=='b8g8r8a8 unorm' and fmt.color_space=='srgb nonlinear' then break end
      end
      -- Our desired format is not supported, so let's select the first reported one
      surface_format = surface_formats[1]
   end

   -- 3) Choose the swapchain extent (ie the size of the images).
   local extent = surface_capabilities.current_extent
   if not extent then -- current_extent is undefined
      local emin, emax = surface_capabilities.min_image_extent, surface_capabilities.max_image_extent
      extent = {width=clamp(WIDTH, emin.width, emax.width), height=clamp(HEIGHT, emin.height, emax.height)}
   end

   -- 4) Choose the present mode.
   local present_mode = 'fifo' -- by the specs, this should be supported
   for _, mode in ipairs(surface_present_modes) do 
      if mode == 'mailbox' then present_mode = mode; break
      -- elseif mode == 'immediate' then present_mode = mode; break
      end
   end

   -- 5) Select the sharing mode. We need concurrent access to the swapchain images
   -- only if we have different queue families for graphics and present operations.
   local sharing_mode, family_indices = 'exclusive', nil
   if graphics_family~=present_family then
      sharing_mode, family_indices = 'concurrent', {graphics_family, present_family}
   end

   -- Eventually create the swapchain.
   swapchain = vk.create_swapchain(device, {
      surface = surface,
      min_image_count = image_count,
      image_format = surface_format.format,
      image_color_space = surface_format.color_space,
      image_extent = extent,
      image_array_layers = 1,
      image_usage = vk.imageusageflags('color attachment'),
      image_sharing_mode = sharing_mode,
      queue_family_indices = family_indices,
      pre_transform = surface_capabilities.current_transform,
      composite_alpha = vk.compositealphaflags('opaque'),
      present_mode = present_mode,
      clipped = true,
   })

   -- Get the swapchain images, and save format and extent for future use
   swapchain_images = vk.get_swapchain_images(swapchain)    
   swapchain_format = surface_format.format
   swapchain_extent = extent
   -- print(#swapchain_images, swapchain_format, swapchain_extent.width, swapchain_extent.height)

   -- Create an image view for each swapchain image
   swapchain_image_views = {}
   for i, image in ipairs(swapchain_images) do
      swapchain_image_views[i] = vk.create_image_view(image, {
         view_type = '2d',
         format = swapchain_format, 
         components = {},
         subresource_range = vk.imagesubresourcerange({'color'}, 0, 1, 0, 1),
      })
   end

   -- Create color resources
   color_format = swapchain_format
   color_image, color_image_mem = create_image(
         swapchain_extent.width, swapchain_extent.height, 1, msaa_samples, color_format, 'optimal', 
         vk.IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | vk.IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
         vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
   
   color_image_view = vk.create_image_view(color_image, {
      view_type = '2d',
      format = color_format, 
      components = {},
      subresource_range = vk.imagesubresourcerange({'color'}, 0, 1, 0, 1),
   })

   transition_image_layout(color_image, color_format, 1, 'undefined', 'color attachment optimal')

   -- Create depth resources
   depth_format = find_depth_format()
   depth_image, depth_image_mem = create_image(
         swapchain_extent.width, swapchain_extent.height, 1, msaa_samples, depth_format, 'optimal', 
         vk.IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

   depth_image_view = vk.create_image_view(depth_image, {
      view_type = '2d',
      format = depth_format, 
      components = {},
      subresource_range = vk.imagesubresourcerange({'depth'}, 0, 1, 0, 1),
   })

   transition_image_layout(depth_image, depth_format, 1, 'undefined', 'depth stencil attachment optimal')

   -- Create render_pass
   local color_attachment = {
      format = swapchain_format,
      samples = msaa_samples,
      load_op = 'clear',
      store_op = 'store',
      initial_layout = 'undefined',
      final_layout = 'color attachment optimal'
   }

   local depth_attachment = {
      format = depth_format,
      samples = msaa_samples,
      load_op = 'clear',
      initial_layout = 'undefined',
      final_layout = 'depth stencil attachment optimal'
   }

   local color_attachment_resolve = {
      format = swapchain_format,
      samples = 1,
      store_op = 'store',
      initial_layout = 'undefined',
      final_layout = 'present src'
   }

   local subpass = { 
      pipeline_bind_point = 'graphics', 
      color_attachments = {{ attachment=0, layout='color attachment optimal' }},
      depth_stencil_attachment = { attachment=1, layout='depth stencil attachment optimal' },
      resolve_attachments = {{ attachment=2, layout='color attachment optimal' }},
   }

   local dependency = {
      src_subpass = 'external',
      dst_subpass = 0,
      src_stage_mask = vk.pipelinestageflags('color attachment output'),
      src_access_mask = 0,
      dst_stage_mask = vk.pipelinestageflags('color attachment output'),
      dst_access_mask = vk.accessflags('color attachment read', 'color attachment write'),      
   }

   render_pass = vk.create_render_pass(device, {
      attachments = { color_attachment, depth_attachment, color_attachment_resolve },
      subpasses = { subpass },
      dependencies = { dependency },
   })

   -- Create pipeline
   local info = {} -- graphicspipelinecreateinfo

   info.stages = {
         { module = vert_shader, stage=vk.SHADER_STAGE_VERTEX_BIT, name='main' },
         { module = frag_shader, stage=vk.SHADER_STAGE_FRAGMENT_BIT, name='main' },
   }

   -- Setup the fixed functions parameters
   info.vertex_input_state = {
      vertex_binding_descriptions = vertex_binding_descriptions, 
      vertex_attribute_descriptions = vertex_attribute_descriptions,
   }

   info.input_assembly_state = { topology='triangle list', primitive_restart_enable=false }

   info.viewport_state = { 
      viewports = { vk.viewport(0, 0, swapchain_extent.width, swapchain_extent.height, 0, 1) },
      scissors = { vk.rect2d(0, 0, swapchain_extent.width, swapchain_extent.height) },
   }

   info.rasterization_state = {
      depth_clamp_enable = false,
      rasterizer_discard_enable = false,
      polygon_mode = 'fill',
      line_width = 1.0,
      cull_mode = vk.cullmodeflags('back'),
      front_face = 'counter clockwise',
      depth_bias_enable = false,
   }

   info.multisample_state = {
      rasterization_samples = msaa_samples,
      sample_shading_enable = false,
   }

   info.depth_stencil_state = {
      depth_test_enable = true,
      depth_write_enable = true,
      depth_compare_op = 'less',
      depth_bounds_test_enable = false,
      stencil_test_enable = false,
   }

   info.color_blend_state = {
      logic_op_enable = false,
      logic_op = 'copy',
      attachments = {
         { color_write_mask = vk.colorcomponentflags('r','g','b','a'), blend_enable=false },
      blend_constants = { 0.0, 0.0, 0.0, 0.0 },
      },
   }

   pipeline_layout = vk.create_pipeline_layout(device, 0, {descriptor_set_layout}, nil)

   info.layout = pipeline_layout
   info.render_pass = render_pass
   info.subpass = 0
-- info.old_swapchain = nil

   graphics_pipeline = vk.create_graphics_pipelines(device, nil, {info})[1]

   -- Create framebuffers
   swapchain_framebuffers = {}
   for i, image_view in ipairs(swapchain_image_views) do
      swapchain_framebuffers[i] = vk.create_framebuffer(device, {
         render_pass = render_pass,
         width = swapchain_extent.width,
         height = swapchain_extent.height,
         layers = 1,
         attachments = {color_image_view, depth_image_view, image_view},
      })
   end

   -- Allocate command buffers
   command_buffers = vk.allocate_command_buffers(command_pool, 'primary', #swapchain_framebuffers)

   -- Record commands
   for i, cb in ipairs(command_buffers) do
      vk.begin_command_buffer(cb, vk.COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
      vk.cmd_begin_render_pass(cb, {
         render_pass = render_pass,
         framebuffer = swapchain_framebuffers[i],
         render_area = { offset={x=0, y=0}, extent=swapchain_extent },
         clear_values = {
            { color={0.0, 0.0, 0.0, 1.0}},
            { depth=1.0, stencil=0},
         },
      }, 'inline')
      vk.cmd_bind_pipeline(cb, 'graphics', graphics_pipeline)
      vk.cmd_bind_vertex_buffers(cb, 0, {vertex_buffer}, {0})
      vk.cmd_bind_index_buffer(cb, index_buffer, 0, 'uint32')
      vk.cmd_bind_descriptor_sets(cb, 'graphics', pipeline_layout, 0, {descriptor_set})
      vk.cmd_draw_indexed(cb, #indices, 1, 0, 0, 0)
      vk.cmd_end_render_pass(cb)
      vk.end_command_buffer(cb)
   end
end

create_swapchain(window, WIDTH, HEIGHT)

-- Set input callbacks --------------------------------------------------------
glfw.set_key_callback(window, function(window, key, scancode, action)
   if key == 'escape' and action == 'press' then
      glfw.set_window_should_close(window, true)
   end
end)

glfw.set_window_size_callback(window, create_swapchain)

-- Main loop ------------------------------------------------------------------

-- Create the semaphores that we will use to synchronize image acquisition,
-- rendering, and presentation.
local image_available_sem = vk.create_semaphore(device)
local render_finished_sem = vk.create_semaphore(device)

collectgarbage()
collectgarbage('stop')
local start_time = glfw.get_time()
while not glfw.window_should_close(window) do
   glfw.poll_events(1/FPS)

   local now = glfw.get_time()
   update_ubo(now - start_time, swapchain_extent.width/swapchain_extent.height)

   -- Draw frame
   
   -- 1) Acquire a swapchain image
   local image_index, result
   while not image_index do
      image_index, result = vk.acquire_next_image(swapchain, 'blocking', image_available_sem)
      if image_index then break
      elseif result == 'out of date' then create_swapchain(window, WIDTH, HEIGHT)
      else error(result)
      end
   end

   -- 2) Render to the image, ie submit for execution the previously recorded commands
   --    for this specific swapchain image
   vk.queue_submit(graphics_queue, {
      {
      command_buffers = {command_buffers[image_index + 1]},
      -- execute the commands up to the 'color attachment output' stage, then
      -- pause and wait for the image_available_sem semaphore to be signaled:
      wait_semaphores = {image_available_sem}, 
      wait_dst_stage_mask = {vk.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
      signal_semaphores = {render_finished_sem}, -- signal this when this operation is finished
      }
   })

   -- 3) Display the rendered image, ie submit it for presentation (this returns the image
   --    to the presentation engine)
   result  = vk.queue_present(present_queue, {
      swapchains = {swapchain},
      image_indices = {image_index},
      wait_semaphores = {render_finished_sem}, -- wait for point 2) to finish before presenting
   })
   if result == 'success' then -- do nothing
   elseif result == 'out of date' or result == 'suboptimal' then 
      create_swapchain(window, WIDTH, HEIGHT)
   else error(result)
   end

   if DEBUG then vk.queue_wait_idle(present_queue) end
   collectgarbage()
end

