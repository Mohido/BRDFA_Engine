#version 450

#define PI 3.14159265359
#define LI 8.


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



/*Structures*/
struct BRDF_Output{
	vec3 diffuse;
	vec3 specular;
};

/*Functions*/
//BRDF_Output brdf(vec3 L, vec3 N, vec3 V);
BRDF_Output brdf(vec3 L, vec3 N, vec3 V, vec2 extra);

uint base_hash(uvec2 p);
vec2 PseudoRandom2D(in int i);

/*Main*/
void main() {
	//outColor = vec4(normalize(ubo.pos_c), 1.0f);
	vec4 texcol = texture(iTexture0, fragTexCoord);
	vec3 N = normalize(inNormal);
	vec3 V = normalize(env.pos_c - vertPosition);
	// vec3 L = -normalize(reflect(V, N));


	// BRDF_Output brdfo = brdf(L, N, V);

	/*Monte-Carlo Setup*/
    vec3 accum = vec3(0.); // MonteCarlo accumalator
    const int scatterCount = int(env.mat_p.z); // Ray samples 
    int bias = int(base_hash(floatBitsToUint(gl_FragCoord.xy))); // int(base_hash(floatBitsToUint(gl_FragCoord.xy)));

	
    float VN = dot(V, N);  

    /*Sphere space transformation matrix*/
    vec3 fst_axis = normalize(cross(V,N));          // (1,0,0)
    vec3 sec_axis = normalize(cross(fst_axis,N));   // (0,0,1)
    mat3 axis = mat3(fst_axis,N,sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
    mat3 inv_axis = transpose(axis);

	// vec3 c = envColor * brdfo.specular;   
    vec4 textureColor = texture(iTexture0, fragTexCoord);

	for(int i = bias; i < scatterCount + bias; i++){
        /*Caculating Sample direction*/
        vec2 hl = PseudoRandom2D(i);
        float ourV_sqrt = sqrt(1. -  hl.y* hl.y);
        vec3 L = normalize( axis * vec3( ourV_sqrt*cos(2.*PI *  hl.x), hl.y, ourV_sqrt*sin(2.*PI *  hl.x)));
        float LN = dot(L,N);
        vec3 envColor = texture(skybox, L).rgb;
        vec3 L_c = LI*envColor;
        BRDF_Output br = brdf(L, N, V, env.mat_p.xy);
        accum += texcol.xyz*L_c*(br.specular + br.diffuse ); //cook_torrance_schlik_brdf(L,V,N)*L_c*LN;//; 
    } 

    // Sphere colouring schemes
    accum /= (float(scatterCount));
    outColor = vec4(accum, 1.0);


	//outColor = vec4(brdfo.specular, 1.);//vec4(texture(texSampler, fragTexCoord));
	// outColor = vec4(brdfo.specular, 1.);
}

/**
  Used for creating a bias from the given point.
*/
uint base_hash(uvec2 p) {
  p = 1103515245U*((p >> 1U)^(p.yx));
  uint h32 = 1103515245U*((p.x)^(p.y>>3U));
  return h32^(h32 >> 16);
}


/**
  An implementation of Holton sequence which is uniformly distributed function.
*/
vec2 PseudoRandom2D(in int i){
  return fract(vec2(i*ivec2(12664745, 9560333))/exp2(24.0));
}
