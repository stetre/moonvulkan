#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")
local glmath = require("moonglmath")

local clamp = glmath.clamp
local FLOATSZ = vk.sizeof('float') -- or simply 4...

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

local vert_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/base.vert"))
local frag_shader = vk.create_shader_module(device, 0, glsl_to_spirv("shaders/base.frag"))

-- Vertex data

local vertices = {
   -- position         color
   {{ 0.0, -0.5}, {1.0, 0.0, 0.0}},
   {{ 0.5,  0.5}, {0.0, 1.0, 0.0}},
   {{-0.5,  0.5}, {0.0, 0.0, 1.0}},
}
local vertexdata = vk.pack('float', vertices)
local vertex_binding_descriptions = {
   { binding=0, stride=5*FLOATSZ, input_rate='vertex' },
}
local vertex_attribute_descriptions = {
   { binding=0, location=0, format='r32g32 sfloat', offset=0 },            -- position
   { binding=0, location=1, format='r32g32b32 sfloat', offset=2*FLOATSZ }, -- color
}

-- Swapchain (re)creation

local swapchain, swapchain_images, swapchain_image_views, swapchain_format, swapchain_extent
local render_pass, pipeline_layout, graphics_pipeline, swapchain_framebuffers
local command_pool, command_buffers

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
      front_face = 'clockwise',
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

   pipeline_layout = vk.create_pipeline_layout(device, 0, nil, nil)

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

   -- Create command pool and allocate command buffers
   if not command_pool then
      command_pool = vk.create_command_pool(device, 0, graphics_family)
   end
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
      vk.cmd_draw(cb, #vertices, 1, 0, 0)
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
while not glfw.window_should_close(window) do
   glfw.wait_events()

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

