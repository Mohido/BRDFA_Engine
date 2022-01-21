#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 pos_c; // camera position in space.
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
    outUVW = inPos.xyz;
    vec3 final = mat3(ubo.view) * inPos;
	gl_Position = ubo.proj * vec4(final.xyz, 1.0f);
}



