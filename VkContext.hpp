#ifndef VKCONTEXT_HPP
#define VKCONTEXT_HPP

#include "VkGeometry.hpp"

enum Uniforms{ GLOBAL, LIGHTS, SIZE_U };
enum Images{ DEPTH, SIZE_I };
enum Pipelines{ BLOOM_P, COLOR, PHONG, SIZE_P };


class VkMesh;


class VkApp{
public:
	static uint32_t WIDTH;
	static uint32_t HEIGHT;
	static std::vector<std::string> ASSETS_PATHS;
	static std::vector<std::string> MODELS_NAMES;
	static std::string DEFAULT_TEXTURE;

	VkApp();
	virtual ~VkApp();
	void run();
	
private:
	GLFWwindow* window;
	std::vector<VkMesh*> meshes;
	VkMesh* currentMesh;
	UniformBufferObject globalUBO;
	LightSources lights;
	Material currentMat;
	float FOV;
	CamInfos camera;
	

	VDeleter< VkInstance > instance;
	VDeleter< VkDebugReportCallbackEXT > callback;
	VDeleter< VkSurfaceKHR > surface;

	VkPhysicalDevice physicalDevice;
	VDeleter< VkDevice > device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VDeleter< VkSwapchainKHR > swapChain;
	std::vector< VkImage > swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector< VDeleter< VkImageView > > swapChainImageViews;
	std::vector< VDeleter< VkFramebuffer > > swapChainFramebuffers;
	
	
	VDeleter< VkRenderPass > renderPass;
	VDeleter< VkDescriptorSetLayout > globalDescriptorSetLayout;
	VDeleter< VkDescriptorSetLayout > texturesDescriptorSetLayout;
	VDeleter< VkDescriptorSetLayout > meshDescriptorSetLayout;
	VDeleter< VkDescriptorSetLayout > bloomDescriptorSetLayout;

	VDeleter< VkPipelineLayout > pipelineLayout;
	VDeleter< VkPipeline > graphicsPipeline[Pipelines::SIZE_P];

	VDeleter< VkCommandPool > commandPool;

	VDeleter< VkImage > depthImage;
	VDeleter< VkDeviceMemory > depthImageMemory;
	VDeleter< VkImageView > depthImageView;

	
	VDeleter< VkBuffer > uniformStagingBuffer[Uniforms::SIZE_U];
	VDeleter< VkDeviceMemory > uniformStagingBufferMemory[Uniforms::SIZE_U];
	VDeleter< VkBuffer > uniformBuffer[Uniforms::SIZE_U];
	VDeleter< VkDeviceMemory > uniformBufferMemory[Uniforms::SIZE_U];


	VDeleter< VkDescriptorPool > descriptorPool;
	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSet bloomHorDescriptorSet;
	
	std::vector< VkCommandBuffer > commandBuffers;

	VDeleter< VkSemaphore > imageAvailableSemaphore;
	VDeleter< VkSemaphore > renderFinishedSemaphore;

	//BLOOM SHIET
	BlurDescriptor verticalBlur;
	BlurDescriptor horizontalBlur;
	
	//VkPipelineLayout radialBlur;
	//VkPipelineLayout scene;
	
	FrameBuffer offScreenFrameBuf;
	FrameBuffer offScreenFrameBufB;
	// One sampler for the frame buffer color attachments
	VDeleter< VkSampler > colorSampler;
	// Used to store commands for rendering and blitting
	// the offscreen scene
	VkCommandBuffer offScreenCmdBuffer;
	// Semaphore used to synchronize between offscreen and final scene rendering
	VDeleter< VkSemaphore > offscreenSemaphore;

	void initWindow();
	void initVulkan();
	void mainLoop();

	static void onWindowResized(GLFWwindow*, int, int);
	static void onMouseScroll(GLFWwindow*, double, double);
	static void onMouseClick(GLFWwindow*, int, int, int);
	static void onCursorMove(GLFWwindow*, double, double);
	static void onKeyPressed(GLFWwindow*,int, int, int, int);

	void updateMVP();

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

	uint32_t getMemoryType( uint32_t, VkFlags);
	
	void prepareOffscreenFramebuffers();
	void prepareOffscreenFramebuffer(FrameBuffer*, VkCommandBuffer);

	friend class VkMesh;
	friend class VkSubMesh;
	friend class MaterialGroup;
	friend class BlurDescriptor;
	friend class FrameBufferAttachment;
	friend class FrameBuffer;
};


#endif
