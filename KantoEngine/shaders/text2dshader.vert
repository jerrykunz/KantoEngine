#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;

layout(binding = 0) uniform ViewProjectionUBO 
{
    mat4 view;
    mat4 proj;
} viewProjUBO;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 5) out flat float TexIndex;

void main()
{	
	Output.Color = a_Color;
	//Output.Color.a = 0.1f;
	Output.TexCoord = a_TexCoord;
	TexIndex = a_TexIndex;
	gl_Position = viewProjUBO.proj * viewProjUBO.view * vec4(a_Position, 1.0);
}
