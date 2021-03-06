#version 450

#define PI 3.14159265359
#define LI 8.


/*Output variables. */
layout(location = 0) out vec4 outcolor;

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

layout(binding = 6) uniform Parameters {
    vec3 extra012;
    vec3 extra345;
    vec3 extra678;
} params;


#define iParameter0 params.extra012.x
#define iParameter1 params.extra012.y
#define iParameter2 params.extra012.z
#define iParameter3 params.extra345.x
#define iParameter4 params.extra345.y
#define iParameter5 params.extra345.z
#define iParameter6 params.extra678.x
#define iParameter7 params.extra678.y
#define iParameter8 params.extra678.z



/*Functions*/
//BRDF_Output brdf(vec3 L, vec3 N, vec3 V);
vec3 render(vec3 L, vec3 N, vec3 V, vec2 textureCord, mat3 worldToLocal);

uint base_hash(uvec2 p);
vec2 PseudoRandom2D(in int i);

/*Main*/
void main() {
	//outcolor = vec4(normalize(ubo.pos_c), 1.0f);
	vec4 texcol = texture(iTexture0, fragTexCoord);
	vec3 N = normalize(inNormal);
	vec3 V = normalize(env.pos_c - vertPosition);
	// vec3 L = -normalize(reflect(V, N));


	// BRDF_Output brdfo = brdf(L, N, V);

	/*Monte-Carlo Setup*/
    
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
    vec3 accum = render(reflect(-V,N), N, V, fragTexCoord, inv_axis); // Starting of the accumalator with the perfect reflection direction.
	for(int i = bias; i < scatterCount + bias - 1; i++){
        /*Caculating Sample direction*/
        vec2 hl = PseudoRandom2D(i);
        float ourV_sqrt = sqrt(1. -  hl.y* hl.y);
        vec3 L = normalize( axis * vec3( ourV_sqrt*cos(2.*PI *  hl.x), hl.y, ourV_sqrt*sin(2.*PI *  hl.x)));
        accum += render(L, N, V, fragTexCoord, inv_axis);   //texcol.xyz*L_c*(br.specular + br.diffuse ); //cook_torrance_schlik_brdf(L,V,N)*L_c*LN;//; 
    } 

    // Sphere colouring schemes
    accum /= (float(scatterCount));
    outcolor = vec4(accum, 1.0);


	//outcolor = vec4(brdfo.specular, 1.);//vec4(texture(texSampler, fragTexCoord));
	// outcolor = vec4(brdfo.specular, 1.);
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
