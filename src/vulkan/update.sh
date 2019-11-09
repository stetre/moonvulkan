ver=1.1.126.0
fromdir=/home/ste/vulkan/$ver/x86_64/include/vulkan
todir=.

cp -fpv $fromdir/vk_platform.h $todir
cp -fpv $fromdir/vulkan_android.h $todir
cp -fpv $fromdir/vulkan_core.h $todir
cp -fpv $fromdir/vulkan_fuchsia.h $todir
cp -fpv $fromdir/vulkan_ggp.h $todir
cp -fpv $fromdir/vulkan.h $todir
cp -fpv $fromdir/vulkan_ios.h $todir
cp -fpv $fromdir/vulkan_macos.h $todir
cp -fpv $fromdir/vulkan_metal.h $todir
#cp -fpv $fromdir/vulkan_mir.h $todir
cp -fpv $fromdir/vulkan_vi.h $todir
cp -fpv $fromdir/vulkan_wayland.h $todir
cp -fpv $fromdir/vulkan_win32.h $todir
cp -fpv $fromdir/vulkan_xcb.h $todir
cp -fpv $fromdir/vulkan_xlib.h $todir
cp -fpv $fromdir/vulkan_xlib_xrandr.h $todir

