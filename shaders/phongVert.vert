#version 450
#extension GL_ARB_separate_shader_objects : enable

#define max_lights 10
#define GLOBAL_UBO 0
#define LIGHTS_UBO 1


layout(set = 0, binding = GLOBAL_UBO) uniform UniformBufferObject {
	//mat4 model;
    mat4 view;
    mat4 proj;
    //mat4 mvp;
	 vec3 camPos;
} ubo;

layout(set = 0, binding = LIGHTS_UBO) uniform LightSources{
	vec4 pos[max_lights];
	vec4 diffuse[max_lights];
	vec4 specular[max_lights];
	vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExpoment
	vec4 spots[max_lights]; // xyz - spotCutoff
} lights;

layout(set = 1, binding = 0) uniform  Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess; 
} mat;

//I NEED MOAR SYNTAXIK SUGAR
#define OFFSET 2
#define DIFFUSE 0
#define DISPLACEMENT 1
#define EMISSIVE 2
#define HEIGHT 3
#define NORMALS 4
#define SPECULAR 5
#define SIZE_T 6



layout(set = 1, binding = 1) uniform Features{ mat4 list; } features;


layout(set = 1, binding = (DIFFUSE + OFFSET)) uniform sampler2D diffuseTexSampler;
layout(set = 1, binding = (DISPLACEMENT + OFFSET)) uniform sampler2D displacementTexSampler;
layout(set = 1, binding = (EMISSIVE + OFFSET)) uniform sampler2D emissiveTexSampler;
layout(set = 1, binding = (HEIGHT + OFFSET)) uniform sampler2D heightTexSampler;
layout(set = 1, binding = (NORMALS + OFFSET)) uniform sampler2D normalsTexSampler;
layout(set = 1, binding = (SPECULAR + OFFSET)) uniform sampler2D specularTexSampler;

layout(set = 2, binding = 0) uniform MeshTranforms{
	mat4 model;
	mat4 mvp;
} transforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;


layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
//layout(location = 3) out vec3 tangentFragPos;
//layout(location = 4) out vec3 tangentViewPos;
layout(location = 3) out mat3 fragTBN;



//Sortie pre-definie
out gl_PerVertex {
   vec4 gl_Position;
};


void main() {
	gl_Position = transforms.mvp * vec4(inPosition, 1.0);
	
	//displacement map
	if( features.list[0][1] > 0.0 ){
		vec4 dv = 1.0 - texture( heightTexSampler, inTexCoord );
		float df =  0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
		vec4 newVertexPos = vec4(inNormal * df * 0.1, 0.0) + vec4(inPosition,1.0);
		gl_Position = transforms.mvp * newVertexPos;
	}
	
	
	fragPos = vec3(transforms.model * vec4(inPosition ,1.0));
	fragNormal = mat3(transpose(inverse(transforms.model))) * inNormal; 
	fragTexCoord = vec2( inTexCoord.x, inTexCoord.y );

	vec3 T = normalize(vec3(transforms.model * vec4(inTangent,   0.0)));
	vec3 B = normalize(vec3(transforms.model * vec4(inBiTangent, 0.0)));
	vec3 N = normalize(vec3(transforms.model * vec4(inNormal,    0.0)));
	fragTBN = mat3(T, B, N);
	//tangentFragPos = fragTBN * fragPos;
	//tangentViewPos = fragTBN * ubo.camPos;
}

