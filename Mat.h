#pragma once

////////////////////////////////////////////////////////////////////////////
//
// used for 3d transformations
//
// | m00 m01 m02 m03 | <- x axis
// | m10 m11 m12 m13 | <- y axis
// | m20 m21 m22 m23 | <- z axis
// | m30 m31 m32 m33 | <- translate

#include "MathDefs.h"
using trimeshVec::Vec;

#define SWAP(x,y) \
	tmp = m[x][y];	\
	m[x][y] = m[y][x];	\
	m[y][x] = tmp;	

template <int D, class _Tp = float>
class Matrix
{
public:
	_Tp m[D][D];
	Matrix()
	{
		eye();
	}

	// No destructor or assignment operator needed

	// Constructor from anything that can be accessed using []
	// This one's pretty aggressive, so marked explicit
	template <class S> explicit Matrix(const S &x)
	{ for (int i = 0; i < D*D; i++) m[i] = _Tp(x[i]); }

	// Array reference and conversion to pointer - no bounds checking
	const _Tp* operator[] (int i) const
	{
		return m[i];
	}
	_Tp* operator[] (int i)
	{
		return m[i];
	}
	operator const _Tp*() const
	{
		return m;
	}
	operator _Tp*()
	{
		return m;
	}

	const _Tp& operator() (int y, int x) const
	{
		return m[y][x];
	};

	_Tp& operator() (int y, int x)
	{
		return m[y][x];
	};

	template <int D2>
	inline Vec<D2,_Tp> getCol(int y) const
	{
		VEC_STATIC_CHECK(D>=D2);
		Vec<D2,_Tp> res;
		for (int i=0; i<D2; ++i)
		{
			res[i] = m[y][i];
		}
		return res;
	};

	template <int D2>
	inline void setCol (unsigned int y, Vec<D2,_Tp> val)
	{
		VEC_STATIC_CHECK(D>=D2);
		for (int i=0; i<D2; ++i)
		{
			m[y][i] = val[i];
		}
	};

	// Member operators
	Matrix<D,_Tp> &operator += (const Matrix<D,_Tp> &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] += x[i];
		}
		return *this;
	}
	Matrix<D,_Tp> &operator -= (const Matrix<D,_Tp> &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] -= x[i];
		}
	}
	Matrix<D,_Tp> &operator *= (const Matrix<D,_Tp> &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] *= x[i];
		}
	}
	Matrix<D,_Tp> &operator *= (const _Tp &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] *= x;
		}
	}
	Matrix<D,_Tp> &operator /= (const Matrix<D,_Tp> &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] /= x[i];
		}
	}
	Matrix<D,_Tp> &operator /= (const _Tp &x)
	{
		for (int i=0; i<D*D; ++i)
		{
			m[i] /= x;
		}
	}

	// binary operatiors
	Matrix<D,_Tp>& operator * (const Matrix<D,_Tp> &x) const
	{
		Matrix<D,_Tp> res;
		for (int y=0; y<D; ++y)
		{
			for (int x=0; x<D; ++x)
			{
				res[y][x] = _Tp(0);
				for (int k=0; k<D; ++k)
				{
					res[y][x] += m[y][x] * x.m[y][x];
				}
			}
		}
		return res;
	}

	size_t size() const
	{
		return D;
	}
	_Tp *begin() { return &(m[0]); }
	const _Tp *begin() const { return &(m[0]); }
	_Tp *end() { return begin() + D*D; }
	const _Tp *end() const { return begin() + D*D; }
	void eye()
	{
		memset(m, 0, D*D*sizeof(_Tp));
		for (int i=0; i<D; ++i)
		{
			m[i][i] = _Tp(1);
		}
	}
	void zero()
	{
		memset(m, 0, D*D*sizeof(_Tp));
	}
	void transpose()
	{
		float tmp;
		for (int y=1; y<D; ++y)
		{
			for (int x=0; x<y; ++x)
			{
				SWAP(y,x);
			}
		}
	}

	// this * m2
	inline const Matrix<D,_Tp>& multiply(const Matrix<D,_Tp>& m2)
	{
		Matrix<D,_Tp> tmp(*this);
		for (int y=0; y<D; ++y)
		{
			for (int x=0; x<D; ++x)
			{
				m[y][x] = _Tp(0);
				for (int k=0; k<D; ++k)
				{
					m[y][x] +=  tmp(y,k) * m2(k,x);
				}
			}
		}
		return *this;
	}

	template<int D2>
	inline const Vec<D2,_Tp>& transformVec (Vec<D2,_Tp> &v) const
	{
		int DD = std::min(D,D2);
		int i,j;
		Vec<D,_Tp> tmp(v);
		for (i = 0; i < DD; i++)
		{
			v[i] = _Tp(0);
			for (j = 0; j < DD; j++)
				v[i] += m[i][j] * tmp[j];
		}
		return (v);
	};

	template <int D2>
	inline const Matrix<D,_Tp>& translate(const Vec<D2,_Tp> &v)
	{
		int DD = std::min(D,D2);
		eye();
		for (int i=0; i<DD; ++i)
		{
			m[i][D-1] = v[i];
		}
		return *this;
	}

	template <int D2>
	inline const Matrix<D,_Tp>& scale(const Vec<D2,_Tp> &v)
	{
		int DD = std::min(D-1,D2);
		eye();
		for (int i=0; i<DD; ++i)
		{
			m[i][i] = v[i];
		}
		return *this;
	}
};

template <class _Tp = float>
class Mat22 : public Matrix<2, _Tp>
{
public:
	template <typename T2>
	Mat22<_Tp>& rotate(T2 degree);

	template <typename T2>
	void map(T2 x, T2 y, T2 *tx, T2 *ty);

	//Mat22<T>& rotate(int degree);
	//Mat22<T>& rotate(float degree);
	//Mat22<T>& rotate(double degree);

	//void map(int x, int y, int *tx, int *ty);
	//void map(double x, double y, double *tx, double *ty);
};

template <class _Tp = float>
class Mat33 : public Matrix<3, _Tp>
{
public:
	template <typename T2>
	Mat33<_Tp>& rotate(T2 degree, const Vec3d& u);
};

template <class _Tp = float>
class Mat44 : public Matrix<4, _Tp>
{
};

#include "Mat.inl"