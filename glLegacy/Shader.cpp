

#include "Shader.hpp"

using namespace std;


//######################### CLASSE SHADER ######################################

Shader::Shader(const string& fileName) {
	programID = glCreateProgram();
	colorFlag = 0;
	shaders[0] = CreateShader( LoadShader(fileName + ".vs"), GL_VERTEX_SHADER);
	shaders[1] = CreateShader( LoadShader(fileName + ".fs"), GL_FRAGMENT_SHADER);

	for( unsigned int i = 0; i < ShaderIndex::LASTS; ++i ){
		glAttachShader(programID, shaders[i]);
	}

	//glBindAttribLocation(programID, 0, "position");
	//glBindAttribLocation(programID, 1, "texCoord");
	//glBindAttribLocation(programID, 2, "normal");


	glLinkProgram(programID);
	CheckShaderError(programID, GL_LINK_STATUS, true, "Error linking shader program");

	//Derniere validation pour etre sur que le programme est utilisable
	glValidateProgram(programID);
	CheckShaderError(programID, GL_LINK_STATUS, true, "Invalid shader program");

	uniforms[UniformIndex::MODEL]      = glGetUniformLocation(programID, "model");
	uniforms[UniformIndex::VIEW]       = glGetUniformLocation(programID, "view");
	uniforms[UniformIndex::PROJECTION] = glGetUniformLocation(programID, "projection");
	uniforms[UniformIndex::VIEWPOS]    = glGetUniformLocation(programID, "viewPos");
	uniforms[UniformIndex::LIGHTPOS]   = glGetUniformLocation(programID, "lightPos");
	uniforms[UniformIndex::MVP]        = glGetUniformLocation(programID, "MVP");




}

Shader::~Shader() {
	for( unsigned int i = 0; i < ShaderIndex::LASTS; ++i){
	    glDetachShader(programID, shaders[i]);
	    glDeleteShader(shaders[i]);
	}
	glDeleteProgram(programID);
}

void Shader::init(glm::mat4* m, glm::mat4* v, glm::mat4* p,glm::vec3* vp,glm::vec3* lp, glm::mat4* mvp){
	Model = m; View = v; Projection = p;
	viewPos = vp; lightPos = lp;
	MVP = mvp;
}



void Shader::bind(){
	glUseProgram(programID);

	glUniformMatrix4fv(uniforms[UniformIndex::MODEL], 1, GL_FALSE, &(*Model)[0][0]);
	glUniformMatrix4fv(uniforms[UniformIndex::VIEW], 1, GL_FALSE, &(*View)[0][0]);
	glUniformMatrix4fv(uniforms[UniformIndex::PROJECTION], 1, GL_FALSE, &(*Projection)[0][0]);
	glUniform3fv(uniforms[UniformIndex::VIEWPOS], 1, &(*viewPos)[0]);
	glUniform3fv(uniforms[UniformIndex::LIGHTPOS], 1, &(*lightPos)[0]);
	glUniformMatrix4fv(uniforms[UniformIndex::MVP], 1, GL_FALSE, &(*MVP)[0][0]);
	glUniform1i(uniforms[UniformIndex::COLORFLAG],colorFlag);

}

std::string Shader::LoadShader(const std::string& fileName){

    std::ifstream file;
    file.open((fileName).c_str());

    std::string output;
    std::string line;
    if(file.is_open()){
        while(file.good()){
            getline(file, line);
			output.append(line + "\n");
        }
    }
    else{
		std::cerr << "Unable to load shader: " << fileName << std::endl;
    }

    file.close();
    return output;
}


void Shader::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage){
    GLint success = 0;
    GLchar error[1024] = { 0 };

    if(isProgram){ glGetProgramiv(shader, flag, &success); }
    else{ glGetShaderiv(shader, flag, &success); }

    if(success == GL_FALSE){
        if(isProgram){ glGetProgramInfoLog(shader, sizeof(error), NULL, error); }
        else{ glGetShaderInfoLog(shader, sizeof(error), NULL, error); }

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}

GLuint Shader::CreateShader(const std::string& text, unsigned int type){
	GLuint shader = glCreateShader(type);

    if(shader == 0){
		std::cerr << "Error compiling shader type " << type << std::endl;
    }
    //Utilisation de tableaux car openGL permet de prendre plusieurs sources a la fois
    const GLchar* p[1];
    p[0] = text.c_str();
    GLint lengths[1];
    lengths[0] = text.length();

    glShaderSource(shader, 1, p, lengths);
    glCompileShader(shader);

    CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error compiling shader");

    return shader;
}

GLuint Shader::getProg()const{ return programID; }
void Shader::setColorFlag(GLint f){ colorFlag = f; }


//#########################################################################################
