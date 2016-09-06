#include "glMesh.hpp"

using namespace std;

int width, height;
//vector<glm::vec3>* vertices = new vector<glm::vec3>();
glMesh* mtest;

glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 mvp;

GLfloat rotationStep = 0.10;
GLfloat FOV = 30.0;
GLfloat fovStep = 1.0;
glm::vec3 camPos; glm::vec3 lightPos;
glm::vec3 moveCamStep;
GLfloat zCoord = 0.0;

static bool quitting = false;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;



void init(){
	//mtest = new glMesh("../assets/OFF/buddha.obj");
	//mtest = new glMesh("../assets/OFF/bunny.obj");
	//mtest = new glMesh("../assets/OFF/max.obj");
	//mtest = new glMesh("../assets/OFF/triceratops.obj");
	//mtest = new glMesh("../assets/OFF/MeshSegmentation.obj");
	//mtest = new glMesh("../assets/OFF/cube.obj");

	//mtest = new glMesh("../assets/flowers/ARose.obj");
	//mtest = new glMesh("../assets/flowers/statue.obj");
	//mtest = new glMesh("../assets/flowers/fireFlower.obj");
	//mtest->png_texture_load("../assets/flowers/fire.png",NULL,NULL);
	//mtest = new glMesh("../assets/flowers/Flower.obj");
	//mtest->png_texture_load("../assets/flowers/Flower_Texture.png",NULL,NULL);
	//mtest = new glMesh("../assets/flowers/rose.obj");
	//mtest->loadTexture("../assets/flowers/rose.png");

	//mtest = new glMesh("../assets/mecha/MechaSonic.obj");
	//mtest = new glMesh("../assets/mecha/dalek.obj");
	//mtest = new glMesh("../assets/mecha/mechaFlower.obj");
	//mtest = new glMesh("../assets/mecha/T-51b_Power_Armor.obj");
	//mtest = new glMesh("../assets/mecha/Gaige.obj");

	//mtest = new glMesh("../assets/chalet/chalet2.obj");

	mtest = new glMesh("../assets/Clank/ClankT.obj");
	//mtest->png_texture_load("../assets/Clank/Texture0.png",NULL,NULL);

	//mtest = new glMesh("../assets/rungholt/rungholtNoNeg.obj");
	//mtest->loadTexture("../assets/rungholt/house-RGBA.png");
	
	//mtest = new glMesh("../assets/Melynx/Melynx.obj");
	//mtest->png_texture_load("../assets/Melynx/Melynx.png",NULL,NULL);

	//mtest = new glMesh("../assets/Mush/Mush.obj");
	//mtest->png_texture_load("../assets/Mush/oddMush.png",NULL,NULL);

	//mtest = new glMesh("../assets/Mush/big.obj");
	//mtest->png_texture_load("../assets/Mush/big.png",NULL,NULL);

	//mtest->fillAretesVives( M_PI / 12 );
	//mtest->segmenter( );
	//mtest->subdiviser();
	//mtest->subdiviser();


	camPos = mtest->defView();
	lightPos = glm::vec3(camPos.x,camPos.y,camPos.z);

	moveCamStep = glm::vec3(camPos.x / 25.0,camPos.y / 25.0,camPos.z / 25.0);
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(FOV), 1.0f, 0.1f, (GLfloat)(camPos.z*10.0));
	//Projection = glm::ortho(0.0f,(GLfloat)w,(GLfloat)h,0.0f,0.1f,100.0f); // In world coordinates

	// Camera matrix
	zCoord = camPos.z;
	View = glm::lookAt(
				glm::vec3(camPos), // Camera is at (4,4,4), in World Space
				glm::vec3(0,camPos.y,0), // and looks at the origin
				glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
				);

	// Model matrix : an identity matrix (model will be at the origin)
	Model =  glm::mat4(1.0f);

	 // Our ModelViewProjection : multiplication of our 3 matrices
	 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

	 // Translation
	 //   Model = glm::translate(Model, glm::vec3(0.1f, 0.2f, 0.5f));
	 //   Rotation around Oz with 45 degrees
	 //   Model = glm::rotate(Model, 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	 //TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector;

	mtest->initShader(&Model,&View,&Projection,&camPos,&lightPos,&mvp);
	glEnable(GL_DEPTH_TEST);  // Active le test de profondeur
	//n'affiche pas les triangles qui sont derriere un maillage fermé (normales qui ne font pas face a la camera)
	glEnable(GL_CULL_FACE);
	//glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
}




void render() {
	SDL_GL_MakeCurrent(window, gl_context);

	//glClearColor(0.15,0.15,0.15,0);
	glClearColor(0.0,0.0,0.0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	
	//glPointSize(5.0);
	mtest->draw();

	//mtest->testDraw();
	//mtest->drawAretesVives();
	//mtest->drawSeg();
	//mtest->drawEdges();
	//mtest->drawGaussImg();
	
	glPopMatrix();
	glFlush();
	SDL_GL_SwapWindow(window);
}


int SDLCALL watch(void *userdata, SDL_Event* event) {
	if( event->type == SDL_APP_WILLENTERBACKGROUND ){ quitting = true; }
	return 1;
}

int main(int argc, char *argv[]) {
	SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("openGL to Vulkan drama", SDL_WINDOWPOS_UNDEFINED,
    									SDL_WINDOWPOS_UNDEFINED,
										800, 600, SDL_WINDOW_OPENGL);

    gl_context = SDL_GL_CreateContext(window);
    SDL_AddEventWatch(watch, NULL);

    glewInit();
    init();
    while(!quitting) {
        while( SDL_PollEvent(&event) ) {
            if(event.type == SDL_QUIT) { quitting = true; }
            if(event.type == SDL_KEYDOWN){
            	if(event.key.keysym.sym == SDLK_z){ mtest->simplifier(0.03); }
            	if(event.key.keysym.sym == SDLK_u){ mtest->catmull_Clark(); }
            	if(event.key.keysym.sym == SDLK_UP){
            		Model = glm::rotate(Model, rotationStep, glm::vec3(1.0f, 0.0f, 0.0f));
            	}
            	if(event.key.keysym.sym == SDLK_DOWN){
            		Model = glm::rotate(Model, -rotationStep, glm::vec3(1.0f, 0.0f, 0.0f));
            	}
            	if(event.key.keysym.sym == SDLK_LEFT){
            		Model = glm::rotate(Model, -rotationStep, glm::vec3(0.0f, 1.0f, 0.0f));
            	}
            	if(event.key.keysym.sym == SDLK_RIGHT){
            		Model = glm::rotate(Model, rotationStep, glm::vec3(0.0f, 1.0f, 0.0f));
            	}
            	if(event.key.keysym.sym == SDLK_e){
                	camPos.z -= moveCamStep.z;
                	View = glm::lookAt(
                					glm::vec3(camPos),
                					glm::vec3(0,camPos.y,0),
                					glm::vec3(0,1,0)
                					);
            	}
            	if(event.key.keysym.sym == SDLK_q){
                	camPos.z += moveCamStep.z;
                	View = glm::lookAt(
                					glm::vec3(camPos),
                					glm::vec3(0,camPos.y,0),
                					glm::vec3(0,1,0)
                					);
            	}
            	if(event.key.keysym.sym == SDLK_r){
            		camPos = mtest->defView();
                	View = glm::lookAt(glm::vec3(camPos), // Camera is at (4,4,4), in World Space
                	    			   glm::vec3(0,camPos.y,0), // and looks at the origin
                	    			   glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                	    			   );
                	Model = glm::mat4(1.0f);
            	}
            	if(event.key.keysym.sym == SDLK_t){
            		mtest->toggleWireframe();
            	}
            	mvp = Projection * View * Model;
            }
            if(event.type == SDL_MOUSEWHEEL){
            	if (event.wheel.y < 0){
                   	camPos.z += moveCamStep.z;
                    View = glm::lookAt( glm::vec3(camPos),
                    					glm::vec3(0,camPos.y,0),
                    					glm::vec3(0,1,0)
                    					);
            	}
            	else{
                   	camPos.z -= moveCamStep.z;
                    View = glm::lookAt( glm::vec3(camPos),
                    					glm::vec3(0,camPos.y,0),
                    					glm::vec3(0,1,0)
                    					);
            	}
            	mvp = Projection * View * Model;
            }
        }

        render();
        SDL_Delay(2);
    }

    SDL_DelEventWatch(watch, NULL);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(0);

}


