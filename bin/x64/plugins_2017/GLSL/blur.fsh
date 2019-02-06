//
// Fragment shader - Blur (Bilateral)
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

const float KERNEL_RADIUS = 3.0;

uniform sampler2D 	sampler0;
uniform sampler2D 	linearDepthSampler;

uniform float 	g_Sharpness;
uniform vec2	g_InvResolutionDirection; // either set x to 1/width or y to 1/height

//-------------------------------------------------------------------------

vec4 BlurFunction(vec2 uv, float r, vec4 center_c, float center_d, inout float w_total)
{
  vec4  c = texture2D( sampler0, uv );
  float d = texture2D( linearDepthSampler, uv).x;
  
  const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
  const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
  
  float ddiff = (d - center_d) * g_Sharpness;
  float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
  w_total += w;

  return c*w;
}

void main (void)
{
	vec2 texCoord = gl_TexCoord [0].st;
	
	vec4  center_c = texture2D( sampler0, texCoord );
	float center_d = texture2D( linearDepthSampler, texCoord).x;
  
	vec4  c_total = center_c;
	float w_total = 1.0;
  
	for (float r = 1.0; r <= KERNEL_RADIUS; ++r)
	{
		vec2 uv = texCoord + g_InvResolutionDirection * r;
		c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
	}

	for (float r = 1.0; r <= KERNEL_RADIUS; ++r)
	{
		vec2 uv = texCoord - g_InvResolutionDirection * r;
		c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
	}

	vec4 out_Color = c_total/w_total;
	
	gl_FragData [0] =  out_Color;
}
