#include "VectOps.h"
#include <cmath>

// m1, m2 and m3 are 16-vectors representing 4x4 matrices
void VectOps::mmMult(double m1[16], double m2[16], double m3[16])
 {
	//var i:int, j:int, k:int;
	int i, j, k;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
		{
			m3[j*4+i] = 0;
			for(k=0; k<4; k++)
				m3[j*4+i] += m1[k*4+i]*m2[j*4+k];
		}
}

// m1, m2 and m3 are 9-vectors representing 3x3 matrices
void VectOps::mmMult3x3(double m1[9], double m2[9], double m3[9])
{
	//var i:int, j:int, k:int;
	int i, j, k;
	for(i=0; i<3; i++)
		for(j=0; j<3; j++)
		{
			m3[j*3+i] = 0;
			for(k=0; k<3; k++)
				m3[j*3+i] += m1[k*3+i]*m2[j*3+k];
		}
}

//
void VectOps::twist_to_mat(double omega[3], double theta, double rm[16])
{
	double omega_hat[16] = {0, omega[2], -omega[1], 0,
							-omega[2], 0, omega[0], 0,
							omega[1], -omega[0], 0, 0,
							0, 0, 0, 1};
	double unit[16] = {1, 0, 0, 0,
					  0, 1, 0, 0,
					  0, 0, 1, 0,
					  0, 0, 0, 1};
	double omega_square[16];//:Array = new Array(16);
	//double temprm[16];
	double sinth, costh;
	int i;
	mmMult(omega_hat, omega_hat, omega_square);
	sinth= sin(theta);
	costh= cos(theta);
	for(i=0; i< 16; i++)
		rm[i]= unit[i] + omega_hat[i]*sinth + omega_square[i]*(1-costh);
	rm[15]= 1;
// 	for (i = 0; i < 3; i++)
// 		for (int j = 0; j < 3; j++)
// 		{
// 			rm[3*i + j] = temprm[4*i + j];
// 		}
	
// 		
// 		omega_hat = unit = omega_square = null;
}
// vs and vd are 4-vectors
void VectOps::mvMult(double m[16], double vs[4], double vd[4])
{
	//var i:int, j:int;
	int i, j;
	for(i=0; i<4; i++)
	{
		vd[i]=0;
		for(j=0; j<4; j++)
			vd[i] += m[j*4+i]*vs[j];
	}
}

// vs and vd are 3-vectors
void VectOps::mvMult3x3(double m[9], double vs[3], double vd[3])
{
	//var i:int, j:int;
	int i, j;
	for(i=0; i<3; i++)
	{
		vd[i]=0;
		for(j=0; j<3; j++)
			vd[i] += m[j*3+i]*vs[j];
	}
}

void VectOps::mmAdd(double m1[9], double m2[9], double m3[9])
{
	for (int i = 0; i < 9; i++)
	{
		m3[i] =  m1[i] + m2[i];
	}
}

void VectOps::mmMinus(double m1[9], double m2[9], double m3[9])
{
	for (int i = 0; i < 9; i++)
	{
		m3[i] =  m1[i] - m2[i];
	}
}

void VectOps::mscale(double scale, double m1[9], double m2[9])
{
	for (int i = 0; i < 9; i++)
	{
		m2[i] = scale * m1[i];
	}
}

void VectOps::Transpose(double m1[9], double m2[9])
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			m2[i*3 + j] = m1[j *3 +i];
		}
}

void VectOps::vec2mat(const Vec3d& v, double m[9])
{
	for (int i = 0; i < 3; i++)
	{
		m[0] = v[0] * v[0];
		m[1] = m[3] = v[0] * v[1];
		m[2] = m[6] = v[0] * v[2];
		m[4] = v[1] * v[1];
		m[5] = m[7] = v[1] * v[2];
		m[8] = v[2] * v[2];
	}
}