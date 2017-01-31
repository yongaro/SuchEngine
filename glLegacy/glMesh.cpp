#include "glMesh.hpp"
#include <png.h>
//#include <pthread.h>


using namespace std;








//############################### CLASSE GLMESH ###########################################
void glMesh::GLContext(){
	glGenVertexArrays(1, &VAO); // Créer le VAO
	glBindVertexArray(VAO); // Lier le VAO pour l'utiliser

	glGenBuffers(BufferIndex::LAST, VBO); // Générations des VBO

	//Position des points
	glBindBuffer(GL_ARRAY_BUFFER, VBO[POS]); // Lier le VBO
	// Définir la taille, les données et le type du VBO
	glBufferData(GL_ARRAY_BUFFER, GPUposition->size() * sizeof(glm::vec3), &GPUposition->front(), GL_STATIC_DRAW); 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); // Définir le pointeur d'attributs des sommets

	//Positions sur la texture
	glBindBuffer(GL_ARRAY_BUFFER, VBO[UV]); // Lier le VBO
	// Définir la taille, les données et le type du VBO
	glBufferData(GL_ARRAY_BUFFER, GPUtexCoord->size() * sizeof(glm::vec3), &GPUtexCoord->front(), GL_STATIC_DRAW); 
	glEnableVertexAttribArray(1);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0); // Définir le pointeur d'attributs pour texture

	//Normale du point
	glBindBuffer(GL_ARRAY_BUFFER, VBO[NORM]); // Lier le VBO
	// Définir la taille, les données et le type du VBO
	glBufferData(GL_ARRAY_BUFFER, GPUnormal->size() * sizeof(glm::vec3), &GPUnormal->front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2); 
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0); // Définir le pointeur d'attributs des normales

	//Indexation
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDEX]);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(model.indices[0]) * model.indices.size(), &model.indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0); // Désactiver le VBO
}

void glMesh::CPUtoGPU(){
	delete GPUposition;
	delete GPUtexCoord;
	delete GPUnormal;

	GPUposition = new vector<glm::vec3>();
	GPUtexCoord = new vector<glm::vec2>();
	GPUnormal = new vector<glm::vec3>();

	for(Face* face: *CPUFaces){
		GPUposition->push_back(glm::vec3( *(face->pts->at(0)->coord)) );
		GPUposition->push_back(glm::vec3( *(face->pts->at(1)->coord)) );
		GPUposition->push_back(glm::vec3( *(face->pts->at(2)->coord)) );

		GPUtexCoord->push_back( glm::vec2( *(face->uv->at(face->pts->at(0)))) );
		GPUtexCoord->push_back( glm::vec2( *(face->uv->at(face->pts->at(1)))) );
		GPUtexCoord->push_back( glm::vec2( *(face->uv->at(face->pts->at(2)))) );

		GPUnormal->push_back( glm::vec3( *(face->nrm->at(face->pts->at(0)))) );
		GPUnormal->push_back( glm::vec3( *(face->nrm->at(face->pts->at(1)))) );
		GPUnormal->push_back( glm::vec3( *(face->nrm->at(face->pts->at(2)))) );

	}
	GLContext();
}


glMesh::glMesh(): Mesh(),
						GPUposition( new vector<glm::vec3>() ),
						GPUtexCoord( new vector<glm::vec2>() ),
						GPUnormal( new vector<glm::vec3>() )
{

		VAO = 0; texture = 0; displayMode = GL_TRIANGLES; mode = GL_FILL; material = new glPipeline("./shaders/phong");
}

glMesh::glMesh(const char* path):Mesh(),
											GPUposition( new vector<glm::vec3>() ),
											GPUtexCoord( new vector<glm::vec2>() ),
											GPUnormal( new vector<glm::vec3>() )
{
	VAO = 0; texture = 0; displayMode = GL_TRIANGLES; mode = GL_FILL; material = new glPipeline("./shaders/phong");
	loadOBJ(path);
}

glMesh::~glMesh(){
	delete GPUposition;
	delete GPUtexCoord;
	delete GPUnormal;

	glDeleteBuffers(BufferIndex::LAST, VBO);
	glDeleteVertexArrays(1, &VAO);
}


unsigned int glMesh::loadTexture(const char * file_name){
    int* width = NULL; int* height = NULL;
    png_byte header[8];


    FILE *fp = fopen(file_name, "rb");
    if (fp == 0){
        perror(file_name);
        return 0;
    }

    // read the header
    int headread = fread(header, 1, 8, fp);
    if( headread < 0 ){ printf("png Header reading problem \n"); }

    if (png_sig_cmp(header, 0, 8)){
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr){
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr){
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info){
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data;
    image_data = (unsigned char*)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL){
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep* row_pointers = (unsigned char**)malloc(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL){
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    unsigned int i;
    for (i = 0; i < temp_height; i++){
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL texture object
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp_width, temp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    this->texture = texture;
    return texture;
}

void glMesh::initShader(glm::mat4* m, glm::mat4* v, glm::mat4* p,glm::vec3* vp,glm::vec3* lp,glm::mat4* mvp){
	material->init(m,v,p,vp,lp,mvp);
}


void glMesh::setColorFlag(int f){ material->setColorFlag(f); }
void glMesh::toggleWireframe(){
	if( mode == GL_FILL ){ mode = GL_LINE; }
	else{ mode = GL_FILL; }
}
void glMesh::draw(){
	material->bind();
	glPolygonMode(GL_FRONT_AND_BACK, mode);
	glBindVertexArray(VAO); // Lier le VAO
	glDrawArrays(displayMode, 0, GPUposition->size()); // Dessiner le VBO
	glBindVertexArray(0); // Délier le VAO
}
void glMesh::testDraw(){
	material->bind();
	glLineWidth(1.0);
	glBegin(GL_LINE_STRIP);
	for(glm::vec3& v : *GPUposition){
		glVertex3f(v.x,v.y,v.z);
	}
	glEnd();

}
void glMesh::drawGaussImg()const{
	material->bind();
	glColor3f(1.0,1.0,1.0);
	//VertexSize(2.0);
	glBegin(GL_POINTS);
	for(glm::vec3 n : *GPUnormal){
		glVertex3f(n.x,n.y,n.z);
	}
	glEnd();
}
void glMesh::drawEdges(){
	material->bind();
	glm::vec3* p1; glm::vec3* p2;
	//cout << CPUEdges->size() << endl;
	for(unsigned int i = 0; i < CPUEdges->size(); ++i){
		p1 =  CPUEdges->at(i)->p1->coord;
		p2 =  CPUEdges->at(i)->p2->coord;
		//glColor3f(0.0,0.0,0.0);
		glLineWidth(2.0);
		glBegin(GL_LINES);
		glVertex3f(p1->x,p1->y,p1->z);
		glVertex3f(p2->x,p2->y,p2->z);
		glEnd();
	}

}
void glMesh::drawAretesVives(){
	material->bind();
	glm::vec3* p1; glm::vec3* p2;

	for(unsigned int i = 0; i < aretesVives->size(); ++i){
 		p1 = aretesVives->at(i)->p1->coord;
		p2 = aretesVives->at(i)->p2->coord;
		glColor3f(1.0,0.0,0.0);
		glLineWidth(5.0);
		glBegin(GL_LINES);
		glVertex3f(p1->x,p1->y,p1->z);
		glVertex3f(p2->x,p2->y,p2->z);
		glEnd();
	}
}
void glMesh::drawSeg()const{
	material->bind();
	glm::vec3* p1; glm::vec3* p2; glm::vec3* p3;

	for(size_t i = 0; i < CPUFaces->size(); ++i ){
		p1 = CPUFaces->at(i)->pts->at(0)->coord;
		p2 = CPUFaces->at(i)->pts->at(1)->coord;
		p3 = CPUFaces->at(i)->pts->at(2)->coord;

		glColor3f(GPUcolorMap[i]->x,GPUcolorMap[i]->y,GPUcolorMap[i]->z);
		glBegin(GL_TRIANGLES);
		glVertex3f(p1->x,p1->y,p1->z);
		glVertex3f(p2->x,p2->y,p2->z);
		glVertex3f(p3->x,p3->y,p3->z);
		glEnd();
	}

}
