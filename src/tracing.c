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
    
static int Type(lua_State *L)
    {
#define TRY(xxx) do { if(test##xxx(L, 1, NULL) != 0) { lua_pushstring(L, ""#xxx); return 1; } } while(0)
    TRY(instance);
    TRY(physical_device);
    TRY(device);
    TRY(queue);
    TRY(command_pool);
    TRY(command_buffer);
    TRY(semaphore);
    TRY(fence);
    TRY(buffer);
    TRY(device_memory);
    TRY(image);
    TRY(event);
    TRY(buffer_view);
    TRY(image_view);
    TRY(shader_module);
    TRY(sampler);
    TRY(render_pass);
    TRY(framebuffer);
    TRY(descriptor_set_layout);
    TRY(descriptor_pool);
    TRY(pipeline_layout);
    TRY(pipeline_cache);
    TRY(query_pool);
    TRY(descriptor_set);
    TRY(pipeline);
    TRY(debug_report_callback);
    TRY(display);
    TRY(display_mode);
    TRY(descriptor_update_template);
    TRY(validation_cache);
    TRY(sampler_ycbcr_conversion);
    TRY(debug_utils_messenger);
    return 0;
#undef TRY
    }

int trace_objects = 0;

static int TraceObjects(lua_State *L)
    {
    trace_objects = checkboolean(L, 1);
    return 0;
    }

static int Now(lua_State *L)
    {
    lua_pushnumber(L, now());
    return 1;
    }

static int Since(lua_State *L)
    {
    double t = luaL_checknumber(L, 1);
    lua_pushnumber(L, since(t));
    return 1;
    }

/* ----------------------------------------------------------------------- */

static const struct luaL_Reg Functions[] = 
    {
        { "type", Type },
        { "trace_objects", TraceObjects },
        { "now", Now },
        { "since", Since },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_tracing(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }


