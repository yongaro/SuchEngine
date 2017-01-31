#version 300 es
 
  in vec3 vertex;
  in vec2 vertexUV;
  in vec3 normal;
  in vec4 vertexColor;
   
 // Values that stay constant for the whole mesh.
 uniform mat4 model;
 uniform mat4 view;
 uniform mat4 projection;
 uniform mat4 MVP;
   
  out vec3 FragPos; 
  out vec2 UV; 
  out vec3 Normal;
  out vec4 vColor;
  
  void main() {
  	gl_Position = MVP * vec4(vertex,1);
  	
  	FragPos = vec3(model * vec4(vertex ,1.0f));
  	UV = vertexUV;
  	Normal = mat3(transpose(inverse(model))) * normal; 
  	vColor = vertexColor;
  }