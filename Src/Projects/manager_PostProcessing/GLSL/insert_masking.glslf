if (useMasking > 0.0)
{
	vec4 mask = texture2D(maskSampler, texCoord );
	vec4 inputColor = texture2D(inputSampler, texCoord);

	fragColor.rgb = mix(fragColor.rgb, inputColor.rgb, mask.r * useMasking);
}	