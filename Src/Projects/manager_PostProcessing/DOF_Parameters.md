# Depth of Field — Parameter Reference

Screen-space depth of field post effect. Uses ring-based bokeh sampling driven by scene depth.

---

## Focus Zone

These parameters define where the in-focus region sits in the scene and how wide it is.

| UI Name | Default | Range | Description |
|---|---|---|---|
| **Focal Distance** | 40.0 | 0.0 – far clip | Distance from the camera to the center of the in-focus zone, in world units. Objects at exactly this distance have zero blur. |
| **Focal Range** | 4.0 | 0.01 – ∞ | Half-width of the in-focus zone in world units. Objects within ±FocalRange of FocalDistance are sharp. Increasing this widens the zone of sharpness. |
| **CoC** | 3.0 | 0.1 – 10.0 | Circle of Confusion in units of 0.01 mm (3.0 = 0.03 mm, standard 35 mm film). Scales the effective focus band width. Values above 3.0 widen the in-focus zone (looser tolerance); values below 3.0 narrow it. At 3.0 the effect is neutral — FocalRange is used as-is. |

> **How they interact:** The actual in-focus band = `FocalRange × (CoC / 3.0)`. Blur begins with a smooth ease-in (smoothstep) at the band boundary, so the transition is never a hard cut.

---

## Aperture / Blur Amount

| UI Name | Default | Range | Description |
|---|---|---|---|
| **F-Stop** | 1.4 | 1.0 – 22.0 | Lens aperture expressed as an f-number. f/1.4 is wide open — objects outside the focus zone reach full blur. f/22 is stopped down — blur grows slowly and may never reach maximum. Mirrors real camera f-stop behaviour. |
| **Blur Radius** | 15.0 | 1.0 – 60.0 | Maximum radius of the bokeh disk in pixels, reached at full blur (f/1.4, far outside focus zone). Directly controls how large out-of-focus circles appear. When using camera DOF properties this is computed automatically from the camera's FocusAngle. |

---

## Sample Quality

Higher values produce smoother, rounder bokeh at the cost of GPU time. For preview use low values; for final quality use high.

| UI Name | Default | Range | Notes |
|---|---|---|---|
| **Samples** | 4 | 1 – 8 | Number of samples on the first (innermost) ring. Each outer ring has `ring_index × Samples` taps. |
| **Rings** | 4 | 1 – 12 | Number of concentric sample rings around each pixel. More rings = larger, smoother bokeh disk and finer fill. Minimum recommended for visible pentagon shape: 4. |

> Total sample count = `Samples × Rings × (Rings + 1) / 2`. At Samples=4, Rings=4 that is 40 taps per pixel.

---

## Bokeh Shape

| UI Name | Default | Range | Description |
|---|---|---|---|
| **Pentagon** | Off | On / Off | When enabled, clips the bokeh disk to a five-sided (pentagonal) aperture shape, matching the look of a 5-blade lens. Requires Samples ≥ 4 and Rings ≥ 4 for a clean result. |
| **Pentagon Feather** | 40.0 | 0.0 – 100.0 | Softness of the pentagon edge (maps to 0.0–1.0 internally). Low values give a crisp hard-edged aperture; high values feather the boundary. Around 30–50 is typical. |

---

## Highlights & Chromatic Aberration

Controls the appearance of bright out-of-focus highlights (specular spots, light sources) and lens fringing.

| UI Name | Default | Range | Description |
|---|---|---|---|
| **Highlight Threshold** | 50.0 | 0.0 – 100.0 | Luminance level above which a sample is considered a highlight and receives a brightness boost in the bokeh (maps to 0.0–1.0 internally). Lower values affect more of the image; higher values only affect the brightest spots. |
| **Highlight Gain** | 200.0 | 0.0 – 500.0 | How strongly bright areas are boosted inside the bokeh disk (maps to 0.0–5.0 internally). Simulates film/sensor bloom on specular highlights in defocused regions. |
| **Bokeh Bias** | 50.0 | 0.0 – 100.0 | Weights sample contribution toward outer rings (maps to 0.0–1.0 internally). Higher values brighten the rim of the bokeh disk relative to its center, matching real lens behaviour where the aperture ring produces a brighter circle edge. At 0 all rings are equal weight. |
| **Bokeh Fringe** | 70.0 | 0.0 – 100.0 | Chromatic aberration amount on each bokeh sample (maps to 0.0–1.0 internally). Each color channel is sampled from a slightly offset position, spreading R, G, B into the characteristic colored fringe seen on real lenses. At 0 there is no color separation. |

---

## Foreground Blur

| UI Name | Default | Description |
|---|---|---|
| **Blur Foreground** | On | When enabled, objects in front of the focal plane (closer to the camera than FocalDistance − FocalRange) are also blurred. When disabled, only the background (objects behind the focus zone) is blurred; foreground remains sharp. |

---

## Auto Focus

Three focus-source modes are available. They are mutually exclusive on the C++ side and resolve in the priority order listed below. **Use Focus Point** is a fourth, independent mechanism that operates in the shader and takes final priority regardless of which C++ mode is active.

| Priority | UI Name | Default | Description |
|---|---|---|---|
| 1 (shader) | **Use Focus Point** | Off | When enabled, the focal distance is determined by sampling the scene depth buffer at a specific screen-space position every frame. The effect continuously re-focuses on whatever geometry is under that point. This overrides any C++-side focal distance, including Camera DOF and Auto Focus. |
| — | **Focus Point** | — | Screen-space UV coordinates (0–100 range, mapped to 0–1 internally) of the screen-space focus target. Only active when Use Focus Point is enabled. |
| 2 (C++) | **Use Camera DOF Properties** | Off | When enabled, FocalDistance is read from the scene camera's built-in focus distance (respecting interest/model targets), and Blur Radius is computed from the camera's FocusAngle. All other parameters still come from the effect properties. Takes precedence over Auto Focus and the static Focal Distance. |
| 3 (C++) | **Auto Focus** | Off | When enabled and a Focus Object is assigned, the focal distance is computed each frame from that object's world-space position projected into camera space. Lets the DOF track any animatable scene object automatically. Overrides the static Focal Distance value; ignored when Use Camera DOF Properties is on. |
| — | **Focus Object** | — | Scene model whose world-space position drives the auto-focus distance. Only used when Auto Focus is on. |
| 4 (C++) | **Focal Distance** (static) | 40.0 | Used as the focal distance when none of the three modes above are active. See Focus Zone section. |

> **Priority example:** if both Use Camera DOF Properties and Use Focus Point are on, the shader reads the depth at the screen-space point and ignores the camera's focus distance entirely.

---

## Masking

| UI Name | Description |
|---|---|
| **Use Masking** | Enables protection of areas from the DOF effect using a mask texture. Masked areas (white in the R channel) are blended back to the original unblurred color. |
| **Masking Channel** | UI property exists but is **not currently honoured by the DOF shader** — the shader always reads the R channel of the mask texture (`mask.r`). Channel selection has no effect on the rendered output. |

---

## Clip Region

Restricts the effect to a horizontal band of the screen. Useful for stylistic effects (tilt-shift look) or to spare UI overlays.

| UI Name | Default | Range | Description |
|---|---|---|---|
| **Upper Clip** | 0.0 | 0.0 – 1.0 | Top edge of the region where the effect is applied, in UV space (0 = top of screen). Pixels above this Y coordinate are passed through unblurred. |
| **Lower Clip** | 0.0 | 0.0 – 1.0 | Bottom edge of the region in UV space (1 = bottom of screen). Pixels below this Y coordinate are passed through unblurred. When both are 0.0 the full screen is affected. |

---

## Noise / Dithering

| UI Name | Default | Description |
|---|---|---|
| **Noise** | On | Adds a per-pixel hash-based offset to each sample position, breaking up the regular ring pattern that becomes visible at low sample counts. Costs nothing (computed analytically per pixel, no texture). Recommended to leave on. |

---

## Debug

| UI Name | Default | Description |
|---|---|---|
| **Debug Blur Value** | Off | Replaces the output with a grayscale visualization of the computed blur amount. White = maximum blur (fully out of focus), black = in focus. Useful for diagnosing FocalDistance, FocalRange, CoC, and F-Stop settings before enabling sampling. |
| **Debug Show Focus** | Off | Overlays a tint on the image: orange tint = in-focus or near-focus region, blue tint = transition zone. Does not disable the DOF effect, so it can be used while reviewing the blurred result. |

---

## Typical Workflows

**Portrait / shallow depth of field**
Set FocalDistance to the subject's distance, FocalRange to 1–3, F-Stop to 1.4–2.8, Blur Radius to 20–30. Enable Blur Foreground if the subject has objects in front of them.

**Cinematic background blur only**
Disable Blur Foreground. Set FocalDistance to the nearest hero object, FocalRange to cover everything you want sharp, F-Stop to 2.8–5.6.

**Tilt-shift / miniature look**
Set Upper Clip and Lower Clip to restrict blur to a horizontal band (e.g., Upper=0.1, Lower=0.7). Use a high Blur Radius (30+) and low F-Stop.

**Performance**
Samples=3, Rings=3 for preview. Samples=4, Rings=4 is a good balance. Samples=5+, Rings=8+ for final quality.
