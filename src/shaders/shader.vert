#version 450

layout(binding = 0) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} camData;

// std140 enforces that the layout on the graphics card is the same as in C++
layout(std140, binding = 1) readonly buffer storageBuffer
{
	mat4 model[];
} ObjectData;

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec4 vertexPosition;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 fragColor;

// TODO: Pass this in
#define width 800.0f
#define height 600.0f

void main()
{

	// camData.viewProjection *
	gl_Position =  ObjectData.model[gl_InstanceIndex] * vertexPosition;

	vec2 uvN = 2.0 * uv - 1.0;
    uvN = vec2(uvN.x, uvN.y * height / width);

	// Pass in uvs 
	//fragColor = vec4(uvN, 0.0, 1.0);
}