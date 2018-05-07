## Vulkan Tutorial with Lua

This is a Lua port of the C++ example code that comes with
Alexander Overvoorde's invaluable [**Vulkan Tutorial**](https://vulkan-tutorial.com/).

Shaders, textures, and models are taken unchanged from the original ([repository](https://github.com/Overv/VulkanTutorial), [license](https://github.com/Overv/VulkanTutorial#license)).

The examples use the following libraries from the [MoonLibs](https://github.com/stetre/moonlibs) collection:

* [MoonVulkan](https://github.com/stetre/moonvulkan): bindings to the Vulkan API.
* [MoonGLFW](https://github.com/stetre/moonglfw): bindings to GLFW, for window/surface creation and input handling.
* [MoonGLMATH](https://github.com/stetre/moonglmath): graphics math library, for vertex transformations.
* [MoonImage](https://github.com/stetre/moonimage): bindings to stb_image, for image loading.
* [MoonAssimp](https://github.com/stetre/moonassimp): bindings to Assimp, for model loading.


Use the `configure.sh` script to set up the runtime environment (you may need to modify it according
to the VulkanSDK version installed on your system), then run the examples with the standard 
Lua interpreter, e.g.:

```sh
$ . configure.sh
$ lua 15_hello_triangle.lua
```

