#include "ScanLine.h"
#include <QImage>
#include <cmath>
#include <cassert>
#include "Point3D.h"

using std::fabs;
using std::max;
using std::min;
using std::tan;
using trimeshVec::normalize;

#define SATURATE(x) ( ((x)>255) ? 255 : (((x)<0) ? 0 : x) )
#define ROUND(x) int((x)+0.5)
#define DEFAULT_COLOR Color4u(255, 255, 255, 255)

#define DISABLE_STATE(state,pro) {(state) &= ~(pro);} 
#define EABLE_STATE(state,pro) {(state) |= (pro);} 

//////////////////////////////////////////////////////////////////////////
CScanLine::CScanLine()
: mType(SL_NONE), mMaxY(-1)
, mbInitialised(false)
, mbHasNormals(true)
{
	_init();
}

CScanLine::CScanLine(int _w, int _h, QImage* _img)
: mType(SL_NONE), mMaxY(-1), mCurY(_h-1)
, mHeight(_h), mWidth(_w), mImg(_img)
, mZBuffer(_h*_w, 1.0)
, mbInitialised(true)
, mbHasNormals(true)
{
	_init();
}

void CScanLine::_init()
{
	mCurColor[0] = 1.0;
	mCurColor[1] = 1.0;
	mCurColor[2] = 1.0;
	mCurColor[3] = 1.0;

	mCurNormal[0] = 0;
	mCurNormal[1] = 0;
	mCurNormal[2] = 0;

	mGlobalAmbient = Color4d(0.1, 0.1, 0.1, 1.0);
	
}

void CScanLine::setRenderTarget(int _w, int _h, QImage *_img)
{
	mCurY = _h-1;
	mWidth = _w;
	mHeight = _h;
	mImg = _img;
	mZBuffer.assign(_w*_h, 1.0);

	mbInitialised = true;
}

CScanLine::~CScanLine(void)
{
	_clear();
}

void CScanLine::_clear()
{
	ETableIterator it = mSortedET.begin();
	ETableIterator it_end = mSortedET.end();
	for (; it!=it_end; ++it)
	{
		EListIterator itl = it->second.begin();
		EListIterator itl_end = it->second.end();
		for (; itl!=itl_end; ++itl )
		{
			SAFE_DELETE(*itl);
		}
	}

	TArrayItor itt = mTriArray.begin();
	TArrayItor itt_end = mTriArray.end();
	for (; itt!=itt_end; ++itt)
	{
		SAFE_DELETE(*itt);
	}

	VBufferItor itv = mVertexBuffer.begin();
	VBufferItor itv_end = mVertexBuffer.end();
	for (; itv!=itv_end; ++itv)
	{
		SAFE_DELETE(*itv);
	}

	mSortedET.clear();
	mTriArray.clear();
	mAEL.clear();

	mVertexBuffer.clear();
}

void CScanLine::clear(int _target, const Color4u& _c /* = Color4u */, double _depth /* = 1.0 */)
{
	if (_target & SL_COLOR_BUFFER)
	{
		mImg->fill(qRgba(_c[0], _c[1], _c[2], _c[3]));
	}
	if (_target & SL_DEPTH_BUFFER)
	{
		// z buffer reassignment
		mZBuffer.assign(mWidth*mHeight, _depth);
	}
}

void CScanLine::begin(TargetType _type)
{
	if (!mbInitialised) return;

	mType = _type;

	_clear();
	mMaxY = -1;
	mCurY = mHeight-1;
}

//------------------------------------------------------------------------------
// Vertices
//------------------------------------------------------------------------------
void CScanLine::vertex3d(double _x, double _y, double _z)
{
	Vertex *t_pv = new Vertex(_x, _y, _z, 1);
	t_pv->color = mCurColor;
	t_pv->normalWorld = mCurNormal;
	mVertexBuffer.push_back(t_pv);
}

void CScanLine::vertex3dv(const Vec3d& v)
{
	vertex3d(v[0], v[1], v[2]);
}

void CScanLine::vertex3fv(const CPoint3D& v)
{
	vertex3d(v.x, v.y, v.z);
}

//------------------------------------------------------------------------------
// Colors
//------------------------------------------------------------------------------
void CScanLine::color3i(unsigned char _r, unsigned char _g, unsigned char _b)
{
	mCurColor[0] = _r/255.0;
	mCurColor[1] = _g/255.0;
	mCurColor[2] = _b/255.0;
	mCurColor[3] = 1.0;
}

void CScanLine::color4i(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
{
	mCurColor[0] = _r/255.0;
	mCurColor[1] = _g/255.0;
	mCurColor[2] = _b/255.0;
	mCurColor[3] = _a/255.0;
}

void CScanLine::color3f(float _r, float _g, float _b)
{
	mCurColor[0] = _r;
	mCurColor[1] = _g;
	mCurColor[2] = _b;
	mCurColor[3] = 1.0;
}

void CScanLine::color4f(float _r, float _g, float _b, float _a)
{
	mCurColor[0] = _r;
	mCurColor[1] = _g;
	mCurColor[2] = _b;
	mCurColor[3] = _a;
}

//------------------------------------------------------------------------------
// Normals
//------------------------------------------------------------------------------
void CScanLine::normal3d(double _nx, double _ny, double _nz)
{
	mCurNormal[0] = _nx;
	mCurNormal[1] = _ny;
	mCurNormal[2] = _nz;
}

void CScanLine::normal3fv(const CPoint3D& _n)
{
	mCurNormal[0] = _n.x;
	mCurNormal[1] = _n.y;
	mCurNormal[2] = _n.z;
}

//void CScanLine::setLight(const Light& light)
//{
//	if (mLights.empty()) 
//		mLights.push_back(light);
//	else 
//		mLights[0] = light;
//
//	mLights.back().direction.norm();
//}

void CScanLine::end()
{
	_modelViewProjectionTransform();
	//_normalizeDeviceCoordinates();
	_screenCoordinates();
	switch (mType)
	{
	case SL_TRIANGLES:
		_addTriangles();
		break;

	case SL_TRIANGLE_STRIP:
		_addTriangleStrip();
		break;

	case SL_TRIANGLE_FAN:
		_addTriangleFan();
		break;

	case SL_QUADS:
		_addQuads();
		break;

	case SL_QUAD_STRIP:
		_addQuadStrip();
		break;

	case SL_POLYGON:
		_addPolygon();
		return;
		break;

	case SL_NONE:
		return;
		break;

	default :
		return;
		break;
	}

	_scanLine();

	mType = SL_NONE;
}

void CScanLine::_addTriangles()
{
	int v_num = mVertexBuffer.size()/3;
	for (int i=0; i<v_num; ++i)
	{
		Vertex *t_v1 = mVertexBuffer[3*i];
		Vertex *t_v2 = mVertexBuffer[3*i+1];
		Vertex *t_v3 = mVertexBuffer[3*i+2];
		
		_addATriangle(t_v1, t_v2, t_v3);
	}
}

void CScanLine::_addATriangle(const Vertex *_v1, const Vertex *_v2, const Vertex *_v3)
{
	// on x-z face, skip
	if (_v1->posScreen[1] == _v2->posScreen[1] && _v1->posScreen[1] == _v3->posScreen[1]) 
		return;

	Normald t_nor = (_v2->posScreen - _v1->posScreen) CROSS (_v3->posScreen - _v1->posScreen);
	if (t_nor[2]<0) // back cull, faster
		return;

	Triangle *tri = new Triangle;
	tri->normal = t_nor;
	tri->d = -(tri->normal DOT _v1->posScreen);
	int t_minY = min(_v3->posScreen[1], min(_v1->posScreen[1], _v2->posScreen[1]));
	int t_maxY = max(_v3->posScreen[1], max(_v1->posScreen[1], _v2->posScreen[1]));
	tri->dy = t_maxY - t_minY;

	int t_id = mTriArray.size();
	_addEdge(_v1, _v2, t_id);
	_addEdge(_v2, _v3, t_id);
	_addEdge(_v3, _v1, t_id);

	mTriArray.push_back(tri);
}

bool CScanLine::_addEdge(const Vertex* _v1, const Vertex* _v2, int _id)
{
	assert(_id==mTriArray.size());

	if (_v1->posScreen[1] == _v2->posScreen[1])
		return false;

	// clipping		

	// swap, so that y coordinates are sorted decreasingly
	if (_v1->posScreen[1] < _v2->posScreen[1])
	{
		const Vertex *t_v = _v1; 
		_v1 = _v2;
		_v2 = t_v;
	}

	// create edge
	Edge *edge = new Edge;
	edge->id = _id;

	edge->dy = _v1->posScreen[1] - _v2->posScreen[1];
	edge->x = _v2->posScreen[0];
	edge->dx = (_v1->posScreen[0] - _v2->posScreen[0]) * 1.0 / edge->dy;
	
	//color 
	const Color4d &c1 = _v1->color;
	const Color4d &c2 = _v2->color;
	edge->color = c2;
	edge->dclr = (c1 - c2);
	edge->dclr /= 1.0 * edge->dy;

	// position in world space
	const Vec4d &p1 = _v1->posWorld;
	const Vec4d &p2 = _v2->posWorld;
	edge->posW = p2;	
	edge->dPosW = (p1 - p2);
	edge->dPosW /= 1.0 * edge->dy;

	// normal in world space
	if (mRenderState.isLighting())
	{
		const Normald &n1 = _v1->normalWorld;
		const Normald &n2 = _v2->normalWorld;
		edge->normalW = n2;
		edge->dNorW = (n1 - n2);
		edge->dNorW /= 1.0 * edge->dy;
	}

	// add new edge
	mSortedET[_v2->posScreen[1]].push_back(edge);

	if (_v2->posScreen[1] < mCurY) 
		mCurY = _v2->posScreen[1];

	if (_v1->posScreen[1] > mMaxY) // && _v1->posScreen[1] < mHeight)
		mMaxY = _v1->posScreen[1];

	return true;
}

void CScanLine::_addTriangleStrip()
{
	int v_num = mVertexBuffer.size();

	for (int i=0; i<v_num-2; ++i)
	{
		Vertex *t_v1 = mVertexBuffer[i];
		Vertex *t_v2 = mVertexBuffer[i+1];
		Vertex *t_v3 = mVertexBuffer[i+2];

		_addATriangle(t_v1, t_v2, t_v3);
	}
}

void CScanLine::_addTriangleFan()
{
}

void CScanLine::_addQuads()
{
	int v_num = mVertexBuffer.size();

	for (int i=0; i<v_num/4; ++i)
	{
		for (int j=0; j<2; ++j)
		{
			Vertex *t_v1 = mVertexBuffer[4*i];
			Vertex *t_v2 = mVertexBuffer[4*i+j+1];
			Vertex *t_v3 = mVertexBuffer[4*i+j+2];

			_addATriangle(t_v1, t_v2, t_v3);
		}
	}
}

void CScanLine::_addQuadStrip()
{
}

void CScanLine::_addPolygon()
{
}

bool CScanLine::_compare_edges(const Edge* e1, const Edge* e2)
{
	return (e1->x == e2->x) ? (e1->dx < e2->dx) : (e1->x < e2->x);
}

void CScanLine::_scanLine()
{
	if (mMaxY>=mHeight) mMaxY=mHeight-1;
	
	int nNextY = mCurY; // the line which will be added next time
	ETableIterator itset_next = mSortedET.begin(); // related iterator
	while (mCurY<=mMaxY)
	{
		ActiveEdge *t_ae;
		AEListItor itae, itae_end;

		// Step 1: add edges at mCurY line to Active Edge List
		//ETableIterator it_set = mSortedET.find(mCurY);
		if (mCurY==nNextY && itset_next != mSortedET.end() 
			&& !(itset_next->second.empty()))
		{
			EListIterator itl = itset_next->second.begin();
			EListIterator itl_end = itset_next->second.end();
			for (; itl!=itl_end; ++itl)
			{
				int id = (*itl)->id;
				itae = mAEL.find(id);
				if (itae!=mAEL.end())
				{
					t_ae = &(itae->second);
					if (t_ae->el->dy == 0)
						t_ae->el = *itl;
					if (t_ae->er->dy == 0)
						t_ae->er = *itl;
				}
				else
				{ // if it's a new triangle
					//Edge *el = *itl;
					//++itl;
					//Edge *er = *itl;
					//if (el->id!=er->id)
					//{
					//	int i = 0;
					//}
					ActiveEdge *t_ae = &mAEL[id];
					//t_ae->el = el;
					//t_ae->er = er;
					t_ae->el = *itl;
					++itl;
					assert(t_ae->el->id == (*itl)->id);
					t_ae->er = *itl;
					if (!_compare_edges(t_ae->el, t_ae->er))
					{
						Edge* t_e = t_ae->er;
						t_ae->er = t_ae->el;
						t_ae->el = t_e;
					}
					// calculate z depth at left
					Triangle &tri = *(mTriArray[id]);
					Edge *t_el = t_ae->el;
					t_ae->zl = -(tri.normal[0]*t_el->x + tri.normal[1]*mCurY + tri.d) / (tri.normal[2]);
					t_ae->dzx = -tri.normal[0] / tri.normal[2];
					t_ae->dzy = -tri.normal[1] / tri.normal[2];
				}
			}
			++itset_next;
			if (itset_next != mSortedET.end())
				nNextY = itset_next->first;
		}// end if (mCurY==nNextY && it_set != mSortedET.end() && !(it_set->second.empty()))

		if (mCurY>=0)
		{
			// Step 2: fill the region in pairs, horizontal operations
			itae = mAEL.begin();
			itae_end = mAEL.end();
			for (; itae!=itae_end; ++itae )
			{
				t_ae = &(itae->second);
				Edge *e1 = t_ae->el;
				Edge *e2 = t_ae->er;

				double t_xl = e1->x;	// valid x on the left side
				double t_xr = e2->x;	// valid x on the right side
				// skip the scan line which is out of region
				if (t_xl>=mWidth || t_xr<0)
				{// out of area
					continue;
				}

				// calculate the interpolated color
				double skip_x = e2->x - e1->x;

				Color4d t_dclr(0, 0, 0, 0);
				Vec4d t_dposW(0,0,0,0);
				Normald t_dnorW(0, 0, 0);
				if (skip_x>0)
				{
					t_dclr = (e2->color - e1->color) / skip_x;
					if ( mRenderState.isSmoothShading() )
					{
						t_dposW = (e2->posW - e1->posW) / skip_x;
						t_dnorW = (e2->normalW - e1->normalW) / skip_x;
					}
				}
				Color4d t_color = e1->color;
				Vec4d t_posW = e1->posW;
				Normald t_norW = e1->normalW;
				double t_zl = t_ae->zl; // z depth on the left side

				// skip the region over the left
				if (t_xl<0)
				{
					t_color += t_dclr*(-t_xl);
					if (mRenderState.isSmoothShading())
					{
						t_posW += t_dposW*(-t_xl);
						t_norW += t_dnorW*(-t_xl);
					}
					t_xl = 0;
				}

				Color4d t_final_clr;
				//for (int pi = std::max(e1->x,0.0); pi<std::min(e2->x+1, (double)mImg->width()); ++pi)
				for (int pi = int(t_xl); pi<std::min(int(t_xr+1), mWidth); ++pi)
				{
					if (t_zl<mZBuffer[mCurY*mWidth+pi])
					{
						t_final_clr = t_color;
						if ( mRenderState.isSmoothShading() )
							_calculateLight(t_posW, t_norW, t_final_clr);
						_setFrameBuffer(mCurY, pi, t_final_clr);
						mZBuffer[mCurY*mWidth+pi] = t_zl;
					}
					// update color, normal, zl
					t_color += t_dclr;
					t_zl += t_ae->dzx;
					if ( mRenderState.isSmoothShading() )
					{
						t_posW += t_dposW;
						t_norW += t_dnorW;
					}
				}
			}
		}

		// Step 3: update the edges, vertical operations
		itae = mAEL.begin();
		itae_end = mAEL.end();
		if (mCurY<0)
		{
			double nMinY = -mCurY; // NOTE
			if (itset_next != mSortedET.end() && !(itset_next->second.empty())
				&& (nNextY-mCurY < nMinY) )
			{
				nMinY = nNextY-mCurY;
			}
			if (nMinY)
			for (; itae!=itae_end; ++itae)
			{
				t_ae = &(itae->second);
				if (t_ae->el->dy < nMinY)
				{
					nMinY = t_ae->el->dy;
				}
				if (t_ae->er->dy < nMinY)
				{
					nMinY = t_ae->er->dy;
				}
			}
			itae = mAEL.begin();
			for (; itae!=itae_end;)
			{
				t_ae = &(itae->second);
				
				// dy
				t_ae->el->dy -= nMinY;
				t_ae->er->dy -= nMinY;

				if (t_ae->el->dy == 0 && t_ae->er->dy == 0)
				{
					itae  = mAEL.erase(itae);
					continue;
				}

				// x, zl
				t_ae->el->x += t_ae->el->dx * nMinY;
				t_ae->er->x += t_ae->er->dx * nMinY;
				t_ae->zl += (t_ae->dzx * t_ae->el->dx + t_ae->dzy) * nMinY;

				// color
				t_ae->el->color += t_ae->el->dclr * nMinY;
				t_ae->er->color += t_ae->er->dclr * nMinY;	

				if ( mRenderState.isSmoothShading() )
				{
					// normal in WS
					t_ae->el->normalW += t_ae->el->dNorW * nMinY;
					t_ae->er->normalW += t_ae->er->dNorW * nMinY;

					// position in WS
					t_ae->el->posW += t_ae->el->dPosW * nMinY;
					t_ae->er->posW += t_ae->er->dPosW * nMinY;
				}

				++itae;
			}
			mCurY += int(nMinY);
		}
		else
		{// mCurY>=0
			for (; itae!=itae_end;)
			{
				// remove the edges whose nearby edge is at the other side of the scan line.
				t_ae = &(itae->second);
				if (t_ae->el->dy == 0 && t_ae->er->dy == 0)
				{
					itae  = mAEL.erase(itae);
				}
				else
				{
					// dy
					--t_ae->el->dy;
					--t_ae->er->dy;

					// x, zl
					t_ae->el->x += t_ae->el->dx;
					t_ae->er->x += t_ae->er->dx;
					t_ae->zl += t_ae->dzx * t_ae->el->dx + t_ae->dzy;

					// color
					t_ae->el->color += t_ae->el->dclr;
					t_ae->er->color += t_ae->er->dclr;	

					if ( mRenderState.isSmoothShading() )
					{
						// normal in WS
						t_ae->el->normalW += t_ae->el->dNorW;
						t_ae->er->normalW += t_ae->er->dNorW;

						// position in WS
						t_ae->el->posW += t_ae->el->dPosW;
						t_ae->er->posW += t_ae->er->dPosW;
					}

					++itae;
				}
			}//end for update edges
			++mCurY;
		}
	}
}

void CScanLine::_setFrameBuffer(int _y, int _x, Color4d& _clr)
{
	if (mRenderState.isBlending())
	{
		QRgb oldclr = mImg->pixel(_x, mHeight-_y-1);
		double t_a = _clr[3];
		_clr[0] = _clr[0] * t_a + qRed(oldclr) * (1-t_a);
		_clr[1] = _clr[1] * t_a + qGreen(oldclr) * (1-t_a);
		_clr[2] = _clr[2] * t_a + qBlue(oldclr) * (1-t_a);
		_clr[3] = 1.0;
	}
	_clr *= 255.0;
	mImg->setPixel(_x, mHeight-_y-1, 
		qRgb(SATURATE(_clr[0]), SATURATE(_clr[1]), SATURATE(_clr[2])) );
}

void CScanLine::_calculateLight(const Vec4d& _pos, const Normald& _nor, Color4d& _clr)
{
	if ( !mRenderState.isLighting())
		return;

	Normald normal = _nor.normed();
	Color4d diffuse, ambient, specular, globalAmbient, finalColor;
	double NdotL, NdotHV, dist, att;

	Vec3d lightDir, halfVector;
	Vec3d eyeDir = mCamera.pos() - _pos;
	dist = eyeDir.length();
	eyeDir.norm();


	switch(mLight.type)
	{
	case SL_LIGHT_DIRECTIONAL:
		lightDir = -mLight.direction;
		lightDir.norm();
		NdotL = std::max((normal DOT lightDir), 0.0);
		//diffuse = mMaterial.diffuse * mLight.diffuse;
		//ambient = mMaterial.ambient * mLight.ambient;
		globalAmbient = mMaterial.ambient * mGlobalAmbient;
		finalColor =globalAmbient;
		// specular
		if (NdotL>0)
		{
			diffuse = _clr * mLight.diffuse;
			ambient = _clr * mLight.ambient;
			halfVector = lightDir + eyeDir;
			halfVector.norm();
			NdotHV = std::max((normal DOT halfVector), 0.0);
			specular = mMaterial.specular * mLight.specular * std::pow(NdotHV, mMaterial.shiness);
			finalColor +=  NdotL * diffuse + specular + ambient;
		}
		_clr = finalColor;
		_clr[3] = 1.0;
		break;
	case SL_LIGHT_POINT:
		lightDir = mLight.position - _pos;
		lightDir.norm();
		NdotL = std::max((normal DOT lightDir), 0.0);
		//diffuse = mMaterial.diffuse * mLight.diffuse;
		//ambient = mMaterial.ambient * mLight.ambient;
		globalAmbient = mMaterial.ambient * mGlobalAmbient;
		finalColor = globalAmbient;
		// specular
		if (NdotL>0)
		{
			diffuse = _clr * mLight.diffuse;
			ambient = _clr * mLight.ambient;
			att = 1.0 / (mLight.attenuation0 + mLight.attenuation1 * dist +
				mLight.attenuation2 * dist *dist);
			halfVector = (lightDir + eyeDir).normed();
			NdotHV = std::max((normal DOT halfVector), 0.0);
			specular = mMaterial.specular * mLight.specular * std::pow(NdotHV, mMaterial.shiness);
			// final color
			finalColor += att * (NdotL * diffuse + ambient + specular);
		}
		_clr = finalColor;
		_clr[3] = 1.0;
		break;
	case SL_LIGHT_SPOT:
		break;
	case SL_LIGHT_NONE:
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------
// Camera Related
//------------------------------------------------------------------------------
void CScanLine::lookAt(const Vec3d& eye, const Vec3d& at, const Vec3d& up)
{
	mCamera.lookAt(eye, at, up);
}

void CScanLine::perspective(double fovy, double aspect, double zNear, double zFar)
{
	mCamera.perspective(fovy, aspect, zNear, zFar);
}

void CScanLine::frustum(double left, double right, double bottom, double top, 
					  double near, double far)
{
	mCamera.frustum(left, right, bottom, top, near, far);
}

void CScanLine::ortho(double left, double right, double bottom, double top, 
					double near, double far)
{
	mCamera.ortho(left, right, bottom, top, near, far);
}

//------------------------------------------------------------------------------
// Transformations
//------------------------------------------------------------------------------
void CScanLine::_normalizeDeviceCoordinates()
{
	VBufferItor it = mVertexBuffer.begin();
	VBufferItor it_end = mVertexBuffer.end();
	for (; it!=it_end; ++it)
	{
		Vertex *t_pv = *it;
		t_pv->posWorld[0] /= t_pv->posWorld[3];
		t_pv->posWorld[1] /= t_pv->posWorld[3];
		t_pv->posWorld[2] /= t_pv->posWorld[3];
		t_pv->posWorld[3] = 1;
	}
}

void CScanLine::_screenCoordinates()
{
	VBufferItor it = mVertexBuffer.begin();
	VBufferItor it_end = mVertexBuffer.end();
	for (; it!=it_end; ++it)
	{
		Vertex *t_pv = *it;
		// round the coordinates
		t_pv->posScreen[0] = ROUND((t_pv->posScreen[0]+1)*mWidth*0.5);
		t_pv->posScreen[1] = ROUND((t_pv->posScreen[1]+1)*mHeight*0.5);
		t_pv->posScreen[2] = 0.5 * t_pv->posScreen[2] + 0.5;
	}
}

void CScanLine::_modelViewProjectionTransform()
{
	Vertex *t_pv;
	Vec4d t_pos;

	VBufferItor it = mVertexBuffer.begin();
	VBufferItor it_end = mVertexBuffer.end();
	for (; it!=it_end; ++it)
	{
		t_pv = *it;
		t_pos = t_pv->posWorld;
		mCamera.transform(t_pos);
		t_pv->posScreen[0] = t_pos[0] / t_pos[3];
		t_pv->posScreen[1] = t_pos[1] / t_pos[3];
		t_pv->posScreen[2] = t_pos[2] / t_pos[3];

		// gouraud shading
		if ( mRenderState.isFlatShading() )
			_calculateLight(t_pv->posWorld, t_pv->normalWorld, t_pv->color);
	}
}

void CScanLine::_vertexTransform()
{
	_modelViewProjectionTransform();
}

//------------------------------------------------------------------------------
// Render States
//------------------------------------------------------------------------------
void CScanLine::setRenderState(int _state, int _val)
{
	mRenderState.setState(_state, _val);
}