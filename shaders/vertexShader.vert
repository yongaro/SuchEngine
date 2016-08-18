#version 450
#extension GL_ARB_separate_shader_objects : enable

#define max_lights 16

struct Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess; 
};



layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	 mat4 mvp;
	 Material mat;
} ubo;

layout(binding = 2) uniform LightSources{
	vec4 pos[max_lights];
	vec4 diffuse[max_lights];
	vec4 specular[max_lights];
	vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExpoment
	vec4 spots[max_lights]; // xyz - spotCutoff
} lights;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;


layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;



//Sortie pre-definie
out gl_PerVertex {
   vec4 gl_Position;
   //float gl_PointSize;
   //float gl_ClipDistance[];
};

void main() {
	gl_Position = ubo.mvp * vec4(inPosition, 1.0);
	fragPos = vec3(ubo.model * vec4(inPosition ,1.0));
	fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal; 
	fragTexCoord = vec2( inTexCoord.x, 1.0 - inTexCoord.y );
}

