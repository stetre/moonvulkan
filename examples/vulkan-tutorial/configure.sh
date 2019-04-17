# Runtime environment configuration for MoonVulkan.
#
# This script sets a few environment variables needed to use MoonVulkan
# in conjunction with the LunarG Vulkan SDK.
#
# You may use it in a shell, like this:
# $ . configure.sh
# or you may want to copy its contents in your .bash_profile.
#
# In order to use it, you'll need to change the following two definitions 
# according to where the SDK is installed on your system, and to the SDK 
# version you are using:
VULKAN_SDK_BASE=$HOME/vulkan
VULKAN_SDK_VERSION=1.1.106.0
VULKAN_SDK=$VULKAN_SDK_BASE/$VULKAN_SDK_VERSION/x86_64

#----------------------------------------------------------------------
# Add the path to VulkanSDK binaries to PATH.
# This is needed, for example, if you want to execute the glslangValidator
# from Lua scripts to compile GLSL shaders to SPIR-V.
TMP=$VULKAN_SDK/bin
case :$PATH: in
 *$TMP*) ;; # already in
 *) export PATH=$PATH:$TMP:;;
esac

# Add the path to Vulkan SDK shared objects to LD_LIBRARY_PATH.
# This is needed by the linker in order to find libvulkan.so.
TMP=$VULKAN_SDK/lib
case :$LD_LIBRARY_PATH: in
 *$TMP*) ;; # already in
 *) export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TMP:;;
esac

# Add the path to the validation layers json descriptions VK_LAYER_PATH.
# This is needed by the Vulkan loader in order to find which validation
# layers are available on your system.
TMP=$VULKAN_SDK/etc/explicit_layer.d
case :$VK_LAYER_PATH: in
 *$TMP*) ;; # already in
 *) export VK_LAYER_PATH=$VK_LAYER_PATH:$TMP:;;
esac

# Note: To activate layers from outside the application, add their names 
# to the VK_INSTANCE_LAYERS environment variable, separated by a colon (':'). 
#
# For example, to activate the api_dump and core_validation layers:
# $ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation
#
