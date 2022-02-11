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
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube map;

/*Structures*/
struct BRDF_Output{
	vec3 diffuse;
	vec3 specular;
};

/*Functions*/
BRDF_Output brdf(vec3 L, vec3 N, vec3 V);


/*Main*/
void main() {
	//outColor = vec4(normalize(ubo.pos_c), 1.0f);
	outColor = texture(texSampler, fragTexCoord);


	/*textures view
	if(ubo.render_opt.x == 0.0f){
		outColor = texture(texSampler, fragTexCoord);
	}
	*/

	/*Normals view
	if(ubo.render_opt.x == 1.0f){
		outColor = vec4(normalize(inNormal), 1.0f);
	}
	*/

	/*Reflection view
	if(ubo.render_opt.x == 2.0f){
		vec3 N = normalize(inNormal);
		vec3 V = normalize(ubo.pos_c - vertPosition);
		vec3 L = -normalize(reflect(V, N));
		//outColor = vec4(L, 1.0f);
		outColor = texture(map, L);
	}
	*/
}
