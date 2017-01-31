#ifndef GLMESH_HPP
#define GLMESH_HPP

#include "glPipeline.hpp"





//########################## glMesh ############################################

enum BufferIndex{
	POS,
	UV,
	NORM,
	INDEX,
	LAST
};


class glMesh : public Mesh{
protected:
	GLuint VAO;
	GLuint VBO[BufferIndex::LAST];
	GLuint texture;
	GLenum displayMode;
	GLenum mode;
	glPipeline* material;


	std::vector< glm::vec3 >* GPUposition;
	std::vector< glm::vec2 >* GPUtexCoord;
	std::vector< glm::vec3 >* GPUnormal;
	
public:
	virtual void GLContext();
	virtual void CPUtoGPU();

	glMesh();
	glMesh(const char*);
	virtual ~glMesh();

	virtual unsigned int loadTexture(const char*);
	virtual void initShader(glm::mat4*, glm::mat4*, glm::mat4*,glm::vec3*,glm::vec3*,glm::mat4*);
	virtual void toggleWireframe();
	virtual void draw();
	virtual void testDraw();
	virtual void drawGaussImg()const;
	virtual void drawEdges();
	virtual void drawAretesVives();
	virtual void drawSeg()const;
	virtual void setColorFlag(int);

	
};


#endif
