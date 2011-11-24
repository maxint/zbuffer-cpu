#ifndef VEC_H
#define VEC_H
/*
Szymon Rusinkiewicz
Princeton University

Vec.h
Class for a constant-length vector

Supports the following operations:
vec v1;			// Initialized to (0,0,0)
vec v2(1,2,3);		// Initializes the 3 components
vec v3(v2);		// Copy constructor
float farray[3];
vec v4 = vec(farray);	// Explicit: "v4 = farray" won't work
Vec<3,double> vd;	// The "vec" used above is Vec<3,float>
point p1, p2, p3;	// Same as vec

v3 = v1 + v2;		// Also -, *, /  (all componentwise)
v3 = 3.5f * v1;		// Also vec * scalar, vec / scalar
// NOTE: scalar has to be the same type:
// it won't work to do double * vec<float>

v3 = v1 DOT v2;		// Actually operator^
v3 = v1 CROSS v2;	// Actually operator%

float f = v1[0];	// Subscript
float *fp = v1;		// Implicit conversion to float *

f = len(v1);		// Length (also len2 == squared length)
f = dist(p1, p2);	// Distance (also dist2 == squared distance)
normalize(v1);		// Normalize (i.e., make it unit length)
// normalize(vec(0,0,0)) => vec(1,0,0)

cout << v1 << endl;	// iostream output in the form (1,2,3)
cin >> v2;		// iostream input using the same syntax

Also defines the utility functions sqr, cube, sgn, swap,
fract, clamp, mix, step, smoothstep, and trinorm
*/


// Windows defines these as macros, which prevents us from using the
// type-safe versions from std::, as well as interfering with method defns
#undef min
#undef max


#include <cmath>
#include <iostream>
#include <algorithm>
using std::min;
using std::max;
using std::swap;
using std::sqrt;
class CPoint3D;


// Let gcc optimize conditional branches a bit better...
#ifndef likely
#  if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#    define likely(x) (x)
#    define unlikely(x) (x)
#  else
#    define likely(x)   (__builtin_expect((x), 1))
#    define unlikely(x) (__builtin_expect((x), 0))
#  endif
#endif


// Boost-like compile-time assertion checking
template <bool X> struct VEC_STATIC_ASSERTION_FAILURE;
template <> struct VEC_STATIC_ASSERTION_FAILURE<true>
{ void operator () () {} };
#define VEC_STATIC_CHECK(expr) VEC_STATIC_ASSERTION_FAILURE<bool(expr)>()



namespace trimeshVec {

	template <int D, class _Tp = float>
	class Vec {
	private:
		_Tp v[D];

	public:
		// Constructor for no arguments.  Everything initialized to 0.
		Vec() { for (int i = 0; i < D; i++) v[i] = _Tp(0); }

		// Constructors for 2-4 arguments
		Vec(_Tp x, _Tp y)
		{ VEC_STATIC_CHECK(D == 2); v[0] = x; v[1] = y; }
		Vec(_Tp x, _Tp y, _Tp z)
		{ VEC_STATIC_CHECK(D == 3); v[0] = x; v[1] = y; v[2] = z; }
		Vec(_Tp x, _Tp y, _Tp z, _Tp w)
		{ VEC_STATIC_CHECK(D == 4); v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
		//  [12/27/2009 maxint]
		template<class T2>
		Vec(const Vec<D, T2> &x)
		{
			for (int i=0; i<D; ++i)
				v[i] = x[i];		
		}
		//  [12/29/2009 maxint]
		template <int D2>
		Vec(const Vec<D2,_Tp> &x)
		{
			VEC_STATIC_CHECK(D2>=D);
			for (int i=0; i<D; ++i)
				v[i] = x[i];
		}
		Vec(const CPoint3D& p)
		{
			if (D==3)
			{
				v[0]=p.x;
				v[1]=p.y;
				v[2]=p.z;
			}
		}

		// Constructor from anything that can be accessed using []
		// This one's pretty aggressive, so marked explicit
		template <class S> explicit Vec(const S &x)
		{ for (int i = 0; i < D; i++) v[i] = _Tp(x[i]); }

		// No destructor or assignment operator needed

		// Array reference and conversion to pointer - no bounds checking
		const _Tp &operator [] (int i) const
		{ return v[i]; }
		_Tp &operator [] (int i)
		{ return v[i]; }
		operator const _Tp * () const
		{ return v; }
		operator const _Tp * ()
		{ return v; }
		operator _Tp * ()
		{ return v; }

		// Member operators
		Vec<D,_Tp> &operator += (const Vec<D,_Tp> &x)
		{ for (int i = 0; i < D; i++) v[i] += x[i];  return *this; }
		Vec<D,_Tp> &operator -= (const Vec<D,_Tp> &x)
		{ for (int i = 0; i < D; i++) v[i] -= x[i];  return *this; }
		Vec<D,_Tp> &operator *= (const Vec<D,_Tp> &x)
		{ for (int i = 0; i < D; i++) v[i] *= x[i];  return *this; }
		Vec<D,_Tp> &operator *= (const _Tp &x)
		{ for (int i = 0; i < D; i++) v[i] *= x;     return *this; }
		Vec<D,_Tp> &operator /= (const Vec<D,_Tp> &x)
		{ for (int i = 0; i < D; i++) v[i] /= x[i];  return *this; }
		Vec<D,_Tp> &operator /= (const _Tp &x)
		{ for (int i = 0; i < D; i++) v[i] /= x;     return *this; }
		
		//  [12/28/2009 maxint]
		template <class T2>
		const Vec<D,_Tp> operator = (const Vec<D,T2> &x)
		{
			for (int i=0; i<D; ++i)
				v[i] = x[i];
			return *this;
		}

		// Outside of class: + - * / % ^ << >>

		// Some partial compatibility with valarrays and vectors
		typedef _Tp value_type;
		size_t size() const
		{ return D; }
		_Tp sum() const
		{ _Tp total = v[0];
		for (int i = 1; i < D; i++) total += v[i];
		return total; }
		_Tp avg() const
		{ return sum() / D; }
		_Tp product() const
		{ _Tp total = v[0];
		for (int i = 1; i < D; i++) total *= v[i];
		return total; }
		_Tp min() const
		{ _Tp m = v[0];
		for (int i = 0; i < D; i++)
			if (v[i] < m)  m = v[i];
		return m; }
		_Tp max() const
		{ _Tp m = v[0];
		for (int i = 1; i < D; i++)
			if (v[i] > m)  m = v[i];
		return m; }
		_Tp *begin() { return &(v[0]); }
		const _Tp *begin() const { return &(v[0]); }
		_Tp *end() { return begin() + D; }
		const _Tp *end() const { return begin() + D; }
		void clear() { for (int i = 0; i < D; i++) v[i] = _Tp(0); }
		bool empty() const
		{ for (int i = 0; i < D; i++)
		if (v[i]) return false;
		return true; }
		//  [12/27/2009 maxint]
		Vec<D,_Tp> normed() const
		{
			Vec<D,_Tp> res = (*this);
			_Tp sum = v[0] * v[0];
			for (int i = 1; i < D; i++)
				sum += v[i] * v[i];
			sum = sqrt(sum);
			for (int i=0; i<D; ++i)
				res[i] /= sum;	
			return res;
		}
		const Vec<D,_Tp>& norm()
		{
			_Tp sum = v[0] * v[0];
			for (int i = 1; i < D; i++)
				sum += v[i] * v[i];
			sum = sqrt(sum);
			for (int i=0; i<D; ++i)
				v[i] /= sum;		
			return *this;
		}
		_Tp length()
		{
			_Tp sum = v[0] * v[0];
			for (int i = 1; i < D; i++)
				sum += v[i] * v[i];
			sum = sqrt(sum);
			return sum;
		}
	};

	typedef Vec<3,float> vec;
	typedef Vec<3,float> point;
	typedef Vec<2,float> vec2;
	typedef Vec<3,float> vec3;
	typedef Vec<4,float> vec4;
	typedef Vec<2,int> ivec2;
	typedef Vec<3,int> ivec3;
	typedef Vec<4,int> ivec4;


	// Nonmember operators that take two Vecs
	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator + (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		return Vec<D,_Tp>(v1) += v2;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator - (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		return Vec<D,_Tp>(v1) -= v2;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator * (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		return Vec<D,_Tp>(v1) *= v2;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator / (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		return Vec<D,_Tp>(v1) /= v2;
	}

#define DOT ^
	template <int D, class _Tp>
	static inline const _Tp operator ^ (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		_Tp sum = v1[0] * v2[0];
		for (int i = 1; i < D; i++)
			sum += v1[i] * v2[i];
		return sum;
	}
	template <class _Tp>
	static inline const _Tp operator ^ (const Vec<3,_Tp> &v1, const Vec<4,_Tp> &v2)
	{
		_Tp sum = v1[0] * v2[0];
		for (int i = 1; i < 3; i++)
			sum += v1[i] * v2[i];
		return sum;
	}
	template <class _Tp>
	static inline const _Tp operator ^ (const Vec<4,_Tp> &v1, const Vec<3,_Tp> &v2)
	{
		return (v2 DOT v1);
	}

#define CROSS %
	template <class _Tp>
	static inline const Vec<3,_Tp> operator % (const Vec<3,_Tp> &v1, const Vec<3,_Tp> &v2)
	{
		return Vec<3,_Tp>(v1[1]*v2[2] - v1[2]*v2[1],
			v1[2]*v2[0] - v1[0]*v2[2],
			v1[0]*v2[1] - v1[1]*v2[0]);
	}
	template <class _Tp>
	static inline const Vec<3,_Tp> operator % (const Vec<4,_Tp> &v1, const Vec<4,_Tp> &v2)
	{
		return Vec<3,_Tp>(v1[1]*v2[2] - v1[2]*v2[1],
			v1[2]*v2[0] - v1[0]*v2[2],
			v1[0]*v2[1] - v1[1]*v2[0]);
	}

	// Component-wise equality and inequality (#include the usual caveats
	// about comparing floats for equality...)
	template <int D, class _Tp>
	static inline bool operator == (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		for (int i = 0; i < D; i++)
			if (v1[i] != v2[i])
				return false;
		return true;
	}

	template <int D, class _Tp>
	static inline bool operator != (const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		for (int i = 0; i < D; i++)
			if (v1[i] != v2[i])
				return true;
		return false;
	}


	// Unary operators
	template <int D, class _Tp>
	static inline const Vec<D,_Tp> &operator + (const Vec<D,_Tp> &v)
	{
		return v;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator - (const Vec<D,_Tp> &v)
	{
		Vec<D,_Tp> result(v);
		for (int i = 0; i < D; i++)
			result[i] = -result[i];
		return result;
	}

	template <int D, class _Tp>
	static inline bool operator ! (const Vec<D,_Tp> &v)
	{
		return v.empty();
	}


	// Vec/scalar operators
	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator * (const _Tp &x, const Vec<D,_Tp> &v)
	{
		Vec<D,_Tp> result(v);
		for (int i = 0; i < D; i++)
			result[i] = x * result[i];
		return result;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator * (const Vec<D,_Tp> &v, const _Tp &x)
	{
		return Vec<D,_Tp>(v) *= x;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator / (const _Tp &x, const Vec<D,_Tp> &v)
	{
		Vec<D,_Tp> result(v);
		for (int i = 0; i < D; i++)
			result[i] = x / result[i];
		return result;
	}

	template <int D, class _Tp>
	static inline const Vec<D,_Tp> operator / (const Vec<D,_Tp> &v, const _Tp &x)
	{
		return Vec<D,_Tp>(v) /= x;
	}


	// iostream operators
	template <int D, class _Tp>
	static inline std::ostream &operator << (std::ostream &os, const Vec<D,_Tp> &v)

	{
		os << "(";
		for (int i = 0; i < D-1; i++)
			os << v[i] << ", ";
		return os << v[D-1] << ")";
	}

	template <int D, class _Tp>
	static inline std::istream &operator >> (std::istream &is, Vec<D,_Tp> &v)
	{
		char c1 = 0, c2 = 0;

		is >> c1;
		if (c1 == '(' || c1 == '[') {
			is >> v[0] >> std::ws >> c2;
			for (int i = 1; i < D; i++) {
				if (c2 == ',')
					is >> v[i] >> std::ws >> c2;
				else
					is.setstate(std::ios::failbit);
			}
		}

		if (c1 == '(' && c2 != ')')
			is.setstate(std::ios::failbit);
		else if (c1 == '[' && c2 != ']')
			is.setstate(std::ios::failbit);

		return is;
	}


	// Utility functions for square and cube, to go along with sqrt and cbrt
	template <class _Tp>
	static inline _Tp sqr(const _Tp &x)
	{
		return x*x;
	}

	template <class _Tp>
	static inline _Tp cube(const _Tp &x)
	{
		return x*x*x;
	}


	// Utility functions based on GLSL
	template <class _Tp>
	static inline _Tp fract(const _Tp &x)
	{
		return x - floor(x);
	}

	template <class _Tp>
	static inline _Tp clamp(const _Tp &x, const _Tp &a, const _Tp &b)
	{
		return x > a ? x < b ? x : b : a;  // returns a on NaN
	}

	template <class _Tp, class S>
	static inline _Tp mix(const _Tp &x, const _Tp &y, const S &a)
	{
		return (S(1)-a) * x + a * y;
	}

	template <class _Tp>
	static inline _Tp step(const _Tp &x, const _Tp &a)
	{
		return x < a ? _Tp(0) : _Tp(1);
	}

	template <class _Tp>
	static inline _Tp smoothstep(const _Tp &x, const _Tp &a, const _Tp &b)
	{
		if (b <= a) return step(x,a);
		_Tp t = (x - a) / (b - a);
		return t <= _Tp(0) ? _Tp(0) : t >= _Tp(1) ? _Tp(1) : t * t * (_Tp(3) - _Tp(2) * t);
	}

	// Area-weighted triangle face normal
	template <class _Tp>
	static inline _Tp trinorm(const _Tp &v0, const _Tp &v1, const _Tp &v2)
	{
		return (typename _Tp::value_type) 0.5 * ((v1 - v0) CROSS (v2 - v0));
	}

	// Sign of a scalar
	template <class _Tp>
	static inline _Tp sgn(const _Tp &x)
	{
		return (x < _Tp(0)) ? _Tp(-1) : _Tp(1);
	}


	// Functions on Vecs
	template <int D, class _Tp>
	static inline const _Tp len2(const Vec<D,_Tp> &v)
	{
		_Tp l2 = v[0] * v[0];
		for (int i = 1; i < D; i++)
			l2 += v[i] * v[i];
		return l2;
	}

	template <int D, class _Tp>
	static inline const _Tp len(const Vec<D,_Tp> &v)
	{
		return sqrt(len2(v));
	}

	template <int D, class _Tp>
	static inline const _Tp dist2(const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		_Tp d2 = sqr(v2[0]-v1[0]);
		for (int i = 1; i < D; i++)
			d2 += sqr(v2[i]-v1[i]);
		return d2;
	}

	template <int D, class _Tp>
	static inline const _Tp dist(const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		return sqrt(dist2(v1,v2));
	}

	template <int D, class _Tp>
	static inline Vec<D,_Tp> normalize(Vec<D,_Tp> &v)
	{
		_Tp l = len(v);
		if (unlikely(l <= _Tp(0))) {
			v[0] = _Tp(1);
			for (int i = 1; i < D; i++)
				v[i] = _Tp(0);
			return v;
		}

		l = _Tp(1) / l;
		for (int i = 0; i < D; i++)
			v[i] *= l;

		return v;
	}

	template <int D, class _Tp>
	static inline void swap(const Vec<D,_Tp> &v1, const Vec<D,_Tp> &v2)
	{
		for (int i = 0; i < D; i++)
			swap(v1[i], v2[i]);
	}

	template <int D, class _Tp>
	static inline Vec<D,_Tp> fabs(const Vec<D,_Tp> &v)
	{
		Vec<D,_Tp> result(v);
		for (int i = 0; i < D; i++)
			if (result[i] < _Tp(0))
				result[i] = -result[i];
		return result;
	}
}

#endif
