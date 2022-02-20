#version 450

// Attributes.
layout(location = 0) in vec3 inUVW;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
//layout(location = 3) in vec3 eyeDirection;

// Textures.
layout(binding = 1) uniform samplerCube skybox;
layout(binding = 2) uniform sampler2D iTexture0;
layout(binding = 3) uniform sampler2D iTexture1;
layout(binding = 4) uniform sampler2D iTexture2;
layout(binding = 5) uniform sampler2D iTexture3;

layout(binding = 6) uniform Parameters {
    vec3 extra012;
    vec3 extra345;
    vec3 extra678;
} params;


// out variables
layout(location = 0) out vec4 outColor;


void main() {
    //outColor = texture(texSampler, fragTexCoord);
    //outColor = vec4(outNormal, 1.0f);
    //outColor = texture(map, outNormal);

    outColor = texture(skybox, inUVW);
}