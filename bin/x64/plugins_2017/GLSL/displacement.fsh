//
// Fragment shader - Simple 
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
// based on Quake2 water effect and Simplex Noise function
// http://mines.lumpylumpy.com/Electronics/Computers/Software/Cpp/Graphics/Bitmap/Textures/Noise/Simplex.php#.U7FwI41dVhs

uniform sampler2D colorSampler;

uniform float	upperClip;
uniform float	lowerClip;

uniform float iTime;
uniform float iSpeed;

uniform float	useQuakeEffect;

uniform float xDistMag;
uniform float yDistMag;

uniform float xSineCycles;
uniform float ySineCycles;


// ---- SETTINGS ----------------------------------------------------------------

//#define speed 1.0

// the amount of shearing (shifting of a single column or row)
// 1.0 = entire screen height offset (to both sides, meaning it's 2.0 in total)
//#define xDistMag 0.02
//#define yDistMag 0.02

// cycle multiplier for a given screen height
// 2*PI = you see a complete sine wave from top..bottom
//#define xSineCycles 6.28
//#define ySineCycles 6.28

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float aastep(float threshold, float value) {
 
    float afwidth = length(vec2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
 
}

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

void main (void)
{
	//vec2 tx = gl_TexCoord [0].st;
	//vec4 color = texture2D( sampler0, tx ); 
	
	// do the scaling.
    // After this, you should consider fragCoord = 0..1, usually,
    // aside from overflow for wide-screen.
    vec2 fragCoord = gl_TexCoord [0].st;

	if (fragCoord.y < upperClip || fragCoord.y > lowerClip)
	{
		vec4 fragColor = texture2D(colorSampler, fragCoord);
		gl_FragColor =  fragColor;
		return;
	}
	
    
    // the value for the sine has 2 inputs:
    // 1. the time, so that it animates.
    // 2. the y-row, so that ALL scanlines do not distort equally.
    
	float ltime = iTime;
	
    float xAngle = ltime + fragCoord.y * ySineCycles;
    float yAngle = ltime + fragCoord.x * xSineCycles;
    
	vec2 distortOffset = vec2(0.0);
	
	if (0.0 == useQuakeEffect)
	{
		distortOffset.x = xDistMag * snoise(vec3(xSineCycles + fragCoord.x, fragCoord.y, 0.1*ltime));
		distortOffset.y = yDistMag * snoise(vec3(fragCoord.x, ySineCycles + fragCoord.y, 0.1*ltime));
	}
	else
	{
		distortOffset = 
			vec2(sin(xAngle), sin(yAngle)) * // amount of shearing
			vec2(xDistMag,yDistMag); // magnitude adjustment
	}
   
    // shear the coordinates
    fragCoord += distortOffset;    
    
    
	vec4 fragColor = texture2D(colorSampler, fragCoord);
	gl_FragColor =  fragColor;
}
