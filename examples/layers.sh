# Simple script to enable/disable a list of validation layers from
# outside a Vulkan application.
# 
# Note: In order for this to work, the VK_LAYER_PATH must be properly
# set to include the path to the validation layers JSON descriptions
# (see configure.sh).
#
# Usage: 
# $ . layers.sh on       # enables the validation layers
# $ . layers.sh off      # disables the validation layers
#
# Set the LAYERS variable with the colon (':') separated list of layers 
# you want to enable when executing the script with the 'on' option:
LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation

progname=${0##*/}

function Usage()
    {
    printf "\n"
    printf "Usage: . $progname on/off\n"
    printf "\n"
    }

function On()
    {
    export VK_INSTANCE_LAYERS=$LAYERS
    echo "VK_INSTANCE_LAYERS=$VK_INSTANCE_LAYERS"
    }

function Off()
    {
    unset VK_INSTANCE_LAYERS
    echo "VK_INSTANCE_LAYERS=$VK_INSTANCE_LAYERS"
    }

case $1 in
    on) On;;
    off) Off;;
    *) Usage;;
esac

