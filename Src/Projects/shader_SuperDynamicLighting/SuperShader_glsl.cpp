
// SuperShader_glsl.cpp
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader_glsl.h"


void TLight::ConstructDefaultLight0(TLight& light, bool inEyeSpace, const glm::mat4& lViewMatrix, const glm::mat4& lViewRotationMatrix)
{
	memset(&light, 0, sizeof(TLight));
	
	// If there is no light in the scene, we must put the two default lights
	// in order to have almost the same behavior as MB. Those two lights are
	// not present as soon as there is one light in the scene.
	const glm::vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
	const glm::vec4 kDirection0(0.2f, -0.2f, -0.6f, 0.0f);

	if (inEyeSpace)
		light.position = lViewMatrix * kPosition;
	else
		light.position = kPosition;

	light.type = LIGHT_TYPE_DIRECTION;

	light.color = glm::vec3(0.8f, 0.8f, 0.8f);
	light.attenuations = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	if (inEyeSpace)
		light.dir = lViewRotationMatrix * kDirection0;
	else
		light.dir = kDirection0;
}

void TLight::ConstructDefaultLight1(TLight& light, bool inEyeSpace, const glm::mat4& lViewMatrix, const glm::mat4& lViewRotationMatrix)
{
	memset(&light, 0, sizeof(TLight));
	
	// If there is no light in the scene, we must put the two default lights
	// in order to have almost the same behavior as MB. Those two lights are
	// not present as soon as there is one light in the scene.
	const glm::vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
	const glm::vec4 kDirection1(-0.6f, -0.4f, 0.75f, 0.0f);

	if (inEyeSpace)
		light.position = lViewMatrix * kPosition;
	else
		light.position = kPosition;

	light.type = LIGHT_TYPE_DIRECTION;

	light.color = glm::vec3(0.8f, 0.8f, 0.8f);
	light.attenuations = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	if (inEyeSpace)
		light.dir = lViewRotationMatrix * kDirection1;
	else
		light.dir = kDirection1;
}
