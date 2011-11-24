#pragma once

#include "BasicStructure.h"

class CCamera
{
public:
	CCamera(void);
	~CCamera(void);

	// camera related
	void lookAt(const Vec3d& eye, const Vec3d& at, const Vec3d& up);
	void perspective(double fovy, double aspect, double zNear, double zFar);
	void frustum(double left, double right, double bottom, double top, double near, double far);
	void ortho(double left, double right, double bottom, double top, double near, double far);
	void transform(Vec4d& v);
	const Vec4d& pos() { return mCameraPos; }

private:
	Mat44d mModelMatrix;
	Mat44d mViewMatrix;
	Mat44d mProjectMatrix;

	Vec4d mCameraPos;
	bool mNeedUpdate; // is it needed to update @mFinalMatrix
	Mat44d mFinalMatrix; // ProjectMatrix * ViewMatrix
};
