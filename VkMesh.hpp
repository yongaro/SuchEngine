#ifndef VKMESH_HPP
#define VKMESH_HPP

#include "VkContext.hpp"
#include <glm/gtx/hash.hpp>



class VkMesh : public Mesh{
protected:
	VkApp* app;
	std::vector<VkBufferVertex> GPUvertices;
	std::vector<int> GPUindices;

	//Textures vulkan
	VDeleter<VkImage> textureImage{app->device, vkDestroyImage};
	VDeleter<VkDeviceMemory> textureImageMemory{app->device, vkFreeMemory};
	VDeleter<VkImageView> textureImageView{app->device, vkDestroyImageView};
	VDeleter<VkSampler> textureSampler{app->device, vkDestroySampler};

	//Geometrie vulkan
	VDeleter<VkBuffer> vertexBuffer{app->device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> vertexBufferMemory{app->device, vkFreeMemory};
	VDeleter<VkBuffer> indexBuffer{app->device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> indexBufferMemory{app->device, vkFreeMemory};

public:
	VkMesh(VkApp*);
	VkMesh(VkApp*,const std::string&);

	virtual void createTextureImage(const std::string&);
	virtual void createTextureImageView();
	virtual void createTextureSampler();
	//virtual void loadTexture(const std::string&);
	
	virtual void createVertexBuffer();
	virtual void createIndexBuffer();
	virtual void CPUtoGPU();
	

	friend class VkApp;
};

namespace std {
	template<> struct hash<VkBufferVertex> {
		size_t operator()(VkBufferVertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}



#endif
