#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")
local glmath = require("moonglmath")

local clamp = glmath.clamp

local WIDTH, HEIGHT = 800, 600
local LAYERS, EXTENSIONS = {}, {}

local DEBUG = arg[1] == "--debug"
if not DEBUG then 
   print("To enable debug, run as: \n$ "..arg[0].." --debug")
end

-- Init window ----------------------------------------------------------------
glfw.window_hint('client api', 'no api')
glfw.window_hint('resizable', false)
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
      vk.DEBUG_REPORT_ERROR_BIT | vk.DEBUG_REPORT_WARNING_BIT,
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

-- Create the swapchain and get the swapchain images
local swapchain, swapchain_images, swapchain_image_views, swapchain_format, swapchain_extent
do
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
end

-- Create pipeline
do 
   -- ...
end

-- Set input callbacks --------------------------------------------------------
glfw.set_key_callback(window, function(window, key, scancode, action)
   if key == 'escape' and action == 'press' then
      glfw.set_window_should_close(window, true)
   end
end)

-- Main loop ------------------------------------------------------------------
while not glfw.window_should_close(window) do
   glfw.wait_events()
end

