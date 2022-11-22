#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
	mat4 normalMat;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

void main()
{
	outNormal = (normalMat * vec4(inNormal, 1)).xyz;
    gl_Position = MVP * vec4(inPosition, 1.0);
}