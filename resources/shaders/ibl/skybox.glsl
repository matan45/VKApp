#type VERTEX
#version 460 core
layout (location = 0) in vec3 position;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
}ubo;

layout (location = 0) out vec3 WorldPos;

void main()
{
    WorldPos = position;

	mat4 rotView = mat4(mat3(ubo.view));
	vec4 clipPos = ubo.projection * rotView * vec4(WorldPos, 1.0);

	gl_Position = clipPos.xyww;
}

#type FRAGMENT
#version 460 core
layout (location = 0) out vec4 FragColor;
layout (location = 0) in vec3 WorldPos;

layout(binding = 1) uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = environmentMap.rgba;
}