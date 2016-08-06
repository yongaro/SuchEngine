#version 300 es
 
 
uniform sampler2D sampler;
in highp vec2 UV;

in highp vec3 FragPos;
in highp vec3 Normal;
in highp vec4 vColor;
uniform highp vec3 viewPos;
uniform highp vec3 lightPos;
//highp vec3 lightColor = vec3(0.3,1.0,0.0);
highp vec3 lightColor = vec3(1.0,1.0,1.0);
uniform int colorFlag;

 
out highp vec4 color;
 
void main() {
  
  highp vec3 ambient = 0.1f * lightColor;
 	highp vec3 norm = normalize(Normal);
	highp vec3 lightDir = normalize(lightPos - FragPos);  
  
  highp float diff = max(dot(norm, lightDir), 0.0);
	highp vec3 diffuse = diff * lightColor * 0.7;
  
  highp float specularStrength = 0.2f;
  highp vec3 viewDir = normalize(viewPos - FragPos);
  highp vec3 reflectDir = reflect(-lightDir, norm);
  highp float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  highp vec3 specular = specularStrength * spec * lightColor;    
     
  //vec3 result = (ambient + diffuse + specular ) * texture(sampler, UV).rgb;
  //vec3 result = (ambient + diffuse + specular ) * lightColor;
  //highp vec3 result = (ambient + diffuse + specular ) * Normal;
  //highp vec3 result = (ambient + diffuse + specular ) * texture(sampler, UV).rgb;
  highp vec3 result = (ambient + diffuse + specular ) * lightColor;
  	
  //color = vec4(Normal,1.0);  	
  //color = vColor;
  color = vec4(result, 1.0f);
  //color = texture(sampler, UV);
    
}
