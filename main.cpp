#include "VkContext.hpp"

uint32_t VkApp::WIDTH = 640;
uint32_t VkApp::HEIGHT = 480;
std::vector<std::string> VkApp::ASSETS_PATHS;
std::vector<std::string> VkApp::MODELS_NAMES;


std::string VkApp::DEFAULT_TEXTURE = "./assets/default_texture.png";

int main(){
	VkApp app;
	
	VkApp::ASSETS_PATHS.push_back("./assets/wall/");
	VkApp::MODELS_NAMES.push_back("Wall.obj");

	VkApp::ASSETS_PATHS.push_back("./assets/WII_U/Eevee/");
	VkApp::MODELS_NAMES.push_back("Eevee.obj");

	VkApp::ASSETS_PATHS.push_back("./assets/WII_U/classic_sonic/");
	VkApp::MODELS_NAMES.push_back("classic_sonic.dae");
	
	//VkApp::ASSETS_PATHS.push_back("./assets/gw2/");
	//VkApp::MODELS_NAMES.push_back("turtle.obj");

	//VkApp::ASSETS_PATHS.push_back("./assets/gems/");
	//VkApp::MODELS_NAMES.push_back("crystal.obj");

	//VkApp::ASSETS_PATHS.push_back("./assets/Chaos Emeralds/");
	//VkApp::MODELS_NAMES.push_back("Green Chaos Emerald.obj");
	
	try { app.run(); }
	catch( const std::runtime_error& e ){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
