#version 450

layout(set = 0, binding = 0) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} camData;

// std140 enforces that the layout on the graphics card is the same as in C++
layout(std140, set = 0, binding = 1) readonly buffer storageBuffer
{
	mat4 model[];
} ObjectData;

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec3 vertexPosition;
layout(location = 2) in vec2 vertexTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = camData.viewProjection * ObjectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
	fragTexCoord = vertexTexCoord;
}