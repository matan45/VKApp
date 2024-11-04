#type VERTEX
#version 460 core
layout (location = 0) in vec3 position;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 WorldPos;

void main()
{
    WorldPos = position;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

	gl_Position = clipPos.xyww;
}

#type FRAGMENT
#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float exposure;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = vec3(1.0) - exp(-envColor * exposure);
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}