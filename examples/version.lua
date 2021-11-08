#!/usr/bin/env lua

vk = require("moonvulkan")

print("MoonVulkan version:", vk._VERSION)
print("Header version (vulkan.h):", vk.HEADER_VERSION)
print("Versions supported by MoonVulkan:", table.concat(vk.API_VERSIONS, ", "))
print("Instance-level version supported by the implementation:",
      vk.version_string(vk.enumerate_instance_version()))

for _, s in ipairs(vk.API_VERSIONS) do
   local ver = vk[s]
   local major, minor, patch, variant = vk.version_numbers(ver)
   print(s .. ": " .. ver .. " (" ..  vk.version_string(ver) ..")" ..
        " major="..major.." minor="..minor.." patch="..patch.." variant="..variant)
end

if not vk.API_VERSION_1_0 then print("API_VERSION_1_0 is not supported") end
if not vk.API_VERSION_1_1 then print("API_VERSION_1_1 is not supported") end
if not vk.API_VERSION_1_2 then print("API_VERSION_1_2 is not supported") end
if not vk.API_VERSION_2_0 then print("API_VERSION_2_0 is not supported") end

print(vk.make_version(1, 0, 0, 0))
print(vk.API_VERSION_1_0)

assert(vk.make_version(1, 0, 0) == vk.API_VERSION_1_0)

