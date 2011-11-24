#include "Point3D.h"
#include <cmath>
#include <cstdlib>

#define Pi 3.14159

using namespace std;
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
CPoint3D::CPoint3D( )	// normal constructor
{
	x = 0; y = 0; z = 0;
}

CPoint3D::CPoint3D( const CPoint3D &v )
{
	x = v.x;
	y = v.y;
	z = v.z;
}

CPoint3D::CPoint3D( int xx, int yy, int zz )	// normal constructor
{
	x = (float)xx; y = (float)yy; z = (float)zz;
}

CPoint3D::CPoint3D( float xx, float yy, float zz )	// normal constructor
{
	x = xx; y = yy; z = zz;
}

CPoint3D::CPoint3D( double xx, double yy, double zz )	// normal constructor
{
	x = (float)xx; y = (float)yy; z = (float)zz;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
CPoint3D::~CPoint3D(void)
{
}

//------------------------------------------------------------------------------
// Plus
//------------------------------------------------------------------------------
const CPoint3D& CPoint3D::operator +=(const CPoint3D &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

//------------------------------------------------------------------------------
// Subtraction
//------------------------------------------------------------------------------
const CPoint3D& CPoint3D::operator -=(const CPoint3D &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

//------------------------------------------------------------------------------
// CVectors plus
//------------------------------------------------------------------------------
CPoint3D CPoint3D::operator + ( const CPoint3D& v )
{
	CPoint3D r(*this);
	r += v;
	return	( r );
}

//------------------------------------------------------------------------------
// CVectors subtraction
//------------------------------------------------------------------------------
CPoint3D CPoint3D::operator - ( const CPoint3D& v )
{
	CPoint3D r(*this);
	r -= v;
	return	( r );
}

//------------------------------------------------------------------------------
// CVectors cross product
//------------------------------------------------------------------------------
CPoint3D CPoint3D::operator * ( const CPoint3D& v )
{
	CPoint3D r;

	r.x = y*v.z - v.y*z;
	r.y = v.x*z - x*v.z;
	r.z = x*v.y - v.x*y;

	return( r );
}

//------------------------------------------------------------------------------
// CPoint3D scale
//------------------------------------------------------------------------------
CPoint3D CPoint3D::operator * ( int k )		// scaled by int
{
	CPoint3D r;

	r.x = x * (float)k;
	r.y = y * (float)k;
	r.z = z * (float)k;

	return( r );
}
CPoint3D CPoint3D::operator * ( float k )	// scaled by float
{
	CPoint3D r;

	r.x = x * k;
	r.y = y * k;
	r.z = z * k;

	return( r );
}
CPoint3D CPoint3D::operator * ( double k )	// scaled by double
{
	CPoint3D r;

	r.x = x * (float)k;
	r.y = y * (float)k;
	r.z = z * (float)k;

	return( r );
}

//------------------------------------------------------------------------------
// CVectors dot product
//------------------------------------------------------------------------------
float CPoint3D::operator & ( const CPoint3D& v )
{
	return ( x * v.x + y * v.y + z * v.z );
}

//------------------------------------------------------------------------------
// CPoint3D length
//------------------------------------------------------------------------------
float	CPoint3D::length()
{
	return( (float)sqrt( x*x + y*y + z*z ) );
}

//------------------------------------------------------------------------------
// CPoint3D unify
//------------------------------------------------------------------------------
void CPoint3D::unify()
{
	float	len = length();

	if( len < 1.0e-20 )
	{
		x = float(rand()%1000+20);
		y = float(rand()%1000+20);
		z = float(rand()%1000+20);
		len=length();
		x/=len;
		y/=len;
		z/=len;
	}


	else
	{
		len = 1.0f/len;
		x *= len;
		y *= len;
		z *= len;
	}
}

void CPoint3D::RangeUnify(float min,float max)
{
	if (x>max)	x=max;
	if (y>max)	y=max;
	if (z>max)	z=max;

	if (x<min)	x=min;
	if (y<min)	y=min;
	if (z<min)	z=min;
}


//Project the vetor to new coordinate frame which is determined by coordX,coordY and coordZ
CPoint3D CPoint3D::ProjectToNewCoordinate(CPoint3D& coordX,CPoint3D& coordY,CPoint3D& coordZ)
{
	return CPoint3D((*this)&coordX,(*this)&coordY,(*this)&coordZ);
}


void VectorToAngle(CPoint3D& vec,float& fPdir,float& fAdir)
{
	//polar angle
	fPdir=atan2(sqrt(vec.x*vec.x+vec.z*vec.z),vec.y);

	//azimuthal angle
	fAdir=atan2(-vec.z,vec.x);
	if (fAdir<0) fAdir+=2*float(Pi);

}
//------------------------------------------------------------------------------
// CVectors blend product
//------------------------------------------------------------------------------
float blend_product(CPoint3D v1, CPoint3D v2, CPoint3D v3)
{
	return( ( v1 * v2 ) & v3 );
}

//------------------------------------------------------------------------------
// bulid a normal by three 3D points
//------------------------------------------------------------------------------
CPoint3D	BuildNormal( CPoint3D p1, CPoint3D p2, CPoint3D p3 )
{
	return ( ( p3-p2 ) * ( p1-p2 ) );
}