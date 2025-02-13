#version 450

layout(binding = 0) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} camData;

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec4 vertexPosition;
layout(location = 2) in vec2 uv;

layout(push_constant) uniform constants
{
	mat4 model;
} ObjectData;

layout(location = 0) out vec4 fragColor;

void main()
{
// camData.viewProjection * ObjectData.model * 
	gl_Position = vertexPosition;
	fragColor = vertexColor;
}