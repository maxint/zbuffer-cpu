#pragma once

class CPoint3D
{
public:
	float	x, y, z;

	CPoint3D ( );						// normal constructor
	CPoint3D ( const CPoint3D &v );		// normal constructor
	CPoint3D ( int, int, int );			// normal constructor
	CPoint3D ( float, float, float );	// normal constructor
	CPoint3D ( double, double, double );// normal constructor
	~CPoint3D ();

	// arithmetic operations
	const CPoint3D&	operator += ( const CPoint3D& v);
	const CPoint3D&	operator -= ( const CPoint3D& v);

	float	operator & ( const CPoint3D& v );		// doc_product
	CPoint3D	operator + ( const CPoint3D& v );
	CPoint3D	operator - ( const CPoint3D& v );
	CPoint3D	operator * ( const CPoint3D& v );		// cross_product
	CPoint3D	operator * ( int k );			// scale by int
	CPoint3D	operator * ( float k );			// scale by float
	CPoint3D	operator * ( double k );		// scale by double
	//		int	operator = ( CPoint3D& v );
	void	unify();
	void	RangeUnify(float min,float max);
	float	length();
	//把自己投影到新的坐标系中间
	CPoint3D ProjectToNewCoordinate(CPoint3D& coordX,CPoint3D& coordY,CPoint3D& coordZ);

};
