#ifndef __VECTOPS_H_
#define __VECTOPS_H_

#include "MathDefs.h"

class VectOps
{
public:
	//////////////////////////////////////////////
	/*4x4 matrix m[16] structure
		0   4   8   12
	    1	5	9	13
		2	6	10	14
		3	7	11	15
	*/

	void static twist_to_mat(double omega[3], double theta, double rm[16]);
	
    void static mmMult(double m1[16], double m2[16], double m3[16] );

	void static mvMult(double m[16], double vs[4], double vd[4]);
	void static mvMult3x3(double m[9], double vs[3], double vd[3]);
	
	void static mmMult3x3(double m1[9], double m2[9], double m3[9]);

	void static mmAdd(double m1[9], double m2[9], double m3[9]);
	void static mmMinus(double m1[9], double m2[9], double m3[9]);
	void static mscale(double scale, double m1[9], double m2[9]);

	void static Transpose(double m1[9], double m2[9]);
	void static vec2mat(const Vec3d& v, double m[9]);		//compute vT * v, v is 3dimension, m is a 3x3 matrix
};

#endif //__VECTOPS_H_