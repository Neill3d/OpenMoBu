# OpenMoBu Post Processing — System Uniforms Reference

A guide for writing shaders for the **User Effect** and the **Texture User Effect**.

---

## How system uniforms work

- A **system uniform** is a uniform with one of the reserved names below. The plugin fills it automatically for every pass, and it does **not** become a property in the UI.
- Declare it with **exactly** the name and type in the tables. If the type doesn't match, the uniform is skipped: it keeps the shader's default value, and the effect Status shows a warning. A `bool` is accepted where a `float` is expected.
- A system uniform that the shader declares but never uses is stripped by the compiler. That's fine: the plugin only fills uniforms that are active.
- Everything else you declare becomes a user property (see [User properties](#user-properties-not-system-uniforms)).
- Matrices are OpenGL column-major, so use them like `projection * modelView * vec4(p, 1.0)`.

### Shader skeleton

The default vertex shader (`GLSL/simple130.glslv`) draws a full-screen quad and passes `texCoord` in the 0..1 range.

```glsl
#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D inputSampler;   // system: the effect input image
uniform vec2      texelSize;      // system: 1 / render target size
uniform float     iTime;          // system: seconds

uniform float     strength_slider; // user property: a 0..100 slider

void main(void)
{
    vec4 color = texture(inputSampler, texCoord);
    FragColor = vec4(color.rgb * (0.01 * strength_slider), color.a);
}
```

Optional text insertions are lines that the loader replaces with a snippet from the `GLSL` folder. The whole line is replaced, so anything after the keyword on that line is dropped. Line endings (LF or CRLF) don't matter:
- `// INSERT: HEADER` inserts `insert_header.glslf`: `#version 140`, `texCoord`, `fragColor`, `inputSampler`, `maskSampler`, `useMasking`, `upperClip`, `lowerClip`.
- `// INSERT: APPLY_IMAGE_CROP` inserts `insert_image_crop.glslf`: it returns the input color outside the clip range.
- `// INSERT: APPLY_MASKING` inserts `insert_masking.glslf`: it mixes the result back to the input by the mask.

### ShaderToy compatibility

With the effect's **ShaderToy Compatibility** property on, the fragment shader is wrapped:
- **Before your code:** the header declares `#version 140`, `uniform vec2 iResolution`, `uniform float iTime`, `uniform sampler2D iChannel0`, `in vec2 texCoord` and `out vec4 FragColor`.
- **After your code:** `main()` calls `mainImage(FragColor, texCoord * iResolution)`.

So you write only `void mainImage(out vec4 fragColor, in vec2 fragCoord)` and use `iResolution`, `iTime` and `iChannel0` as in ShaderToy. Note that `iResolution` is a `vec2` here, not a `vec3` as in ShaderToy.

---

## Input textures (`sampler2D`)

| Name | Slot | Content | Notes |
|---|---|---|---|
| `inputSampler` | 0 | The effect input image (the result of the previous effect in the chain, or the scene). | |
| `iChannel0` | 0 | Same as `inputSampler`. | ShaderToy naming. |
| `depthSampler` | 1 | Scene depth buffer, non-linear (window-space depth, 0..1). | Bound only when an effect in the chain uses it. |
| `linearDepthSampler` | 2 | Linear depth: positive camera-space distance in scene units (`.r`). | Produced only when an effect in the chain uses it. It can be half resolution. |
| `normalSampler` | 3 | World-space normals. | **Not produced yet**, so don't rely on it. |
| `maskSampler` | 4 | The mask channel selected for the effect; the mask value is in `.r`. | Use it together with `useMasking`. |

In the **Texture User Effect** there is no scene render pass, so `depthSampler`, `linearDepthSampler`, `normalSampler` and `maskSampler` aren't available. The texture reports them as unsupported.

The Texture User Effect also works without an input layer. Its first layer is the `inputSampler` image. With no layer connected, `inputSampler` is a blank image filled with the texture's **Background Color**, and the texture size is **Blank Width** × **Blank Height** (1024 × 1024 by default).

---

## Masking and clipping (`float`)

These come from the effect's masking and clipping settings.

| Name | Range | Meaning |
|---|---|---|
| `useMasking` | 0 or 1 | 1 when the effect's **Use Masking** is on, or when the post-processing settings have **Enable Masking For All Effects** on. |
| `upperClip` | 0..1 | Lower edge of the processed band, in `texCoord.y` space. |
| `lowerClip` | 0..1 | Upper edge of the processed band, in `texCoord.y` space. |

Typical use:

```glsl
if (texCoord.y < upperClip || texCoord.y > lowerClip)
{
    FragColor = texture(inputSampler, texCoord); // outside the band: pass the input through
    return;
}
...
if (useMasking > 0.0)
{
    float mask = texture(maskSampler, texCoord).r;
    FragColor.rgb = mix(FragColor.rgb, texture(inputSampler, texCoord).rgb, mask * useMasking);
}
```

- For built-in effects, the clip values come from the post-processing settings: `upperClip` = **Bottom Clip Percent** / 100, and `lowerClip` = 1 − **Top Clip Percent** / 100.
- User effects have no clip properties of their own, so they get `upperClip = 0` and `lowerClip = 1`, which means no clipping.

---

## Resolution (`vec2`)

| Name | Value |
|---|---|
| `gResolution` | Render target size in pixels, for example `(1920, 1080)`. |
| `iResolution` | Same as `gResolution` (ShaderToy naming, `vec2`). |
| `uInvResolution` | `1.0 / resolution`. |
| `texelSize` | `1.0 / resolution`. |

These describe the **render target** of the effect. That is smaller than the viewport when the effect runs downscaled (the user effect **Resolution** property set to 1/2 or 1/4) or renders into an internal buffer. For sampling neighbouring pixels of the input, use `texelSize`.

---

## Time (`float`, `vec4`)

| Name | Type | Value |
|---|---|---|
| `iTime` | `float` | System time in seconds. In the effects chain it counts from the moment the post-processing context started; in the Texture User Effect it is MotionBuilder's system time. **Not** the timeline time: it keeps running when playback is stopped. |
| `iDate` | `vec4` | Local date and time: `(year, month 1..12, day 1..31, seconds since midnight)`. |

---

## Camera

All values are for the camera of the processed view, evaluated for the current frame. In a file render, every frame is evaluated right before it is rendered.

### Position and matrices

| Name | Type | Value |
|---|---|---|
| `cameraPosition` | `vec3` | World-space camera position. |
| `modelView` | `mat4` | Camera view matrix (world → camera space; the camera looks down −Z). |
| `projection` | `mat4` | Camera projection matrix. |
| `modelViewProj` | `mat4` | `projection * modelView`. |
| `invModelViewProj` | `mat4` | Inverse of `modelViewProj` (clip space → world space). |
| `prevModelViewProj` | `mat4` | `modelViewProj` of the previous frame (motion vectors). |
| `zNear` | `float` | Camera **Near Plane** distance, in scene units. |
| `zFar` | `float` | Camera **Far Plane** distance, in scene units. |
| `cameraIsOrtho` | `float` | 1 for an orthogonal camera, 0 for a perspective one. |

### Lens and focus

| Name | Type | Value |
|---|---|---|
| `cameraFocusDistance` | `float` | Focus distance in scene units, resolved from the camera's **Focus Distance Source**: **Camera Interest** gives the view depth of the interest, **Focus Model** gives the view depth of that model, otherwise **Focus Specific Distance**. It's the same value DOF uses with *Use Camera DOF Properties*. |
| `cameraFocusAngle` | `float` | Camera **Focus Angle**, in degrees. |
| `cameraFocalLength` | `float` | Camera **Focal Length**, in mm. |
| `cameraFov` | `float` | Camera **Field Of View** property, in degrees. Whether it is horizontal or vertical follows the camera's aperture mode. |
| `cameraFocalLengthPx` | `float` | Focal length in pixels of the **viewport height** (`0.5 * viewHeight * projection[1][1]`), 0 for an orthogonal camera. |

### Without a scene camera

The **Texture User Effect** has no scene camera. It gets identity matrices, a camera position at the origin, `zNear = 1` and `zFar = 1000`, and all lens values set to 0.

### Examples

```glsl
uniform sampler2D depthSampler;
uniform sampler2D linearDepthSampler;
uniform mat4  invModelViewProj;
uniform float zNear, zFar;
uniform float cameraFocusDistance;

// world position of a pixel from the depth buffer
vec3 WorldPosition(vec2 uv)
{
    float d = texture(depthSampler, uv).r;
    vec4 clip = vec4(uv * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
    vec4 world = invModelViewProj * clip;
    return world.xyz / world.w;
}

// linear depth from the depth buffer (perspective camera), the same value as linearDepthSampler
float LinearDepth(vec2 uv)
{
    float d = texture(depthSampler, uv).r;
    return zNear * zFar / (zFar - d * (zFar - zNear));
}

// 0 at the focus distance, growing with the distance from it
float DefocusAmount(vec2 uv)
{
    return abs(texture(linearDepthSampler, uv).r - cameraFocusDistance) / max(cameraFocusDistance, 0.0001);
}
```

---

## User properties (not system uniforms)

Any other active uniform becomes a property of the user effect object.

- **Supported types:** `float`, `int`, `bool`, `vec2`, `vec3`, `vec4`, `sampler2D`. Other types (`samplerCube`, `ivec*`, `mat3`, `mat4`, …) and arrays are skipped with a warning in Status.
- **Name postfixes**, which are removed from the property name:

  | Postfix | Uniform type | Property |
  |---|---|---|
  | `_slider` | `float` | Number with a 0..100 range. |
  | `_flag` | `float` or `bool` | Checkbox. A `float` uniform still receives 0.0 or 1.0. |
  | `_color` | `vec3` / `vec4` | RGB / RGBA color picker. |
  | `_wstoss` | `vec2` | A world-space `vec3` property, converted into a screen-space `vec2`. |

  Any `bool` uniform is a checkbox, with or without `_flag`.
- **`sampler2D`** is an object property: connect a texture, or another user effect to use its result.
- **`vec3 <name>_object`** is an object slot: connect a scene object (see [Object slots](#object-slots)).
- **A uniform that disappears** (removed, or stripped by the compiler because it's unused) keeps its property, together with its value and animation. Status shows the number of unused properties, and **Remove Unused Properties** deletes them.
- **Names are reserved:** a uniform can't have the name of one of the effect object's own properties (for example `Active` or `Resolution`). Such a uniform is skipped with a warning.

---

## Object slots

An object slot lets a shader follow a scene object, such as a light, a null or a mesh, the way the lens flare follows its **Flare Light**.

```glsl
uniform vec3  sun_object;            // world position of the connected object -> creates the "sun" slot
uniform vec3  sun_object_color;      // light colour, white for a non-light
uniform float sun_object_intensity;  // light intensity, 1.0 for MotionBuilder's default 100
uniform vec3  sun_object_screen;     // (u, v, view depth) of the object on screen
uniform float sun_object_connected;  // 1 when an object is connected, 0 otherwise
```

- **The slot.** `uniform vec3 <name>_object;` creates one property, **<name>**, on the user effect. It's an object slot that takes a single model: drag a light, a null or any other model onto it from the Navigator or the Schematic view.
- **Companion uniforms.** Uniforms named `<name>_object_<field>` don't create properties of their own; they are filled from the slot **<name>**. Declare only the ones the shader needs. The slot is created even when the shader uses only companions.

  | Uniform | Type | Value |
  |---|---|---|
  | `<name>_object` | `vec3` | World position of the object. |
  | `<name>_object_color` | `vec3` | The light's **Diffuse Color**; white for a non-light. |
  | `<name>_object_intensity` | `float` | The light's **Intensity** / 100, so the default light gives 1.0; 1.0 for a non-light. |
  | `<name>_object_screen` | `vec3` | Screen position `(u, v)` in 0..1 and view depth in scene units. The depth is negative when the object is behind the camera. |
  | `<name>_object_connected` | `float` or `bool` | 1 when an object is connected, 0 otherwise. |

- **Nothing connected:** position, screen and connected are 0, while colour is white and intensity is 1. Multiply by `<name>_object_connected` to fade the effect out.
- **Every frame:** the values are evaluated for the current frame, so an animated object is followed without delay, including in a file render.
- **Wrong type:** a companion declared with the wrong type (for example `vec4 <name>_object_color`) is skipped with a warning in Status.
- **Unknown fields:** an unknown field after `_object_` (for example `float my_object_scale`) is an ordinary property.
- **Several objects:** use one slot per object (`sun_object`, `moon_object`, …); arrays aren't supported.
- **Texture User Effect:** there is no camera, so `<name>_object_screen` is always 0. The other values work.
- **Removed uniforms:** like other properties, a slot whose uniforms are removed keeps its connection and counts as unused, until you press **Remove Unused Properties**.

Example (a glow at the object on screen):

```glsl
uniform sampler2D inputSampler;
uniform vec2  gResolution;
uniform vec3  sun_object_screen;
uniform vec3  sun_object_color;
uniform float sun_object_intensity;
uniform float sun_object_connected;

void main(void)
{
    vec4 color = texture(inputSampler, texCoord);
    vec2 delta = (texCoord - sun_object_screen.xy) * vec2(gResolution.x / gResolution.y, 1.0);
    float glow = 1.0 - smoothstep(0.0, 0.2, length(delta));
    float visible = sun_object_connected * step(0.0, sun_object_screen.z);
    FragColor = vec4(color.rgb + sun_object_color * sun_object_intensity * glow * visible, color.a);
}
```
