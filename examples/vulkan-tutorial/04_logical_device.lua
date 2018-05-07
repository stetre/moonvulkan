#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")

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
      vk.DEBUG_REPORT_ERROR_BIT_EXT | vk.DEBUG_REPORT_WARNING_BIT_EXT,
      function(...) print("DEBUG", ...) end
   )
end

-- Select a physical device
local physdev, graphics_family
for _, pd in ipairs(vk.enumerate_physical_devices(instance)) do
   -- Search for a graphics capable queue family
   for i, qf in ipairs(vk.get_physical_device_queue_family_properties(pd)) do
      if qf.queue_count > 0 and (qf.queue_flags & vk.QUEUE_GRAPHICS_BIT) ~= 0 then
         graphics_family = i-1 -- queue_family_indices are 0-based
         break
      end
   end
   if graphics_family then physdev = pd; break end
end
assert(physdev, "cannot find a suitable physical device")

-- Create a logical device
local device = vk.create_device(physdev, {
   queue_create_infos = {
      {queue_family_index=graphics_family, queue_priorities={1.0}},
   },
   enabled_features = {},
})

-- Get an handle to the graphics queue created with the device
local graphics_queue = vk.get_device_queue(device, graphics_family, 0)
-- Note that also queue indices are 0-based, like queue family indices.
-- In general, all indices that are native to the Vulkan C API are kept as
-- 0-based also in the MoonVulkan API bindings. 
-- They are not translated to Lua-style 1-based indices to avoid confusion
-- when analyzing dumps produced by the validation layers.

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

-- Cleanup --------------------------------------------------------------------
if debug_callback then vk.destroy_debug_report_callback(debug_callback) end
vk.destroy_device(device)
vk.destroy_instance(instance)
glfw.destroy_window(window)

