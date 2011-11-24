#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <cassert>
#include <list>
#include <vector>

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete (p); (p)=0; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] (p); (p)=0; }
#endif


//////////////////////////////////////////////////////////////////////

#include "AccessObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAccessObj::CAccessObj()
{
	m_pModel = NULL;
}

CAccessObj::~CAccessObj()
{
	Destory();
}

//////////////////////////////////////////////////////////////////////
// Equal: compares two vectors and returns true if they are
// equal (within a certain threshold) or false if not. An epsilon
// that works fairly well is 0.000001.
//
// u - array of 3 GLfloats (float u[3])
// v - array of 3 GLfloats (float v[3]) 
//////////////////////////////////////////////////////////////////////
bool CAccessObj::Equal(CPoint3D * u, CPoint3D * v, float epsilon)
{
	if (objAbs(u->x - v->x) < epsilon &&
		objAbs(u->y - v->y) < epsilon &&
		objAbs(u->z - v->z) < epsilon) 
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// FindGroup: Find a group in the model
//////////////////////////////////////////////////////////////////////
COBJgroup * CAccessObj::FindGroup(char* name)
{
	COBJgroup * group;
	
	assert(m_pModel);
	
	group = m_pModel->pGroups;
	while(group) 
	{
		if (!strcmp(name, group->name))
			break;
		group = group->next;
	}
	
	return group;
}

//////////////////////////////////////////////////////////////////////
// AddGroup: Add a group to the model
//////////////////////////////////////////////////////////////////////
COBJgroup * CAccessObj::AddGroup(char* name)
{
	COBJgroup* group;

	group = FindGroup(name);
	if (!group)
	{
		group = new COBJgroup;
		sprintf_s(group->name, 256, "%s", name);
		group->nTriangles = 0;
		group->pTriangles = NULL;
		group->next = m_pModel->pGroups;
		m_pModel->pGroups = group;
		m_pModel->nGroups++;
	}

	return group;
}

//////////////////////////////////////////////////////////////////////
// DirName: return the directory given a path
//
// path - filesystem path
//
// NOTE: the return value should be free'd.
//////////////////////////////////////////////////////////////////////
char * CAccessObj::DirName(char* path)
{
	static char dir[256];
	char *s;
	
	sprintf_s(dir,256, "%s", path);
	
	s = strrchr(dir, '\\');
	if (s)	s[1] = '\0';
	else	dir[0] = '\0';
	
	return dir;
}

//////////////////////////////////////////////////////////////////////
// FirstPass: first pass at a Wavefront OBJ file that gets all the
// statistics of the model (such as #vertices, #normals, etc)
//
// model - properly initialized COBJmodel structure
// file  - (fopen'd) file descriptor 
//////////////////////////////////////////////////////////////////////
bool CAccessObj::FirstPass(FILE* file) 
{
	unsigned int    nVertices;		/* number of vertices in m_pModel */
	unsigned int    nNormals;		/* number of normals in m_pModel */
	unsigned int    nTexCoords;		/* number of texcoords in m_pModel */
	unsigned int    nTriangles;		/* number of triangles in m_pModel */
	COBJgroup* group;			/* current group */
	unsigned  v, n, t;
	char      buf[129];
	
	/* make a default group */
	group = AddGroup("default");
	
	nVertices = nNormals = nTexCoords = nTriangles = 0;
	while(fscanf(file, "%128s", buf, 129) != EOF)
	{
		switch(buf[0])
		{
		case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch(buf[1])
			{
			case '\0':			/* vertex */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				nVertices++;
				break;
			case 'n':				/* normal */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				nNormals++;
				break;
			case 't':				/* texcoord */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				nTexCoords++;
				break;
			default:
				printf("FirstPass(): Unknown token \"%s\".\n", buf);
				exit(1);
				break;
			}
			break;

		case 'u':
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'g':				/* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			buf[strlen(buf)-1] = '\0';	/* nuke '\n' */
			group = AddGroup(buf);
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%128s", buf, 129);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//"))
			{
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				nTriangles++;
				group->nTriangles++;
				while(fscanf(file, "%d//%d", &v, &n) > 0)
				{
					nTriangles++;
					group->nTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
			{
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				nTriangles++;
				group->nTriangles++;
				while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0)
				{
					nTriangles++;
					group->nTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2)
			{
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				nTriangles++;
				group->nTriangles++;
				while(fscanf(file, "%d/%d", &v, &t) > 0)
				{
					nTriangles++;
					group->nTriangles++;
				}
			}
			else
			{
				/* v */
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				nTriangles++;
				group->nTriangles++;
				while(fscanf(file, "%d", &v) > 0)
				{
					nTriangles++;
					group->nTriangles++;
				}
			}
			break;
			
		default:
			/* eat up rest of line */
			if (isalpha(buf[0]))
			{
				fgets(buf, sizeof(buf), file);
			}
			else
				return false;
			break;
		}
	}
	
	/* set the stats in the m_pModel structure */
	m_pModel->nVertices  = nVertices;
	m_pModel->nNormals   = nNormals;
	m_pModel->nTriangles = nTriangles;
	
	/* allocate memory for the triangles in each group */
	group = m_pModel->pGroups;
	while(group)
	{
		if (group->nTriangles > 0)
			group->pTriangles = new unsigned int [group->nTriangles];
		group->nTriangles = 0;
		group = group->next;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// SecondPass: second pass at a Wavefront OBJ file that gets all
// the data.
//
// model - properly initialized COBJmodel structure
// file  - (fopen'd) file descriptor 
//////////////////////////////////////////////////////////////////////
void CAccessObj::SecondPass(FILE* file) 
{
	unsigned int	nVertices;		/* number of vertices in m_pModel */
	unsigned int	nNormals;		/* number of normals in m_pModel */
	unsigned int	nTriangles;		/* number of triangles in m_pModel */
	CPoint3D *	vertices;		/* array of vertices  */
	CPoint3D *	normals;		/* array of normals */
	COBJgroup *	group;			/* current group pointer */
	unsigned int	v, n, t;
	char		buf[128];

	/* set the pointer shortcuts */
	vertices     = m_pModel->vpVertices;
	normals      = m_pModel->vpNormals;
	group        = m_pModel->pGroups;
	
	/* on the second pass through the file, read all the data into the
	allocated arrays */
	nVertices = nNormals = 1;
	nTriangles = 0;

	int nNormalAdd = 0;
	int nNormalCount = 0;

	while(fscanf(file, "%s", buf) != EOF)
	{
		switch(buf[0])
		{
		case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch(buf[1])
			{
			case '\0':			/* vertex */
				fscanf(file, "%f %f %f", 
					&vertices[nVertices].x, 
					&vertices[nVertices].y, 
					&vertices[nVertices].z);
				nVertices++;
				break;
			case 'n':				/* normal */
				fscanf(file, "%f %f %f", 
					&normals[nNormals].x,
					&normals[nNormals].y, 
					&normals[nNormals].z);
				nNormals++;
				nNormalCount ++;
				break;
			}
			break;

		case 'g':				/* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			buf[strlen(buf)-1] = '\0';	/* nuke '\n' */
			group = FindGroup(buf);
			nNormalAdd += nNormalCount;
			nNormalCount = 0;
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//"))
			{
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				Tri(nTriangles).vindices[0] = v;
				Tri(nTriangles).nindices[0] = n;// + nNormalAdd;
				fscanf(file, "%d//%d", &v, &n);
				Tri(nTriangles).vindices[1] = v;
				Tri(nTriangles).nindices[1] = n;// + nNormalAdd;
				fscanf(file, "%d//%d", &v, &n);
				Tri(nTriangles).vindices[2] = v;
				Tri(nTriangles).nindices[2] = n;// + nNormalAdd;
				group->pTriangles[group->nTriangles++] = nTriangles;
				nTriangles++;
				while(fscanf(file, "%d//%d", &v, &n) > 0)
				{
					Tri(nTriangles).vindices[0] = Tri(nTriangles-1).vindices[0];
					Tri(nTriangles).nindices[0] = Tri(nTriangles-1).nindices[0];
					Tri(nTriangles).vindices[1] = Tri(nTriangles-1).vindices[2];
					Tri(nTriangles).nindices[1] = Tri(nTriangles-1).nindices[2];
					Tri(nTriangles).vindices[2] = v;
					Tri(nTriangles).nindices[2] = n;// + nNormalAdd;
					group->pTriangles[group->nTriangles++] = nTriangles;
					nTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
			{
				/* v/t/n */
				Tri(nTriangles).vindices[0] = v;
				Tri(nTriangles).nindices[0] = n;// + nNormalAdd;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				Tri(nTriangles).vindices[1] = v;
				Tri(nTriangles).nindices[1] = n;// + nNormalAdd;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				Tri(nTriangles).vindices[2] = v;
				Tri(nTriangles).nindices[2] = n;// + nNormalAdd;
				group->pTriangles[group->nTriangles++] = nTriangles;
				nTriangles++;
				while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0)
				{
					Tri(nTriangles).vindices[0] = Tri(nTriangles-1).vindices[0];
					Tri(nTriangles).nindices[0] = Tri(nTriangles-1).nindices[0];
					Tri(nTriangles).vindices[1] = Tri(nTriangles-1).vindices[2];
					Tri(nTriangles).nindices[1] = Tri(nTriangles-1).nindices[2];
					Tri(nTriangles).vindices[2] = v;
					Tri(nTriangles).nindices[2] = n;// + nNormalAdd;
					group->pTriangles[group->nTriangles++] = nTriangles;
					nTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2)
			{
				/* v/t */
				Tri(nTriangles).vindices[0] = v;
				fscanf(file, "%d/%d", &v, &t);
				Tri(nTriangles).vindices[1] = v;
				fscanf(file, "%d/%d", &v, &t);
				Tri(nTriangles).vindices[2] = v;
				group->pTriangles[group->nTriangles++] = nTriangles;
				nTriangles++;
				while(fscanf(file, "%d/%d", &v, &t) > 0)
				{
					Tri(nTriangles).vindices[0] = Tri(nTriangles-1).vindices[0];
					Tri(nTriangles).vindices[1] = Tri(nTriangles-1).vindices[2];
					Tri(nTriangles).vindices[2] = v;
					group->pTriangles[group->nTriangles++] = nTriangles;
					nTriangles++;
				}
			}
			else
			{
				/* v */
				sscanf(buf, "%d", &v);
				Tri(nTriangles).vindices[0] = v;
				fscanf(file, "%d", &v);
				Tri(nTriangles).vindices[1] = v;
				fscanf(file, "%d", &v);
				Tri(nTriangles).vindices[2] = v;
				group->pTriangles[group->nTriangles++] = nTriangles;
				nTriangles++;
				while(fscanf(file, "%d", &v) > 0)
				{
					Tri(nTriangles).vindices[0] = Tri(nTriangles-1).vindices[0];
					Tri(nTriangles).vindices[1] = Tri(nTriangles-1).vindices[2];
					Tri(nTriangles).vindices[2] = v;
					group->pTriangles[group->nTriangles++] = nTriangles;
					nTriangles++;
				}
			}
			break;
			
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Dimensions: Calculates the dimensions (width, height, depth) of
// a model.
//
// model      - initialized COBJmodel structure
// dimensions - array of 3 GLfloats (float dimensions[3])
//////////////////////////////////////////////////////////////////////
void CAccessObj::Dimensions(float* dimensions)
{
	unsigned int i;
	CPoint3D vMax, vMin;
	
	assert(m_pModel);
	assert(m_pModel->vpVertices);
	assert(dimensions);
	
	/* get the max/mins */
	vMax = vMin = m_pModel->vpVertices[0];
	for (i = 1; i <= m_pModel->nVertices; i++)
	{
		if (vMax.x < m_pModel->vpVertices[i].x)
			vMax.x = m_pModel->vpVertices[i].x;
		if (vMin.x > m_pModel->vpVertices[i].x)
			vMin.x = m_pModel->vpVertices[i].x;
		
		if (vMax.y < m_pModel->vpVertices[i].y)
			vMax.y = m_pModel->vpVertices[i].y;
		if (vMin.y > m_pModel->vpVertices[i].y)
			vMin.y = m_pModel->vpVertices[i].y;
		
		if (vMax.z < m_pModel->vpVertices[i].z)
			vMax.z = m_pModel->vpVertices[i].z;
		if (vMin.z > m_pModel->vpVertices[i].z)
			vMin.z = m_pModel->vpVertices[i].z;
	}
	
	/* calculate m_pModel width, height, and depth */
	dimensions[0] = vMax.x-vMin.x;
	dimensions[1] = vMax.y-vMin.y;
	dimensions[2] = vMax.z-vMin.z;
}

//////////////////////////////////////////////////////////////////////
// Scale: Scales a model by a given amount.
// 
// model - properly initialized COBJmodel structure
// scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
//////////////////////////////////////////////////////////////////////
void CAccessObj::Scale(float scale)
{
	unsigned int i;
	
	for (i = 1; i <= m_pModel->nVertices; i++)
		m_pModel->vpVertices[i] = m_pModel->vpVertices[i] * scale;
}

//////////////////////////////////////////////////////////////////////
// ReverseWinding: Reverse the polygon winding for all polygons in
// this model.  Default winding is counter-clockwise.  Also changes
// the direction of the normals.
// 
// model - properly initialized COBJmodel structure 
//////////////////////////////////////////////////////////////////////
void CAccessObj::ReverseWinding()
{
	unsigned int i, swap;
	
	assert(m_pModel);
	
	for (i = 0; i < m_pModel->nTriangles; i++)
	{
		swap = Tri(i).vindices[0];
		Tri(i).vindices[0] = Tri(i).vindices[2];
		Tri(i).vindices[2] = swap;
		
		if (m_pModel->nNormals)
		{
			swap = Tri(i).nindices[0];
			Tri(i).nindices[0] = Tri(i).nindices[2];
			Tri(i).nindices[2] = swap;
		}
	}
	
	/* reverse facet normals */
	for (i = 1; i <= m_pModel->nFacetnorms; i++)
		m_pModel->vpFacetNorms[i] = m_pModel->vpFacetNorms[i]*(-1);
	
	/* reverse vertex normals */
	for (i = 1; i <= m_pModel->nNormals; i++)
		m_pModel->vpNormals[i] = m_pModel->vpNormals[i]*(-1);
}

//////////////////////////////////////////////////////////////////////
// FacetNormals: Generates facet normals for a model (by taking the
// cross product of the two vectors derived from the sides of each
// triangle).  Assumes a counter-clockwise winding.
//
// model - initialized COBJmodel structure
//////////////////////////////////////////////////////////////////////
void CAccessObj::FacetNormals()
{
	unsigned int  i;
	CPoint3D u, v;
	
	assert(m_pModel);
	assert(m_pModel->vpVertices);
	
	/* clobber any old facetnormals */
	SAFE_DELETE_ARRAY(m_pModel->vpFacetNorms);
	
	/* allocate memory for the new facet normals */
	m_pModel->nFacetnorms = m_pModel->nTriangles;
	m_pModel->vpFacetNorms = new CPoint3D [m_pModel->nFacetnorms + 1];
	
	for (i = 0; i < m_pModel->nTriangles; i++)
	{
		m_pModel->pTriangles[i].findex = i+1;
		
		u = m_pModel->vpVertices[Tri(i).vindices[1]] - m_pModel->vpVertices[Tri(i).vindices[0]];
		v = m_pModel->vpVertices[Tri(i).vindices[2]] - m_pModel->vpVertices[Tri(i).vindices[0]];

		m_pModel->vpFacetNorms[i+1] = u * v;
		m_pModel->vpFacetNorms[i+1].unify();
	}
}

//////////////////////////////////////////////////////////////////////
// VertexNormals: Generates smooth vertex normals for a model.
// First builds a list of all the triangles each vertex is in.  Then
// loops through each vertex in the the list averaging all the facet
// normals of the triangles each vertex is in.  Finally, sets the
// normal index in the triangle for the vertex to the generated smooth
// normal.  If the dot product of a facet normal and the facet normal
// associated with the first triangle in the list of triangles the
// current vertex is in is greater than the cosine of the angle
// parameter to the function, that facet normal is not added into the
// average normal calculation and the corresponding vertex is given
// the facet normal.  This tends to preserve hard edges.  The angle to
// use depends on the model, but 90 degrees is usually a good start.
//
// model - initialized COBJmodel structure
// angle - maximum angle (in degrees) to smooth across
//////////////////////////////////////////////////////////////////////
void CAccessObj::VertexNormals(float angle)
{
	OBJnode*  node;
	typedef list<OBJnode*> NodeList;
	typedef NodeList::iterator NodeListItor;
	typedef vector<NodeList> ListArray;
	typedef ListArray::iterator ListArrItor;
	ListArray members;
	CPoint3D *  normals;
	unsigned int    nNormals;
	CPoint3D   t_vAverage;
	float   dot, cos_angle;
	unsigned int    i;
	
	assert(m_pModel);
	assert(m_pModel->vpFacetNorms);
	
	/* calculate the cosine of the angle (in degrees) */
	cos_angle = (float)cos(angle * 3.14159265 / 180.0);
	
	/* nuke any previous normals */
	SAFE_DELETE_ARRAY(m_pModel->vpNormals);
	
	/* allocate space for new normals */
	m_pModel->nNormals = m_pModel->nTriangles * 3; /* 3 normals per triangle */
	m_pModel->vpNormals = new CPoint3D [m_pModel->nNormals+1];
	
	/* allocate a structure that will hold a linked list of triangle
	indices for each vertex */
	members.resize(m_pModel->nVertices + 1);
	
	/* for every triangle, create a node for each vertex in it */
	for (i = 0; i < m_pModel->nTriangles; i++)
	{
		node = new OBJnode;
		node->triIdx = i;
		node->index = 0;
		members[Tri(i).vindices[0]].push_back(node);
		node = new OBJnode;
		node->triIdx = i;		
		node->index = 1;
		members[Tri(i).vindices[1]].push_back(node);
		node = new OBJnode;
		node->triIdx = i;		
		node->index = 2;
		members[Tri(i).vindices[2]].push_back(node);
	}
	
	/* calculate the average normal for each vertex */
	nNormals = 1;
	for (i = 1; i <= m_pModel->nVertices; i++)
	{
		if (members[i].empty())
		{
			continue;
#ifdef _DEBUG
			//fprintf(stderr, "VertexNormals(): vertex w/o a triangle\n");
#endif // _DEBUG
		}

		// calculate an average normal for this vertex by averaging the
		// facet normal of every triangle this vertex is in
		NodeListItor itl = members[i].begin();
		NodeListItor itl_end = members[i].end();

		for (; itl!=itl_end; ++itl)
		{
			/* only average if the dot product of the angle between the two
			facet normals is greater than the cosine of the threshold
			angle -- or, said another way, the angle between the two
			facet normals is less than (or equal to) the threshold angle */
			node = *itl;
			t_vAverage = CPoint3D(0, 0, 0);
			NodeListItor itl_t = members[i].begin();
			for (; itl_t!=itl_end; ++itl_t)
			{
				OBJnode* node_t = *itl_t;
				dot = m_pModel->vpFacetNorms[Tri(node->triIdx).findex] &
					m_pModel->vpFacetNorms[Tri(node_t->triIdx).findex];
				if (dot > cos_angle)
				{
					t_vAverage += m_pModel->vpFacetNorms[Tri(node_t->triIdx).findex];
					/* we averaged at least one normal! */
				}
			}
			/* normalize the averaged normal */
			t_vAverage.unify();

			/* add the normal to the vertex normals list */
			m_pModel->vpNormals[nNormals] = t_vAverage;
			Tri(node->triIdx).nindices[node->index] = nNormals;
			++nNormals;
		}
	}
		
	/* free the member information */
	ListArrItor ita = members.begin();
	ListArrItor ita_end = members.end();
	for (; ita!=ita_end; ++ita)
	{
		NodeListItor itl = ita->begin();
		NodeListItor itl_end = ita->end();
		for (; itl!=itl_end; ++itl)
		{
			SAFE_DELETE(*itl);
		}
	}	
}


//////////////////////////////////////////////////////////////////////
// objDelete: Deletes a COBJmodel structure.
//
// model - initialized COBJmodel structure
//////////////////////////////////////////////////////////////////////
void CAccessObj::Destory()
{
	delete m_pModel;
	m_pModel = NULL;
}

//////////////////////////////////////////////////////////////////////
// objReadOBJ: Reads a model description from a Wavefront .OBJ file.
// Returns a pointer to the created object which should be free'd with
// objDelete().
//
// filename - name of the file containing the Wavefront .OBJ format data.  
//////////////////////////////////////////////////////////////////////
bool CAccessObj::LoadOBJ(const char* filename)
{
	FILE*     file;

	// open the file
	file = fopen(filename, "r");
	if (!file)
	{
		fprintf(stderr, "objReadOBJ() failed: can't open data file \"%s\".\n",
			filename);
		exit(1);
	}
	
	// save old model for invalid obj file
	COBJmodel *pOldModel = m_pModel;

	// allocate a new model
	m_pModel = new COBJmodel;

	sprintf(m_pModel->pathname, "%s", filename);

	m_pModel->nVertices	= 0;
	m_pModel->vpVertices	= NULL;
	m_pModel->nNormals	= 0;
	m_pModel->vpNormals	= NULL;
	m_pModel->nFacetnorms	= 0;
	m_pModel->vpFacetNorms	= NULL;
	m_pModel->nTriangles	= 0;
	m_pModel->pTriangles	= NULL;
	m_pModel->nGroups	= 0;
	m_pModel->pGroups	= NULL;
	m_pModel->position	= CPoint3D (0, 0, 0);
	
	// make a first pass through the file to get a count of the number
	// of vertices, normals, texcoords & triangles
	if (FirstPass(file))
	{	
		SAFE_DELETE(pOldModel);

		/* allocate memory */
		m_pModel->vpVertices = new CPoint3D [m_pModel->nVertices + 1];
		m_pModel->pTriangles = new COBJtriangle [m_pModel->nTriangles];
		if (m_pModel->nNormals)
		{
			m_pModel->vpNormals = new CPoint3D [m_pModel->nNormals + 1];
		}
		
		/* rewind to beginning of file and read in the data this pass */
		rewind(file);
		SecondPass(file);

		// Calc bounding box
		CalcBoundingBox();
	}
	else
	{
		// restore old model
		if (m_pModel != NULL) Destory();
		m_pModel = pOldModel;
	}
	
	/* close the file */
	fclose(file);

	return (pOldModel==NULL);
}

void CAccessObj::Boundingbox(CPoint3D &vMax, CPoint3D &vMin)
{
	vMax = m_vMax;
	vMin = m_vMin;
}

void CAccessObj::CalcBoundingBox()
{
	unsigned int i;

	m_vMax = m_vMin = m_pModel->vpVertices[1];
	for (i = 1; i <= m_pModel->nVertices; i++)
	{
		if (m_vMax.x < m_pModel->vpVertices[i].x)
			m_vMax.x = m_pModel->vpVertices[i].x;
		if (m_vMin.x > m_pModel->vpVertices[i].x)
			m_vMin.x = m_pModel->vpVertices[i].x;
		
		if (m_vMax.y < m_pModel->vpVertices[i].y)
			m_vMax.y = m_pModel->vpVertices[i].y;
		if (m_vMin.y > m_pModel->vpVertices[i].y)
			m_vMin.y = m_pModel->vpVertices[i].y;
		
		if (m_vMax.z < m_pModel->vpVertices[i].z)
			m_vMax.z = m_pModel->vpVertices[i].z;
		if (m_vMin.z > m_pModel->vpVertices[i].z)
			m_vMin.z = m_pModel->vpVertices[i].z;
	}

	CPoint3D vCent = (m_vMax + m_vMin)*0.5f;

	for (i = 1; i <= m_pModel->nVertices; i++)
		m_pModel->vpVertices[i] = m_pModel->vpVertices[i] - vCent;

	m_vMax = m_vMax - vCent;
	m_vMin = m_vMin -vCent;
}

//////////////////////////////////////////////////////////////////////////
// Unified the model to center
//////////////////////////////////////////////////////////////////////////
void CAccessObj::UnifiedModel()
{
	if (m_pModel==NULL) return;

	CPoint3D vDiameter = m_vMax - m_vMin;
	float radius = vDiameter.length() * 0.4f / 1.414f;
	Scale(1.0f/radius);
	CalcBoundingBox();
	if (m_pModel->nNormals==0) 
	{
		FacetNormals();
		VertexNormals(90.f);
	}
}