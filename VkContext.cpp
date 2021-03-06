
#include "VkContextGenericPart.hpp"


//############################### STRUCT UNIFORMBUFFEROBJECT ##############################
UniformBufferObject::UniformBufferObject():view( glm::lookAt(glm::vec3(2.5f, 2.5f, 2.5f),
														                   glm::vec3(0.0f, 1.25f, 0.0f),
														                   glm::vec3(0.0f, 1.0f, 0.0f))
														       ),
														 proj( glm::perspective(glm::radians(45.0f),
														                        VkApp::WIDTH / (float)VkApp::HEIGHT,
														                        0.1f, 10.0f)
														       ),
                                           camPos()                                     
{
	proj[1][1] *= -1;
}

LightSources::LightSources(){
	//Initialisation IMPORTANTE de toutes les lumieres
	for( size_t i = 0; i < max_lights; ++i ){
		pos[i] = glm::vec4(0.0f, 0.0f, 0.0f, -1.0f);
		diffuse[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		specular[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		attenuation[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		spots[i] = glm::vec4(0.0f,-1.0f,0.0f, 180.0f);
	}
}

//############################### STRUCT BLOOMDESCRIPTOR ##############################


BlurDescriptor::BlurDescriptor(VkApp* vkapp):blurInfos(),descriptorSet(),
                                               uniformStagingBuffer(vkapp->device, vkDestroyBuffer),
                                               uniformStagingBufferMemory(vkapp->device, vkFreeMemory),
                                               uniformBuffer(vkapp->device, vkDestroyBuffer),
                                               uniformBufferMemory(vkapp->device, vkFreeMemory){}


void BlurDescriptor::createDescriptorSetLayout(VkApp* vkapp){
	//BloomDescriptor::descriptorSetLayout = VDeleter<VkDescriptorSetLayout>(vkapp->device, vkDestroyDescriptorSetLayout);

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {};
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_ALL;

	//Creation of the bloom data DescriptorSetLayout
	VkDescriptorSetLayoutCreateInfo layoutInfos = {};
	layoutInfos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfos.bindingCount = bindings.size();
	layoutInfos.pBindings = bindings.data();

	if( vkCreateDescriptorSetLayout(vkapp->device, &layoutInfos, nullptr, &(vkapp->bloomDescriptorSetLayout)) != VK_SUCCESS ){
		throw std::runtime_error("failed to create descriptor set layout!");
	}
	
}
void BlurDescriptor::createDescriptorSet(){

}



//############################## CLASS VKAPP ##############################################

//TODO
VkApp::VkApp():window(NULL),meshes(),
               globalUBO(),lights(),currentMat(),FOV(60),camera(),

               instance(vkDestroyInstance),
               callback(instance, DestroyDebugReportCallbackEXT),
               surface(instance, vkDestroySurfaceKHR),

               physicalDevice(VK_NULL_HANDLE),
               device(vkDestroyDevice),

               graphicsQueue(),
               presentQueue(),

               swapChain(device, vkDestroySwapchainKHR),
               swapChainImages(),swapChainImageFormat(),swapChainExtent(),
               swapChainImageViews(),swapChainFramebuffers(),

               renderPass(device, vkDestroyRenderPass),
               globalDescriptorSetLayout(device, vkDestroyDescriptorSetLayout),
               texturesDescriptorSetLayout(device, vkDestroyDescriptorSetLayout),
               meshDescriptorSetLayout(device, vkDestroyDescriptorSetLayout),
               bloomDescriptorSetLayout(device, vkDestroyDescriptorSetLayout),
               pipelineLayout(device, vkDestroyPipelineLayout),
              

               commandPool(device, vkDestroyCommandPool),

               depthImage(device, vkDestroyImage),
               depthImageMemory(device, vkFreeMemory),
               depthImageView(device, vkDestroyImageView),

               
               descriptorPool(device, vkDestroyDescriptorPool),
               globalDescriptorSet(),

               commandBuffers(),

               imageAvailableSemaphore(device, vkDestroySemaphore),
               renderFinishedSemaphore(device, vkDestroySemaphore),
               //bloom
               verticalBlur(this),
               horizontalBlur(this),
               offScreenFrameBuf(this),
               offScreenFrameBufB(this),
               colorSampler(device, vkDestroySampler),
               offScreenCmdBuffer(),
               offscreenSemaphore(device, vkDestroySemaphore)
               
{
	for( size_t i = 0; i < Uniforms::SIZE_U; ++i ){
		uniformStagingBuffer[i] = VDeleter< VkBuffer >(device, vkDestroyBuffer);
		uniformStagingBufferMemory[i] = VDeleter< VkDeviceMemory >(device, vkFreeMemory);
		uniformBuffer[i] = VDeleter< VkBuffer >(device, vkDestroyBuffer);
		uniformBufferMemory[i] = VDeleter< VkDeviceMemory >(device, vkFreeMemory);
	}

	for(size_t i = 0; i < Pipelines::SIZE_P; ++i){
		graphicsPipeline[i] = VDeleter<VkPipeline>(device, vkDestroyPipeline);
	}
}




VkApp::~VkApp(){
	for( size_t i = 0; i < meshes.size(); ++i ){
		delete meshes.at(i);
	}
}

void VkApp::run(){
	initWindow();
	initVulkan();
	mainLoop();
}


void VkApp::initWindow(){
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(VkApp::WIDTH, VkApp::HEIGHT, "Vulkan", nullptr, nullptr);
	
	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, VkApp::onWindowResized);
	glfwSetScrollCallback(window, VkApp::onMouseScroll);
	glfwSetMouseButtonCallback(window,VkApp::onMouseClick);
	glfwSetCursorPosCallback(window, VkApp::onCursorMove);
	glfwSetKeyCallback(window,VkApp::onKeyPressed);
}

void VkApp::initVulkan(){
	createInstance();
	setupDebugCallback();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();

	loadAssets();

	createUniformBuffer();
	createDescriptorPool();
	prepareOffscreenFramebuffers();
	createDescriptorSet();
	createCommandBuffers();
	createSemaphores();
}

void VkApp::mainLoop(){
	while( !glfwWindowShouldClose(window) ){
		glfwPollEvents();

		updateUniformBuffer();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void VkApp::onWindowResized(GLFWwindow* window, int width, int height){
	if( width == 0 || height == 0 ){ return; }

	VkApp* app = reinterpret_cast<VkApp*>( glfwGetWindowUserPointer(window) );
	app->recreateSwapChain();
}

void VkApp::onMouseClick(GLFWwindow* window, int mouseButton, int action, int mods){
	if( action == GLFW_PRESS ){
		VkApp* app = reinterpret_cast<VkApp*>( glfwGetWindowUserPointer(window) );
		
		if( mouseButton == GLFW_MOUSE_BUTTON_LEFT ){
			if( app->camera.modifMode != ModificationStates::ROTATION ){
				app->camera.modifMode = ModificationStates::ROTATION;
				glfwGetCursorPos(window, &(app->camera.clickedX), &(app->camera.clickedY));
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				return;
			}
			if( app->camera.modifMode == ModificationStates::ROTATION ){
				app->camera.modifMode = ModificationStates::NONE;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				return;
			}
		}
	}
}


void VkApp::onKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods){
	if( action == GLFW_PRESS ){
		VkApp* app = reinterpret_cast<VkApp*>( glfwGetWindowUserPointer(window) );
		
		//mode switch
		if( key == GLFW_KEY_F ){
			if( app->camera.modifMode != ModificationStates::ORIENT_CAMERA ){
				app->camera.modifMode = ModificationStates::ORIENT_CAMERA;
				return;
			}
			if( app->camera.modifMode == ModificationStates::ORIENT_CAMERA ){
				app->camera.modifMode = ModificationStates::NONE;
				return;
			}
		}
		//camera movements
		bool movedPitch = false;
		bool movedYaw = false;
		if( key == GLFW_KEY_W ){ app->camera.rotatePitch = glm::rotate(glm::mat4(1.0f), 0.25f, app->camera.right );     movedPitch = true;} 
		if( key == GLFW_KEY_A ){ app->camera.rotateYaw = glm::rotate(glm::mat4(1.0f), 0.25f, app->camera.up );          movedYaw = true;}
		if( key == GLFW_KEY_S ){ app->camera.rotatePitch = glm::rotate(glm::mat4(1.0f), -0.25f, app->camera.right );    movedPitch = true;}
		if( key == GLFW_KEY_D ){ app->camera.rotateYaw = glm::rotate(glm::mat4(1.0f), -0.25f, app->camera.up );         movedYaw = true;}
		if( key == GLFW_KEY_SPACE ){
			app->camera.pos.y += app->camera.backupPos.y / 10.0f;
			app->camera.target.y += app->camera.backupPos.y / 10.0f;
			app->globalUBO.camPos = glm::vec3(app->camera.pos);
			app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
			return;
		}
		if( key == GLFW_KEY_X ){
			app->camera.pos.y -= app->camera.backupPos.y / 10.0f;
			app->camera.target.y -= app->camera.backupPos.y / 10.0f;
			app->globalUBO.camPos = glm::vec3(app->camera.pos);
			app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
			return;
		}
	//Reset
		if( key == GLFW_KEY_R ){
			//app->currentMesh->matrices.model = glm::mat4(1.0f);
			app->camera.rotatePitch = glm::mat4(1.0f);
			app->camera.rotateYaw = glm::mat4(1.0f);
			app->camera.pos = glm::vec3(app->camera.backupPos);
			app->globalUBO.camPos = glm::vec3(app->camera.pos);
			app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
			return;
		}
		if( movedPitch ){ app->camera.pos = glm::vec3(app->camera.rotatePitch * glm::vec4(app->camera.pos - app->camera.target, 1.0f)) + app->camera.target; }
		if( movedYaw ){ app->camera.pos = glm::vec3(app->camera.rotateYaw * glm::vec4(app->camera.pos - app->camera.target, 1.0f)) + app->camera.target; }
		if( movedYaw || movedPitch ){
			app->globalUBO.camPos = glm::vec3(app->camera.pos);
			app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
			return;
		}
		
		//speed modifications
		if( key == GLFW_KEY_LEFT_SHIFT ){
			if( app->camera.speed != ModificationStates::SPRINT ){
				app->camera.speed = ModificationStates::SPRINT;
				return;
			}
			else{
				app->camera.speed = ModificationStates::NORMAL;
				return;
			}
		}
		if( key == GLFW_KEY_LEFT_CONTROL ){
			if( app->camera.speed != ModificationStates::WALK ){
				app->camera.speed = ModificationStates::WALK;
				return;
			}
			else{
				app->camera.speed = ModificationStates::NORMAL;
				return;
			}
		}
	}
}

void VkApp::onMouseScroll(GLFWwindow* window, double scrollX, double scrollY){
	VkApp* app = reinterpret_cast<VkApp*>( glfwGetWindowUserPointer(window) );

	if( scrollY < 0 ){ app->camera.pos.z -= app->camera.backupPos.z / 25.0f; }
	if( scrollY > 0 ){ app->camera.pos.z += app->camera.backupPos.z / 25.0f; }
	
	app->globalUBO.camPos = glm::vec3(app->camera.pos);
	app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
}




void VkApp::onCursorMove(GLFWwindow* window, double newX, double newY){
	VkApp* app = reinterpret_cast<VkApp*>( glfwGetWindowUserPointer(window) );

	if( app->camera.modifMode != ModificationStates::NONE ){
		
		if( app->camera.modifMode == ModificationStates::ORIENT_CAMERA ){}//SOON
		if( app->camera.modifMode == ModificationStates::ROTATION ){
			double xPos; double yPos;
			double deadZone = 5.0;
			bool xMoved = false; bool yMoved = false;
			glfwGetCursorPos(window, &xPos, &yPos);

			
			if( app->camera.clickedX < (xPos - deadZone) ){ app->camera.rotateYaw = glm::rotate(glm::mat4(1.0f), 0.1f, app->camera.up ); xMoved = true; }  //left
			if( app->camera.clickedX > (xPos + deadZone) ){ app->camera.rotateYaw = glm::rotate(glm::mat4(1.0f), -0.1f, app->camera.up ); xMoved = true; } //right
			if( app->camera.clickedY > (yPos + deadZone) ){ app->camera.rotatePitch = glm::rotate(glm::mat4(1.0f), 0.1f, app->camera.right ); yMoved = true; } //Right
			if( app->camera.clickedY < (yPos - deadZone) ){ app->camera.rotatePitch = glm::rotate(glm::mat4(1.0f), -0.1f, app->camera.right ); yMoved = true; } //left
	
			if( yMoved ){ app->camera.pos = glm::vec3(app->camera.rotatePitch * glm::vec4(app->camera.pos - app->camera.target, 1.0f)) + app->camera.target; }
			if( xMoved ){ app->camera.pos = glm::vec3(app->camera.rotateYaw * glm::vec4(app->camera.pos - app->camera.target, 1.0f)) + app->camera.target; }
			app->globalUBO.camPos = glm::vec3(app->camera.pos);
			app->globalUBO.view = glm::lookAt(app->camera.pos, app->camera.target, app->camera.up);
			//app->updateMVP();

			glfwSetCursorPos(window, app->camera.clickedX, app->camera.clickedY);
		}
	}
}


void VkApp::updateMVP(){
	for( VkMesh* mesh : meshes ){ mesh->updateMVP(globalUBO.proj, globalUBO.view); }
}

void VkApp::loadAssets(){
	for(size_t i = 0; i < VkApp::MODELS_NAMES.size(); ++i){
		meshes.push_back( new VkMesh(this) );
		meshes.back()->loadMesh( ASSETS_PATHS[i], MODELS_NAMES[i] );
	}
	
	globalUBO.camPos = meshes.at(0)->getCamPos();
	globalUBO.camPos.x /= 2.0f; globalUBO.camPos.y /= 2.0f; globalUBO.camPos.z *= 1.5f;
	camera.pos = glm::vec3(globalUBO.camPos);
	camera.backupPos = glm::vec3(globalUBO.camPos);
	camera.target = glm::vec3(globalUBO.camPos.x,globalUBO.camPos.y, 0.0f);
	
	camera.targetToCam = glm::vec3(camera.pos - camera.target);
	camera.right = glm::normalize(glm::cross(camera.targetToCam, camera.up));

	globalUBO.proj = glm::perspective(glm::radians(FOV), 1.0f, 0.1f, (float)(globalUBO.camPos.z*10.0));
	globalUBO.view = glm::lookAt(camera.pos, camera.target, camera.up);
	globalUBO.proj[1][1] *= -1;

	
	if( meshes.size() > 1 ){
		for(size_t i = 1; i < VkApp::MODELS_NAMES.size(); ++i){
			meshes.at(i)->matrices.model = glm::translate(glm::mat4(1.0f),glm::vec3(globalUBO.camPos.x * i * 0.95f, 0.0f, 0.0f));
		}
	}
	meshes.at(0)->matrices.model = glm::scale( meshes.at(0)->matrices.model, glm::vec3(1.5f));
	meshes.at(2)->matrices.model = glm::scale( meshes.at(2)->matrices.model, glm::vec3(1.25f));
	meshes.at(1)->matrices.model = glm::scale( meshes.at(1)->matrices.model, glm::vec3(0.05f));
	meshes.at(1)->matrices.model = glm::rotate( meshes.at(1)->matrices.model ,glm::radians(-2.5f), glm::vec3(0.0f,0.0f,1.0f) );
	//meshes.at(3)->matrices.model = glm::scale( meshes.at(3)->matrices.model, glm::vec3(0.3f));
	//meshes.at(3)->matrices.model = glm::translate( meshes.at(3)->matrices.model, glm::vec3(0.0f, globalUBO.camPos.y / 0.02f, 0.0f));
	
	updateMVP();
	
	//Lights loading
	lights.pos[1] = glm::vec4(globalUBO.camPos, 1.0f);
	//lights.diffuse[1] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	lights.specular[1] = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	lights.attenuation[1] = glm::vec4(0.0f, 0.1f, 0.0f, 1.0f);
	//lights.spots[1] = glm::vec4(camera.targetToCam, 45.0f);

	lights.pos[4] = glm::vec4(1.0f, 1.0f, 1.0f, -1.0f);
	lights.diffuse[4] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	lights.specular[4] = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	lights.attenuation[4] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void VkApp::recreateSwapChain(){
	vkDeviceWaitIdle(device);

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	createCommandBuffers();
}



void VkApp::createRenderPass(){
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;
	subPass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if( vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS ){
		throw std::runtime_error("failed to create render pass!");
	}
}



void VkApp::createDescriptorSetLayout(){
	//Filling structures for the lights and camera (view + projection) descriptor layout
	std::array<VkDescriptorSetLayoutBinding, Uniforms::SIZE_U> globalBindings = {};
	std::array<VkDescriptorSetLayoutBinding, Uniforms::SIZE_U> uniformsLayoutBinding = {};
	for(uint32_t i = 0; i < Uniforms::SIZE_U; ++i){
		uniformsLayoutBinding[i].binding = i;
		uniformsLayoutBinding[i].descriptorCount = 1;
		uniformsLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformsLayoutBinding[i].pImmutableSamplers = nullptr;
		uniformsLayoutBinding[i].stageFlags = VK_SHADER_STAGE_ALL;//VK_SHADER_STAGE_VERTEX_BIT;

		globalBindings[i] = uniformsLayoutBinding[i];
	}

	//Filling structures for the material descriptor layout
	std::array<VkDescriptorSetLayoutBinding, MaterialUniforms::SIZE_MU> matUniformsLayoutBinding = {};
	std::array<VkDescriptorSetLayoutBinding, (Textures::SIZE_T + MaterialUniforms::SIZE_MU)> texSetBindings = {};
	for(uint32_t i = 0; i < MaterialUniforms::SIZE_MU; ++i){
		matUniformsLayoutBinding[i].binding = i;
		matUniformsLayoutBinding[i].descriptorCount = 1;
		matUniformsLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		matUniformsLayoutBinding[i].pImmutableSamplers = nullptr;
		matUniformsLayoutBinding[i].stageFlags = VK_SHADER_STAGE_ALL;//VK_SHADER_STAGE_VERTEX_BIT;

		texSetBindings[i] = matUniformsLayoutBinding[i];
	}
	
	std::array<VkDescriptorSetLayoutBinding, Textures::SIZE_T> samplersLayoutBinding = {};
	for(uint32_t i = 0; i < Textures::SIZE_T; ++i){
		samplersLayoutBinding[i].binding = i+MaterialUniforms::SIZE_MU;
		samplersLayoutBinding[i].descriptorCount = 1;
		samplersLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplersLayoutBinding[i].pImmutableSamplers = nullptr;
		samplersLayoutBinding[i].stageFlags = VK_SHADER_STAGE_ALL;//VK_SHADER_STAGE_FRAGMENT_BIT;

		texSetBindings[i+MaterialUniforms::SIZE_MU] = samplersLayoutBinding[i];
	}

	//Filling structures for the mesh model and mvp descriptor layout
	VkDescriptorSetLayoutBinding meshBindings;
	meshBindings.binding = 0;
	meshBindings.descriptorCount = 1;
	meshBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	meshBindings.pImmutableSamplers = nullptr;
	meshBindings.stageFlags = VK_SHADER_STAGE_ALL;
	std::array<VkDescriptorSetLayoutBinding, 1> meshUniformsLayoutBinding = {meshBindings};

	
	//Creation of the lights and transforms descriptor layout
	VkDescriptorSetLayoutCreateInfo globalLayoutInfos = {};
	globalLayoutInfos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	globalLayoutInfos.bindingCount = globalBindings.size();
	globalLayoutInfos.pBindings = globalBindings.data();

	if( vkCreateDescriptorSetLayout(device, &globalLayoutInfos, nullptr, &globalDescriptorSetLayout) != VK_SUCCESS ){
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	//Creation of the material descriptor layout
	VkDescriptorSetLayoutCreateInfo materialLayoutInfos = {};
	materialLayoutInfos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	materialLayoutInfos.bindingCount = texSetBindings.size();
	materialLayoutInfos.pBindings = texSetBindings.data();

	if( vkCreateDescriptorSetLayout(device, &materialLayoutInfos, nullptr, &texturesDescriptorSetLayout) != VK_SUCCESS ){
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	//Creation of the mesh model and mvp descriptor layout
	VkDescriptorSetLayoutCreateInfo meshLayoutInfos = {};
	meshLayoutInfos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	meshLayoutInfos.bindingCount = meshUniformsLayoutBinding.size();
	meshLayoutInfos.pBindings = meshUniformsLayoutBinding.data();

	if( vkCreateDescriptorSetLayout(device, &meshLayoutInfos, nullptr, &meshDescriptorSetLayout) != VK_SUCCESS ){
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	BlurDescriptor::createDescriptorSetLayout(this);
}




void VkApp::createGraphicsPipeline(){
	auto vertShaderCode = readFile("./shaders/SPIR-V/phongVert.spirv");
	auto fragShaderCode = readFile("./shaders/SPIR-V/phongFrag.spirv");

	VDeleter<VkShaderModule> vertShaderModule{device, vkDestroyShaderModule};
	VDeleter<VkShaderModule> fragShaderModule{device, vkDestroyShaderModule};
	createShaderModule(vertShaderCode, vertShaderModule);
	createShaderModule(fragShaderCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = VkBufferVertex::getBindingDescription();
	auto attributeDescriptions = VkBufferVertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//VK_POLYGON_MODE_LINE; //VK_POLYGON_MODE_FILL; 
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
		
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDescriptorSetLayout setLayouts[] = {globalDescriptorSetLayout, texturesDescriptorSetLayout, meshDescriptorSetLayout};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = setLayouts;

	if( vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS ){
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if( vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline[Pipelines::PHONG]) != VK_SUCCESS ){
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}




void VkApp::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size(), VDeleter<VkFramebuffer>{device, vkDestroyFramebuffer});

	for( size_t i = 0; i < swapChainImageViews.size(); i++ ){
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if( vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS ){
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}










void VkApp::createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	             uniformStagingBuffer[Uniforms::GLOBAL], uniformStagingBufferMemory[Uniforms::GLOBAL]);
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	             uniformBuffer[Uniforms::GLOBAL], uniformBufferMemory[Uniforms::GLOBAL]);

	VkDeviceSize lightsbufferSize = sizeof(LightSources);
	createBuffer(lightsbufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	             uniformStagingBuffer[Uniforms::LIGHTS], uniformStagingBufferMemory[Uniforms::LIGHTS]);
	createBuffer(lightsbufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	             uniformBuffer[Uniforms::LIGHTS], uniformBufferMemory[Uniforms::LIGHTS]);
}

//purely random numbers for now
void VkApp::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 50;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 90;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 150;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VkApp::createDescriptorSet(){
	VkDescriptorSetLayout layouts[] = {globalDescriptorSetLayout};
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if( vkAllocateDescriptorSets(device, &allocInfo, &globalDescriptorSet) != VK_SUCCESS ){
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	std::array<VkDescriptorBufferInfo, Uniforms::SIZE_U> buffersInfos = {};
	buffersInfos[Uniforms::GLOBAL].buffer = uniformBuffer[Uniforms::GLOBAL];
	buffersInfos[Uniforms::GLOBAL].offset = 0;
	buffersInfos[Uniforms::GLOBAL].range = sizeof(UniformBufferObject);
	
	buffersInfos[Uniforms::LIGHTS].buffer = uniformBuffer[Uniforms::LIGHTS];
	buffersInfos[Uniforms::LIGHTS].offset = 0;
	buffersInfos[Uniforms::LIGHTS].range = sizeof(LightSources);


	std::array<VkWriteDescriptorSet, Uniforms::SIZE_U> descriptorWrites = {};
	for(uint32_t i = 0; i < Uniforms::SIZE_U; ++i){
		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = globalDescriptorSet;
		descriptorWrites[i].dstBinding = i;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pBufferInfo = &buffersInfos[i];//&bufferInfo;
	}

	vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	
	for( VkMesh* mesh : meshes ){ mesh->createDescriptorSet(); }
}







void VkApp::createCommandBuffers() {
	if( commandBuffers.size() > 0 ){
		vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
	}

	commandBuffers.resize( swapChainFramebuffers.size() );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	if( vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS ){
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for( size_t i = 0; i < commandBuffers.size(); ++i ){
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[Pipelines::PHONG]);

		for( VkMesh* mesh : meshes ){ mesh->render(commandBuffers[i]); }
		
		vkCmdEndRenderPass(commandBuffers[i]);

		if( vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS ){
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void VkApp::createSemaphores() {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if( vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		 vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ){

		throw std::runtime_error("failed to create semaphores!");
	}
}

void VkApp::updateUniformBuffer(){
	//static auto startTime = std::chrono::high_resolution_clock::now();	
	//auto currentTime = std::chrono::high_resolution_clock::now();
	//float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
	//globalUBO.model = glm::rotate(glm::mat4(),time * glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	
	updateMVP();
	lights.pos[1] = glm::vec4(globalUBO.camPos,1.0f);
	//lights.spots[1] = glm::vec4(-camera.targetToCam, 45.0f);
	//lights.diffuse[1] = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	//lights.specular[1] = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	//lights.attenuation[1] = glm::vec4(0.0f,0.0,15.0,1.0);

	//lights.pos[4] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	//lights.diffuse[4] = glm::vec4(1.0f, 0.4f, 0.0f, 1.0f);
	//lights.specular[4] = glm::vec4(1.0f, 0.4f, 0.0f, 1.0f);
	//lights.attenuation[4] = glm::vec4(0.0f,0.1,0.01,1.0);



	//Sending data to uniforms buffer
	void* data;
	vkMapMemory(device, uniformStagingBufferMemory[Uniforms::GLOBAL], 0, sizeof(UniformBufferObject), 0, &data);
	memcpy(data, &globalUBO, sizeof(UniformBufferObject));
	//memcpy( &((UniformBufferObject*)data)->mat, &(globalUBO.mat), sizeof(Material));
	vkUnmapMemory(device, uniformStagingBufferMemory[Uniforms::GLOBAL]);
	copyBuffer(uniformStagingBuffer[Uniforms::GLOBAL], uniformBuffer[Uniforms::GLOBAL], sizeof(UniformBufferObject));

	void* lightsData;
	vkMapMemory(device, uniformStagingBufferMemory[Uniforms::LIGHTS], 0, sizeof(LightSources), 0, &lightsData);
	memcpy(lightsData, &lights, sizeof(LightSources));
	vkUnmapMemory(device, uniformStagingBufferMemory[Uniforms::LIGHTS]);
	copyBuffer(uniformStagingBuffer[Uniforms::LIGHTS], uniformBuffer[Uniforms::LIGHTS], sizeof(LightSources));
}


//####################################### STRUCT FRAMEBUFFER #######################################################
FrameBufferAttachment::FrameBufferAttachment(VkApp* vkapp):image(vkapp->device, vkDestroyImage),
                                                           mem(vkapp->device, vkFreeMemory),
                                                           view(vkapp->device, vkDestroyImageView){}

FrameBuffer::FrameBuffer(VkApp* vkapp):app(vkapp),width(),height(),
                                       frameBuffer(vkapp->device, vkDestroyFramebuffer),
                                       color(vkapp), depth(vkapp){}





void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, 
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

void setImageLayout( VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
                     VkImageLayout oldImageLayout, VkImageLayout newImageLayout );


// Setup the offscreen framebuffer for rendering the mirrored scene
// The color attachment of this framebuffer will then be sampled from
void VkApp::prepareOffscreenFramebuffer(FrameBuffer* frameBuf, VkCommandBuffer cmdBuffer){
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
	
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;

	VkMemoryRequirements memReqs;
	
	VkImageViewCreateInfo colorImageView = {};
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

	
	
	if( vkCreateImage(device, &image, nullptr, &frameBuf->color.image) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to create image");
	}
	vkGetImageMemoryRequirements( device, frameBuf->color.image, &memReqs );
	
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType( memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	if( vkAllocateMemory( device, &memAlloc, nullptr, &frameBuf->color.mem) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to allocate memory");
	}
	if( vkBindImageMemory( device, frameBuf->color.image, frameBuf->color.mem, 0) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to bind image memory");
	}

	// Set the initial layout to shader read instead of attachment 
	// This is done as the render loop does the actualy image layout transitions
	setImageLayout( cmdBuffer, frameBuf->color.image,
	                VK_IMAGE_ASPECT_COLOR_BIT,
	                VK_IMAGE_LAYOUT_UNDEFINED,
	                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	
	colorImageView.image = frameBuf->color.image;
	if( vkCreateImageView( device, &colorImageView, nullptr, &frameBuf->color.view) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to create image view");
	}

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
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	if( vkCreateImage(device, &image, nullptr, &frameBuf->depth.image) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to create image");
	}
	vkGetImageMemoryRequirements( device, frameBuf->depth.image, &memReqs );
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if( vkAllocateMemory( device, &memAlloc, nullptr, &frameBuf->depth.mem ) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to allocate device memory");
	}
	if( vkBindImageMemory( device, frameBuf->depth.image, frameBuf->depth.mem, 0 ) != VK_SUCCESS ){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to bind image memory");
	}

	setImageLayout( cmdBuffer, frameBuf->depth.image,
	                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
	                VK_IMAGE_LAYOUT_UNDEFINED,
	                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
	depthStencilView.image = frameBuf->depth.image;
	if( vkCreateImageView(device, &depthStencilView, nullptr, &frameBuf->depth.view) != VK_SUCCESS){
		throw std::runtime_error("prepareOffscreenFramebuffer -- failed to create image view");
	}

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

	if( vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &frameBuf->frameBuffer) != VK_SUCCESS ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffer -- failed to create frameBuffer");
	}
}




// Sets up the command buffer that renders the scene to the offscreen frame buffer
// The blur method used in this example is multi pass and renders the vertical
// blur first and then the horizontal one.
// While it's possible to blur in one pass, this method is widely used as it
// requires far less samples to generate the blur
void VkApp::prepareOffscreenFramebuffers(){
	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	if( vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &cmdBuffer) != VK_SUCCESS ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to allocate command buffer");
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	if( vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo) != VK_SUCCESS ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to begin command buffer");
	}

	
	prepareOffscreenFramebuffer(&offScreenFrameBuf, cmdBuffer);
	prepareOffscreenFramebuffer(&offScreenFrameBufB, cmdBuffer);

	if( vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to end command buffer");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	if( vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to submit to queue");
	}
	if( vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to wait idle");
	}
	vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);

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
	if( vkCreateSampler(device, &sampler, nullptr, &colorSampler) ){
		throw std::runtime_error("VkApp::prepareOffscreenFramebuffers -- failed to create sampler");
	}	
}




uint32_t VkApp::getMemoryType( uint32_t typeBits, VkFlags properties ){
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	
	for( uint32_t i = 0; i < 32; ++i ){
		if( (typeBits & 1) == 1 ){
			if( (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties ){
				return i;
			}
		}
		typeBits >>= 1;
	}

	throw std::runtime_error("getMemoryType -- failed to find memory type");
	return 0;
}



void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, 
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange){
	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch( oldImageLayout ){
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source 
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		std::cout << "setImageLayout -- unsupported image layout" << std::endl;
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch( newImageLayout ){
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
		
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from and writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if( imageMemoryBarrier.srcAccessMask == 0 ){
			//imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;//NOT WORKING
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		std::cout << "setImageLayout -- unsupported image layout" << std::endl;
		break;
	}
	
	// Put barrier on top
	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier( cmdbuffer, srcStageFlags, destStageFlags, 
	                      0, 
	                      0, nullptr,
	                      0, nullptr,
	                      1, &imageMemoryBarrier);
}


// Fixed sub resource on first mip level and layer
void setImageLayout( VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
                     VkImageLayout oldImageLayout, VkImageLayout newImageLayout ){
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	setImageLayout(cmdbuffer, image, aspectMask, oldImageLayout, newImageLayout, subresourceRange);
}
