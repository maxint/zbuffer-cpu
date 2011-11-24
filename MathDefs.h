#ifndef _MATHDEFS_H_
#define _MATHDEFS_H_

#include "Vec.h"

//////////////////////////////////////////////////////////////////////////
// Useful Structures
//
typedef unsigned char byte;
typedef trimeshVec::Vec<2, double>	Vec2d;
typedef trimeshVec::Vec<3, int>		Vec3i;
typedef trimeshVec::Vec<3, double>	Vec3d;
typedef trimeshVec::Vec<4, double>	Vec4d;

typedef trimeshVec::Vec<3, byte> Color3u;
typedef trimeshVec::Vec<4, byte> Color4u;
typedef Vec3d Color3d;
typedef Vec4d Color4d;
typedef Vec3d Normald;

#endif// _MATHDEFS_H_