#version 450


// Attributes.
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 vertPosition;

layout(location = 0) out vec4 outColor;

// Textures.
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube map;



void main() {
    vec3 camPosition = normalize(vec3(ubo.view[3]));
    outColor = vec4(camPosition, 1.0f);
    
    //outColor = texture(texSampler, fragTexCoord);
    //outColor = texture(texSampler, fragTexCoord);
}