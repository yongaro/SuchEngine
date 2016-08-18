#include "VkMesh.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//######################################## DELETER CLASS ##############################################################################
template <typename T>
void VDeleter<T>::cleanup(){
	if( object != VK_NULL_HANDLE ){ deleter(object); }
	object = VK_NULL_HANDLE;
}



template <typename T>
VDeleter<T>::VDeleter():VDeleter( [](T _) {} ){}

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
//######################################################################################################################################








VkMesh::VkMesh(VkApp* vkapp):Mesh(),app(vkapp),
                             //Initialisation des structures pour texture
                             textureImage(vkapp->device, vkDestroyImage),
                             textureImageMemory(vkapp->device, vkFreeMemory),
                             textureImageView(vkapp->device, vkDestroyImageView),
                             textureSampler(vkapp->device, vkDestroySampler),
                             //Initialisation des buffers pour la geometrie
                             vertexBuffer(vkapp->device, vkDestroyBuffer),
                             vertexBufferMemory(vkapp->device, vkFreeMemory),
                             indexBuffer(vkapp->device, vkDestroyBuffer),
                             indexBufferMemory(vkapp->device, vkFreeMemory){}


VkMesh::VkMesh(VkApp* vkapp, const std::string& path):VkMesh(vkapp){ loadOBJ(path.c_str()); }




void VkMesh::createTextureImage(const std::string& texturePath){
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if( !pixels ){ throw std::runtime_error("failed to load texture image!"); }

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
	                 VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	app->transitionImageLayout(stagingImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	app->transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	app->copyImage(stagingImage, textureImage, texWidth, texHeight);

	app->transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	createTextureImageView();
}

void VkMesh::createTextureImageView(){
	app->createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
	createTextureSampler();
}




void VkMesh::createTextureSampler(){
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

	if( vkCreateSampler(app->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS ){
		throw std::runtime_error("failed to create texture sampler!");
	}
}












void VkMesh::createVertexBuffer(){
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



void VkMesh::createIndexBuffer(){
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




void VkMesh::CPUtoGPU(){
	GPUvertices.clear();
	GPUindices.clear();
	
	std::unordered_map<VkBufferVertex, int> vertexMap;
	VkBufferVertex temp[3];
	
	for( Face* f : *CPUFaces ){
		temp[0] = VkBufferVertex( *(f->pts->at(0)->coord),*(f->nrm->at(f->pts->at(0))),*(f->uv->at(f->pts->at(0))) );
		temp[1] = VkBufferVertex( *(f->pts->at(1)->coord),*(f->nrm->at(f->pts->at(1))),*(f->uv->at(f->pts->at(1))) );
		temp[2] = VkBufferVertex( *(f->pts->at(2)->coord),*(f->nrm->at(f->pts->at(2))),*(f->uv->at(f->pts->at(2))) );

		for(size_t i = 0; i < 3; ++i){
			if( vertexMap[temp[i]] == 0 ){
				GPUvertices.push_back( VkBufferVertex(temp[i]) );
				vertexMap[temp[i]] = GPUvertices.size();
			}
			GPUindices.push_back( vertexMap[temp[i]] - 1 );
		}
	}
	createVertexBuffer();
}
