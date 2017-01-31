#ifndef VKSTRUCTURES_HPP
#define VKSTRUCTURES_HPP

#include <vulkan/vulkan.h>
//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include "Mesh.hpp"


#include <iostream>
#include <stdexcept>
#include <functional>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <array>
#include <set>
#include <unordered_map>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/hash.hpp>


//Phong
#define max_lights 10

//Bloom
#define TEX_DIM 256
#define TEX_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define TEX_FILTER VK_FILTER_LINEAR;
// Offscreen frame buffer properties
#define FB_DIM TEX_DIM
#define FB_COLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM

class VkApp;

extern VkResult CreateDebugReportCallbackEXT(VkInstance, const VkDebugReportCallbackCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugReportCallbackEXT*);
extern void DestroyDebugReportCallbackEXT(VkInstance, VkDebugReportCallbackEXT, const VkAllocationCallbacks*);



template <typename T>
class VDeleter {
private:
	T object;
	std::function<void(T)> deleter;
	void cleanup();
	
public:
	VDeleter();
	VDeleter( std::function<void(T, VkAllocationCallbacks*)> );
	VDeleter( const VDeleter<VkInstance>&, std::function<void(VkInstance, T, VkAllocationCallbacks*)> );
	VDeleter( const VDeleter<VkDevice>&, std::function<void(VkDevice, T, VkAllocationCallbacks*)> );

	~VDeleter();
	T* operator &();
	operator T() const;
};




//Vulkan specific structures
struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete(){ return graphicsFamily >= 0 && presentFamily >= 0; }
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};






//GLFW callback infos
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







//Bloom strutures
struct BlurData {
	glm::vec2 scale_strength; //x = blurScale -- y = blurStrength
	uint32_t texWidth;
	uint32_t texHeight;
	uint32_t flag;

	BlurData():scale_strength(1.0f, 1.5f),texWidth(TEX_DIM),texHeight(TEX_DIM),flag(0){}
};


struct BlurDescriptor{
	BlurData blurInfos;
	VkDescriptorSet descriptorSet;
	
	VDeleter<VkBuffer> uniformStagingBuffer;
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory;
	VDeleter<VkBuffer> uniformBuffer;
	VDeleter<VkDeviceMemory> uniformBufferMemory;
	
	BlurDescriptor(VkApp*);
	static void createDescriptorSetLayout(VkApp*);
	void createDescriptorSet();
};


// Framebuffer for offscreen rendering
struct FrameBufferAttachment {
	VDeleter<VkImage> image;
	VDeleter<VkDeviceMemory> mem;
	VDeleter<VkImageView> view;
	
	FrameBufferAttachment(VkApp*);
};


struct FrameBuffer {
	VkApp* app;
	int32_t width, height;
	VDeleter<VkFramebuffer> frameBuffer;
	FrameBufferAttachment color;
	FrameBufferAttachment depth;

	FrameBuffer(VkApp*);
	// Setup the offscreen framebuffer for rendering the mirrored scene
	// The color attachment of this framebuffer will then be sampled from
	//void prepareOffscreenFramebuffer(FrameBuffer*, VkCommandBuffer);
};






#endif
