#version 450
//#extension GL_ARB_separate_shader_objects : enable

#define max_lights 10

layout(std140) uniform globalMatrices {
    mat4 view;
    mat4 proj;
	 vec3 camPos;
} globalMat;


layout(std140) uniform lightSources{
	vec4 pos[max_lights];
	vec4 diffuse[max_lights];
	vec4 specular[max_lights];
	vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExpoment
	vec4 spots[max_lights]; // xyz - spotCutoff
} lights;

layout(std140) uniform Material{
	vec4 Ka;//ambient;
	vec4 Kd;//diffuse;
	vec4 Ks;//specular;
	float shininess; 
} mat;

layout(std140) uniform Features{ mat4 list; } features;

layout(std140) uniform MeshTranforms{
	mat4 model;
	mat4 mvp;
} meshTransforms;

//I NEED MOAR SYNTAXIK SUGAR
#define DIFFUSE 0
#define DISPLACEMENT 1
#define EMISSIVE 2
#define HEIGHT 3
#define NORMALS 4
#define SPECULAR 5
layout(binding = DIFFUSE) uniform sampler2D diffuseTexSampler;
layout(binding = DISPLACEMENT) uniform sampler2D displacementTexSampler;
layout(binding = EMISSIVE) uniform sampler2D emissiveTexSampler;
layout(binding = HEIGHT) uniform sampler2D heightTexSampler;
layout(binding = NORMALS) uniform sampler2D normalsTexSampler;
layout(binding = SPECULAR) uniform sampler2D specularTexSampler;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;


layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN;



//Sortie pre-definie
out gl_PerVertex {
   vec4 gl_Position;
};


void main() {
	gl_Position = meshTransforms.mvp * vec4(inPosition, 1.0);
	
	//displacement map -- pas terrible -- a refaire
	if( features.list[0][1] > 0.0 ){
		vec4 dv = 1.0 - texture( heightTexSampler, inTexCoord );
		float df =  0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
		vec4 newVertexPos = vec4(inNormal * df * 0.1, 0.0) + vec4(inPosition,1.0);
		gl_Position = meshTransforms.mvp * newVertexPos;
	}
	
	
	fragPos = vec3(meshTransforms.model * vec4(inPosition ,1.0));
	fragNormal = mat3(transpose(inverse(meshTransforms.model))) * inNormal; 
	fragTexCoord = vec2( inTexCoord.x, inTexCoord.y );

	vec3 T = normalize(vec3(meshTransforms.model * vec4(inTangent,   0.0)));
	vec3 B = normalize(vec3(meshTransforms.model * vec4(inBiTangent, 0.0)));
	vec3 N = normalize(vec3(meshTransforms.model * vec4(inNormal,    0.0)));
	fragTBN = mat3(T, B, N);
}

