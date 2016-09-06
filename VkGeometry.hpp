#ifndef VKGEOMETRY_HPP
#define VKGEOMETRY_HPP

#include "VkStructures.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>


class VkApp;

struct VkBufferVertex{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	VkBufferVertex();
	VkBufferVertex(glm::vec3, glm::vec3, glm::vec2, glm::vec3, glm::vec3);
	VkBufferVertex(const VkBufferVertex&);
	~VkBufferVertex();
	
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
	bool operator==( const VkBufferVertex& )const;
};





enum Textures{ DIFF, DISPLACEMENT, EMISSIVE, HEIGHT, NORMALS, SPECULAR, SIZE_T };
enum MaterialUniforms{MATERIAL, FEATURES, SIZE_MU};
struct Features{
	glm::mat4 list;
	// Kd - disp - Ke - bump
	// Kn - Ks
	Features():list(){
		list[0][0] = -1.0f; list[0][1] = -1.0f; list[0][2] = -1.0f; list[0][3] = -1.0f;
		list[1][0] = -1.0f; list[1][1] = -1.0f;
	}
};



struct MaterialGroup{
	Material mat;
	VkApp* app;
	VkDescriptorSet textureDescriptorSet;
	Features features;
	
	//Textures vulkan
	VDeleter<VkImage> textureImage[Textures::SIZE_T];
	VDeleter<VkDeviceMemory> textureImageMemory[Textures::SIZE_T];
	VDeleter<VkImageView> textureImageView[Textures::SIZE_T];
	VDeleter<VkSampler> textureSampler[Textures::SIZE_T];

	//Material infos and texture usage buffers
	VDeleter<VkBuffer> uniformStagingBuffer[MaterialUniforms::SIZE_MU];
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory[MaterialUniforms::SIZE_MU];
	VDeleter<VkBuffer> uniformBuffer[MaterialUniforms::SIZE_MU];
	VDeleter<VkDeviceMemory> uniformBufferMemory[MaterialUniforms::SIZE_MU];

	
	MaterialGroup(VkApp*);
	virtual ~MaterialGroup();

	void createTextureImage(const char*, uint32_t);
	void createDescriptorSet();
	void updateDescriptorSet();
	
	void createUniformBuffers();
	void updateUniformBuffers();
};


struct VkSubMesh{
	VkApp* app;
	std::vector<VkBufferVertex> GPUvertices;
	std::vector<int> GPUindices;

	//Geometrie vulkan
	VDeleter<VkBuffer> vertexBuffer;
	VDeleter<VkDeviceMemory> vertexBufferMemory;
	VDeleter<VkBuffer> indexBuffer;
	VDeleter<VkDeviceMemory> indexBufferMemory;

	MaterialGroup* mat;
	
	VkSubMesh(VkApp*);
	~VkSubMesh();

	void createVertexBuffer();
	void createIndexBuffer();
	void render(VkCommandBuffer&, VkDescriptorSet);
};




struct MeshTransforms{
	glm::mat4 model;
	glm::mat4 mvp;

	MeshTransforms():model(1.0f),mvp(1.0f){}
	void updateMVP(glm::mat4& proj, glm::mat4& view){ mvp = proj * view * model; }
};


class VkMesh{
public:
	VkApp* app;
	std::vector<VkSubMesh*> subMeshes;
	std::vector<MaterialGroup*> materials;

	MeshTransforms matrices;
	VkDescriptorSet meshDescriptorSet;
	//Model and MVP matrices
	VDeleter<VkBuffer> uniformStagingBuffer;
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory;
	VDeleter<VkBuffer> uniformBuffer;
	VDeleter<VkDeviceMemory> uniformBufferMemory;
	
	VkMesh(VkApp*);
	virtual ~VkMesh();
	
	virtual void loadMesh(const std::string&, const std::string&);
	virtual void loadScene(const aiScene*, const std::string&);
	virtual void render(VkCommandBuffer&);
	virtual glm::vec3 getCamPos();
	virtual void createDescriptorSet();
	virtual void updateDescriptorSet();

	virtual void createUniformBuffer();
	virtual void updateUniformBuffer();
	virtual void updateMVP(glm::mat4, glm::mat4);
};



#endif
