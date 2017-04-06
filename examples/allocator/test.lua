#!/usr/bin/env lua
-- MoonVulkan example: Custom allocator.

glfw = require("moonglfw")
vk = require("moonvulkan")

-- 1) Import the allocator module, and get the light userdata
allocator = require("allocator")
allocatorLUD = allocator.get()
allocator.trace(true)

if not glfw.vulkan_supported() then
    error("MoonGLFW was not compiled with Vulkan support")
end

extension_names = glfw.get_required_instance_extensions()

-- 2) Pass the allocator to the instance creating function:
instance = vk.create_instance({ enabled_extension_names = extension_names }, allocatorLUD)

glfw.window_hint('client api', 'no api')
window = glfw.create_window(640, 480, "My first GLFW/Vulkan window")

-- 3) Pass the allocator to the surface creating function (MoonGLFW):
surfaceRAW = glfw.create_window_surface(window, instance:raw(), allocatorLUD)

-- 4) We need to pass it also to created_surface(), so that MoonVulkan
--    will use it instead of NULL when destroying the surface.
surface = vk.created_surface(instance, surfaceRAW, allocatorLUD)

--[[ Event loop:
while not glfw.window_should_close(window) do
   glfw.poll_events()
end
--]]

