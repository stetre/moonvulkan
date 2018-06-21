## MoonVulkan: Lua bindings for Vulkan

MoonVulkan is a Lua binding library for the [Khronos Vulkan&#8482; API](https://www.khronos.org/vulkan).

It runs on GNU/Linux <!-- and on Windows (MSYS2/MinGW) --> and requires 
[Lua](http://www.lua.org/) (>=5.3) and Vulkan (>= 1.0).



_Author:_ _[Stefano Trettel](https://www.linkedin.com/in/stetre)_

[![Lua logo](./doc/powered-by-lua.gif)](http://www.lua.org/)

#### License

MIT/X11 license (same as Lua). See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://stetre.github.io/moonvulkan/doc/index.html).

#### Getting and installing

Setup the build environment as described [here](https://github.com/stetre/moonlibs), then:

```sh
$ git clone https://github.com/stetre/moonvulkan
$ cd moonvulkan
moonvulkan$ make
moonvulkan$ sudo make install
```

To use MoonVulkan, you'll also need at least one 
[Vulkan capable device](http://vulkan.gpuinfo.org/) with
[updated drivers](https://www.howtogeek.com/242045/how-to-get-the-latest-nvidia-amd-or-intel-graphics-drivers-on-ubuntu/) 
and the 
[Vulkan loader](https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/master/loader/LoaderAndLayerInterface.md).

To install the **Vulkan loader** you can either install the latest version of the 
the [LunarG VulkanSDK](https://vulkan.lunarg.com/sdk/home) (preferred, since it is
frequently updated and it comes with validation layers, glslangValidator, etc), or
you can install the loader that comes with your Linux distro, e.g. on Ubuntu:

```sh
$ sudo apt install libvulkan-dev
```

Note that MoonVulkan does not link directly to the Vulkan loader (`libvulkan.so` on Linux), 
but it builds dynamically its own internal dispatch tables instead. 
As a consequence, `libvulkan.so` is not needed at compile time but it is required 
at runtime, so you have to make sure that it is reachable by the linker. 
You can do this by installing it in the standard search directories (e.g. `/usr/lib/`),
or by properly setting the LD_LIBRARY_PATH environment variable in the shell where you execute
the Lua scripts. 

For example, assuming you are using the VulkanSDK version 1.1.77.0:
```sh
$ export LD_LIBRARY_PATH=<path-to-vulkan-sdk>/1.1.77.0/x86_64/lib
$ lua -e "require('moonvulkan')"     # just tests if it works
```
 

#### Example

The example below creates a Vulkan instance, enumerates the available GPUs, selects
the first one (if any), retrieves its properties and prints a few of them.

Other examples can be found in the **examples/** directory contained in the release package.

```lua
-- MoonVulkan example: hello.lua

vk = require("moonvulkan")

-- Create a Vulkan instance:
instance = vk.create_instance({
   application_info = {
      application_name = 'Hello',
      application_version = 1,
      api_version = vk.make_version(1,0,0)
   },
   enabled_layer_names = { 
      'VK_LAYER_LUNARG_standard_validation', 
   -- 'VK_LAYER_LUNARG_api_dump', -- uncomment to see API call dumps
   }
})

-- Enumerate physical devices:
physdevs = vk.enumerate_physical_devices(instance)
print("Number of available physical devices: " ..#physdevs)
assert(#physdevs > 0)

-- Select the first device:
gpu = physdevs[1]

-- Get its properties, and print a few:
props = vk.get_physical_device_properties(gpu)
print("Device name: ".. props.device_name)
print("Device type: ".. props.device_type)
print("Driver version: ".. vk.version_string(props.driver_version))
print("API version: ".. vk.version_string(props.api_version))
-- ...

-- Note: objects are automatically destroyed at exit so there is no need for cleanup
```

The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ lua hello.lua
```

#### See also

* [MoonLibs - Graphics and Audio Lua Libraries](https://github.com/stetre/moonlibs).
