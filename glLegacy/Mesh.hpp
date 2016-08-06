#ifndef Mesh_HPP
#define Mesh_HPP

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



#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Mesh;
struct Face;
struct Edge;


struct Vertex{
	glm::vec3* coord;
	std::vector<glm::vec2*>* uv;
	std::vector<glm::vec3*>* nrm;
	std::vector<Edge*>* edges;
	unsigned int index;

	Vertex(glm::vec3* crd):coord(crd),
							uv( new std::vector<glm::vec2*>() ),
							nrm( new std::vector<glm::vec3*>() ),
							edges( new std::vector<Edge*>() ),
							index(0){}
	~Vertex(){}
	bool equals(Vertex*)const;
	bool equalsLoc(Vertex*)const;
	Edge* existEdge(Vertex*);
	Edge* addEdge(Vertex*,Mesh*);
	bool addNrm(glm::vec3*);
	bool addUV(glm::vec2*);
	glm::vec3* averageNrm();
	void catmullClarkMove(glm::vec3*,glm::vec3*);
	std::vector<Face*>* getNbhFaces();
};



struct Edge{
	Vertex* p1;
	Vertex* p2;
	Face* f1;
	Face* f2;
	bool sharp;
	unsigned int index;

	Edge(Vertex*, Vertex*);
	~Edge();
	void add(Face*);
	Face* getNeighb(Face*);
	bool equals(Edge* ,std::vector<glm::vec3>* );
	Vertex* getCatmullClark(Vertex*, Vertex*);
};



struct Face{
	std::vector<Vertex*>* pts;
	std::unordered_map<Vertex*,glm::vec3*>* nrm;
	std::unordered_map<Vertex*,glm::vec2*>* uv;
	Edge* e1;
	Edge* e2;
	Edge* e3;
	unsigned int index;

	Face( Edge*, Edge*, Edge* ,Vertex*, Vertex*, Vertex*);

	Face(Face& f){
		pts = new std::vector<Vertex*>();
		nrm = new std::unordered_map<Vertex*,glm::vec3*>();
		uv = new std::unordered_map<Vertex*,glm::vec2*>();
		for(Vertex* p : *(f.pts)){
			pts->push_back( new Vertex(*p) );
		}
		e1 = f.e1; e2 = f.e2; e3 = f.e3;
		index = f.index;
	}
	void add(Vertex*);
	bool addUV(Vertex*, glm::vec2*);
	bool addNrm(Vertex*, glm::vec3*);
	std::vector<Face*>* getneighb();
	glm::vec3 getNormal();
	float diedre(Face& );
	Edge* getCommonEdge( Face& );
	bool delimiter( glm::vec3*, glm::vec3** );
	Vertex* getOpp( Edge* );
	Edge* getEdge(Vertex*,Vertex*);
	Vertex* getCatmullClark();
};



class Mesh {
protected:
	std::vector< Vertex* >* CPUPoints;
	std::vector< Edge* >* CPUEdges;
	std::vector< Face* >* CPUFaces;
	std::vector< Edge* >* aretesVives;
	glm::vec3** GPUcolorMap;
	


public:
	Mesh();
	Mesh(const char*);
	virtual ~Mesh();

	virtual void CPUtoGPU();//
	virtual glm::vec3 defView();
	virtual bool loadOBJ(const char*);
	virtual void calcNormals();
	

	virtual void addPoint(Vertex*);
	virtual void addEdge(Edge*);
	virtual void addFace(Face*);

	virtual void addEdges(Edge**,size_t);
	static void* getEdgeRef(void*);

	virtual void addSharpEdge(Edge*);
	virtual void fillAretesVives(float);
	virtual void segmenter();
	virtual std::vector<Vertex*>* getEdgeNbh(Vertex*)const;
	//virtual void setColorFlag(GLint);


	virtual void simplifier(float);
	virtual void checkHoles();

	virtual void catmull_Clark();

	friend class Edge;
	friend class Face;
};



namespace MeshTools{
	struct ThreadWrapper{
		Mesh* ref;
		void* toAdd;
		bool newRes;
		ThreadWrapper(Mesh* r,void* ta):ref(r),toAdd(ta),newRes(false){}
	};
}
#endif
