
//////////////////////////////////////////////////////////////////////////////////////
//
//	== Composite Master Tool ==
// GLSL Vertex Shader for 3d Fog effect
//
//  Author Sergey Solohin (Neill3d) 2014-2016
//		e-mail to: s@neill3d.com
//			www.neill3d.com
//
//////////////////////////////////////////////////////////////////////////////////////

attribute vec4	position;
attribute vec2 texcoord;
attribute vec4 attrNear;
attribute vec4 attrFar;

varying vec3 viewDir;
varying vec3 viewOrig;

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * position;
	gl_TexCoord [0].st = texcoord;
	
	viewOrig = attrNear.xyz;
	viewDir = attrFar.xyz;
}
