#!/usr/bin/env lua

vk = require("moonvulkan")

print("MoonVulkan version:", vk._VERSION)
print("Header version (vulkan.h):", vk.HEADER_VERSION)
print("Supported versions:", table.concat(vk.API_VERSIONS, ","))

for _, s in ipairs(vk.API_VERSIONS) do
   local ver = vk[s]
   local major, minor, patch = vk.version_numbers(ver)
   print(s .. ": " .. ver .. " (" ..  vk.version_string(ver) ..")" ..
        " major="..major.." minor="..minor.." patch="..patch)
end

if not vk.API_VERSION_1_0 then print("API_VERSION_1_0 is not supported") end
if not vk.API_VERSION_2_0 then print("API_VERSION_2_0 is not supported") end

print(vk.make_version(1, 0, 0))
print(vk.API_VERSION_1_0)

assert(vk.make_version(1, 0, 0) == vk.API_VERSION_1_0)

