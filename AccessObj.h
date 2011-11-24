// AccessObj.h: interface for the CAccessObj class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MY_ACCESS_OBJ_
#define _MY_ACCESS_OBJ_

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "Point3D.h"

#define objMax(a,b)	(((a)>(b))?(a):(b))
#define objMin(a,b)	(((a)<(b))?(a):(b))
#define objAbs(x)	(((x)>0.f)?(x):(-x))

#define Tri(x) (m_pModel->pTriangles[(x)])

// --------------------------------------------------------------------
// COBJtriangle: defines a triangle in a model.
class COBJtriangle
{
public:
	unsigned int vindices[3];	// array of triangle vertex indices 
	unsigned int nindices[3];	// array of triangle normal indices 
	unsigned int findex;		// index of triangle facet normal 
	COBJtriangle()
	{
		vindices[0] = vindices[1] = vindices[2] = 
			nindices[0] = nindices[1] = nindices[2] = 0;
	}
	~COBJtriangle()	{}
};// ------------------------------------------------------------------

// --------------------------------------------------------------------
// COBJgroup: defines a group in a model.
class COBJgroup
{
public:
	char name[256];			// name of this group 
	unsigned int nTriangles;	// number of triangles in this group 
	unsigned int* pTriangles;	// array of triangle indices 
	float m_fTran_X;
	float m_fTran_Y;
	float m_fTran_Z;
	float m_fScale_X;
	float m_fScale_Y;
	float m_fScale_Z;
	float m_fRotate_X;
	float m_fRotate_Y;
	float m_fRotate_Z;
	class COBJgroup* next;		// pointer to next group in model 

	COBJgroup()
	{
		nTriangles = 0;
		pTriangles = NULL;
		next = NULL;
	}

	~COBJgroup()
	{
		if (nTriangles)
			delete [] pTriangles;
		pTriangles = NULL;
		nTriangles = 0;
	}
};// ------------------------------------------------------------------

// --------------------------------------------------------------------
// COBJmodel: defines a model.
class COBJmodel
{
public:
	char pathname[256];			// path to this model 
	unsigned int nVertices;		// number of vertices in model 
	CPoint3D* vpVertices;		// array of vertices 
	unsigned int nNormals;		// number of normals in model 
	CPoint3D* vpNormals;		// array of normals 
	unsigned int nFacetnorms;	// number of facetnorms in model 
	CPoint3D* vpFacetNorms;		// array of facetnorms 
	unsigned int nTriangles;	// number of triangles in model 
	COBJtriangle* pTriangles;	// array of triangles 
	unsigned int nGroups;		// number of groups in model 
	COBJgroup* pGroups;			// linked list of groups 
	CPoint3D position;			// position of the model 

	// construction
	COBJmodel()
	{
		nVertices   = 0;
		vpVertices  = NULL;
		nNormals    = 0;
		vpNormals   = NULL;
		nFacetnorms = 0;
		vpFacetNorms= NULL;
		nTriangles  = 0;
		pTriangles  = NULL;
		nGroups     = 0;
		pGroups     = NULL;
		position    = CPoint3D(0, 0, 0);
	}

	// free all memory
	void	Destory()
	{
		COBJgroup *group;

		if (vpVertices)		delete [] vpVertices;
		if (vpNormals)		delete [] vpNormals;
		if (vpFacetNorms)	delete [] vpFacetNorms;
		if (pTriangles)		delete [] pTriangles;

		while(pGroups)
		{
			group = pGroups;
			pGroups = pGroups->next;
			delete group;
		}

		nVertices    = 0;
		vpVertices   = NULL;
		nNormals     = 0;
		vpNormals    = NULL;
		nFacetnorms  = 0;
		vpFacetNorms = NULL;
		nTriangles   = 0;
		pTriangles   = NULL;
		nGroups      = 0;
		pGroups      = NULL;
		position     = CPoint3D(0, 0, 0);
	}

	// destruction
	~COBJmodel()
	{
		Destory();
	}
};// ------------------------------------------------------------------

// --------------------------------------------------------------------
// A temporal calss
class OBJnode
{
public:
	unsigned int triIdx;
	unsigned int index; // index in triangle

	OBJnode()
		: triIdx(0)
		, index(0)
	{
	}
};// ------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Definition of the OBJ R/W class 
///////////////////////////////////////////////////////////////////////////////
class CAccessObj
{
public:
	CAccessObj();
	~CAccessObj();

	COBJmodel *m_pModel;

protected:
	CPoint3D m_vMax, m_vMin;

	void CalcBoundingBox();
	bool Equal(CPoint3D * u, CPoint3D * v, float epsilon);

	COBJgroup* FindGroup(char* name);
	COBJgroup* AddGroup(char* name);

	char* DirName(char* path);
	bool FirstPass(FILE* file);
	void SecondPass(FILE* file);
	void Dimensions(float* dimensions);
	void Scale(float scale);
	void ReverseWinding();
	void FacetNormals();
	void VertexNormals(float angle);

public:
	void Destory();
	void Boundingbox(CPoint3D &vMax, CPoint3D &vMin);
	bool LoadOBJ(const char* filename);
	void UnifiedModel();
};

#endif