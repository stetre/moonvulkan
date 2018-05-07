#!/usr/bin/env lua

local glfw = require("moonglfw")
local vk = require("moonvulkan")

local WIDTH, HEIGHT = 800, 600

-- Init window ----------------------------------------------------------------
glfw.window_hint('client api', 'no api')
glfw.window_hint('resizable', false)
local window = glfw.create_window(WIDTH, HEIGHT, "Vulkan")

-- Init Vulkan ----------------------------------------------------------------
-- ...

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
glfw.destroy_window(window)

