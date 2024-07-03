
//
// Fragment Shader - Depth Of Field
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

// DoF with bokeh GLSL shader v2.4
// by Martins Upitis (martinsh) (devlog-martinsh.blogspot.com)

uniform	sampler2D	colorSampler;
uniform sampler2D	depthSampler;
uniform sampler2D	blurSampler;
uniform sampler2D	maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

uniform float		focalDistance, focalRange;
uniform float		textureWidth, textureHeight;

uniform float 		zNear;
uniform float 		zFar;

#define PI  3.14159265

float width = textureWidth; //texture width
float height = textureHeight; //texture height

vec2 texel = vec2(1.0/width,1.0/height);

//uniform variables from external script

float focalDepth = focalDistance;  //focal distance value in meters, but you may use autofocus option below
float focalLength = focalRange; //focal length in mm
uniform float fstop; // = 0.5; //f-stop value
bool showFocus = false; //show debug focus point and focal range (red = focal point, green = focal range)

uniform float debugBlurValue;

uniform vec4 focusPoint;

/* 
make sure that these two values are the same for your camera, otherwise distances will be wrong.
*/

float znear = zNear; //camera clipping start
float zfar = zFar; //camera clipping end

//------------------------------------------
//user variables

uniform int samples; // = 3; //samples on the first ring
uniform int rings; // = 3; //ring count

uniform float blurForeground;

uniform bool manualdof; // = false; //manual dof calculation
uniform float ndofstart; // = 1.0; //near dof blur start
uniform float ndofdist; // = 2.0; //near dof blur falloff distance
uniform float fdofstart; // = 1.0; //far dof blur start
uniform float fdofdist; // = 3.0; //far dof blur falloff distance

uniform float CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

uniform bool autofocus; // = false; //use autofocus in shader? disable if you use external focalDepth value
uniform vec2 focus; // = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float maxblur = 1.0; //clamp value of max blur (0.0 = no blur,1.0 default)

uniform float threshold; // = 0.5; //highlight threshold;
uniform float gain; // = 2.0; //highlight gain;

uniform float bias; // = 0.5; //bokeh edge bias
uniform float fringe; // = 0.7; //bokeh chromatic aberration/fringing

uniform bool noise; // = true; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

bool depthblur = false; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

/*
next part is experimental
not looking good with small sample and ring count
looks okay starting from samples = 4, rings = 4
*/

uniform bool pentagon; // = false; //use pentagon as bokeh shape?
uniform float feather; // = 0.4; //pentagon shape feather


//------------------------------------------


float penta(vec2 coords) //pentagonal shape
{
	float scale = float(rings) - 1.3;
	vec4  HS0 = vec4( 1.0,         0.0,         0.0,  1.0);
	vec4  HS1 = vec4( 0.309016994, 0.951056516, 0.0,  1.0);
	vec4  HS2 = vec4(-0.809016994, 0.587785252, 0.0,  1.0);
	vec4  HS3 = vec4(-0.809016994,-0.587785252, 0.0,  1.0);
	vec4  HS4 = vec4( 0.309016994,-0.951056516, 0.0,  1.0);
	vec4  HS5 = vec4( 0.0        ,0.0         , 1.0,  1.0);
	
	vec4  one = vec4( 1.0 );
	
	vec4 P = vec4((coords),vec2(scale, scale)); 
	
	vec4 dist = vec4(0.0);
	float inorout = -4.0;
	
	dist.x = dot( P, HS0 );
	dist.y = dot( P, HS1 );
	dist.z = dot( P, HS2 );
	dist.w = dot( P, HS3 );
	
	dist = smoothstep( -feather, feather, dist );
	
	inorout += dot( dist, one );
	
	dist.x = dot( P, HS4 );
	dist.y = HS5.w - abs( P.z );
	
	dist = smoothstep( -feather, feather, dist );
	inorout += dist.x;
	
	return clamp( inorout, 0.0, 1.0 );
}

float bdepth(vec2 coords) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	vec2 offset[9];
	
	vec2 wh = vec2(texel.x, texel.y) * dbsize;
	
	offset[0] = vec2(-wh.x,-wh.y);
	offset[1] = vec2( 0.0, -wh.y);
	offset[2] = vec2( wh.x -wh.y);
	
	offset[3] = vec2(-wh.x,  0.0);
	offset[4] = vec2( 0.0,   0.0);
	offset[5] = vec2( wh.x,  0.0);
	
	offset[6] = vec2(-wh.x, wh.y);
	offset[7] = vec2( 0.0,  wh.y);
	offset[8] = vec2( wh.x, wh.y);
	
	kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
	for( int i=0; i<9; i++ )
	{
		float tmp = texture2D(depthSampler, coords + offset[i]).r;
		d += tmp * kernel[i];
	}
	
	return d;
}


vec3 color(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	col.r = texture2D(colorSampler,coords + vec2(0.0,1.0)*texel*fringe*blur).r;
	col.g = texture2D(colorSampler,coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
	col.b = texture2D(colorSampler,coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
	return col+mix(vec3(0.0),col,thresh*blur);
}

vec2 rand(vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}

vec3 debugFocus(vec3 col, float blur, float depth)
{
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

	return col;
}

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

/*
float LogarithmicDepth()
{
	float d = texture2D(depthSampler, gl_TexCoord[0].st).x;
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	return z;
}

float LinearizeDepth()
{
  float n = zNear; // camera z near
  float f = zFar; // camera z far
  float z = texture2D(depthSampler, gl_TexCoord[0].st).x;
  //return (2.0 * n) / (f + n - z * (f - n));	
  
  //float z_b = texture2D(depthBuffTex, vTexCoord).x;
  float z_b = z;
  float z_n = 2.0 * z_b - 1.0;
  float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
  return z_e;
}
*/
void ComputeDepth(out float depth, in vec2 texCoord)
{
	float d = texture2D(depthSampler, texCoord).x;
	
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	
	float n = zNear;
	float f = zFar;
	float lz = d;
	
	// nvidia depth range doesn't need that !
	lz = 2.0 * d - 1.0; 
	lz = (2.0 * n) / (f + n - lz * (f - n));	
	
	depth = lz;
}

void main() 
{
	//scene depth calculation
	
	vec2 texCoord = gl_TexCoord[0].st;
	
	if (texCoord.y < upperClip || texCoord.y > lowerClip)
	{
		vec4 fragColor = texture2D(colorSampler, texCoord);
		gl_FragColor =  fragColor;
		return;
	}
	
	float depth = 0.0;
	ComputeDepth(depth, texCoord);
	
	/*
	if (logarithmicDepth > 0.0)
	{
		depth = LogarithmicDepth();
	}
	else
	{
		depth = LinearizeDepth();
	}
	*/
	//depth = linearize(texture2D(depthSampler,gl_TexCoord[0].xy).x);
	/*
	if (depthblur)
	{
		depth = linearize(bdepth(gl_TexCoord[0].xy));
	}
	*/
	//focal plane calculation
	
	float fDepth = focalDepth / zFar;
	
	if (focusPoint.w > 0.0)
	{
		ComputeDepth(fDepth, focusPoint.xy);
	}
	
	/*
	if (autofocus)
	{
		fDepth = linearize(texture2D(depthSampler,focus).x);
	}
	*/
	//dof blur factor calculation
	
	float blur = 0.0;
	
	if (manualdof)
	{    
		float a = depth-fDepth; //focal plane
		float b = (a-fdofstart)/fdofdist; //far DoF
		float c = (-a-ndofstart)/ndofdist; //near Dof
		blur = (a>0.0)?b:c;
	}
	else
	{
		float f = focalLength * 0.05; //focal length in mm
		float d = fDepth *1000.0; //focal plane in mm
		float o = depth *1000.0; //depth in mm
		
		float a = (o*f)/(o-f); 
		float b = (d*f)/(d-f); 
		float c = (d-f)/(d*fstop*CoC); 
		
		blur = 10.0 * abs(a-b)*c;
		
		a = depth - fDepth; // focal plane
		if (blurForeground == 0.0 && a < 0.0)
			blur = 0.0;
		//if (blurForeground==false) blur = (a>0.0)?blur:0.0;
	}
	
	blur = clamp(blur,0.0,1.0);
	
	if (debugBlurValue > 0.0)
	{
		vec4 outColor2 = vec4(blur, blur, blur, 1.0);
		gl_FragColor = outColor2;
		return;
	}
	
	// calculation of pattern for ditering
	
	vec2 noise = rand(gl_TexCoord[0].xy)*namount*blur;
	
	// getting blur x and y step factor
	
	float w = (1.0/width)*blur*maxblur+noise.x;
	float h = (1.0/height)*blur*maxblur+noise.y;
	
	// calculation of final color
	
	vec3 inputColor = texture2D(colorSampler, texCoord).rgb;
	vec3 col = inputColor;
	
	if(blur > 0.05) //some optimization thingy
	{
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= rings; i += 1)
		{   
			ringsamples = i * samples;
			
			for (int j = 0 ; j < ringsamples ; j += 1)   
			{
				float step = PI*2.0 / float(ringsamples);
				float pw = (cos(float(j)*step)*float(i));
				float ph = (sin(float(j)*step)*float(i));
				float p = 1.0;
				if (pentagon)
				{ 
					p = penta(vec2(pw,ph));
				}
				col += color(gl_TexCoord[0].xy + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(rings)),bias)*p;  
				s += 1.0*mix(1.0,(float(i))/(float(rings)),bias)*p;   
			}
		}
		col /= s; //divide by sample count
	}
	
	if (showFocus)
	{
		col = debugFocus(col, blur, depth);
	}
	
	vec4 mask = vec4(0.0);
	if (useMasking > 0.0)
	{
		mask = texture2D(maskSampler, texCoord);
	}

	gl_FragColor.rgb = mix(col, inputColor, mask.r * useMasking);
	gl_FragColor.a = 1.0;
}
// end of a shader

