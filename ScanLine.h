#pragma once

#include <vector>
#include <list>
#include <map>
#include "BasicStructure.h"
#include "Camera.h"
#include "RenderState.h"

class QImage;
class CPoint3D;

class CScanLine
{
private:

	// 3D vertex
	class Vertex 
	{
	public:
		Vec4d posWorld;			// position in world space
		Vec3d posScreen;		// position in screen space
		Color4d color;			// color
		Normald normalWorld;	// normal in world space
		Vertex(double _x, double _y, double _z, double _w)
			: posWorld(_x, _y, _z, _w)
		{
		}
	};

	// 根据yMax将边放入相应的类中
	class Edge
	{
	public:
		double x;			// 边的上端点的x坐标
		double dx;			// 相邻两条扫描线交点的x坐标差
		int dy;				// 边跨越的扫描线数目，相当于yMax

		Color4d color;		// 端点颜色
		Color4d dclr;		// 相邻扫描线颜色差

		Normald normalW;	// 端点法向（World Space）
		Normald dNorW;	// 相邻扫描线法向差（World Space）

		Vec4d posW;			// 端点位置（World Space）
		Vec4d dPosW;		// 相邻扫描线位置差（World Space）

		int id;				// 边所属多边形编号
	};

	class ActiveEdge 
	{
	public:
		Edge *el, *er;	// 左右边
						/*	以和左交点所在边相交的扫描线数为初值，
							以后向下每处理一条扫描线减1 */		

		double zl;		/*	左交点处多边形所在平面的深度值；*/
		double dzx;		/*	沿扫描线向右走过一个像素时，多边形所在
							平面的深度增量。对于平面方程，dzx＝-a/c
							(c≠0)；*/
		double dzy;		/*	沿y方向向下移过一根扫描线时，多边形所
							在平面的深度增量。对于平面方程，dzy＝b/c
							(c≠0)；*/
	//	int id;		// 已经可以从Edge中得到了
	};

	// 根据yMax将多边形放入相应的类中
	class Triangle
	{
	public:
		Vec3d normal;	// 面法线(Screen Space）
		double d;		// surface equation
		int dy;			// 多边形跨越的扫描线数目
	};

	typedef std::list<Edge*> EdgeList;
	typedef EdgeList::iterator EListIterator;

	typedef std::map<int, EdgeList, std::less<int> > EdgeTable;
	typedef EdgeTable::iterator ETableIterator;

	typedef std::vector<Triangle*> TriangleArray;
	typedef TriangleArray::iterator TArrayItor;

	typedef std::map<int, ActiveEdge, std::less<int> > ActiveEdgeList;
	typedef ActiveEdgeList::iterator AEListItor;

	typedef std::vector<Vertex*> VertexBuffer;
	typedef VertexBuffer::iterator VBufferItor;

	//typedef std::vector<Color4u> ColorBuffer;
	//typedef std::vector<Normald> NormalBuffer;
	//typedef NormalBuffer::iterator NorBufItor;

	//typedef std::vector<Light> LightArray;
	//typedef LightArray::iterator LightItor;

	typedef std::vector<int> IndexBuffer;
	typedef IndexBuffer::iterator IBufferItor;

public:
	CScanLine();
	CScanLine(int _w, int _h, QImage *_img);
	~CScanLine(void);

	void setRenderTarget(int _w, int _h, QImage *_img);

	// start create target
	void begin(TargetType _type);
	void end();

	void vertex3d(double _x, double _y, double _z);
	void vertex3dv(const Vec3d& v);
	void vertex3fv(const CPoint3D& v);
	void normal3d(double _nx, double _ny, double _nz);
	void normal3fv(const CPoint3D& n);
	void color3f(float _r, float _g, float _b);
	void color4f(float _r, float _g, float _b, float _a);
	void color3i(unsigned char _r, unsigned char _g, unsigned char _b);
	void color4i(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a);

	//void setLight(const Light& light);
	void clear(int _target, const Color4u& _c = Color4u(0,0,0,255), double _depth = 1.0);

	// camera related, facade design model
	void lookAt(const Vec3d& eye, const Vec3d& at, const Vec3d& up);
	void perspective(double fovy, double aspect, double zNear, double zFar);
	void frustum(double left, double right, double bottom, double top, double near, double far);
	void ortho(double left, double right, double bottom, double top, double near, double far);

	void setRenderState(int _state, int _val);
	const CRenderState& renderState() { return mRenderState; }

private:
	void _init();
	void _clear();
	bool _compare_edges(const Edge* e1, const Edge* e2);
	bool _addEdge(const Vertex* _v1, const Vertex* _v2, int _id);
	void _addATriangle(const Vertex *_v1, const Vertex *_v2, const Vertex *_v3);
	void _addTriangles();
	void _addTriangleStrip();
	void _addTriangleFan();
	void _addQuads();
	void _addQuadStrip();
	void _addPolygon();
	// scan a line at mCurY
	void _scanLine();

	// 光亮度计算
	void _calculateLight(const Vec4d& _pos, const Normald& _nor, Color4d& _clr);
	void _setFrameBuffer(int _y, int _x, Color4d& _clr);

	void _modelViewProjectionTransform();
	void _normalizeDeviceCoordinates();
	void _screenCoordinates();

	// Step 1. Vertex Transformation
	void _vertexTransform();

private:
	bool mbInitialised;		// 是否设置好RenderTarget
	bool mbHasNormals;		// 是否有法向量数据
	int mMaxY;				// contained
	int mCurY;				// contained
	TargetType mType;
	int mHeight, mWidth;
	QImage *mImg;

	EdgeTable mSortedET;		// sorted edge table
	ActiveEdgeList mAEL;		// active edge list
	TriangleArray mTriArray;	// sorted polygon table

	VertexBuffer mVertexBuffer; // 顶点
	IndexBuffer mIndexBuffer;	// 索引
	std::vector<double> mZBuffer;

	//ColorBuffer mColorBuffer;	
	//NormalBuffer mNormalBuffer;	

	Color4d mCurColor;		// 颜色
	Normald mCurNormal;		// 法向

	CCamera mCamera;		// 摄像机

	CRenderState mRenderState;		// 绘制状态开关
	//LightArray mLights;

public:
	Material mMaterial;		// 材质属性
	Light mLight;
	Color4d mGlobalAmbient; // 全局环境光
};
