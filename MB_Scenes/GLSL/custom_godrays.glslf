
// screen space light god rays

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D iChannel0;  // Rendered scene or brightness texture
uniform vec2 lightPosition_wstoss;      // Light position in screen space (0.0 to 1.0)
uniform float exposure;          // Strength of the god rays effect (default 0.3)
uniform int numSamples;          // Number of samples for raymarching (default 100)
uniform float decay;             // Decay factor for ray brightness (default 0.9)
uniform float weight;            // Weight of each sample (default 0.4)

void main()
{
    // Direction from the fragment to the light source
    vec2 delta = lightPosition_wstoss - texCoord;
    delta /= float(numSamples); // Step size for raymarching

    vec4 color = vec4(0.0);
    vec2 currentTexCoord = texCoord;

    // Accumulate light samples along the ray direction
    for (int i = 0; i < numSamples; i++) {
        currentTexCoord += delta;
        vec4 sample = texture(iChannel0, currentTexCoord);
        sample *= weight;
        sample *= pow(decay, float(i));
        color += sample;
    }

    // Combine with the original scene
    vec4 original = texture(iChannel0, texCoord);
    FragColor = original + color * exposure;
}