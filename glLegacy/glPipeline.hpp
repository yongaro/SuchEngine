#ifndef GLPIPELINE_HPP
#define GLPIPELINE_HPP

#include "Mesh.hpp"


#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <algorithm>



//##################### glpipeline ##################################

enum UniformIndex{
	MODEL,
	VIEW,
	PROJECTION,
	VIEWPOS,
	LIGHTPOS,
	MVP,
	COLORFLAG,
	LASTU
};

enum ShaderIndex{
	VERTEX,
	FRAG,
	LASTS
};


class glPipeline {
protected:
 GLuint programID;

 glm::mat4* Model;
 glm::mat4* View;
 glm::mat4* Projection;
 glm::vec3* viewPos;
 glm::vec3* lightPos;
 glm::mat4* MVP;
 GLint colorFlag;

 GLuint shaders[ShaderIndex::LASTS];
 GLuint uniforms[UniformIndex::LASTU];
public:
	glPipeline(const std::string&);
	virtual ~glPipeline();
	void init(glm::mat4*, glm::mat4*, glm::mat4*,glm::vec3*,glm::vec3*,glm::mat4*);
	void bind();
	std::string LoadShader(const std::string&);
	void CheckShaderError(GLuint, GLuint, bool, const std::string&);
	GLuint CreateShader(const std::string&, unsigned int);
	GLuint getProg()const;
	void setColorFlag(GLint);
	GLint getColorFlag()const;
};


#endif
