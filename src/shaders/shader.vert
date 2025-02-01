#version 450

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec3 vertexPosition;

layout(push_constant) uniform constants
{
	mat4 model;
} ObjectData;

layout(location = 0) out vec4 fragColor;

void main()
{
	gl_Position = ObjectData.model * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
}