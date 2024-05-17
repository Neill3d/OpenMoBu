
uniform	vec4			solidColor;
uniform	vec4			bottomColor;
uniform float			gradient;

#ifdef USE_MASK
	uniform sampler2D		maskSampler;
	uniform float			maskInverse;
	
	bool CheckForTexelReject(vec2 incoords)
	{
		float maskValue = abs(maskInverse - texture2D( maskSampler, incoords ).r);
		return (maskValue == 0.0);
	}
	vec4 ApplyMask(vec2 incoords, vec4 srccolor, vec4 dstcolor)
	{
		float maskValue = abs(maskInverse - texture2D( maskSampler, incoords ).r);
		return mix(srccolor, dstcolor, maskValue);
	}
#else
	bool CheckForTexelReject(vec2 incoords)
	{
		return false;
	}
	vec4 ApplyMask(vec2 incoords, vec4 srccolor, vec4 dstcolor)
	{
		return dstcolor;
	}
#endif

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	gl_FragColor = ApplyMask(tx, vec4(solidColor.rgb, 0.0), solidColor);
}
