#version 450
#extension GL_ARB_separate_shader_objects : enable

#define max_lights 10

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

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform LightSources{
	vec4 pos[max_lights];
	vec4 diffuse[max_lights];
	vec4 specular[max_lights];
	vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExpoment
	vec4 spots[max_lights]; // xyz - spotCutoff
} lights;



layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;




vec4 scene_ambient = vec4(0.0, 0.0, 0.0, 1.0);

void main(){
	
	vec3 normalDirection = normalize(fragNormal);
	vec3 viewDirection = normalize(vec3(inverse(ubo.view) * vec4(0.0, 0.0, 0.0, 1.0) - vec4(fragPos,1.0) ));
	vec3 lightDirection;
	float attenuation;

	// initialize total lighting with ambient lighting
  vec3 totalLighting = vec3(scene_ambient) * vec3(ubo.mat.ambient);

  for(int i = 0; i < max_lights; ++i){
	  //current light is used ?
	  if(lights.pos[i].z < 0){ continue; }
	  
	  
	  if( 0.0 == lights.pos[i].w ){// directional light?
		  attenuation = 1.0; // no attenuation
		  lightDirection = normalize(vec3(lights.pos[i]));
	  } 
	  else{ // point light or spotlight (or other kind of light)
		  vec3 positionToLightSource = vec3(lights.pos[i] - vec4(fragPos,1.0));
		  float distance = length(positionToLightSource);
		  lightDirection = normalize(positionToLightSource);
		  attenuation = 1.0 / (lights.attenuation[i].x //constant
		                       + lights.attenuation[i].y * distance //linear
		                       + lights.attenuation[i].z * distance * distance); //quadratic
      
		  if( lights.spots[1].z <= 90.0 ){ // spotlight?
			  float clampedCosine = max(0.0, dot(-lightDirection, lights.spots[1].xyz));
			  if( clampedCosine < cos(radians(lights.spots[i].z)) ){ attenuation = 0.0; } // outside of spotlight cone?
			  else{ attenuation = attenuation * pow(clampedCosine, lights.attenuation[i].z); }
		  }
	  }

	  vec3 ambientLighting = vec3(scene_ambient) * vec3(ubo.mat.ambient);
  
	  vec3 diffuseReflection = attenuation
		  * vec3(lights.diffuse[i]) * vec3(ubo.mat.diffuse)
		  * max(0.0, dot(normalDirection, lightDirection));
  
	  vec3 specularReflection;
	  if( dot(normalDirection, lightDirection) < 0.0 ){ // light source on the wrong side?
		  specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
	  }
	  else{ // light source on the right side
		  specularReflection = attenuation * vec3(lights.specular[i]) * vec3(ubo.mat.specular) 
			  * pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), ubo.mat.shininess);
	  }
	  totalLighting = totalLighting + diffuseReflection + specularReflection;
  }
  
  //gamma correction
  float gamma = 2.2;
  totalLighting.rgb = pow(totalLighting.rgb,vec3(1.0/gamma));
  outColor = vec4(totalLighting, 1.0);
}
