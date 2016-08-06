#include "Mesh.hpp"
#include <png.h>
#include <pthread.h>


using namespace std;

//############################### STRUCT Vertex ###############################################


bool Vertex::equals(Vertex* ref)const{
	return ((coord == ref->coord) && (uv == ref->uv) && (nrm == ref->nrm) );
}
bool Vertex::equalsLoc(Vertex* ref)const{
	return (coord == ref->coord);
}

Edge* Vertex::existEdge(Vertex* other){
	for(Edge* e : *edges){
		if(e->p1 == this && e->p2 == other){ return e; }
		if(e->p2 == this && e->p1 == other){ return e; }
	}
	return NULL;
}

Edge* Vertex::addEdge(Vertex* ref,Mesh* toAdd){
	Edge* res = existEdge(ref);
	if(res == NULL){
		res = new Edge(this,ref);
		edges->push_back(res);
		ref->edges->push_back(res);
		toAdd->addEdge(res);
	}
	return res;
}

bool Vertex::addNrm(glm::vec3* ref){
	for(glm::vec3* n : *nrm){
		if(n == ref){ return false; }
	}
	nrm->push_back(ref);
	return true;
}
bool Vertex::addUV(glm::vec2* ref){
	for(glm::vec2* u : *uv){
		if(u == ref){ return false; }
	}
	uv->push_back(ref);
	return true;
}



//############################### STRUCT EDGE ###############################################


Edge::Edge(Vertex* a, Vertex* b){
		p1 = a; p2 = b;
		f1 = NULL; f2 = NULL;
		sharp = false;
		index = 0;
	}
Edge::~Edge(){}

void Edge::add(Face* t){
	if( f1 == NULL ){ f1 = t; }
	else if( f2 == NULL ){ f2 = t; }
}

Face* Edge::getNeighb(Face* ref){
	if(f1 != ref){ return f1; }
	if(f2 != ref){ return f2; }
	return NULL;
}

bool Edge::equals(Edge* edg,std::vector<glm::vec3>* pts){
	return (p1 == edg->p1 && p2 == edg->p2) ;
}



//############################### STRUCT FACE ###############################################



Face::Face( Edge* a, Edge* b, Edge* c,  Vertex* p1, Vertex* p2, Vertex* p3 ){
		pts = new std::vector<Vertex*>();
		nrm = new std::unordered_map<Vertex*,glm::vec3*>();
		uv = new std::unordered_map<Vertex*,glm::vec2*>();
		e1 = a; e2 = b; e3 = c;
		pts->push_back(p1); pts->push_back(p2); pts->push_back(p3);
		a->add(this); b->add(this); c->add(this);
		index = 0;
	}

void Face::add(Vertex* p){ pts->push_back(p); }

bool Face::addUV(Vertex* refP, glm::vec2* refU){
	auto search = uv->find(refP);
	if(search == uv->end() ){
		uv->emplace(refP,refU);
		return true;
	}
	return false;
}
bool Face::addNrm(Vertex* refP, glm::vec3* refN){
	auto search = nrm->find(refP);
	if(search == nrm->end() ){
		nrm->emplace(refP,refN);
		return true;
	}
	return false;
}



std::vector<Face*>* Face::getneighb(){
	std::vector<Face*>* res = new std::vector<Face*>();
	Face* temp;

	temp = e1->getNeighb(this);
	if(temp != NULL){ res->push_back(temp); }
	temp = e2->getNeighb(this);
	if(temp != NULL){ res->push_back(temp); }
	temp = e3->getNeighb(this);
	if(temp != NULL){ res->push_back(temp); }

	return res;
}
glm::vec3 Face::getNormal(){
	float nx=0; float ny=0; float nz = 0;
	glm::vec3* p1; glm::vec3* p2; glm::vec3* p3;
	float norme = 0;

	p1 = pts->at(0)->coord;
	p2 = pts->at(1)->coord;
	p3 = pts->at(2)->coord;


	nx = (p2->y - p1->y )*(p3->z - p1->z) - (p2->z - p1->z)*(p3->y - p1->y);
	ny = (p2->z - p1->z )*(p3->x - p1->x) - (p2->x - p1->x)*(p3->z - p1->z);
	nz = (p2->x - p1->x )*(p3->y - p1->y) - (p2->y - p1->y)*(p3->x - p1->x);
	if(nx == 0 && ny == 0 && nz == 0){ std::cout << "bad normal Face::getNormal" << std::endl; }
	norme =  sqrt((nx*nx)+(ny*ny)+(nz*nz));

	return glm::vec3(nx / norme, ny / norme, nz / norme);
}

float Face::diedre(Face& f){
	glm::vec3 n1;
	glm::vec3 n2;

	n1 = this->getNormal();
	n2 = f.getNormal();

	return acos( (n1.x*n2.x) + (n1.y*n2.y) + (n1.z*n2.z) );
}
Edge* Face::getCommonEdge(Face& f){
	if( f.e1->f1 == this ){ return f.e1; }
	if( f.e1->f2 == this ){ return f.e1; }
	if( f.e2->f1 == this ){ return f.e2; }
	if( f.e2->f2 == this ){ return f.e2; }
	if( f.e3->f1 == this ){ return f.e3; }
	if( f.e3->f2 == this ){ return f.e3; }

	return NULL;
}
bool Face::delimiter( glm::vec3* color , glm::vec3** colorMap ){
	std::vector<Face*>* nbh = getneighb();
    Edge* commonEdge = NULL;

	if( colorMap[index] == NULL){ colorMap[index] = color; }
	else{ return false; }

	for( Face* f : *nbh ){
		commonEdge = this->getCommonEdge(*f);
		if( !commonEdge->sharp ){ f->delimiter(color,colorMap); }
	}
	return true;
}
Vertex* Face::getOpp(Edge* e){
	for(Vertex* pt : *pts){
		if( pt != e->p1 && pt != e->p2 ){
			return pt;
		}
	}
	return NULL;
}



//############################### CLASSE MESH ###############################################



void Mesh::addEdge(Edge* ref){
	CPUEdges->push_back(ref);
	ref->index = CPUEdges->size() - 1;
}

void Mesh::addFace(Face* ref){
	CPUFaces->push_back(ref);
	ref->index = CPUFaces->size() - 1;
}

void Mesh::addPoint(Vertex* ref){
	CPUPoints->push_back(ref);
	ref->index = CPUPoints->size() - 1;
}




Mesh::Mesh():CPUPoints( new vector<Vertex*>()), CPUEdges( new vector<Edge*>() ),
				 CPUFaces( new vector<Face*>() ),aretesVives(NULL),GPUcolorMap( NULL ){}

Mesh::Mesh(const char* path ):CPUPoints( new vector<Vertex*>()),CPUEdges( new vector<Edge*>() ),
										CPUFaces( new vector<Face*>() ),aretesVives(NULL),GPUcolorMap( NULL )
{
	loadOBJ(path);
}



 //A completer
Mesh::~Mesh(){ }


void Mesh::CPUtoGPU(){ cout << "Abstract function Mesh::CPUtoGPU should not be called" << endl; }

glm::vec3 Mesh::defView(){
	float axis[6];

	axis[0] = CPUPoints->at(0)->coord->x; axis[1] = CPUPoints->at(0)->coord->x;
	axis[2] = CPUPoints->at(0)->coord->y; axis[3] = CPUPoints->at(0)->coord->y;
	axis[4] = CPUPoints->at(0)->coord->z; axis[5] = CPUPoints->at(0)->coord->z;
	for(Vertex* v: *CPUPoints){
		if( v->coord->x < axis[0] ){ axis[0] = v->coord->x; }
		if( v->coord->x > axis[1] ){ axis[1] = v->coord->x; }
		if( v->coord->y < axis[2] ){ axis[2] = v->coord->y; }
		if( v->coord->y > axis[3] ){ axis[3] = v->coord->y; }
		if( v->coord->z < axis[4] ){ axis[4] = v->coord->z; }
		if( v->coord->z > axis[5] ){ axis[5] = v->coord->z; }
	}

	float distx = axis[1] - axis[0];
	float disty = axis[3] - axis[2];
	float distz = axis[5] - axis[4];

	return glm::vec3( distx / 2 , disty / 2   , distz * 6  );
}



bool Mesh::loadOBJ(const char* path){
	glm::vec3 vertex; glm::vec2 uv;  glm::vec3 normalt;
	Vertex** pts = new Vertex*[3]();
	Edge** edges = new Edge*[3]();
	Face* face = NULL;

	vector<glm::vec3*>* CPUposition = new vector<glm::vec3*>();
	vector<glm::vec2*>* CPUtexCoord = new vector<glm::vec2*>();
	vector<glm::vec3*>* CPUnormal = new vector<glm::vec3*>();

  
  char lineHeader[128];
  int scanfCheck = 0;
  FILE * file = fopen(path, "r");
  if( file == NULL ){
    cout << "Impossible d'ouvrir le fichier !" << endl;
    return false;
  }

  while( true ){
     int res = fscanf(file, "%s", lineHeader);
     if (res == EOF){ break; }

     if ( strcmp( lineHeader, "v" ) == 0 ){
       scanfCheck = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
       CPUposition->push_back( new glm::vec3(vertex) );
       addPoint( new Vertex(CPUposition->back()) );
     }
     else if ( strcmp( lineHeader, "vt" ) == 0 ){
       scanfCheck = fscanf(file, "%f %f\n", &uv.x, &uv.y );
       CPUtexCoord->push_back(new glm::vec2(uv));
     }
     else if ( strcmp( lineHeader, "vn" ) == 0 ){
       scanfCheck = fscanf(file, "%f %f %f\n", &normalt.x, &normalt.y, &normalt.z );
       CPUnormal->push_back(new glm::vec3(normalt));
     }
     else if ( strcmp( lineHeader, "f" ) == 0 ){
       unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
       scanfCheck = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
			    &vertexIndex[0], &uvIndex[0], &normalIndex[0],
			    &vertexIndex[1], &uvIndex[1], &normalIndex[1],
			    &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
       if( scanfCheck != 9 ){
			 printf("Il faut exactement des faces triangles avec 3 informations par point: %d\n",scanfCheck);
			 return false;
       }


       pts[0] = CPUPoints->at( vertexIndex[0]-1 ); pts[0]->addUV(CPUtexCoord->at(uvIndex[0]-1)); pts[0]->addNrm(CPUnormal->at(normalIndex[0]-1 ));
       pts[1] = CPUPoints->at( vertexIndex[1]-1 ); pts[1]->addUV(CPUtexCoord->at(uvIndex[1]-1)); pts[1]->addNrm(CPUnormal->at(normalIndex[1]-1 ));
       pts[2] = CPUPoints->at( vertexIndex[2]-1 ); pts[2]->addUV(CPUtexCoord->at(uvIndex[2]-1)); pts[2]->addNrm(CPUnormal->at(normalIndex[2]-1 ));

       edges[0] = pts[0]->addEdge(pts[1],this);
       edges[1] = pts[0]->addEdge(pts[2],this);
       edges[2] = pts[1]->addEdge(pts[2],this);



       face = new Face( edges[0],edges[1],edges[2],pts[0],pts[1],pts[2] );
       //INSERTION HASHMAP ICI

       face->addNrm( pts[0], CPUnormal->at(normalIndex[0]-1) );
       face->addNrm( pts[1], CPUnormal->at(normalIndex[1]-1) );
       face->addNrm( pts[2], CPUnormal->at(normalIndex[2]-1) );

       face->addUV( pts[0], CPUtexCoord->at(uvIndex[0]-1) );
       face->addUV( pts[1], CPUtexCoord->at(uvIndex[1]-1) );
       face->addUV( pts[2], CPUtexCoord->at(uvIndex[2]-1) );

       addFace( face );
       //cout << vertexIndex[0] << endl;
     }
  }

  calcNormals();
  //CPUtoGPU();
  delete[] pts;
  delete[] edges;
  fclose(file);
  cout << "DONE" << endl;
  return true;
}

void Mesh::calcNormals(){
	glm::vec3 n; // norme de la face courante
	float tn = 0;
	glm::vec3* ntemp;

	for(Face* face : *CPUFaces){
		face->nrm->at(face->pts->at(0))->x = 0.0; face->nrm->at(face->pts->at(0))->y = 0.0; face->nrm->at(face->pts->at(0))->z = 0.0;
		face->nrm->at(face->pts->at(1))->x = 0.0; face->nrm->at(face->pts->at(1))->y = 0.0; face->nrm->at(face->pts->at(1))->z = 0.0;
		face->nrm->at(face->pts->at(2))->x = 0.0; face->nrm->at(face->pts->at(2))->y = 0.0; face->nrm->at(face->pts->at(2))->z = 0.0;
	}

	for(Face* face: *CPUFaces ){
		n = face->getNormal();

		ntemp = face->nrm->at(face->pts->at(0));
		ntemp->x += n.x; ntemp->y += n.y; ntemp->z += n.z;
		tn = sqrt( pow(ntemp->x,2) + pow(ntemp->y,2) + pow(ntemp->z,2)) ;
		ntemp->x /= tn; ntemp->y /= tn; ntemp->z /= tn;

		ntemp = face->nrm->at(face->pts->at(1));
		ntemp->x += n.x; ntemp->y += n.y; ntemp->z += n.z;
		tn = sqrt( pow(ntemp->x,2) + pow(ntemp->y,2) + pow(ntemp->z,2)) ;
		ntemp->x /= tn; ntemp->y /= tn; ntemp->z /= tn;

		ntemp = face->nrm->at(face->pts->at(2));
		ntemp->x += n.x; ntemp->y += n.y; ntemp->z += n.z;
		tn = sqrt( pow(ntemp->x,2) + pow(ntemp->y,2) + pow(ntemp->z,2)) ;
		ntemp->x /= tn; ntemp->y /= tn; ntemp->z /= tn;

	}

	CPUtoGPU();
}

void Mesh::addEdges(Edge** edges,size_t size){
	pthread_t threadsID[size];
	int rc = 0;
	MeshTools::ThreadWrapper* wrap = NULL;
	void* res[size];

	//Recherche concurrente des references
	for(size_t i = 0; i < size; ++i){
		wrap = new MeshTools::ThreadWrapper(this,(void*)(edges[i]));
		rc =  pthread_create(&threadsID[i], NULL,Mesh::getEdgeRef, (void *)wrap);
		if( rc ){ cout << "Error:unable to create thread, " << rc << endl; }
	}

	//Attente des resultats
	for(size_t i = 0; i < size; ++i){
		rc = pthread_join( threadsID[i], &res[i] );
		if( rc ){ cout << "Error:unable to join thread, " << rc << endl; }
		if(res[i] == NULL){ cout << "HERPY DERPY" << endl; }
	}

	//Ajout des resultat
	for(size_t i = 0; i < size; ++i){
		edges[i] = (Edge*)((MeshTools::ThreadWrapper*)res[i])->toAdd;
		if( ((MeshTools::ThreadWrapper*)res[i])->newRes ){
			CPUEdges->push_back((Edge*)((MeshTools::ThreadWrapper*)res[i])->toAdd);
			CPUEdges->back()->index = CPUEdges->size() - 1;
		}
		delete (MeshTools::ThreadWrapper*)res[i];
	}
}

void* Mesh::getEdgeRef(void* wrap){
	Mesh* mRef = ((MeshTools::ThreadWrapper*)wrap)->ref;
	Edge* ref = (Edge*)((MeshTools::ThreadWrapper*)wrap)->toAdd;

	for(Edge* e : *(mRef->CPUEdges) ){
		if( ( e->p1 == ref->p1 && e->p2 == ref->p2 ) || ( e->p2 == ref->p1 && e->p1 == ref->p2 ) ){
			delete ref;
			((MeshTools::ThreadWrapper*)wrap)->toAdd = e;
			//pthread_exit(wrap);
			return wrap;
		}
	}
	((MeshTools::ThreadWrapper*)wrap)->newRes = true;
	//pthread_exit(wrap);
	return wrap;
}
void Mesh::addSharpEdge(Edge* edg){
	for(Edge* e : *aretesVives){
		if( (e->p1 == edg->p1 && e->p2 == edg->p2) || (e->p1 == edg->p2 && e->p2 == edg->p1) ){
			return;
		}
	}
	aretesVives->push_back(edg);
}
void Mesh::fillAretesVives(float seuil){
	aretesVives = new vector<Edge*>();
	vector<Face*>* iterator;
	float tempAngle = 0.0;
	Edge* res = NULL;

	for(Face* f : *CPUFaces){
		iterator = f->getneighb();
		for(Face* nbh : *iterator){
			tempAngle = f->diedre(*nbh);

			if( (tempAngle > seuil) &&  (tempAngle < (M_PI - seuil)) ){

				res = f->getCommonEdge(*nbh);
				res->sharp = true;
				addSharpEdge(res);
			}
		}
		delete iterator;
	}

}
void Mesh::segmenter(){
	GPUcolorMap = new glm::vec3*[ CPUFaces->size() ]();
	float r,g,b;
	glm::vec3* color;

	//srand (time(NULL));

	for( Face* f : *CPUFaces ){
		r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		color = new glm::vec3(r,g,b);

		if( GPUcolorMap[f->index] == NULL ){ f->delimiter( color, GPUcolorMap ); }
		else{ delete color; }
	}
}
vector<Vertex*>* Mesh::getEdgeNbh(Vertex* p)const{
	vector<Vertex*>* res = new vector<Vertex*>();

	for(Edge* edge : *CPUEdges){
		if( edge->p1 == p ){ res->push_back(edge->p2); }
		if( edge->p2 == p ){ res->push_back(edge->p1); }
		if( res->size() == 6 ){ break; }
	}
	return res;
}



//##############################################################################3
struct Box{
	float xbegin; float xend;
	float ybegin; float yend;
	float zbegin; float zend;
	vector<Vertex*> pts;

	Box(float xb, float xe, float yb, float ye, float zb, float ze ){
		xbegin = xb; xend = xe;
		ybegin = yb; yend = ye;
		zbegin = zb; zend = ze;
	}
	bool isInBox(Vertex* ref){
		return (ref->coord->x >= xbegin && ref->coord->x < xend ) &&
			   (ref->coord->y >= ybegin && ref->coord->y < yend ) &&
			   (ref->coord->z >= zbegin && ref->coord->z < zend );
	}

	Vertex* getMergePoint(){
		glm::vec3* coor = new glm::vec3(0.0,0.0,0.0);
		//glm::vec2* tex = new glm::vec2(0.0,0.0);
		//glm::vec3* normal = new glm::vec3(0.0,0.0,0.0);
		Vertex* res = NULL;

		for( Vertex* p : pts){
			coor->x += p->coord->x; coor->y += p->coord->y; coor->z += p->coord->z;
		}
		coor->x /= pts.size(); coor->y /= pts.size(); coor->z /= pts.size();

		res = new Vertex(coor);
		//return new Vertex(coor,tex,normal);
		return res;
	}
	void merge(){
		Vertex* m = getMergePoint();
		for(Vertex* p : pts){
			p->coord = new glm::vec3( *(m->coord) );
		}
	}
};




void Mesh::simplifier(float reduc){
	float axis[6];

	axis[0] = CPUPoints->at(0)->coord->x; axis[1] = CPUPoints->at(0)->coord->x;
	axis[2] = CPUPoints->at(0)->coord->y; axis[3] = CPUPoints->at(0)->coord->y;
	axis[4] = CPUPoints->at(0)->coord->z; axis[5] = CPUPoints->at(0)->coord->z;
	for( Vertex* v: *CPUPoints ){
		if( v->coord->x < axis[0] ){ axis[0] = v->coord->x; }
		if( v->coord->x > axis[1] ){ axis[1] = v->coord->x; }
		if( v->coord->y < axis[2] ){ axis[2] = v->coord->y; }
		if( v->coord->y > axis[3] ){ axis[3] = v->coord->y; }
		if( v->coord->z < axis[4] ){ axis[4] = v->coord->z; }
		if( v->coord->z > axis[5] ){ axis[5] = v->coord->z; }
	}

	float distx = axis[1] - axis[0];
	float disty = axis[3] - axis[2];
	float distz = axis[5] - axis[4];

	float xstep = distx * reduc;
	float ystep = disty * reduc;
	float zstep = distz * reduc;

	vector<Box*> boxlist;

	for( float x = axis[0]; x < axis[1] ; x+=xstep ){
		for( float y = axis[2]; y < axis[3]; y+=ystep ){
			for( float z = axis[4]; z < axis[5]; z+=zstep ){
				//cout << "[" << x << ";" << x+xstep << "] [" << y << ";" << y+ystep << "] [" << z << ";" << z+zstep << "]" << endl;
				boxlist.push_back(new Box(x,x+xstep,y,y+ystep,z,z+zstep));
				for(Vertex* pt : *CPUPoints){
					if(boxlist.back()->isInBox(pt)){
						boxlist.back()->pts.push_back(pt);
					}
				}
			}
		}
	}
	for( Box* b : boxlist){
		if(b->pts.size() != 0){
			b->merge();
		}
	}
	//CPUtoGPU();
	calcNormals();
}

void Mesh::checkHoles(){
	for(Edge* edg : *CPUEdges){
		if(edg->f1 == NULL || edg->f2 == NULL ){
			cout << "TROU " << edg->index << endl;
			cout << edg->p1->edges->size() << " " << edg->p2->edges->size() << endl;
		}
	}
	//cout << CPUEdges->size() << endl;
}

//######################################## SUBDIVISION ###########################################
glm::vec3* Vertex::averageNrm(){
	glm::vec3* res = new glm::vec3();

	for(glm::vec3* n : *nrm){
		res->x += n->x; res->y += n->y; res->z += n->z;
	}

	res->x /= nrm->size(); res->y /= nrm->size(); res->z /= nrm->size();
	return res;
}


void Vertex::catmullClarkMove(glm::vec3* avgFaces, glm::vec3* avgEdges){
	float valence = edges->size();
	glm::vec3 resCoord(0,0,0);

	resCoord.x += avgFaces->x / valence; resCoord.y += avgFaces->y / valence; resCoord.z += avgFaces->z / valence;
	resCoord.x += 2.0*avgEdges->x / valence; resCoord.y += 2.0*avgEdges->y / valence; resCoord.z += 2.0*avgEdges->z / valence;
	resCoord.x += coord->x*(valence-3.0) / valence; resCoord.y += coord->y*(valence-3.0) / valence; resCoord.z += coord->z*(valence-3.0) / valence;

	coord->x = resCoord.x; coord->y = resCoord.y; coord->z = resCoord.z;
}

vector<Face*>* Vertex::getNbhFaces(){
	vector<Face*>* res = new vector<Face*>();

	for(Edge* e : *edges){
		if( e->f1 != NULL && find(res->begin(), res->end(), e->f1) == res->end() ){ res->push_back(e->f1); }
		if( e->f2 != NULL && find(res->begin(), res->end(), e->f2) == res->end() ){ res->push_back(e->f2); }
	}
	return res;
}


Vertex* Edge::getCatmullClark(Vertex* fp1, Vertex* fp2){
	Vertex* res = NULL;
	glm::vec3* resCoord = new glm::vec3();
	glm::vec3* resNrm = new glm::vec3();
	glm::vec3* p1Nrm = NULL; glm::vec3* p2Nrm = NULL;
	glm::vec2* resUV = new glm::vec2();

	glm::vec2* f1p1UV = NULL; glm::vec2* f2p1UV = NULL;
	glm::vec2* f1p2UV = NULL;


	resCoord->x += p1->coord->x; resCoord->y += p1->coord->y; resCoord->z += p1->coord->z;
	resCoord->x += p2->coord->x; resCoord->y += p2->coord->y; resCoord->z += p2->coord->z;

	p1Nrm = p1->averageNrm(); p2Nrm = p2->averageNrm();
	resNrm->x += p1Nrm->x; resNrm->y += p1Nrm->y; resNrm->z += p1Nrm->z;
	resNrm->x += p2Nrm->x; resNrm->y += p2Nrm->y; resNrm->z += p2Nrm->z;

	if(fp1 != NULL && fp2 != NULL){
		resCoord->x += fp1->coord->x; resCoord->y += fp1->coord->y; resCoord->z += fp1->coord->z;
		resCoord->x += fp2->coord->x; resCoord->y += fp2->coord->y; resCoord->z += fp2->coord->z;
		resCoord->x /= 4; resCoord->y /= 4; resCoord->z /= 4;

		resNrm->x += fp1->nrm->at(0)->x; resNrm->y += fp1->nrm->at(0)->y; resNrm->z += fp1->nrm->at(0)->z;
	    resNrm->x += fp2->nrm->at(0)->x; resNrm->y += fp2->nrm->at(0)->y; resNrm->z += fp2->nrm->at(0)->z;
		resNrm->x /= 4; resNrm->y /= 4; resNrm->z /= 4;


		f1p1UV = f1->uv->at(p1); f2p1UV = f2->uv->at(p1);
		f1p2UV = f1->uv->at(p2);
		if(f1p1UV == f2p1UV){
			resUV->x += f1p1UV->x; resUV->y += f1p1UV->y;
			resUV->x += f1p2UV->x; resUV->y += f1p2UV->y;
			resUV->x += fp1->uv->at(0)->x; resUV->y += fp1->uv->at(0)->y;
			resUV->x += fp2->uv->at(0)->x; resUV->y += fp2->uv->at(0)->y;
			resUV->x /= 4.0; resUV->y /= 4.0;
		}
		//Rajouter code pour multi-sets de textures
	}
	else{
		resCoord->x /= 2; resCoord->y /= 2; resCoord->z /= 2;
		resNrm->x /= 2; resNrm->y /= 2; resNrm->z /= 2;
		if(f1 != NULL){
			resUV->x += f1->uv->at(p1)->x / 2; resUV->y += f1->uv->at(p1)->y / 2;
			resUV->x += f1->uv->at(p2)->x / 2; resUV->y += f1->uv->at(p2)->y / 2;
		}
		if(f2 != NULL){
			resUV->x += f2->uv->at(p1)->x / 2; resUV->y += f2->uv->at(p1)->y / 2;
			resUV->x += f2->uv->at(p2)->x / 2; resUV->y += f2->uv->at(p2)->y / 2;
		}
	}



	//Moyenne UV manquante


	res = new Vertex(resCoord);
	res->addNrm(resNrm);
	res->addUV(resUV);

	delete p1Nrm;
	delete p2Nrm;
	return res;
}

Edge* Face::getEdge(Vertex* p1,Vertex* p2){
	if(e1->p1 == p1 && e1->p2 == p2){ return e1; }
	if(e1->p2 == p1 && e1->p1 == p2){ return e1; }

	if(e2->p1 == p1 && e2->p2 == p2){ return e2; }
	if(e2->p2 == p1 && e2->p1 == p2){ return e2; }

	if(e3->p1 == p1 && e3->p2 == p2){ return e3; }
	if(e3->p2 == p1 && e3->p1 == p2){ return e3; }

	return NULL;
}


Vertex* Face::getCatmullClark(){
	Vertex* res = NULL;
	glm::vec3* resCoord = new glm::vec3();
	glm::vec3* resNrm = new glm::vec3();
	glm::vec2* resUV = new glm::vec2();

	for(Vertex* p : *pts){
		resCoord->x += p->coord->x; resCoord->y += p->coord->y; resCoord->z += p->coord->z;
		resNrm->x += nrm->at(p)->x; resNrm->y += nrm->at(p)->y; resNrm->z += nrm->at(p)->z;
		resUV->x += uv->at(p)->x; resUV->y += uv->at(p)->y;
	}
	resCoord->x /= pts->size() ; resCoord->y /= pts->size(); resCoord->z /= pts->size();
	resNrm->x /= pts->size() ; resNrm->y /= pts->size(); resNrm->z /= pts->size();
	resUV->x /= pts->size() ; resUV->y /= pts->size();

	res = new Vertex(resCoord);
	res->addNrm(resNrm);
	res->addUV(resUV);

	return res;
}

void Mesh::catmull_Clark(){
	vector<Vertex*> facePoints;
	vector<Vertex*> edgePoints;
	vector<Edge*>* oldEdges = CPUEdges;
	vector<Face*>* oldFaces = CPUFaces;
	glm::vec3* avgF = NULL;
	glm::vec3* avgE = NULL;
	Vertex* temp = NULL;
	vector<Face*>* ptNbh = NULL;

	cout << "STEP 1" << endl;
	//Creations des nouveaux points pour chaque face
	for(Face* f : *CPUFaces){ facePoints.push_back(f->getCatmullClark() ); }

	cout << "STEP 2" << endl;
	//Creation des points pour chaque arete
	for(Edge* e : *CPUEdges){
		if(e->f1 != NULL && e->f2 != NULL ){
			edgePoints.push_back( e->getCatmullClark(facePoints.at(e->f1->index),facePoints.at(e->f2->index)) );
		}
		else{
			edgePoints.push_back( e->getCatmullClark(NULL,NULL) );
		}
	}

	cout << "STEP 3" << endl;
	//Deplacement des points de controles
	for(Vertex* p : *CPUPoints){
		ptNbh = p->getNbhFaces();
		avgF = new glm::vec3();

		//Moyenne des nouveaux points des faces voisines
		for(Face* f : *ptNbh){
			temp = facePoints.at(f->index);
			avgF->x += temp->coord->x; avgF->y += temp->coord->y; avgF->z += temp->coord->z;
		}
		avgF->x /= ptNbh->size(); avgF->y /= ptNbh->size(); avgF->z /= ptNbh->size();
		delete ptNbh;

		//Moyenne des nouveaux points des aretes voisines
		avgE = new glm::vec3();
		for(Edge* e : *(p->edges)){
			temp = edgePoints.at(e->index);
			avgE->x += temp->coord->x; avgE->y += temp->coord->y; avgE->z += temp->coord->z;
		}
		avgE->x /= p->edges->size(); avgE->y /= p->edges->size(); avgE->z /= p->edges->size();

		//Deplacement du point via la moyenne ponderee de catmull-clark
		p->catmullClarkMove(avgF,avgE);
		p->edges->clear();
	}

	//for(Vertex* p : *CPUPoints){ p->edges->clear(); }
	for(Vertex* p : facePoints){ addPoint(p); }
	for(Vertex* p : edgePoints){ addPoint(p); }

	cout << "STEP 4" << endl;
	CPUFaces = new vector<Face*>();
	CPUEdges = new vector<Edge*>();
	Edge* nEdges[12];
	Face* nFaces[6];
	Vertex* nPoints[7];

	for(Face* f : *oldFaces){
		//0-2 points de controle | 3-5 points d'aretes | 6 point de face
		for(size_t i = 0; i < f->pts->size(); ++i){ nPoints[i] = f->pts->at(i); }
		nPoints[3] = edgePoints.at( f->getEdge(f->pts->at(0),f->pts->at(1))->index );
		nPoints[4] = edgePoints.at( f->getEdge(f->pts->at(1),f->pts->at(2))->index );
		nPoints[5] = edgePoints.at( f->getEdge(f->pts->at(0),f->pts->at(2))->index );
		nPoints[6] = facePoints.at( f->index );

		//0 - 5 aretes vers le point de la face | 6 - 11 aretes entre points de controles et points d'aretes
		for(size_t i = 0; i < 6; ++i){ nEdges[i] = nPoints[i]->addEdge(nPoints[6],this); }
		nEdges[6] = nPoints[0]->addEdge(nPoints[3],this);
		nEdges[7] = nPoints[0]->addEdge(nPoints[5],this);
		nEdges[8] = nPoints[1]->addEdge(nPoints[3],this);
		nEdges[9] = nPoints[1]->addEdge(nPoints[4],this);
		nEdges[10] = nPoints[4]->addEdge(nPoints[2],this);
		nEdges[11] = nPoints[2]->addEdge(nPoints[5],this);

		//Creation des 6 nouvelles faces
		nFaces[0] = new Face(nEdges[6],nEdges[0],nEdges[1],nPoints[0],nPoints[3],nPoints[6]);
		nFaces[0]->addNrm(nPoints[0],f->nrm->at(nPoints[0])); nFaces[0]->addUV(nPoints[0],f->uv->at(nPoints[0]));
		nFaces[0]->addNrm(nPoints[3],nPoints[3]->nrm->at(0)); nFaces[0]->addUV(nPoints[3],nPoints[3]->uv->at(0));
		nFaces[0]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[0]->addUV(nPoints[6],nPoints[6]->uv->at(0));

		nFaces[1] = new Face(nEdges[7],nEdges[0],nEdges[5],nPoints[0],nPoints[6],nPoints[5]);
		nFaces[1]->addNrm(nPoints[0],f->nrm->at(nPoints[0])); nFaces[1]->addUV(nPoints[0],f->uv->at(nPoints[0]));
		nFaces[1]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[1]->addUV(nPoints[6],nPoints[6]->uv->at(0));
		nFaces[1]->addNrm(nPoints[5],nPoints[5]->nrm->at(0)); nFaces[1]->addUV(nPoints[5],nPoints[5]->uv->at(0));

		nFaces[2] = new Face(nEdges[8],nEdges[1],nEdges[2],nPoints[3],nPoints[1],nPoints[6]);
		nFaces[2]->addNrm(nPoints[3],nPoints[3]->nrm->at(0)); nFaces[2]->addUV(nPoints[3],nPoints[3]->uv->at(0));
		nFaces[2]->addNrm(nPoints[1],f->nrm->at(nPoints[1])); nFaces[2]->addUV(nPoints[1],f->uv->at(nPoints[1]));
		nFaces[2]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[2]->addUV(nPoints[6],nPoints[6]->uv->at(0));

		nFaces[3] = new Face(nEdges[2],nEdges[3],nEdges[9],nPoints[6],nPoints[1],nPoints[4]);
		nFaces[3]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[3]->addUV(nPoints[6],nPoints[6]->uv->at(0));
		nFaces[3]->addNrm(nPoints[1],f->nrm->at(nPoints[1])); nFaces[3]->addUV(nPoints[1],f->uv->at(nPoints[1]));
		nFaces[3]->addNrm(nPoints[4],nPoints[4]->nrm->at(0)); nFaces[3]->addUV(nPoints[4],nPoints[4]->uv->at(0));

		nFaces[4] = new Face(nEdges[3],nEdges[4],nEdges[10],nPoints[6],nPoints[4],nPoints[2]);
		nFaces[4]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[4]->addUV(nPoints[6],nPoints[6]->uv->at(0));
		nFaces[4]->addNrm(nPoints[4],nPoints[4]->nrm->at(0)); nFaces[4]->addUV(nPoints[4],nPoints[4]->uv->at(0));
		nFaces[4]->addNrm(nPoints[2],f->nrm->at(nPoints[2])); nFaces[4]->addUV(nPoints[2],f->uv->at(nPoints[2]));

		nFaces[5] = new Face(nEdges[5],nEdges[4],nEdges[11],nPoints[5],nPoints[6],nPoints[2]);
		nFaces[5]->addNrm(nPoints[5],nPoints[5]->nrm->at(0)); nFaces[5]->addUV(nPoints[5],nPoints[5]->uv->at(0));
		nFaces[5]->addNrm(nPoints[6],nPoints[6]->nrm->at(0)); nFaces[5]->addUV(nPoints[6],nPoints[6]->uv->at(0));
		nFaces[5]->addNrm(nPoints[2],f->nrm->at(nPoints[2])); nFaces[5]->addUV(nPoints[2],f->uv->at(nPoints[2]));

		for(size_t i = 0; i < 6; ++i ){ addFace(nFaces[i]); }
		delete f;
	}

	for(Edge* e : *oldEdges){ delete e; }


	delete oldFaces;
	delete oldEdges;
	//calcNormals();
	CPUtoGPU();
}
