#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")
local glmath = require("moonglmath")

local clamp = glmath.clamp
local vec3, mat4 = glmath.vec3, glmath.mat4
local rotate, translate, scale = glmath.rotate, glmath.translate, glmath.scale
local look_at, perspective = glmath.look_at, glmath.perspective
local transpose = glmath.transpose

local FLOATSZ = vk.sizeof('float') -- or simply 4...

local FPS = 60
local WIDTH, HEIGHT = 800, 600
local LAYERS, EXTENSIONS = {}, {}

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
   if physdev then -- retrieve some surface details and check if they suit us.
      surface_capabilities = vk.get_physical_device_surface_capabilities(pd, surface)
      surface_formats = vk.get_physical_device_surface_formats(pd, surface)
      surface_present_modes = vk.get_physical_device_surface_present_modes(pd, surface)
      if #surface_formats > 0 and #surface_present_modes > 0 then break end -- found
      physdev = nil -- not suitable, try with the next one (if any)
   end
end
assert(physdev, "cannot find a suitable physical device")

-- Create a logical device
local device = vk.create_device(physdev, {
   -- If the two queue families are different we must create a queue from
   -- each of them, otherwise we must create only one queue.
   queue_create_infos = graphics_family ~= present_family
      and {{queue_family_index=graphics_family, queue_priorities={1.0}},
           {queue_family_index=present_family,  queue_priorities={1.0}}}
      or  {{queue_family_index=graphics_family, queue_priorities={1.0}}},
   enabled_features = {},
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

local vert_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/ubo.vert"))
local frag_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/ubo.frag"))

-- Vertex data and element indices

local vertices = {
   -- position         color                                   Vulkan coord. system has
   {{-0.5, -0.5}, {1.0, 0.0, 0.0}}, -- 0  0__1     _|___+x  flipped y-axis wrt to OpenGL.
   {{ 0.5, -0.5}, {0.0, 1.0, 0.0}}, -- 1  |\ |     /|       It also has 'half z', 
   {{ 0.5,  0.5}, {0.0, 0.0, 1.0}}, -- 2  |_\|    / |       ie z goes from 0 to 1 in NDC
   {{-0.5,  0.5}, {1.0, 1.0, 1.0}}, -- 3  3  2  +z  +y      instead of going from -1 to 1.
}

local vertexdata = vk.pack('float', vertices)
local vertex_binding_descriptions = {
   { binding=0, stride=5*FLOATSZ, input_rate='vertex' },
}
local vertex_attribute_descriptions = {
   { binding=0, location=0, format='r32g32 sfloat', offset=0 },            -- position
   { binding=0, location=1, format='r32g32b32 sfloat', offset=2*FLOATSZ }, -- color
}

local indices = { 0, 1, 2, 2, 3, 0 } -- clockwise orientation
local indexdata = vk.pack('uint16', indices)

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

local function copy_buffer(src_buffer, dst_buffer, size)
   local cb = vk.allocate_command_buffers(command_pool, 'primary', 1)[1]
   vk.begin_command_buffer(cb, vk.COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
   vk.cmd_copy_buffer(cb, src_buffer, dst_buffer, {{size=size}})
   vk.end_command_buffer(cb)
   vk.queue_submit(graphics_queue, {{command_buffers = {cb}}})
   vk.queue_wait_idle(graphics_queue)
   vk.free_command_buffers({cb})
end

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
   }
})

-- Create descriptor pool and allocate descriptor set 
local descriptor_pool = vk.create_descriptor_pool(device, 0, 1, {
   { type='uniform buffer', descriptor_count=1 },
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
   }
}, nil)

-- Swapchain (re)creation

local swapchain, swapchain_images, swapchain_image_views, swapchain_format, swapchain_extent
local render_pass, pipeline_layout, graphics_pipeline, swapchain_framebuffers, command_buffers

local function create_swapchain(window, width, height)
   WIDTH, HEIGHT = width, height

   if swapchain then -- swapchain is being recreated, cleanup old
      vk.device_wait_idle(device)
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

   -- Create render_pass
   local color_attachment = {
      format = swapchain_format,
      samples = 1,
      load_op = 'clear',
      store_op = 'store',
      initial_layout = 'undefined',
      final_layout = 'present src'
   }

   local subpass = { 
      pipeline_bind_point = 'graphics', 
      color_attachments = {{ attachment=0, layout='color attachment optimal' }},
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
      attachments = { color_attachment },
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
      rasterization_samples = 1,
      sample_shading_enable = false,
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
         attachments = {image_view},
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
         clear_values = {{ color={0.0, 0.0, 0.0, 1.0}}},
      }, 'inline')
      vk.cmd_bind_pipeline(cb, 'graphics', graphics_pipeline)
      vk.cmd_bind_vertex_buffers(cb, 0, {vertex_buffer}, {0})
      vk.cmd_bind_index_buffer(cb, index_buffer, 0, 'uint16')
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

