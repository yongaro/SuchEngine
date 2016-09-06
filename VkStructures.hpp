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
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM

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
	int32_t texWidth = TEX_DIM;
	int32_t texHeight = TEX_DIM;
	float blurScale = 1.0f;
	float blurStrength = 1.5f;
	uint32_t horizontal;
};


// Framebuffer for offscreen rendering
struct FrameBufferAttachment {
	VDeleter<VkImage> image;
	VDeleter<VkDeviceMemory> mem;
	VDeleter<VkImageView> view;
	
	FrameBufferAttachment(const VDeleter<VkDevice>&);
};
struct FrameBuffer {
	int32_t width, height;
	VDeleter<VkFramebuffer> frameBuffer;
	FrameBufferAttachment color;
	FrameBufferAttachment depth;

	FrameBuffer(const VDeleter<VkDevice>&);
};

/*

// Setup the offscreen framebuffer for rendering the mirrored scene
// The color attachment of this framebuffer will then be sampled from
void VkApp::prepareOffscreenFramebuffer(FrameBuffer *frameBuf, VkCommandBuffer cmdBuffer){
	frameBuf->width = FB_DIM;
	frameBuf->height = FB_DIM;
	
	VkFormat fbColorFormat = FB_COLOR_FORMAT;
	
	// Find a suitable depth format
	VkFormat fbDepthFormat = findDepthFormat();
	
	// Color attachment
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.pNext = NULL;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = fbColorFormat;
	image.extent.width = frameBuf->width;
	image.extent.height = frameBuf->height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	// We will sample directly from the color attachment
	image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	
	VkMemoryAllocateInfo memAlloc = {}
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;

	VkMemoryRequirements memReqs;
	
	VkImageViewCreateInfo colorImageView = {}
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.pNext = NULL;
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = fbColorFormat;
	colorImageView.flags = 0;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;

	VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &frameBuf->color.image));
	vkGetImageMemoryRequirements(device, frameBuf->color.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &frameBuf->color.mem));
	VK_CHECK_RESULT(vkBindImageMemory(device, frameBuf->color.image, frameBuf->color.mem, 0));

	// Set the initial layout to shader read instead of attachment 
	// This is done as the render loop does the actualy image layout transitions
	vkTools::setImageLayout(
	                        cmdBuffer, 
	                        frameBuf->color.image, 
	                        VK_IMAGE_ASPECT_COLOR_BIT, 
	                        VK_IMAGE_LAYOUT_UNDEFINED, 
	                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	colorImageView.image = frameBuf->color.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &frameBuf->color.view));

	// Depth stencil attachment
	image.format = fbDepthFormat;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.pNext = NULL;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = fbDepthFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &frameBuf->depth.image));
	vkGetImageMemoryRequirements(device, frameBuf->depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &frameBuf->depth.mem));
	VK_CHECK_RESULT(vkBindImageMemory(device, frameBuf->depth.image, frameBuf->depth.mem, 0));

	vkTools::setImageLayout(
	                        cmdBuffer,
	                        frameBuf->depth.image, 
	                        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
	                        VK_IMAGE_LAYOUT_UNDEFINED, 
	                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	depthStencilView.image = frameBuf->depth.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &frameBuf->depth.view));

	VkImageView attachments[2];
	attachments[0] = frameBuf->color.view;
	attachments[1] = frameBuf->depth.view;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = renderPass;
	fbufCreateInfo.attachmentCount = 2;
	fbufCreateInfo.pAttachments = attachments;
	fbufCreateInfo.width = frameBuf->width;
	fbufCreateInfo.height = frameBuf->height;
	fbufCreateInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &frameBuf->frameBuffer));
}

// Prepare the offscreen framebuffers used for the vertical- and horizontal blur 
void prepareOffscreenFramebuffers(){
	VkCommandBuffer cmdBuffer = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	prepareOffscreenFramebuffer(&offScreenFrameBuf, cmdBuffer);
	prepareOffscreenFramebuffer(&offScreenFrameBufB, cmdBuffer);
	VulkanExampleBase::flushCommandBuffer(cmdBuffer, queue, true);

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.pNext = NULL;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 0;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &colorSampler));
}
*/

#endif
