#version 450


// Attributes.
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;

layout(location = 0) out vec4 outColor;

// Textures.
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube map;

void main() {
    //outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(outNormal, 1.0f);
    //outColor = texture(texSampler, fragTexCoord);
}