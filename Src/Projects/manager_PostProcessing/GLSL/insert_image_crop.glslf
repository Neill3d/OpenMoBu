if (texCoord.y < upperClip || texCoord.y > lowerClip)
{
	vec4 color = texture(inputSampler, texCoord);
	fragColor = color;
	return;
}