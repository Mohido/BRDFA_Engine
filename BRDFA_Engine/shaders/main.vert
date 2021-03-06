#version 450


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 pos_c; // Camera position
    vec3 mat_p; // Material parameters (Roughness, anistropy)
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outPosition;


void main() {


    vec4 vertInWorld = ubo.model * vec4(inPosition, 1.0f);
    outPosition = vec3(vertInWorld.xyz) / vertInWorld.w;
    gl_Position = ubo.proj * ubo.view * vertInWorld;
    
    outNormal = transpose(inverse(mat3(ubo.model))) * inNormal;

    outColor = inColor;
    fragTexCoord = inTexCoord;

    
}