#ifndef VKCONTEXT_HPP
#define VKCONTEXT_HPP

#include <vulkan/vulkan.h>
//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Mesh.hpp"


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

#define max_lights 10


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


//Mesh structures
struct VkBufferVertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	VkBufferVertex();
	VkBufferVertex(glm::vec3,glm::vec3,glm::vec2);
	VkBufferVertex(const VkBufferVertex&);
	~VkBufferVertex();
	
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
	bool operator==( const VkBufferVertex& )const;
};

//Phong structures
struct Material{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;

	Material();
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 mvp;
	Material mat;
	
	UniformBufferObject();
	void updateMatrices();
};


struct LightSources{
	glm::vec4 pos[max_lights];
	glm::vec4 diffuse[max_lights];
	glm::vec4 specular[max_lights];
	glm::vec4 attenuation[max_lights]; //constant - linear - quadratic - spotExponent
	glm::vec4 spots[max_lights]; // xyz - spotCutoff
	LightSources();
};



class VkMesh;


class VkApp{
public:
	static const unsigned int WIDTH = 800;
	static const unsigned int HEIGHT = 600;
	//const std::string MODEL_PATH = "./assets/chalet/chalet2.obj";
	const std::string MODEL_PATH = "./assets/mecha/MechaSonic.obj";
	const std::string TEXTURE_PATH = "./assets/chalet/chalet.jpg";
	
	VkApp();
	virtual ~VkApp();
	void run();

private:
	GLFWwindow* window;
	VkMesh* mesh;
	float FOV;
	glm::vec3 camPos;
	glm::vec3 moveCamStep;
	UniformBufferObject globalUBO;
	LightSources lights;
	

	VDeleter<VkInstance> instance;
	VDeleter<VkDebugReportCallbackEXT> callback;
	VDeleter<VkSurfaceKHR> surface;

	VkPhysicalDevice physicalDevice;
	VDeleter<VkDevice> device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VDeleter<VkSwapchainKHR> swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VDeleter<VkImageView>> swapChainImageViews;
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;
	
	
	VDeleter<VkRenderPass> renderPass;
	VDeleter<VkDescriptorSetLayout> descriptorSetLayout;
	VDeleter<VkPipelineLayout> pipelineLayout;
	VDeleter<VkPipeline> graphicsPipeline;

	VDeleter<VkCommandPool> commandPool;

	VDeleter<VkImage> depthImage;
	VDeleter<VkDeviceMemory> depthImageMemory;
	VDeleter<VkImageView> depthImageView;

	
	VDeleter<VkBuffer> uniformStagingBuffer;
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory;
	VDeleter<VkBuffer> uniformBuffer;
	VDeleter<VkDeviceMemory> uniformBufferMemory;
	
	VDeleter<VkBuffer> lightsStagingBuffer;
	VDeleter<VkDeviceMemory> lightsStagingBufferMemory;
	VDeleter<VkBuffer> lightsBuffer;
	VDeleter<VkDeviceMemory> lightsBufferMemory;


	VDeleter<VkDescriptorPool> descriptorPool;
	VkDescriptorSet descriptorSet;

	std::vector<VkCommandBuffer> commandBuffers;

	VDeleter<VkSemaphore> imageAvailableSemaphore;
	VDeleter<VkSemaphore> renderFinishedSemaphore;

	void initWindow();
	void initVulkan();
	void mainLoop();

	static void onWindowResized(GLFWwindow*, int, int);

	void recreateSwapChain();
	void createInstance();

	void setupDebugCallback();

	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();

	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

	VkFormat findDepthFormat();

	void createImageView(VkImage, VkFormat, VkImageAspectFlags, VDeleter<VkImageView>&);
	void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags,
	                 VkMemoryPropertyFlags, VDeleter<VkImage>&, VDeleter<VkDeviceMemory>&);

	void transitionImageLayout(VkImage, VkImageLayout, VkImageLayout);

	void copyImage(VkImage, VkImage, uint32_t, uint32_t);

	void loadAssets();
	void createUniformBuffer();
	void updateUniformBuffer();

	void createDescriptorPool();
	void createDescriptorSet();

	void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VDeleter<VkBuffer>&, VDeleter<VkDeviceMemory>&);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer);
	void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
	uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);

	void createCommandBuffers();
	void createSemaphores();
	

	void drawFrame() ;
	void createShaderModule(const std::vector<char>&, VDeleter<VkShaderModule>&);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
	bool isDeviceSuitable(VkPhysicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	static std::vector<char> readFile(const std::string&);
	static VkBool32 debugCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);

	friend class VkMesh;
};

#endif
