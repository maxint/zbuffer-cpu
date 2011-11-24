#ifndef _BASIC_STRUCTURE_H_
#define _BASIC_STRUCTURE_H_

#include "MathDefs.h"
#include "Mat.h"


//////////////////////////////////////////////////////////////////////////
// Render System Flags
//////////////////////////////////////////////////////////////////////////

// Target Type 
enum TargetType { SL_NONE = 0,
SL_TRIANGLES, SL_TRIANGLE_STRIP, SL_TRIANGLE_FAN,
SL_QUADS, SL_QUAD_STRIP, SL_POLYGON
};

// Light Type
enum LightType { SL_LIGHT_NONE = 0,
SL_LIGHT_POINT, SL_LIGHT_SPOT, SL_LIGHT_DIRECTIONAL };


//////////////////////////////////////////////////////////////////////////
// Useful Macro
//////////////////////////////////////////////////////////////////////////
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete (p); (p)=0; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] (p); (p)=0; }
#endif

//////////////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////////////

// Matrix
typedef Mat22<double> Mat22d;
typedef Mat33<double> Mat33d;
typedef Matrix<4, double> Mat44d;

// Light
class Light
{
public:
	LightType type;
	Color4d ambient;
	Color4d diffuse;
	Color4d specular;
	Vec4d position;
	Vec3d direction;
	float spot_falloff;
	float spot_exponent;
	float attenuation0;
	float attenuation1;
	float attenuation2;
	Light()
		: type(SL_LIGHT_NONE)
		, ambient(0, 0 ,0, 1)
		, diffuse(1, 1, 1, 1)
		, specular(1, 1, 1, 1)
		, position(0, 0, 1, 1)
		, direction(0, 0, -1)
		, spot_falloff(180)
		, attenuation0(1)
		, attenuation1(0)
		, attenuation2(0)
		, spot_exponent(30)
	{
	}
};

class Material
{
public:
	Color4d ambient;
	Color4d diffuse;
	Color4d ambient_diffuse;
	Color4d specular;
	double shiness;
	Color4d emission;

	Material()
		: ambient(0.2, 0.2, 0.2, 1.0)
		, diffuse(0.8, 0.8, 0.8, 1.0)
		, ambient_diffuse(0.5, 0.5, 0.5, 1.0)
		, specular(0.0, 0.0, 0.0, 1.0)
		, shiness(0.0)
		, emission(0.0, 0.0, 0.0, 1.0)
	{
	}
};

#endif //_BASIC_STRUCTURE_H_