
// thanks for Christian Pötzsch
// and his great article - https://www.imgtec.com/blog/speeding-up-gpu-barrel-distortion-correction-in-mobile-vr/

#version 120

uniform vec2 centre;
uniform sampler2D texSampler;

void main(void)
{
	vec2 texFrg = gl_TexCoord[0].st;

	vec4 col = vec4(0.0, 0.0, 0.0, 1.0); /* base colour */
	float alpha = 0.2; /* lens parameter */

	/* Left/Right eye are slightly off centre */
	/* Normalize to [-1, 1] and put the centre to "centre" */
	vec2 p1 = vec2(2.0 * texFrg - 1.0) - centre;
 
	/* Transform */
	vec2 p2 = p1 / (1.0 - alpha * length(p1));
 
	/* Back to [0, 1] */
	p2 = (p2 + centre + 1.0) * 0.5;
 
	if (all(greaterThanEqual(p2, vec2(0.0))) &&
		all(lessThanEqual(p2, vec2(1.0))))
	{
		col = texture2D(texSampler, p2);
	}
 
	gl_FragColor = col;
}