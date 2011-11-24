#include "Camera.h"
#include <cassert>

CCamera::CCamera(void)
: mNeedUpdate(true)
, mCameraPos(0, 0, 1, 1)
{
}

CCamera::~CCamera(void)
{
}

//------------------------------------------------------------------------------
// Camera Related
//------------------------------------------------------------------------------
void CCamera::lookAt(const Vec3d& eye, const Vec3d& at, const Vec3d& up)
{
	mCameraPos[0] = eye[0];
	mCameraPos[1] = eye[1];
	mCameraPos[2] = eye[2];

	Vec3d t_dir = eye - at;
	t_dir.norm();
	Vec3d t_xaxis = (up CROSS t_dir);
	t_xaxis.norm();
	Vec3d t_up = (t_dir CROSS t_xaxis);
	mViewMatrix.eye();
	mViewMatrix.setCol(0, t_xaxis);
	mViewMatrix.setCol(1, t_up);
	mViewMatrix.setCol(2, t_dir);
	mViewMatrix(3, 3) = 1;
	Mat44d t_mat;
	t_mat.translate(-eye);
	mViewMatrix.multiply(t_mat);
	
	mNeedUpdate = true;
}

void CCamera::perspective(double fovy, double aspect, double zNear, double zFar)
{
	assert(fovy>0 && aspect>0 && zNear>0 && zFar>zNear);
	double top = tan(fovy/2)*zNear;
	double right = top * aspect;
	frustum(-right, right, -top, top, zNear, zFar);

	mNeedUpdate = true;
}

void CCamera::frustum(double left, double right, double bottom, double top, 
						double near, double far)
{
	assert(right!=left && top!=bottom && far!=near);
	mProjectMatrix.zero();
	double skip_x = right - left;
	double skip_y = top - bottom;
	double skip_z = far - near;
	mProjectMatrix(0,0) = 2*near / skip_x;
	mProjectMatrix(1,1) = 2*near / skip_y;
	mProjectMatrix(2,2) = -(far+near) / skip_z;
	mProjectMatrix(0,2) = (left+right) /skip_x;
	mProjectMatrix(1,2) = (top+bottom) /skip_y;
	mProjectMatrix(2,3) = -2*far*near/skip_z;
	mProjectMatrix(3,2) = -1;

	mNeedUpdate = true;
}

void CCamera::ortho(double left, double right, double bottom, double top, 
					  double near, double far)
{
	assert(right!=left && top!=bottom && far!=near);
	mProjectMatrix.zero();
	double skip_x = right - left;
	double skip_y = top - bottom;
	double skip_z = far - near;
	mProjectMatrix(0,0) = 2 / skip_x;
	mProjectMatrix(1,1) = 2 / skip_y;
	mProjectMatrix(2,2) = -2 / skip_z;
	mProjectMatrix(3,3) = 1;
	mProjectMatrix(0,3) = -(left+right) /skip_x;
	mProjectMatrix(1,3) = -(top+bottom) /skip_y;
	mProjectMatrix(2,3) = -(far+near) / skip_z;

	mNeedUpdate = true;
}

void CCamera::transform(Vec4d& v)
{
	if (mNeedUpdate)
	{
		mFinalMatrix = mProjectMatrix;
		mFinalMatrix.multiply(mViewMatrix);
		mNeedUpdate = false;
	}
	mFinalMatrix.transformVec(v);
}
