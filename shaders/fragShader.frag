#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

vec3 viewPos = vec3(ubo.view[0]);
vec3 lightColors = vec3(1.0,1.0,1.0);
vec3 lightPos = vec3(0.0,0.0,4.0);//vec3(viewPos);

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;


layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;



void main(){
	/*
	highp vec3 ambient = 0.1f * lightColors;
 	highp vec3 norm = normalize(fragNormal);
	highp vec3 lightDir = normalize(lightPos - fragPos);  
  
  	highp float diff = max(dot(norm, lightDir), 0.0);
	highp vec3 diffuse = diff * lightColors * 0.7;
  
  	highp float specularStrength = 0.2f;
  	highp vec3 viewDir = normalize(viewPos - fragPos);
	highp vec3 reflectDir = reflect(-lightDir, norm);
	highp float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	highp vec3 specular = specularStrength * spec * lightColors;    
     
  	//vec3 result = (ambient + diffuse + specular ) * texture(sampler, UV).rgb;
  	//vec3 result = (ambient + diffuse + specular ) * lightColor;
  	//highp vec3 result = (ambient + diffuse + specular ) * vec3(vColor);
  	highp vec3 result = (ambient + diffuse + specular ) * texture(texSampler, fragTexCoord).rgb;
  	//highp vec3 result = (ambient + diffuse + specular ) * lightColors;
  	outColor = vec4(result,1.0);
  	//outColor = vec4(fragNormal,1.0);
	*/

	
	//calculate normal in world coordinates
	mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
	vec3 normal = normalize(normalMatrix * fragNormal);
	
	//calculate the location of this fragment (pixel) in world coordinates
	vec3 fragPosition = vec3(ubo.model * vec4(fragPos, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = lightPos - fragPosition;

	//calculate the cosine of the angle of incidence
	float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
	brightness = clamp(brightness, 0.0, 1.0);

	//calculate final color of the pixel, based on:
	// 1. The angle of incidence: brightness
	// 2. The color/intensities of the light: light.intensities
	// 3. The texture and texture coord: texture(tex, fragTexCoord)
	vec4 surfaceColor = texture(texSampler, fragTexCoord);
	outColor = vec4(brightness * lightColors * surfaceColor.rgb, surfaceColor.a);
	//outColor = vec4(brightness * lightColors, 1.0);
	
	//outColor = vec4(fragNormal,1.0);
	//outColor = texture(texSampler, fragTexCoord);
	
}
