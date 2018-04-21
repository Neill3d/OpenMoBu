/*
 * Copyright 2016 Mario Guggenberger <mg@protyposis.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Immersive 360 degree image shader
// Maps a equirectangular/spherical texture to a sphere
// Supports 3D VR modes: side by side, top and bottom
//
// This is basically a simple ray tracer modeled after the following guide:
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection

//precision highp float;

#version 120

uniform sampler2D s_Texture;
uniform vec2 u_TextureSize;
uniform mat4 rotation;
uniform int mode;
varying vec2 v_TextureCoord;

#define PIP2    1.5707963 // PI/2
#define PI      3.1415629
#define TWOPI   6.2831853 // 2PI

#define MODE_MONO 0
#define MODE_STEREO_SBS 1 // side by side
#define MODE_STEREO_TAB 2 // top and bottom

vec4 trace(in vec2 p, in vec4 uv_rect)
{
    vec3 D = normalize(vec3(p, 1.0)); // ray direction D

    // calculate hit point of ray on sphere
    vec3 sp = (rotation * vec4(-D, 1.0)).xyz;
	
    // calculate texture mapping for hit point
    float phi = atan(sp.z, sp.x);
    float theta = acos(sp.y);

    // Spherical mapping from sphere to texture
    float u = 0.5 - (phi + PI) / TWOPI + 0.25;
    float v = (theta + PIP2) / PI - 0.5;

    // transform and clamp to requested uv (sub)section
    // (mod simulates texture wrap mode GL_REPEAT)
    u = uv_rect.x + mod(u * uv_rect.z, uv_rect.z);
    v = uv_rect.y + mod(v * uv_rect.w, uv_rect.w);

    return texture2D(s_Texture, vec2(u, v));
}

void main (void)
{
    // Scale texture space to (-1,1) in both axes
    vec2 p = -1.0 + 2.0 * v_TextureCoord;

    // Setup uv rect, i.e. a rect (x, y, width, height) that defines a (sub)section of the texture
    vec4 uv_rect = vec4(0.0, 0.0, 1.0, 1.0);
	//int localmode = 1;
    if(mode == MODE_MONO) {
        uv_rect = vec4(0.0, 0.0, 1.0, 1.0); // mono: complete texture
    }
    else if(mode == MODE_STEREO_SBS) {
        if(p.x < 0.0) {
            p.x += 0.5; // left half of screen
            uv_rect = vec4(0.0, 0.0, 0.5, 1.0); // left half of texture
        } else {
            p.x -= 0.5; // right half of screen
            uv_rect = vec4(0.5, 0.0, 0.5, 1.0); // right half of texture
        }
    }
    else if(mode == MODE_STEREO_TAB) {
        if(p.x < 0.0) {
            p.x += 0.5; // left half of screen
            uv_rect = vec4(0.0, 0.0, 1.0, 0.5); // top half of texture
        } else {
            p.x -= 0.5; // right half of screen
            uv_rect = vec4(0.0, 0.5, 1.0, 0.5); // bottom half of texture
        }
    }

    // Scale to aspect ratio
        p.x *= u_TextureSize.x / u_TextureSize.y;
	//vec4 color = texture2D(s_Texture, v_TextureCoord);
    gl_FragColor =  trace(p, uv_rect);
}
