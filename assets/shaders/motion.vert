#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 previousMVP;
	mat4 currentMVP;
	mat4 normalMat;
	vec4 screenResolution;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 previousPositionDiff;

void main()
{
	outNormal = (normalMat * vec4(inNormal, 1)).xyz;
    gl_Position = currentMVP * vec4(inPosition, 1.0);

	vec4 previousPositionClip = previousMVP * vec4(inPosition, 1.0f);
	vec4 currentPositionClip = currentMVP * vec4(inPosition, 1.0f);
	vec3 previousPositionNDC = (previousPositionClip / previousPositionClip.w).xyz;
	vec3 currentPositionNDC = (currentPositionClip / currentPositionClip.w).xyz;
	vec3 previousPositionScreen = vec3((screenResolution.x) * previousPositionNDC.x + (previousPositionNDC.x + 1920/2), (screenResolution.y * previousPositionNDC.y) + (screenResolution.y + previousPositionNDC.y), 0);
	vec3 currentPositionScreen = vec3((screenResolution.x) * currentPositionNDC.x + (currentPositionNDC.x + 1920/2), (screenResolution.y * currentPositionNDC.y) + (screenResolution.y + currentPositionNDC.y), 0);
	previousPositionDiff = previousPositionScreen - currentPositionScreen;
}