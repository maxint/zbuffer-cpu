#ifndef _MAT_INL_
#define _MAT_INL_

template <typename _Tp>
template <typename T2>
Mat22<_Tp>& Mat22<_Tp>::rotate(T2 degree)
{
	double sina = 0, cosa = 0;

	if (degree == 90. || degree == -270.)
		sina = 1.;
	else if (degree == 270. || degree == -90.)
		sina = -1.;
	else if (degree == 180.)
		cosa = -1.;
	else{
		double b = 3.14159 * degree / 180.0;     // convert to radians
		sina = std::sin(double(b));               // fast and convenient
		cosa = std::cos(double(b));
	}
	m[0][0] = m[1][1] = cosa;
	m[0][1] = -sina;
	m[1][0] = sina;

	return *this;
}

template <typename _Tp>
template <typename T2>
void Mat22<_Tp>::map(T2 x, T2 y, T2 *tx, T2 *ty)
{
	*tx = T2(m[0][0]*x + m[0][1]*y);
	*ty = T2(m[1][0]*x + m[1][1]*y);
}

template <typename _Tp>
template <typename T2>
Mat33<_Tp>& Mat33<_Tp>::rotate(T2 degree, const Vec3d &u)
{
	double sina = 0, cosa = 0;

	if (degree == 90. || degree == -270.)
		sina = 1.;
	else if (degree == 270. || degree == -90.)
		sina = -1.;
	else if (degree == 180.)
		cosa = -1.;
	else{
		double b = 3.14159 * degree / 180.0;     // convert to radians
		sina = std::sin(double(b));               // fast and convenient
		cosa = std::cos(double(b));
	}
	double uxx = u[0]*u[0];
	double uyy = u[1]*u[1];
	double uzz = u[2]*u[2];
	double uxy = u[0]*u[1];
	double uxz = u[0]*u[2];
	double uyz = u[1]*u[2];
	m[0][0] = uxx+(1-uxx)*cosa;
	m[1][1] = uyy+(1-uyy)*cosa;
	m[2][2] = uzz+(1-uzz)*cosa;
	m[0][1] = uxy*(1-cosa)-u[2]*sina;
	m[0][1] = uxz*(1-cosa)+u[1]*sina;
	m[1][0] = uxy*(1-cosa)+u[2]*sina;
	m[1][2] = uyz*(1-cosa)-u[0]*sina;
	m[2][0] = uxz*(1-cosa)-u[1]*sina;
	m[2][1] = uyz*(1-cosa)+u[0]*sina;

	return *this;
}
#endif//_MAT_INL_