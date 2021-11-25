/* The MIT License (MIT)
 *
 * Copyright (c) 2017 Stefano Trettel
 *
 * Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

lua_State *moonvulkan_L = NULL;

static void AtExit(void)
    {
    if(moonvulkan_L)
        {
        enums_free_all(moonvulkan_L);
        moonvulkan_atexit_getproc();
        moonvulkan_L = NULL;
        }
    }

int luaopen_moonvulkan(lua_State *L)
/* Lua calls this function to load the module */
    {
    moonvulkan_L = L;

    moonvulkan_utils_init(L);
    atexit(AtExit);

    lua_newtable(L); /* the vk table */

    /* add vk functions: */
    moonvulkan_open_getproc(L);
    moonvulkan_open_versions(L);
    moonvulkan_open_tracing(L);
    moonvulkan_open_datahandling(L);
    moonvulkan_open_enums(L);
    moonvulkan_open_flags(L);
    moonvulkan_open_instance(L);
    moonvulkan_open_physical_device(L);
    moonvulkan_open_layers(L);
    moonvulkan_open_device(L);
    moonvulkan_open_queue(L);
    moonvulkan_open_command_pool(L);
    moonvulkan_open_command_buffer(L);
    moonvulkan_open_semaphore(L);
    moonvulkan_open_fence(L);
    moonvulkan_open_buffer(L);
    moonvulkan_open_device_memory(L);
    moonvulkan_open_image(L);
    moonvulkan_open_event(L);
    moonvulkan_open_buffer_view(L);
    moonvulkan_open_image_view(L);
    moonvulkan_open_shader_module(L);
    moonvulkan_open_sampler(L);
    moonvulkan_open_render_pass(L);
    moonvulkan_open_framebuffer(L);
    moonvulkan_open_descriptor_set_layout(L);
    moonvulkan_open_descriptor_pool(L);
    moonvulkan_open_pipeline_layout(L);
    moonvulkan_open_pipeline_cache(L);
    moonvulkan_open_query_pool(L);
    moonvulkan_open_descriptor_set(L);
    moonvulkan_open_pipeline(L);
    moonvulkan_open_cmd(L);
    moonvulkan_open_surface(L);
    moonvulkan_open_swapchain(L);
    moonvulkan_open_debug_report_callback(L);
    moonvulkan_open_display(L);
    moonvulkan_open_display_mode(L);
    moonvulkan_open_descriptor_update_template(L);
    moonvulkan_open_validation_cache(L);
    moonvulkan_open_sampler_ycbcr_conversion(L);
    moonvulkan_open_debug_utils_messenger(L);

    /* Add functions implemented in Lua */
    lua_pushvalue(L, -1); lua_setglobal(L, "moonvulkan");
    if(luaL_dostring(L, "require('moonvulkan.constructors')") != 0) lua_error(L);
    lua_pushnil(L);  lua_setglobal(L, "moonvulkan");

    return 1;
    }

