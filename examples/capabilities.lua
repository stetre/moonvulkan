#!/usr/bin/env lua
-- MoonVulkan example: capabilities.lua
--
-- Quick and dirty application that inspects the system and prints on stdout 
-- the available layers, extensions, physical devices and their properties.
--
-- Suggested usage:
-- $ ./capabilities.lua | less
-- or
-- $ ./capabilities.lua > mycaps.txt
--

vk = require("moonvulkan")

function P(t, name) -- deep print of the fields of table t
   local name = name or ""
   for k, v in pairs(t) do
      local n = name.."."..tostring(k)
      if type(v) == "table" then
         P(v, n)
      else
         print(n .. ": ".. tostring(v))
      end
   end
end

print("-----------------------------------------------------------------------------")
print("Extensions provided by the Vulkan implementation or by implicitly enabled layers:")
ext = vk.enumerate_instance_extension_properties()
print()
P(ext, "extension")

print("\n-----------------------------------------------------------------------------")
print("Available instance layers and extensions:")
layers = vk.enumerate_instance_layer_properties()

for _, props in ipairs(layers) do
   print()
   P(props, "layer")
   local ext = vk.enumerate_instance_extension_properties(props.layer_name)
   P(ext, "layer.extension")
end

print("\n-----------------------------------------------------------------------------")

instance = vk.create_instance({
   enabled_extension_names = { "VK_KHR_get_physical_device_properties2" },
})


physdevs = vk.enumerate_physical_devices(instance)
n_physdevs = #physdevs
if n_physdevs == 0 then
   print("No physical devices found")
   return
else
   print("Found " .. n_physdevs .. " physical device" .. ((n_physdevs > 1) and "s" or ""))
end

formats = vk.enum('format') -- table of all values for the 'format' enum

for i, gpu in ipairs(physdevs) do
   print()
   local name = "gpu"..(i-1).."."

   local properties = vk.get_physical_device_properties(gpu)
   P(properties, name.."properties")

   local extensions = vk.enumerate_device_extension_properties(gpu)
   P(extensions, name.."extensions")

   local features = vk.get_physical_device_features(gpu)
   P(features, name.."features")

   local families = vk.get_physical_device_queue_family_properties(gpu)
   P(families, name.."queue_family")

   local memprops = vk.get_physical_device_memory_properties(gpu)
   P(memprops, name.."memory_properties")

   for _, format in ipairs(formats) do
      local props = vk.get_physical_device_format_properties(gpu, format)
      P(props, name.."formatproperties["..format.."]")
   end
end

