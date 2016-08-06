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




extern VkResult CreateDebugReportCallbackEXT(VkInstance, const VkDebugReportCallbackCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugReportCallbackEXT*);
extern void DestroyDebugReportCallbackEXT(VkInstance, VkDebugReportCallbackEXT, const VkAllocationCallbacks*);





template <typename T>
class VDeleter {
private:

	T object;
	std::function<void(T)> deleter;

	void cleanup(){
		if( object != VK_NULL_HANDLE ){ deleter(object); }
		object = VK_NULL_HANDLE;
	}
	
public:
	
	VDeleter():VDeleter( [](T _) {} ){}

	VDeleter( std::function<void(T, VkAllocationCallbacks*)> deletef ){
		object = VK_NULL_HANDLE;
		this->deleter = [=](T obj) { deletef(obj, nullptr); };
	}

	VDeleter( const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef ){
		object = VK_NULL_HANDLE;
		this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
	}

	VDeleter( const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef ){
		object = VK_NULL_HANDLE;
		this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
	}

	~VDeleter(){ cleanup(); }
	T* operator &(){ cleanup(); return &object; }
	operator T() const{ return object; }

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




struct LightSource{
	glm::vec4 pos;
	glm::vec3 intensities;
	float attenuation;
	float ambientCoeff;
	float coneAngle;
	glm::vec3 coneDirection;
};



struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	//glm::mat4 modelView;
};


class VkMesh;


class VkApp{
public:
	~VkApp();
	void run();
private:
	const unsigned int WIDTH = 800;
	const unsigned int HEIGHT = 600;
	const std::string MODEL_PATH = "./assets/chalet/chalet2.obj";
	const std::string TEXTURE_PATH = "./assets/chalet/chalet.jpg";
	GLFWwindow* window;
	VkMesh* mesh;

	VDeleter<VkInstance> instance{vkDestroyInstance};
	VDeleter<VkDebugReportCallbackEXT> callback{instance, DestroyDebugReportCallbackEXT};
	VDeleter<VkSurfaceKHR> surface{instance, vkDestroySurfaceKHR};

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> device{vkDestroyDevice};

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VDeleter<VkSwapchainKHR> swapChain{device, vkDestroySwapchainKHR};
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VDeleter<VkImageView>> swapChainImageViews;
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;

	VDeleter<VkRenderPass> renderPass{device, vkDestroyRenderPass};
	VDeleter<VkDescriptorSetLayout> descriptorSetLayout{device, vkDestroyDescriptorSetLayout};
	VDeleter<VkPipelineLayout> pipelineLayout{device, vkDestroyPipelineLayout};
	VDeleter<VkPipeline> graphicsPipeline{device, vkDestroyPipeline};

	VDeleter<VkCommandPool> commandPool{device, vkDestroyCommandPool};

	VDeleter<VkImage> depthImage{device, vkDestroyImage};
	VDeleter<VkDeviceMemory> depthImageMemory{device, vkFreeMemory};
	VDeleter<VkImageView> depthImageView{device, vkDestroyImageView};

	
	VDeleter<VkBuffer> uniformStagingBuffer{device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory{device, vkFreeMemory};
	VDeleter<VkBuffer> uniformBuffer{device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> uniformBufferMemory{device, vkFreeMemory};

	VDeleter<VkDescriptorPool> descriptorPool{device, vkDestroyDescriptorPool};
	VkDescriptorSet descriptorSet;

	std::vector<VkCommandBuffer> commandBuffers;

	VDeleter<VkSemaphore> imageAvailableSemaphore{device, vkDestroySemaphore};
	VDeleter<VkSemaphore> renderFinishedSemaphore{device, vkDestroySemaphore};

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
