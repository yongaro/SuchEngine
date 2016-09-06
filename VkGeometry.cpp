#include "VkContext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>

//######################################## DELETER CLASS ##############################################################################
template <typename T>
void VDeleter<T>::cleanup(){
	if( object != VK_NULL_HANDLE ){ deleter(object); }
	object = VK_NULL_HANDLE;
}



template <typename T>
VDeleter<T>::VDeleter(){}//:VDeleter( [](T _) {} ){}

template <typename T>
VDeleter<T>::VDeleter( std::function<void(T, VkAllocationCallbacks*)> deletef ){
	object = VK_NULL_HANDLE;
	this->deleter = [=](T obj) { deletef(obj, nullptr); };
}


template <typename T>
VDeleter<T>::VDeleter( const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef ){
	object = VK_NULL_HANDLE;
	this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
}

template <typename T>
VDeleter<T>::VDeleter( const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef ){
	object = VK_NULL_HANDLE;
	this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
}

template <typename T>
VDeleter<T>::~VDeleter(){ cleanup(); }

template <typename T>
T* VDeleter<T>::operator &(){ cleanup(); return &object; }

template <typename T>
VDeleter<T>::operator T() const{ return object; }



//########################################## GEOMETRY CLASSES ##########################################################################


//############################# STRUCT VERTEX ##########################################################################################
VkBufferVertex::VkBufferVertex():pos(),normal(),texCoord(),tangent(),bitangent(){}
VkBufferVertex::VkBufferVertex(glm::vec3 rPos, glm::vec3 nrm, glm::vec2 UV, glm::vec3 tgt, glm::vec3 bitgt):pos(rPos),normal(nrm),texCoord(UV), tangent(tgt), bitangent(bitgt){}
VkBufferVertex::VkBufferVertex(const VkBufferVertex& ref):VkBufferVertex(ref.pos, ref.normal, ref.texCoord, ref.tangent, ref.bitangent){}
VkBufferVertex::~VkBufferVertex(){}

VkVertexInputBindingDescription VkBufferVertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(VkBufferVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 5> VkBufferVertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VkBufferVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VkBufferVertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(VkBufferVertex, texCoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(VkBufferVertex, tangent);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(VkBufferVertex, bitangent);
	
	return attributeDescriptions;
}

bool VkBufferVertex::operator==( const VkBufferVertex& other )const{
	return pos == other.pos && ( ( normal == other.normal && texCoord == other.texCoord ) && ( tangent == other.tangent && bitangent == other.bitangent ) );
}


//############################# STRUCT MATERIALGROUP  ##########################################################################################

MaterialGroup::MaterialGroup(VkApp* vkapp):mat(),app(vkapp){
	for(size_t i = 0; i < Textures::SIZE_T; ++i){
		textureImage[i] = VDeleter<VkImage>(vkapp->device, vkDestroyImage);
		textureImageMemory[i] = VDeleter<VkDeviceMemory>(vkapp->device, vkFreeMemory);
		textureImageView[i] = VDeleter<VkImageView>(vkapp->device, vkDestroyImageView);
		textureSampler[i] = VDeleter<VkSampler>(vkapp->device, vkDestroySampler);
		//features.list[i] = -1.0f;
	}

	for(size_t i = 0; i < MaterialUniforms::SIZE_MU; ++i){
		uniformStagingBuffer[i] = VDeleter<VkBuffer>(vkapp->device, vkDestroyBuffer);
		uniformStagingBufferMemory[i] = VDeleter<VkDeviceMemory>(vkapp->device, vkFreeMemory);
		uniformBuffer[i] = VDeleter<VkBuffer>(vkapp->device, vkDestroyBuffer);
		uniformBufferMemory[i] = VDeleter<VkDeviceMemory>(vkapp->device, vkFreeMemory);
	}
}

MaterialGroup::~MaterialGroup(){}

void MaterialGroup::createTextureImage(const char* texturePath, uint32_t index){
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if( !pixels ){ throw std::runtime_error( stbi_failure_reason() ); }

	VDeleter<VkImage> stagingImage{app->device, vkDestroyImage};
	VDeleter<VkDeviceMemory> stagingImageMemory{app->device, vkFreeMemory};
	
	app->createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
	                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

	void* data;
	vkMapMemory(app->device, stagingImageMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, (size_t) imageSize);
	vkUnmapMemory(app->device, stagingImageMemory);

	stbi_image_free(pixels);

	app->createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	                 VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[index], textureImageMemory[index]);

	app->transitionImageLayout(stagingImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	app->transitionImageLayout(textureImage[index], VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	app->copyImage(stagingImage, textureImage[index], texWidth, texHeight);

	app->transitionImageLayout(textureImage[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	
	app->createImageView(textureImage[index], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView[index]);
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if( vkCreateSampler(app->device, &samplerInfo, nullptr, &(textureSampler[index]) ) != VK_SUCCESS ){
		throw std::runtime_error("failed to create texture sampler!");
	}
}


void MaterialGroup::createDescriptorSet(){
	VkDescriptorSetLayout layouts[] = {app->texturesDescriptorSetLayout};
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = app->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	
	if( vkAllocateDescriptorSets(app->device, &allocInfo, &textureDescriptorSet ) != VK_SUCCESS ){
		throw std::runtime_error("failed to allocate descriptor set!");
	}
	updateDescriptorSet();
}

void MaterialGroup::updateDescriptorSet(){
	std::array<VkDescriptorBufferInfo, MaterialUniforms::SIZE_MU> materialBufferInfos;
	materialBufferInfos[MaterialUniforms::MATERIAL].buffer = uniformBuffer[MaterialUniforms::MATERIAL];
	materialBufferInfos[MaterialUniforms::MATERIAL].offset = 0;
	materialBufferInfos[MaterialUniforms::MATERIAL].range = sizeof(Material);

	materialBufferInfos[MaterialUniforms::FEATURES].buffer = uniformBuffer[MaterialUniforms::FEATURES];
	materialBufferInfos[MaterialUniforms::FEATURES].offset = 0;
	materialBufferInfos[MaterialUniforms::FEATURES].range = sizeof(Features);

	
	std::array<VkDescriptorImageInfo, Textures::SIZE_T> imagesInfos = {};
	std::array<VkWriteDescriptorSet, (Textures::SIZE_T + MaterialUniforms::SIZE_MU)> descriptorWrites = {};
	
	for( uint32_t i = 0; i < MaterialUniforms::SIZE_MU; ++i ){
		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = textureDescriptorSet;
		descriptorWrites[i].dstBinding = i;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pBufferInfo = &materialBufferInfos[i];
	}

	
	for( uint32_t i = 0; i < Textures::SIZE_T; ++i ){
		imagesInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imagesInfos[i].imageView = textureImageView[i];
		imagesInfos[i].sampler = textureSampler[i];
		
		descriptorWrites[i+MaterialUniforms::SIZE_MU].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].dstSet = textureDescriptorSet;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].dstBinding = i+MaterialUniforms::SIZE_MU;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].dstArrayElement = 0;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].descriptorCount = 1;
		descriptorWrites[i+MaterialUniforms::SIZE_MU].pImageInfo = &imagesInfos[i];
	}
	

	vkUpdateDescriptorSets(app->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void MaterialGroup::createUniformBuffers(){
		VkDeviceSize materialbufferSize = sizeof(Material);
	app->createBuffer(materialbufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                  uniformStagingBuffer[MaterialUniforms::MATERIAL], uniformStagingBufferMemory[MaterialUniforms::MATERIAL]);
	app->createBuffer(materialbufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                  uniformBuffer[MaterialUniforms::MATERIAL], uniformBufferMemory[MaterialUniforms::MATERIAL]);

	VkDeviceSize featuresbufferSize = sizeof(Features);
	app->createBuffer(featuresbufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                  uniformStagingBuffer[MaterialUniforms::FEATURES], uniformStagingBufferMemory[MaterialUniforms::FEATURES]);
	app->createBuffer(featuresbufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                  uniformBuffer[MaterialUniforms::FEATURES], uniformBufferMemory[MaterialUniforms::FEATURES]);

	updateUniformBuffers();
}

void MaterialGroup::updateUniformBuffers(){
	void* materialData;
	vkMapMemory(app->device, uniformStagingBufferMemory[MaterialUniforms::MATERIAL], 0, sizeof(Material), 0, &materialData);
	memcpy(materialData, &mat, sizeof(Material));
	vkUnmapMemory(app->device, uniformStagingBufferMemory[MaterialUniforms::MATERIAL]);
	app->copyBuffer(uniformStagingBuffer[MaterialUniforms::MATERIAL], uniformBuffer[MaterialUniforms::MATERIAL], sizeof(Material));

	void* featuresData;
	vkMapMemory(app->device, uniformStagingBufferMemory[MaterialUniforms::FEATURES], 0, sizeof(Features), 0, &featuresData);
	memcpy(featuresData, &features, sizeof(Features));
	vkUnmapMemory(app->device, uniformStagingBufferMemory[MaterialUniforms::FEATURES]);
	app->copyBuffer(uniformStagingBuffer[MaterialUniforms::FEATURES], uniformBuffer[MaterialUniforms::FEATURES], sizeof(Features));

}


//############################# STRUCT SUBMESH ##########################################################################################
VkSubMesh::VkSubMesh(VkApp* vkapp):app(vkapp),GPUvertices(),GPUindices(),
                                   //Initialisation des buffers pour la geometrie
                                   vertexBuffer(vkapp->device, vkDestroyBuffer),
                                   vertexBufferMemory(vkapp->device, vkFreeMemory),
                                   indexBuffer(vkapp->device, vkDestroyBuffer),
                                   indexBufferMemory(vkapp->device, vkFreeMemory),
                                   mat(NULL){}

VkSubMesh::~VkSubMesh(){}


void VkSubMesh::createVertexBuffer(){
	VkDeviceSize bufferSize = sizeof(GPUvertices[0]) * GPUvertices.size();

	VDeleter<VkBuffer> stagingBuffer{app->device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> stagingBufferMemory{app->device, vkFreeMemory};
	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, GPUvertices.data(), (size_t) bufferSize);
	vkUnmapMemory(app->device, stagingBufferMemory);

	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	app->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	createIndexBuffer();
}



void VkSubMesh::createIndexBuffer(){
	VkDeviceSize bufferSize = sizeof(GPUindices[0]) * GPUindices.size();

	VDeleter<VkBuffer> stagingBuffer{app->device, vkDestroyBuffer};
	VDeleter<VkDeviceMemory> stagingBufferMemory{app->device, vkFreeMemory};
	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                  stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, GPUindices.data(), (size_t) bufferSize);
	vkUnmapMemory(app->device, stagingBufferMemory);

	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	app->copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}


void VkSubMesh::render(VkCommandBuffer& cmdBuffer, VkDescriptorSet meshDescriptorSet){
	VkBuffer vertexBuffers[] = {vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	
	VkDescriptorSet sets[3] = { app->globalDescriptorSet, mat->textureDescriptorSet, meshDescriptorSet };
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipelineLayout, 0, 3, sets, 0, NULL);
	vkCmdDrawIndexed(cmdBuffer, GPUindices.size(), 1, 0, 0, 0);
}






//############################# STRUCT MESH ##########################################################################################
VkMesh::VkMesh(VkApp* vkapp):app(vkapp),subMeshes(),materials(),
                             uniformStagingBuffer(vkapp->device, vkDestroyBuffer),
                             uniformStagingBufferMemory(vkapp->device, vkFreeMemory),
                             uniformBuffer(vkapp->device, vkDestroyBuffer),
                             uniformBufferMemory(vkapp->device, vkFreeMemory)
{
	//createDescriptorSet();
	createUniformBuffer();
}
VkMesh::~VkMesh(){
	for(unsigned int i = 0; i < subMeshes.size(); ++i){ delete subMeshes.at(i); }
	for(unsigned int i = 0; i < materials.size(); ++i){ delete materials.at(i); }
}


void VkMesh::loadMesh(const std::string& assetPath, const std::string& fileName){
	std::string pFile = assetPath + fileName;
	// Create an instance of the Importer class
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile( pFile, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_RemoveRedundantMaterials  | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType );
  
  // If the import failed, report it
  if( !scene ){ throw std::runtime_error("failed to load Mesh with assimp!"); }
  // Now we can access the file's contents. 
  loadScene( scene, assetPath );
}




void VkMesh::loadScene( const aiScene* sc, const std::string& assetPath ){
	aiVector3D Zero3D(0.0f, 0.0f, 0.0f); //Default vector if component is not delivered by assimp

	//Texture and materials recovery
	if( sc->HasMaterials() ){
		for( unsigned int i = 0; i < sc->mNumMaterials; ++i ){
			MaterialGroup* matg = new MaterialGroup(app);
			const aiMaterial* material = sc->mMaterials[i];
			aiString texturePath;
			std::string path;
			
			aiString name;
			material->Get(AI_MATKEY_NAME,name);
			std::cout << "\e[1;33m New Material -- \e[0m"<< name.C_Str() << std::endl;
			
			if( material->GetTextureCount(aiTextureType_DIFFUSE) > 0 && material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture DIFF trouvay \e[1;31m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::DIFF);
				matg->features.list[0][0] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::DIFF);  }

			if( material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0 && material->GetTexture(aiTextureType_DISPLACEMENT, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture DISPLACEMENT trouvay \e[1;32m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::DISPLACEMENT);
				matg->features.list[0][1] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::DISPLACEMENT);  }

			if( material->GetTextureCount(aiTextureType_EMISSIVE) > 0 && material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture EMISSIVE trouvay \e[1;33m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::EMISSIVE);
				matg->features.list[0][2] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::EMISSIVE);  }

			if( material->GetTextureCount(aiTextureType_HEIGHT) > 0 && material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture HEIGHT trouvay \e[1;34m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::HEIGHT);
				matg->features.list[0][3] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::HEIGHT);  }

			if( material->GetTextureCount(aiTextureType_NORMALS) > 0 && material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture NRM trouvay \e[1;35m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::NORMALS);
				matg->features.list[1][0] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::NORMALS);  }


			if( material->GetTextureCount(aiTextureType_SPECULAR) > 0 && material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS ){
				path = assetPath + std::string( texturePath.C_Str() );
				std::replace( path.begin(), path.end(), '\\', '/');
				std::cout << "\t texture SPEC trouvay \e[1;36m" << path.c_str() << "\e[0m"<< std::endl;
				matg->createTextureImage(path.c_str(), Textures::SPECULAR);
				matg->features.list[1][1] = 1.0f;
			}
			else{ matg->createTextureImage(VkApp::DEFAULT_TEXTURE.c_str(), Textures::SPECULAR);  }


			//Material infos
			aiColor3D color (0.f,0.f,0.f);
			material->Get(AI_MATKEY_COLOR_AMBIENT,color);
			matg->mat.ambient = glm::vec4(color.r, color.g, color.b, 1.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE,color);
			matg->mat.diffuse = glm::vec4(color.r, color.g, color.b, 1.0f);
			material->Get(AI_MATKEY_COLOR_SPECULAR,color);
			matg->mat.specular = glm::vec4(color.r, color.g, color.b, 1.0f);
			material->Get(AI_MATKEY_SHININESS,color);
			matg->mat.shininess = color.r / 4.0f; //divided by 4 because of an obj spec misunderstood in assimp
			
			std::cout << std::endl;
			matg->createUniformBuffers();
			materials.push_back(matg);
		}
	}
	
	for( unsigned int i = 0; i < sc->mNumMeshes; ++i ){
		VkSubMesh* subM_temp = new VkSubMesh(app);
		//Recovering vertices data
		for( unsigned int j = 0; j < sc->mMeshes[i]->mNumVertices; ++j ){
			aiVector3D* pPos = &(sc->mMeshes[i]->mVertices[j]);
			aiVector3D* pNormal = &(sc->mMeshes[i]->mNormals[j]);
			aiVector3D* pTexCoord = (sc->mMeshes[i]->HasTextureCoords(0)) ? &(sc->mMeshes[i]->mTextureCoords[0][j]) : &Zero3D;
			aiVector3D* pTangent = (sc->mMeshes[i]->HasTangentsAndBitangents()) ? &(sc->mMeshes[i]->mTangents[j]) : &Zero3D;
			aiVector3D* pBiTangent = (sc->mMeshes[i]->HasTangentsAndBitangents()) ? &(sc->mMeshes[i]->mBitangents[j]) : &Zero3D;

			VkBufferVertex vertex_temp( glm::vec3(pPos->x, pPos->y, pPos->z), 
			                            glm::vec3(pNormal->x, pNormal->y, pNormal->z),
			                            glm::vec2(pTexCoord->x , 1.0f - pTexCoord->y),
			                            glm::vec3(pTangent->x, pTangent->y, pTangent->z),
			                            glm::vec3(pBiTangent->x, pBiTangent->y, pBiTangent->z)
			                            );

			subM_temp->GPUvertices.push_back(vertex_temp);
		}

		//Recovering vertices indices
		for( unsigned int j = 0; j < sc->mMeshes[i]->mNumFaces; ++j ){
			const aiFace& face = sc->mMeshes[i]->mFaces[j];
			if( face.mNumIndices != 3 ){ continue; }
			
			subM_temp->GPUindices.push_back(face.mIndices[0]);
			subM_temp->GPUindices.push_back(face.mIndices[1]);
			subM_temp->GPUindices.push_back(face.mIndices[2]);
		}

		//Recovering material
		subM_temp->mat = materials.at( sc->mMeshes[i]->mMaterialIndex );
		subM_temp->createVertexBuffer();
		subM_temp->createIndexBuffer();
		subMeshes.push_back(subM_temp);
	}
}


void VkMesh::render(VkCommandBuffer& cmdBuffer){
	for(VkSubMesh* subm : subMeshes){ subm->render(cmdBuffer,meshDescriptorSet); }
}

glm::vec3 VkMesh::getCamPos(){
	float axis[6];
	axis[0] = 0.0f; axis[1] = 0.0f;
	axis[2] = 0.0f; axis[3] = 0.0f;
	axis[4] = 0.0f; axis[5] = 0.0f;

	for(VkSubMesh* subm : subMeshes){
		for(VkBufferVertex& v : subm->GPUvertices){
			if( v.pos.x < axis[0] ){ axis[0] = v.pos.x; }
			if( v.pos.x > axis[1] ){ axis[1] = v.pos.x; }
			if( v.pos.y < axis[2] ){ axis[2] = v.pos.y; }
			if( v.pos.y > axis[3] ){ axis[3] = v.pos.y; }
			if( v.pos.z < axis[4] ){ axis[4] = v.pos.z; }
			if( v.pos.z > axis[5] ){ axis[5] = v.pos.z; }
		}
	}
	
	float distx = axis[1] - axis[0];
	float disty = axis[3] - axis[2];
	float distz = axis[5] - axis[4];

	return glm::vec3( distx, disty, distz  );
}

void VkMesh::createDescriptorSet(){
	//Create the mesh descriptorSet
	VkDescriptorSetLayout layouts[] = {app->meshDescriptorSetLayout};
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = app->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	
	if( vkAllocateDescriptorSets(app->device, &allocInfo, &meshDescriptorSet ) != VK_SUCCESS ){
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	//create each submesh descriptor set
	for(MaterialGroup* mg : materials){ mg->createDescriptorSet(); }
	updateDescriptorSet();
}

void VkMesh::updateDescriptorSet(){
	//update mesh descriptorSet
	std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
	VkDescriptorBufferInfo meshBufferInfos;
	meshBufferInfos.buffer = uniformBuffer;
	meshBufferInfos.offset = 0;
	meshBufferInfos.range  = sizeof(MeshTransforms);

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = meshDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &meshBufferInfos;
	vkUpdateDescriptorSets(app->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

	//update subMeshes descriptor set
	for(MaterialGroup* mg : materials){ mg->updateDescriptorSet(); }
}



void VkMesh::createUniformBuffer(){
	VkDeviceSize bufferSize = sizeof(MeshTransforms);

	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                  uniformStagingBuffer, uniformStagingBufferMemory);

	app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                  uniformBuffer, uniformBufferMemory);

	updateUniformBuffer();
}



void VkMesh::updateUniformBuffer(){
	void* data;

	vkMapMemory(app->device, uniformStagingBufferMemory, 0, sizeof(MeshTransforms), 0, &data);
	memcpy(data, &matrices, sizeof(MeshTransforms));

	vkUnmapMemory(app->device, uniformStagingBufferMemory);
	app->copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(MeshTransforms));
}

void VkMesh::updateMVP(glm::mat4 proj, glm::mat4 view){
	matrices.updateMVP(proj,view);
	updateUniformBuffer();
		
}
