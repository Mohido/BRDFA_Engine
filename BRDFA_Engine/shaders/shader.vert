#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;


/*
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outPosition;
*/

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;


void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.xyz, 1.0);

	outPos = vec3(ubo.model * vec4(inPosition, 1.0));
	outNormal = mat3(ubo.model) * inNormal;

	vec3 lightPos = vec3(0.0f, 5.0f, 0.0f);
	outLightVec = lightPos.xyz - outPos.xyz;
	outViewVec = -outPos.xyz;

    /*
    vec4 vertInWorld = ubo.model * vec4(inPosition, 1.0f);
    outPosition = vec3(vertInWorld.xyz) / vertInWorld.w;
    gl_Position = ubo.proj * ubo.view * vertInWorld;
    
    outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;

    fragColor = inColor;
    fragTexCoord = inTexCoord;

    */
}