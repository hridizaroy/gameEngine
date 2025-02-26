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


#define width 800.0f
#define height 600.0f

void main()
{
// camData.viewProjection * ObjectData.model * 
	gl_Position = vertexPosition;
	fragColor = vertexColor;

	vec2 uvN = 2.0 * uv - 1.0;
    uvN = vec2(uvN.x, uvN.y * height / width);

	fragColor = vec4(uvN, 0.0, 1.0);
	//fragColor = vec4(uvN, 0.0, 1.0);
}