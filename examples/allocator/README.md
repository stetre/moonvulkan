
## MoonVulkan custom allocator example

This example shows how to use a custom memory allocator for Vulkan objects in MoonVulkan.

The allocator is implemented as a typical Lua C module (the code is in **main.c**). 
The module provides a _get_(&nbsp;) function that returns the _allocatorLUD_, and 
a _trace_(&nbsp;) function to enable/disable tracing of allocator calls (traces
are simply printfs on stdout).

To compile it:

```sh
$ make
```

This will generate the module library **allocator.so**.

The **test.lua** example shows how to use of the custom allocator. It just creates an
instance and a window + surface for it, using the custom allocator both for the instance 
and for the surface, with tracing enabled.

To run the example, set LD_LIBRARY_PATH with the path to the Vulkan loader, and run:

```sh
$ ./test.lua
```

(You may also want to enable Vulkan validation layers to see that the allocator is
actually used in API calls.)

