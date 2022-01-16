#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

//layout(location = 0) out vec3 fragColor;
layout (location = 0) out vec3 outUVW;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal;
// layout(location = 3) out vec3 eyeDirection;

void main() {
/*
    mat4 inverseProjection = inverse(ubo.proj);
    mat3 inverseModelview = transpose(mat3(ubo.view));
    vec3 unprojected = (inverseProjection * inPos).xyz;
    eyeDirection = inverseModelview * unprojected;
    gl_Position = vec4(inPos.xyz, 1.0);
    */

    //vec4 temp = ubo.view * ubo.model * vec4(inPos ,1.0f);
    //outUVW = vec3(temp.xyz/temp.w);
    outUVW = inPos.xzy;


	// Convert cubemap coordinates into Vulkan coordinate space
	//outUVW.x *= -1.0;


    vec3 final = mat3(ubo.view) * inPos;
    //final.z *= -1.0;


	gl_Position = ubo.proj * vec4(final.xyz, 1.0f);
    //gl_Position.x *= -1.0f;
}



