#version 450
#extension GL_ARB_separate_shader_objects : enable

#define max_lights 10
#define GLOBAL_UBO 0
#define LIGHTS_UBO 1

layout(set = 0, binding = GLOBAL_UBO) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
	 vec3 camPos;
} ubo;

layout(set = 0, binding = LIGHTS_UBO) uniform LightSources{
	vec4 pos[max_lights];
	vec4 diffuse[max_lights];
	vec4 specular[max_lights];
	vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExpoment
	vec4 spots[max_lights]; // xyz - spotCutoff
} lights;

layout(set = 1, binding = 0) uniform Material{
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

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 inFragTexCoord;
layout(location = 3) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;




vec4 scene_ambient = vec4(0.01, 0.01, 0.01, 1.0);
float height_scale = 0.01;


vec3 surfacePos = fragPos;
vec2 fragTexCoord = inFragTexCoord;
vec4 surfaceColor = vec4(1.0, 1.0, 1.0, 1.0);

vec3 ApplyLight(int index, vec3 surfaceColor, vec3 N) {
	vec3 currentLightPos = lights.pos[index].xyz;
	vec3 L = normalize(currentLightPos - surfacePos.xyz);
	vec3 V = normalize(ubo.camPos);
	vec3 R = reflect(-L, N);
	float attenuation = 1.0;

	
	if( lights.pos[index].w == 0.0 ){
		//directional light
		L = normalize( currentLightPos );
		attenuation = 1.0; //no attenuation for directional lights
	}
	else{
		//point light
		float distanceToLight = length(currentLightPos - surfacePos);
		attenuation = 1.0 / (1.0
		                     + lights.attenuation[index].x //constant
		                     + lights.attenuation[index].y * distanceToLight //linear
		                     + lights.attenuation[index].z * distanceToLight * distanceToLight); //quadratic
	}

	//ambient
	vec3 ambient = scene_ambient.rgb * mat.ambient.rgb;
	
	//diffuse
	vec3 diffuse = max(dot(N, L), 0.0) * surfaceColor.rgb * lights.diffuse[index].rgb * mat.diffuse.rgb;
    
	//specular
	vec3 specular = pow(max(dot(R, V), 0.0), mat.shininess)  * lights.specular[index].rgb * mat.specular.rgb;
	if( features.list[1][1] > 0.0 ){ specular *= texture(specularTexSampler, fragTexCoord).rgb; }
	//else{ specular *= mat.specular.rgb; }

	//linear color (color before gamma correction)
	return ambient + attenuation*(diffuse + specular);
}


//steep parallax mapping
vec2 parallaxMapping(){
	vec3 tangentViewPos = fragTBN * ubo.camPos;
	vec3 tangentFragPos = fragTBN * fragPos;
	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	
	
	// number of depth layers
	const float minLayers = 64;
	const float maxLayers = 256;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	//float numLayers = mix(maxLayers, minLayers, abs(dot(normalize(fragNormal), viewDir)));;
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy * height_scale; 
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords     = inFragTexCoord;
	float currentDepthMapValue = texture(heightTexSampler, currentTexCoords).r;
  
	while( currentLayerDepth < currentDepthMapValue ){
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(heightTexSampler, currentTexCoords).r;
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}
	
	return currentTexCoords;
}   



void main() {
	//parallax mapping
	height_scale = 0.2;
	if( features.list[0][3] > 0.0 ){ fragTexCoord = parallaxMapping(); }
	
	//normal map
	vec3 tangentNormal = normalize( texture(normalsTexSampler, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 normal = normalize(fragNormal);
	if( features.list[1][0] > 0.0 ){ normal =  normalize(fragTBN * tangentNormal); }

	//phong base informations
	
	vec3 surfaceToCamera = normalize(ubo.camPos - surfacePos);

	//texture for diffuse lighting
	
	if( features.list[0][0] > 0.0 ){ surfaceColor = texture(diffuseTexSampler, fragTexCoord); }
	else{ surfaceColor = vec4(1.0, 1.0, 1.0, 1.0); }

	//combine color from all the lights
	vec3 linearColor = vec3(0.0);
	if( features.list[0][2] > 0.0 ){ linearColor = texture(emissiveTexSampler, fragTexCoord).rgb; }
	else{
		for( int i = 0; i < max_lights; ++i ){
			if( lights.pos[i].w < 0.0 ){ continue; } //if light is used
			linearColor += ApplyLight(i, surfaceColor.rgb, normal);
		}
	}
	//final color with gamma correction
	vec3 gamma = vec3(1.0/2.2);
	outColor = vec4(pow(linearColor, gamma), surfaceColor.a);
	//outColor = vec4(linearColor, surfaceColor.a);
}
