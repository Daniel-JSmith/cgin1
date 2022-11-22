#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 previousPositionDiff;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outVelocity;

vec4 SURFACE_COLOR = vec4(0,1,0,1) * 0.25f;
vec3 AMBIENT_LIGHT = vec3(1,1,1) * 0.0125f;
vec3 LIGHT_DIRECTION = vec3(-0.25f, 0, 1);

void main()
{
    outColor = SURFACE_COLOR * max(0, dot(inNormal, LIGHT_DIRECTION)) + vec4(AMBIENT_LIGHT, 1);
    outVelocity = vec4(previousPositionDiff, 1.0f);
}
