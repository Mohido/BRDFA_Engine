#version 450


// Attributes.


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 vertPosition;

/*
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewVec;
layout (location = 3) in vec3 inLightVec;
*/

layout(location = 0) out vec4 outColor;

// Textures.
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 pos_c; // camera position in space.
} ubo;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube map;



void main() {
	/*
    vec3 cI = normalize (inPos);
	vec3 cR = reflect (cI, normalize(inNormal));
	mat4 invModel = inverse(ubo.model);

	cR = vec3( invModel * vec4(cR, 0.0));
	// Convert cubemap coordinates into Vulkan coordinate space
	// cR.xz *= -1.0;	// Vulkan has the negatives inversed for somereason.

	vec3 N = normalize(inNormal);
	vec4 color = texture(map, N + inPos);

	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 ambient = vec3(0.5) * color.rgb;
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.5);
	//outColor = vec4(normalize(ubo.pos_c), 1.0f); //vec4(ambient + diffuse * color.rgb + specular, 1.0);
	*/



    //vec3 camPosition = normalize(vec3(ubo.view[3]));
    //outColor = vec4(camPosition, 1.0f);
    outColor = texture(texSampler, fragTexCoord);
    //outColor = texture(texSampler, fragTexCoord);
}