#!/usr/bin/env lua
-- Draw a textured triangle with depth testing.
--
-- This code is adapted from glfw-3.2.1/tests/vulkan.c (http://www.glfw.org).
-- See copyright notice in moonvulkan/thirdparty/triangle-demo.license.
--

vk = require("moonvulkan")
glfw = require("moonglfw")
glmath = require("moonglmath")  -- math library

--vk.trace_objects(true)

VERTEX_BUFFER_BIND_ID = 0
APP_SHORT_NAME = "vulkan"
APP_LONG_NAME = "The Vulkan Triangle Demo Program"
TEX_FORMAT = 'b8g8r8a8 unorm'
TEX_COLORS = {{0xffff0000, 0xff00ff00}}

function dbgFunc(instance, flags, objType, objectRaw, location, code, layerPrefix, message)
   if flags & vk.DEBUG_REPORT_ERROR_BIT_EXT ~= 0 then
        print(string.format("ERROR: [%s] Code %d : %s", layerPrefix, code, message))
   elseif flags & vk.DEBUG_REPORT_WARNING_BIT_EXT ~= 0 then
        print(string.format("WARNING: [%s] Code %d : %s", layerPrefix, code, message))
   end
end

function memory_type_from_properties(demo, typeBits, requirements_mask) 
   -- Search memtypes to find first index with those properties
   for i = 1, 32 do
      if (typeBits & 1) == 1 then -- Type is available, does it match user properties?
         if (demo.memory_properties.memory_types[i].property_flags & 
                 requirements_mask) == requirements_mask then
            return i-1
         end
      end
      typeBits = typeBits >> 1
   end
   return nil -- No memory types matched, return failure
end

function create_shader_code () 
   -- Pre-compiled SPIR-V shaders from the GLFW tests/vulkan.c demo
   local frag_shader_code = vk.pack('ubyte', {
    0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x08, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64, 0x2e, 0x34, 0x35, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x04, 0x00, 0x09, 0x00,
    0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73, 0x65, 0x70, 0x61, 0x72,
    0x61, 0x74, 0x65, 0x5f, 0x73, 0x68, 0x61, 0x64, 0x65, 0x72, 0x5f, 0x6f,
    0x62, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x00, 0x00, 0x04, 0x00, 0x09, 0x00,
    0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73, 0x68, 0x61, 0x64, 0x69,
    0x6e, 0x67, 0x5f, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x5f,
    0x34, 0x32, 0x30, 0x70, 0x61, 0x63, 0x6b, 0x00, 0x05, 0x00, 0x04, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x05, 0x00, 0x09, 0x00, 0x00, 0x00, 0x75, 0x46, 0x72, 0x61,
    0x67, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x74, 0x65, 0x78, 0x00, 0x05, 0x00, 0x05, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x74, 0x65, 0x78, 0x63, 0x6f, 0x6f, 0x72, 0x64,
    0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x47, 0x00, 0x04, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x16, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x3b, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x19, 0x00, 0x09, 0x00, 0x0a, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x03, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0x0a, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x04, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0x0e, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
    0x0f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x57, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x0e, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x03, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
    0x38, 0x00, 0x01, 0x00
   })

   local vert_shader_code = vk.pack('ubyte', {
    0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x08, 0x00,
    0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64, 0x2e, 0x34, 0x35, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00,
    0x04, 0x00, 0x09, 0x00, 0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73,
    0x65, 0x70, 0x61, 0x72, 0x61, 0x74, 0x65, 0x5f, 0x73, 0x68, 0x61, 0x64,
    0x65, 0x72, 0x5f, 0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x00, 0x00,
    0x04, 0x00, 0x09, 0x00, 0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73,
    0x68, 0x61, 0x64, 0x69, 0x6e, 0x67, 0x5f, 0x6c, 0x61, 0x6e, 0x67, 0x75,
    0x61, 0x67, 0x65, 0x5f, 0x34, 0x32, 0x30, 0x70, 0x61, 0x63, 0x6b, 0x00,
    0x05, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e,
    0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x74, 0x65, 0x78, 0x63, 0x6f, 0x6f, 0x72, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x61, 0x74, 0x74, 0x72,
    0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x67, 0x6c, 0x5f, 0x50, 0x65, 0x72, 0x56, 0x65, 0x72, 0x74, 0x65, 0x78,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74,
    0x69, 0x6f, 0x6e, 0x00, 0x06, 0x00, 0x07, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x69, 0x6e, 0x74,
    0x53, 0x69, 0x7a, 0x65, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x43,
    0x6c, 0x69, 0x70, 0x44, 0x69, 0x73, 0x74, 0x61, 0x6e, 0x63, 0x65, 0x00,
    0x05, 0x00, 0x03, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x03, 0x00, 0x17, 0x00, 0x00, 0x00, 0x70, 0x6f, 0x73, 0x00,
    0x05, 0x00, 0x05, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x56,
    0x65, 0x72, 0x74, 0x65, 0x78, 0x49, 0x44, 0x00, 0x05, 0x00, 0x06, 0x00,
    0x1d, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x49, 0x6e, 0x73, 0x74, 0x61,
    0x6e, 0x63, 0x65, 0x49, 0x44, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x47, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x47, 0x00, 0x03, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x17, 0x00, 0x00, 0x00,
    0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
    0x1c, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x47, 0x00, 0x04, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x13, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x21, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x16, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x07, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00,
    0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x15, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x00, 0x00,
    0x0f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x04, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    0x1e, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
    0x12, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x3b, 0x00, 0x04, 0x00, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x04, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00, 0x1b, 0x00, 0x00, 0x00,
    0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
    0x1b, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x03, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
    0x41, 0x00, 0x05, 0x00, 0x19, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00,
    0x13, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x03, 0x00,
    0x1a, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
    0x38, 0x00, 0x01, 0x00
   })
   --assert(#frag_shader_code %4 == 0)
   --assert(#vert_shader_code %4 == 0)
   return { vert = vert_shader_code, frag = frag_shader_code }
end


function demo_flush_init_cmd(demo) 
   if not demo.setup_cmd then return end
   vk.end_command_buffer(demo.setup_cmd)
   vk.queue_submit(demo.queue, {{ command_buffers = {demo.setup_cmd} }}, nil)
   vk.queue_wait_idle(demo.queue)
   vk.free_command_buffers({demo.setup_cmd})
   demo.setup_cmd = nil
end

function demo_set_image_layout(demo, image, aspectMask, old_image_layout, new_image_layout)
   if not demo.setup_cmd then
      demo.setup_cmd = vk.allocate_command_buffers(demo.cmd_pool, 'primary', 1)[1]
      vk.begin_command_buffer(demo.setup_cmd, 0)
   end

   local dst_access_mask = 0
   if new_image_layout == 'transfer dst optimal' then
      -- Make sure anything that was copying from this image has completed
      dst_access_mask = vk.ACCESS_TRANSFER_READ_BIT
   end

   if new_image_layout == 'color attachment optimal' then
      dst_access_mask = vk.ACCESS_COLOR_ATTACHMENT_WRITE_BIT
   end

   if new_image_layout == 'depth stencil attachment optimal' then
      dst_access_mask = vk.ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
   end

   if new_image_layout == 'shader read only optimal' then
      -- Make sure any Copy or CPU writes to image are flushed
      dst_access_mask = vk.ACCESS_SHADER_READ_BIT | vk.ACCESS_INPUT_ATTACHMENT_READ_BIT
   end

   local image_memory_barrier = {
      src_access_mask = 0,
      dst_access_mask = dst_access_mask,
      old_layout = old_image_layout,
      new_layout = new_image_layout,
      image = image,
      subresource_range = vk.imagesubresourcerange(aspectMask, 0, 1, 0, 1)
   }

   vk.cmd_pipeline_barrier(demo.setup_cmd, 
      vk.PIPELINE_STAGE_TOP_OF_PIPE_BIT, vk.PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 
      nil, nil, { image_memory_barrier }
   )
end


function demo_draw_build_cmd(demo)
   vk.begin_command_buffer(demo.draw_cmd, 0)

   vk.cmd_begin_render_pass(demo.draw_cmd, {
      render_pass = demo.render_pass,
      framebuffer = demo.framebuffers[demo.current_buffer],
      render_area = { offset={x=0, y=0}, extent={width=demo.width, height=demo.height}},
      clear_values = { 
         { color={ 0.2, 0.2, 0.2, 0.2 }}, 
         { depth = demo.depthStencil, stencil = 0 },
      },
   }, 'inline')

   vk.cmd_bind_pipeline(demo.draw_cmd, 'graphics', demo.pipeline)

   vk.cmd_bind_descriptor_sets(demo.draw_cmd, 'graphics', demo.pipeline_layout, 0, {demo.desc_set}, nil)

   vk.cmd_set_viewport(demo.draw_cmd, 0, 
      {{ x=0, y=0, width=demo.width, height=demo.height, min_depth=0.0, max_depth=1.0 }})

   vk.cmd_set_scissor(demo.draw_cmd, 0, 
      {{ offset={x=0, y=0}, extent={width=demo.width, height=demo.height} }})

   vk.cmd_bind_vertex_buffers(demo.draw_cmd, VERTEX_BUFFER_BIND_ID, {demo.vertices.buf}, {0})

   vk.cmd_draw(demo.draw_cmd, 3, 1, 0, 0)

   vk.cmd_end_render_pass(demo.draw_cmd)

   local prePresentBarrier = {
      src_access_mask = vk.ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      dst_access_mask = vk.ACCESS_MEMORY_READ_BIT,
      old_layout = 'color attachment optimal',
      new_layout = 'present src',
      image = demo.buffers[demo.current_buffer].image,
      subresource_range = vk.imagesubresourcerange({'color'}, 0, 1, 0, 1),
   }

   vk.cmd_pipeline_barrier(demo.draw_cmd, 
      vk.PIPELINE_STAGE_ALL_COMMANDS_BIT, vk.PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 
      nil, nil, { prePresentBarrier })

   vk.end_command_buffer(demo.draw_cmd)
end


function demo_draw(demo)
    local presentCompleteSemaphore = vk.create_semaphore(demo.device)

   -- Get the index of the next available swapchain image:
   local index, message = 
      vk.acquire_next_image(demo.swapchain, 'blocking', presentCompleteSemaphore, nil)
   assert(index)
   demo.current_buffer = index + 1 -- because index is 0 based

   -- Assume the command buffer has been run on current_buffer before so
   -- we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
   demo_set_image_layout(demo, demo.buffers[demo.current_buffer].image, { 'color' }, 
                     'present src', 'color attachment optimal')
   demo_flush_init_cmd(demo)

   -- Wait for the present complete semaphore to be signaled to ensure
   -- that the image won't be rendered to until the presentation
   -- engine has fully released ownership to the application, and it is
   -- okay to render to the image.

   -- FIXME/TODO: DEAL WITH VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
   demo_draw_build_cmd(demo)

   vk.queue_submit(demo.queue, {{
      wait_semaphores = { presentCompleteSemaphore },
      wait_dst_stage_mask = { vk.PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT },
      command_buffers = { demo.draw_cmd },
   }})

   local result = vk.queue_present(demo.queue, {
      swapchains = {demo.swapchain},
      image_indices = {demo.current_buffer-1},
   })
   --assert(result == 'success', "result = "..result)
   if result ~= 'success' then print("queue_present result = "..result) end

   vk.queue_wait_idle(demo.queue)

   vk.destroy_semaphore(presentCompleteSemaphore)
end


function prepare_swapchain(demo) 
   local oldSwapchain = demo.swapchain

   -- Check the surface capabilities and formats
   local surfCapabilities = vk.get_physical_device_surface_capabilities(demo.gpu, demo.surface)
   local presentModes = vk.get_physical_device_surface_present_modes(demo.gpu, demo.surface)

   local swapchainExtent = {}
   if not surfCapabilities.current_extent then
      -- If the surface size is undefined, the size is set to the size of the images requested.
      swapchainExtent.width = demo.width
      swapchainExtent.height = demo.height
   else 
      -- If the surface size is defined, the swap chain size must match
      swapchainExtent = surfCapabilities.current_extent
      demo.width = surfCapabilities.current_extent.width
      demo.height = surfCapabilities.current_extent.height
   end

   -- Determine the number of VkImage's to use in the swap chain (we desire to own only 1 image 
   -- at a time, besides the images being displayed and queued for display):
   local desiredNumberOfSwapchainImages = surfCapabilities.min_image_count + 1
   if surfCapabilities.max_image_count > 0 and
      desiredNumberOfSwapchainImages > surfCapabilities.max_image_count then
      -- Application must settle for fewer images than desired:
      desiredNumberOfSwapchainImages = surfCapabilities.max_image_count
   end

   local preTransform
   if surfCapabilities.supported_transforms & vk.SURFACE_TRANSFORM_IDENTITY_BIT_KHR ~= 0 then
      preTransform = vk.SURFACE_TRANSFORM_IDENTITY_BIT_KHR
   else
      preTransform = surfCapabilities.current_transform
   end

   demo.swapchain = vk.create_swapchain(demo.device, {
      surface = demo.surface,
      min_image_count = desiredNumberOfSwapchainImages,
      image_format = demo.format,
      image_color_space = demo.color_space,
      image_extent = { width = swapchainExtent.width, height = swapchainExtent.height },
      image_usage = vk.IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      pre_transform = preTransform,
      compositeAlpha = vk.COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      image_array_layers = 1,
      image_sharingMode = 'exclusive',
      present_mode = 'fifo',
      old_swapchain = oldSwapchain,
      clipped = true,
    })

   -- If we just re-created an existing swapchain, we should destroy the old swapchain at this point.
   -- Note: destroying the swapchain also cleans up all its associated presentable images once the 
   -- platform is done with them.
   if oldSwapchain then vk.destroy_swapchain(oldSwapchain) end

   local swapchainImages = vk.get_swapchain_images(demo.swapchain)

   local info = { -- imageviewcreateinfo
      flags = 0,
      view_type = '2d',
      format = demo.format,
      components = { r='r', g='g', b='b', a='a' },
      subresource_range = vk.imagesubresourcerange({'color'}, 0, 1, 0, 1),
   }

   demo.buffers = {}
   for i, image in ipairs(swapchainImages) do
      -- Render loop will expect image to have been used before and in 'present src' layout 
      -- and will change to 'color attachment optimal', so init the image to that state
      demo_set_image_layout(demo, image, {'color'}, 'undefined', 'present src')
      demo.buffers[i] = {}
      demo.buffers[i].image = image
      demo.buffers[i].view = vk.create_image_view(image, info)
   end

   demo.current_buffer = 1 -- = index + 1
end


function prepare_depth_buffer(demo)
   local depth_format = 'd16 unorm'
   -- create image
   demo.depth = {}
   demo.depth.format = depth_format

   demo.depth.image = vk.create_image(demo.device, {
      flags = 0,
      image_type = '2d',
      format = depth_format,
      extent = { width=demo.width, height=demo.height, depth=1},
      mip_levels = 1,
      array_layers = 1,
      samples = 1,
      tiling = 'optimal',
      usage = vk.imageusageflags('depth stencil attachment'),
   })

   -- get memory requirements for this object
   local mem_reqs = vk.get_image_memory_requirements(demo.depth.image)

   -- select memory size and type
   local allocation_size = mem_reqs.size
   local memory_type_index = memory_type_from_properties(demo, mem_reqs.memory_type_bits, 0) 

   demo.depth.mem = vk.allocate_memory(demo.device, allocation_size, memory_type_index)

   vk.bind_image_memory(demo.depth.image, demo.depth.mem, 0)

   demo_set_image_layout(demo, demo.depth.image, 
         {'depth'}, 'undefined', 'depth stencil attachment optimal')

   -- create image view
   demo.depth.view = vk.create_image_view(demo.depth.image, {
      flags = 0,
      format = depth_format,
      view_type = '2d',
      subresource_range = vk.imagesubresourcerange({'depth'}, 0, 1, 0, 1),
   })
end

function demo_prepare_texture_image(demo, tex_colors, tiling, usage, required_props) 
   local tex_width, tex_height = 2, 2
   local tex_obj = {}
   tex_obj.tex_width = tex_width
   tex_obj.tex_height = tex_height
   
   tex_obj.image = vk.create_image(demo.device, {
      flags = 0,
      image_type = '2d',
      format = TEX_FORMAT,
      extent = { width=tex_width, height=tex_height, depth=1 },
      mip_levels = 1,
      array_layers = 1,
      samples = 1,
      tiling = tiling,
      usage = usage,
   })

   local mem_reqs = vk.get_image_memory_requirements(tex_obj.image)

   local allocation_size = mem_reqs.size
   local memory_type_index = memory_type_from_properties(demo, mem_reqs.memory_type_bits, required_props) 

   tex_obj.mem = vk.allocate_memory(demo.device, allocation_size, memory_type_index)
   vk.bind_image_memory(tex_obj.image, tex_obj.mem, 0)

   if required_props & vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT ~= 0 then
      local layout = vk.get_image_subresource_layout(tex_obj.image, {
         aspect_mask = vk.IMAGE_ASPECT_COLOR_BIT,
         mip_level = 0,
         array_layer = 0,
      })

      vk.map_memory(tex_obj.mem, 0, 'whole size', 0)
      for y = 0, tex_height-1 do
         local offset = layout.row_pitch * y
         local data = {}
         for x = 0, tex_width - 1 do
            data[x+1] = tex_colors[((x & 1) ~ (y & 1)) + 1]
         end
         vk.write_memory(tex_obj.mem, offset, vk.pack('uint32', data))
      end

      vk.unmap_memory(tex_obj.mem)
   end

   tex_obj.imageLayout = 'shader read only optimal'
   demo_set_image_layout(demo, tex_obj.image, {'color'}, 'undefined', tex_obj.imageLayout)
   -- setting the image layout does not reference the actual memory so no need to add a mem ref
   return tex_obj
end

function demo_destroy_texture_image(demo, tex_obj)
   -- clean up staging resources
   vk.destroy_image(tex_obj.image)
   vk.free_memory(tex_obj.mem)
end


function prepare_textures(demo)
   local props = vk.get_physical_device_format_properties(demo.gpu, TEX_FORMAT)
   demo.textures = {}

   for i = 1, #TEX_COLORS do
      if (props.linear_tiling_features & vk.FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ~= 0 and
            not demo.use_staging_buffer then
         -- Device can texture using linear textures
         demo.textures[i] = demo_prepare_texture_image(demo, TEX_COLORS[i],
                 'linear', vk.IMAGE_USAGE_SAMPLED_BIT, vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      elseif (props.optimal_tiling_features & vk.FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ~= 0 then
         -- Must use staging buffer to copy linear texture to optimized
         local staging_texture = demo_prepare_texture_image(demo, TEX_COLORS[i], 
                  'linear', vk.IMAGE_USAGE_TRANSFER_SRC_BIT, vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT)
         demo.textures[i] = demo_prepare_texture_image(demo, TEX_COLORS[i],
                  'optimal', vk.IMAGE_USAGE_TRANSFER_DST_BIT | vk.IMAGE_USAGE_SAMPLED_BIT,
                              vk.MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

         demo_set_image_layout(demo, staging_texture.image, {'color'}, 
                              staging_texture.imageLayout, 'transfer src optimal')

         demo_set_image_layout(demo, demo.textures[i].image, {'color'}, 
                              demo.textures[i].imageLayout, 'transfer dst optimal')

         local copy_region = {
            src_subresource = vk.imagesubresourcelayers({'color'}, 0, 0, 1),
            src_offset = { x=0, y=0, z=0 },
            dst_subresource = vk.imagesubresourcelayers({'color'}, 0, 0, 1),
            dst_offset = { x=0, y=0, z=0 },
            extent = vk.extent3d(staging_texture.tex_width, staging_texture.tex_height, 1),
         }
      
         vk.cmd_copy_image(demo.setup_cmd, staging_texture.image, 'transfer src optimal',
                demo.textures[i].image, 'transfer dst optimal', {copy_region})

         demo_set_image_layout(demo, demo.textures[i].image, {'color'}, 'transfer dst optimal',
                                  demo.textures[i].imageLayout)

         demo_flush_init_cmd(demo)

         demo_destroy_texture_image(demo, staging_texture)
      else
         error("No support for '".. TEX_FORMAT .."' as texture image format")
      end


      -- create sampler
      demo.textures[i].sampler = vk.create_sampler(demo.device, {
         mag_filter = 'nearest',
         min_filter = 'nearest',
         mipmap_mode = 'nearest',
         address_mode_u = 'repeat',
         address_mode_v = 'repeat',
         address_mode_w = 'repeat',
         mid_lod_bias = 0.0,
         anisotropy_enable = false,
         max_anisotropy = 1.0,
         compare_enable = false,
         compare_op = 'never',
         min_lod = 0.0,
         max_lod = 0.0,
         border_color = 'float opaque white',
         unnormalized_coordinates = false,
      })

      -- create image view
      demo.textures[i].view = vk.create_image_view(demo.textures[i].image, {
         flags = 0,
         view_type = '2d',
         format = TEX_FORMAT,
         components = { r='r', g='g', b='b', a='a' },
         subresource_range = vk.imagesubresourcerange({'color'}, 0, 1, 0, 1),
      })
   end 
end

function prepare_vertices(demo) 
   local vb = {
   --    position            texcoord
      { -1.0, -1.0,  0.25,   0.0, 0.0 },
      {  1.0, -1.0,  0.25,   1.0, 0.0 },
      {  0.0,  1.0,  1.0,    0.5, 1.0 },
   }
   local data = vk.pack('float', vb)

   demo.vertices = {}
   demo.vertices.buf = vk.create_buffer(demo.device, {
      size = #data,
      usage = vk.bufferusageflags('vertex buffer'),
   })

   local mem_reqs = vk.get_buffer_memory_requirements(demo.vertices.buf)

   -- select memory size and type
   local allocation_size = mem_reqs.size
   local memory_type_index = 
      memory_type_from_properties(demo, mem_reqs.memory_type_bits, vk.MEMORY_PROPERTY_HOST_VISIBLE_BIT) 

   demo.vertices.mem = vk.allocate_memory(demo.device, allocation_size, memory_type_index)

   vk.map_memory(demo.vertices.mem, 0, 'whole size', 0)
   vk.write_memory(demo.vertices.mem, 0, data)
   vk.unmap_memory(demo.vertices.mem)

   vk.bind_buffer_memory(demo.vertices.buf, demo.vertices.mem, 0)

   local floatsz = vk.sizeof('float') -- or simply 4 ...
   demo.vertices.vi = { -- pipelinevertexinputstatecreateinfo  
      vertex_binding_descriptions = {
         { binding=VERTEX_BUFFER_BIND_ID, stride = 5*floatsz, input_rate='vertex' },
      },
      vertex_attribute_descriptions = {
         { binding=VERTEX_BUFFER_BIND_ID, location=0, format='r32g32b32 sfloat', offset=0 }, 
         { binding=VERTEX_BUFFER_BIND_ID, location=1, format='r32g32 sfloat', offset=3*floatsz },
      },
   }
end

function prepare_descriptor_layout(demo)
   demo.desc_layout = vk.create_descriptor_set_layout(demo.device, 0, {
      { 
      binding=0,
      descriptor_type = 'combined image sampler',
      descriptor_count = #TEX_COLORS,
      stage_flags = vk.SHADER_STAGE_FRAGMENT_BIT,
      },
   })

   demo.pipeline_layout = vk.create_pipeline_layout(demo.device, 0, { demo.desc_layout })
end

function prepare_render_pass(demo)
   demo.render_pass = vk.create_render_pass(demo.device, {
      attachments = {
         {
         format = demo.format,
         samples = 1,
         load_op = 'clear',
         store_op = 'store',
         initial_layout = 'color attachment optimal',
         final_layout = 'color attachment optimal',
         },
         {
         format = demo.depth.format,
         samples = 1,
         load_op = 'clear',
         store_op = 'dont care',
         initial_layout = 'depth stencil attachment optimal',
         final_layout = 'depth stencil attachment optimal',
         },
      },
      subpasses = {
         {
         flags = 0,
         pipeline_bind_point = 'graphics',
         color_attachments = {
            { attachment=0, layout='color attachment optimal' },
         },
         depth_stencil_attachment = { attachment=1, layout='depth stencil attachment optimal' },
         },
      },
   })
end



function prepare_pipeline(demo)
   local shader_code = create_shader_code()
   local vert_shader_module = vk.create_shader_module(demo.device, 0, shader_code.vert)
   local frag_shader_module = vk.create_shader_module(demo.device, 0, shader_code.frag)

   demo.pipeline = vk.create_graphics_pipelines(demo.device, nil, {{
      flags = 0,
      stages = { 
         { stage=vk.SHADER_STAGE_VERTEX_BIT, module=vert_shader_module, name='main' }, 
         { stage=vk.SHADER_STAGE_FRAGMENT_BIT, module=frag_shader_module, name='main' }, 
      },
      vertex_input_state = demo.vertices.vi,
      input_assembly_state = { topology='triangle list' },
      tessellation_state = nil,
      viewport_state = { viewport_count=1, scissor_count=1 },
      rasterization_state = { 
         polygon_mode='fill', 
         cull_mode=vk.CULL_MODE_BACK_BIT,
         front_face = 'clockwise',
      },
      multisample_state = { rasterization_samples=1 },
      depth_stencil_state = {
         depth_test_enable = true,
         depth_write_enable = true,
         depth_compare_op = 'less or equal',
         back = { fail_op='keep', pass_op='keep', compare_op='always' },
         front = { fail_op='keep', pass_op='keep', compare_op='always' },
      },
      color_blend_state = {
         attachments = {
            { blend_enable = false, color_write_mask = vk.colorcomponentflags('r', 'g', 'b', 'a') },
         }
      },
      dynamic_state = { dynamic_states = { 'viewport', 'scissor' }},
      layout = demo.pipeline_layout,
      render_pass = demo.render_pass,
   }})[1]

   vk.destroy_shader_module(frag_shader_module)
   vk.destroy_shader_module(vert_shader_module)
end

function prepare_descriptor_pool(demo)
   demo.desc_pool = vk.create_descriptor_pool(demo.device, 0, 1, { 
         { type='combined image sampler', descriptor_count = #TEX_COLORS }
   })
end


function prepare_descriptor_set(demo)
   demo.desc_set = vk.allocate_descriptor_sets(demo.desc_pool, {demo.desc_layout})[1]

   local imageinfo = {}
   for _, tex in ipairs(demo.textures) do  
      imageinfo[#imageinfo+1] = { 
         sampler=tex.sampler, 
         image_view=tex.view, 
         image_layout = 'general' 
      }
   end

   vk.update_descriptor_sets(demo.device, {{
      dst_set = demo.desc_set,
      descriptor_type = 'combined image sampler',
      image_info = imageinfo,
      }}, nil)
end



function prepare_framebuffers(demo)
   local info = {
      render_pass = demo.render_pass,
      width = demo.width,
      height = demo.height,
      layers = 1,
   }

   demo.framebuffers = {}
   for i, buffer in ipairs(demo.buffers) do
      info.attachments = { buffer.view, demo.depth.view }
      demo.framebuffers[i] = vk.create_framebuffer(demo.device, info)
   end
end

function demo_prepare(demo) 
   demo.draw_cmd = vk.allocate_command_buffers(demo.cmd_pool, 'primary', 1)[1]
   prepare_swapchain(demo)
   prepare_depth_buffer(demo)
   prepare_textures(demo)
   prepare_vertices(demo)
   prepare_descriptor_layout(demo)
   prepare_render_pass(demo)
   prepare_pipeline(demo)
   prepare_descriptor_pool(demo)
   prepare_descriptor_set(demo)
   prepare_framebuffers(demo)
end
   

function init_vulkan(demo)
   local instance_validation_layers = nil

   -- Look for validation layers 
   if demo.validate then 
      instance_validation_layers = {
        "VK_LAYER_LUNARG_mem_tracker",
        "VK_LAYER_GOOGLE_unique_objects",
      }

      local instance_layers = vk.enumerate_instance_layer_properties(true)
      for _, name in ipairs(instance_validation_layers) do
         if not instance_layers[name] then
            error("failed to find required validation layer '"..name.."'")
         end
      end
   end

   -- Look for instance extensions required by GLFW
   demo.instance_extensions = glfw.get_required_instance_extensions()
   if demo.validate then table.insert(demo.instance_extensions, 'VK_EXT_debug_report') end

   local instance_extensions = vk.enumerate_instance_extension_properties(nil, true)

   for _, name in ipairs(demo.instance_extensions) do
      if not instance_extensions[name] then
         error("failed to find required instance extension '"..name.."'")
      end
   end

   -- Create instance
   demo.inst = vk.create_instance({
      application_info = {
         application_name = APP_SHORT_NAME,
         application_version = 0,
         engine_name = APP_SHORT_NAME,
         engine_version = 0,
         api_version = vk.API_VERSION_1_0,
      },
      enabled_layer_names = instance_validation_layers,
      enabled_extension_names = demo.instance_extensions,
   })

   -- Enumerate physical devices
   local physical_devices =vk.enumerate_physical_devices(demo.inst)
   assert(#physical_devices > 0, "no physical devices found")
   demo.gpu = physical_devices[1] -- for this demo we just grab the first physical device

   -- Look for device extensions
   demo.device_extensions = {
      'VK_KHR_swapchain' 
   }

   local device_extensions = vk.enumerate_device_extension_properties(demo.gpu, nil, true)
   for _, name in ipairs(demo.device_extensions) do
      if not device_extensions[name] then 
         error("failed to find required device extension '"..name.."'")
      end
   end

   if demo.validate then -- register the debug report callback
      demo.msg_callback = vk.create_debug_report_callback(demo.inst, 
               vk.debugreportflags('error', 'warning'), dbgFunc)
   end
         
   -- Get physical device properties (including queue families info)
   demo.gpu_props = vk.get_physical_device_properties(demo.gpu)

   demo.queue_props = vk.get_physical_device_queue_family_properties(demo.gpu)
   assert(#demo.queue_props >= 1)

   -- Graphics queue and MemMgr queue can be separate.
   -- TODO: Add support for separate queues, including synchronization,
   --       and appropriate tracking for QueueSubmit
end


function init_device(demo) 
   demo.device = vk.create_device(demo.gpu, {
      queue_create_infos = { 
         { queue_family_index = demo.graphics_queue_node_index, queue_priorities = { 0.0 } },
      },
      enabled_extension_names = demo.device_extensions,
   })
end


function init_surface(demo)
   -- Create a WSI surface for the window:
   local surfaceRAW = glfw.create_window_surface(demo.window, demo.inst:raw())
   demo.surface = vk.created_surface(demo.inst, surfaceRAW)

   -- Iterate over each queue family to learn whether it supports presenting:
   local supportsPresent = { }
   for i = 1, #demo.queue_props do
      supportsPresent[i] = vk.get_physical_device_surface_support(demo.gpu, i-1, demo.surface)
   end

   -- Search for a graphics and a present queue family, try to find one that supports both
   local graphicsQueueNodeIndex, presentQueueNodeIndex

   for i, props in ipairs(demo.queue_props) do
      if (props.queue_flags & vk.QUEUE_GRAPHICS_BIT) ~= 0 then
         if supportsPresent[i] then 
            presentQueueNodeIndex = i-1
            graphicsQueueNodeIndex = i-1
            break
         end
         if not graphicsQueueNodeIndex then graphicsQueueNodeIndex = i-1 end
      end
      if not presentQueueNodeIndex then presentQueueNodeIndex = i-1 end
   end      

   -- Generate error if could not find both a graphics and a present queue
   if not graphicsQueueNodeIndex or not presentQueueNodeIndex then
      error("Could not find a graphics and a present queue")
   end

   -- TODO: Add support for separate queues, including presentation,
   --       synchronization, and appropriate tracking for QueueSubmit.
   -- NOTE: While it is possible for an application to use a separate graphics
   --       and a present queues, this demo program assumes it is only using one:
   if graphicsQueueNodeIndex ~= presentQueueNodeIndex then
      error("Could not find a common graphics and a present queue")
   end

   demo.graphics_queue_node_index = graphicsQueueNodeIndex

   init_device(demo)

   demo.queue = vk.get_device_queue(demo.device, demo.graphics_queue_node_index, 0)

   -- Get the list of formats that are supported:
   local surfFormats = vk.get_physical_device_surface_formats(demo.gpu, demo.surface)
   -- If the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has no
   -- preferred format.  Otherwise, at least one supported format will be returned.
   if #surfFormats == 1 and surfFormats[1].format == 'undefined' then
      demo.format = 'b8g8r8a8 unorm'
   else 
      demo.format = surfFormats[1].format
   end
   demo.color_space = surfFormats[1].color_space

   -- Get Memory information and properties
   demo.memory_properties = vk.get_physical_device_memory_properties(demo.gpu)

   demo.cmd_pool = vk.create_command_pool(demo.device, 
      vk.COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, demo.graphics_queue_node_index)
end


function demo_free(demo) 
   for _, framebuffer in ipairs(demo.framebuffers) do
      vk.destroy_framebuffer(framebuffer)
   end

   vk.destroy_descriptor_pool(demo.desc_pool)

   if demo.setup_cmd then vk.free_command_buffers({demo.setup_cmd}) end
   vk.free_command_buffers({demo.draw_cmd})

   vk.destroy_pipeline(demo.pipeline)
   vk.destroy_render_pass(demo.render_pass)
   vk.destroy_pipeline_layout(demo.pipeline_layout)
   vk.destroy_descriptor_set_layout(demo.desc_layout)

   vk.destroy_buffer(demo.vertices.buf)
   vk.free_memory(demo.vertices.mem)

   for _, tex in ipairs(demo.textures) do  
      vk.destroy_image_view(tex.view)
      vk.destroy_image(tex.image)
      vk.free_memory(tex.mem)
      vk.destroy_sampler(tex.sampler)
   end

   for _, buffer in ipairs(demo.buffers) do
      vk.destroy_image_view(buffer.view)
   end

   vk.destroy_image_view(demo.depth.view)
   vk.destroy_image(demo.depth.image)
   vk.free_memory(demo.depth.mem)
end


function demo_resize(demo) 
   demo_free(demo)
   -- Re-create the swapchain:
   demo_prepare(demo)
end

-- GLFW callbacks
function demo_key_callback(win, key, scancode, action) 
   if key == 'escape' and action == 'release' then
      glfw.set_window_should_close(win, true)
   end
end

function demo_refresh_callback(win, focused) 
   local demo = window_info[win]
   demo_draw(demo)
end

function demo_resize_callback(win, width, height) 
   local demo = window_info[win]
   demo.width = width
   demo.height = height
   demo_resize(demo)
end

-- MAIN ----------------------------------------------------------------------------------
demo = {}

for _, opt in ipairs(arg) do
   if opt == "--use_staging" then 
      demo.use_staging_buffer = true
   elseif opt == "--validate" then 
      demo.validate = true 
   end
end

glfw.set_error_callback(function(code, description) 
    print("GLFW error: " .. description)
end)

if not glfw.vulkan_supported() then error("Vulkan is non supported by MoonGLFW") end

init_vulkan(demo)

demo.width = 300
demo.height = 300
demo.depthStencil = 1.0
demo.depthIncrement = -0.01

-- create window
glfw.window_hint('client api', 'no api')
demo.window = glfw.create_window(demo.width, demo.height, APP_LONG_NAME)

window_info = {} -- window specific info
window_info[demo.window] = demo -- glfwSetWindowUserPointer(demo->window, demo)

glfw.set_window_refresh_callback(demo.window, demo_refresh_callback)
glfw.set_framebuffer_size_callback(demo.window, demo_resize_callback)
glfw.set_key_callback(demo.window, demo_key_callback)

-- create surface and device
init_surface(demo)
demo_prepare(demo)

collectgarbage()
-- event loop
while not glfw.window_should_close(demo.window) do
   glfw.wait_events_timeout(1/60) -- glfw.poll_events()
   demo_draw(demo)

   if demo.depthStencil > 0.99 then 
      demo.depthIncrement = -0.001
   end
   if demo.depthStencil < 0.8 then
      demo.depthIncrement = 0.001
   end

   demo.depthStencil = demo.depthStencil + demo.depthIncrement

   -- Wait for work to finish before updating MVP.
   vk.device_wait_idle(demo.device)
   collectgarbage('step')
end


