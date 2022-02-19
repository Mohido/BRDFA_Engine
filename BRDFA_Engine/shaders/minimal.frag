#version 450

/*Output variables. */
layout(location = 0) out vec4 outColor;

/*Incoming variables*/
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 vertPosition;

/*Uniforms*/
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 pos_c;				// camera position in space.
	vec3 mat_p;				// material options (Roughness, anistropy)
} env;

layout(binding = 1) uniform samplerCube skybox;
layout(binding = 2) uniform sampler2D iTexture0;
layout(binding = 3) uniform sampler2D iTexture1;
layout(binding = 4) uniform sampler2D iTexture2;
layout(binding = 5) uniform sampler2D iTexture3;


/*Main*/
void main() {
	outColor = vec4(texture(iTexture0, fragTexCoord));
}
