#version 450

// Attributes.
layout(location = 0) in vec3 inUVW;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
//layout(location = 3) in vec3 eyeDirection;

// Textures.
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube map;

// out variables
layout(location = 0) out vec4 outColor;


void main() {
    //outColor = texture(texSampler, fragTexCoord);
    //outColor = vec4(outNormal, 1.0f);
    //outColor = texture(map, outNormal);

    outColor = texture(map, inUVW);
}