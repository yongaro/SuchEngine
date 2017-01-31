#ifndef GLSTRUCTURES_HPP
#define GLSTRUCTURES_HPP

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cstring>
#include <array>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>


//Phong
#define max_lights 10

enum ModificationStates{ NONE, ROTATION, ORIENT_CAMERA, NORMAL, SPRINT, WALK };

//Uniforms structures
struct CamInfos{
	uint32_t modifMode;
	double clickedX;
	double clickedY;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 pos;
	glm::vec3 backupPos;
	glm::vec3 target;
	glm::mat4 rotateYaw;
	glm::mat4 rotatePitch;
	glm::vec3 targetToCam;	
	uint32_t speed;

	CamInfos():modifMode(ModificationStates::NONE),
	           clickedX(), clickedY(),
	           up(0.0f, 1.0f, 0.0f), right(1.0f, 0.0f, 0.0f),
	           pos(), backupPos(), target(),
	           rotateYaw(1.0f), rotatePitch(1.0f),
	           targetToCam(),
	           speed(ModificationStates::NORMAL){}
};


struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 camPos;
	
	
	UniformBufferObject();
};


struct LightSources{
	glm::vec4 pos[max_lights];
	glm::vec4 diffuse[max_lights];
	glm::vec4 specular[max_lights];
	glm::vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExponent
	glm::vec4 spots[max_lights]; // xyz - spotCutoff
	LightSources();
};


//Phong-global structures
struct Material{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;

	Material():ambient(0.8, 0.8, 0.8, 1.0),
	           diffuse(0.8, 0.8, 0.8, 1.0),
	           specular(0.8, 0.8, 0.8, 1.0),
	           shininess(16.0){}
};

#endif
