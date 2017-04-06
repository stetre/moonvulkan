#!/usr/bin/env lua
-- MoonVulkan example: surface.lua
--
-- Creates a window+surface using MoonVulkan in conjunction with MoonGLFW.

glfw = require("moonglfw")
vk = require("moonvulkan")

-- 1) Get the list of extensions required by GLFW to create surfaces:
extension_names = glfw.get_required_instance_extensions()
print("Required extensions: " .. table.concat(extension_names, ', '))

-- 2) Create a Vulkan instance, enabling the required extensions:
instance = vk.create_instance({ enabled_extension_names = extension_names })

-- 3) Create a window, not tied to any particular API:
glfw.window_hint('client api', 'no api')
window = glfw.create_window(640, 480, "My first GLFW/Vulkan window")

-- 4) Create the surface for the window. 
surfaceRAW = glfw.create_window_surface(window, instance:raw())

-- 5) Finally, pass the surface to MoonVulkan. 
-- This will create and return a MoonVulkan 'surface' userdata (binding object)
-- for future reference, and guarantee that the surface will be automatically
-- destroyed at exit or when the instance itself is destroyed:
surface = vk.created_surface(instance, surfaceRAW)


glfw.set_key_callback(window,
   function (window, key, scancode, action)
      if key == 'escape' and action == 'press' then
         glfw.set_window_should_close(window, true)
      end
end)

collectgarbage() -- always a good idea before entering the event loop

-- Event loop:
print("Press ESC to close the window")
while not glfw.window_should_close(window) do
   glfw.poll_events()
end

