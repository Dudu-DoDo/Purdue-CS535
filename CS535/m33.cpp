#include "m33.h"
#include "MathTool.h"

V3 M33::GetColumn(int c)
{
	M33 m = *this;
	return V3(m[0][c], m[1][c], m[2][c]);
}

void M33::SetColumn(int c, V3 v)
{
	rows[0][c] = v[0];
	rows[1][c] = v[1];
	rows[2][c] = v[2];
}

float M33::Det()
{
	M33 m = *this;
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

void M33::SetRotate(int axis, float degree)
{
	auto &ret = *this;
	auto rad = Deg2Rad(degree);
	switch (axis)
	{
	case 0:		// x
		ret[0] = V3(1.0f, 0.0f, 0.0f);
		ret[1] = V3(0.0f, cos(rad), -sin(rad));
		ret[2] = V3(0.0f, sin(rad), cos(rad));
		break;
	case 1:		// y
		ret[0] = V3(cos(rad),0.0f, sin(rad));
		ret[1] = V3(0.0f, 1.0f, 0.0f);
		ret[2] = V3(-sin(rad), 0.0f, cos(rad));
		break;
	case 2:		// z
		ret[0] = V3(cos(rad), -sin(rad), 0.0f);
		ret[1] = V3(sin(rad), cos(rad), 0.0f);
		ret[2] = V3(0.0f, 0.0f, 1.0f);
		break;
	default:
		cerr << "Error axis!" << endl;
		return;
		break;
	}
}

M33 M33::Inverse()
{
	M33 m0 = *this;
	M33 m1 = M33();
	m1[0] = V3(m0[1][1] * m0[2][2] - m0[1][2] * m0[2][1],
		m0[0][2] * m0[2][1] - m0[0][1] * m0[2][2],
		m0[0][1] * m0[1][2] - m0[0][2] * m0[1][1]);

	m1[1] = V3(m0[1][2] * m0[2][0] - m0[1][0] * m0[2][2],
		m0[0][0] * m0[2][2] - m0[0][2] * m0[2][0],
		m0[0][2] * m0[1][0] - m0[0][0] * m0[1][2]);
	
	m1[2] = V3(m0[1][0] * m0[2][1] - m0[1][1] * m0[2][0],
		m0[0][1] * m0[2][0] - m0[0][0] * m0[2][1],
		m0[0][0] * m0[1][1] - m0[0][1] * m0[1][0]);

	return m1 / Det();
}

M33 M33::Transpose()
{
	M33 m = *this, res;
	res[0] = m.GetColumn(0);
	res[1] = m.GetColumn(1);
	res[2] = m.GetColumn(2);
	return res;
}

V3& M33::operator[](int r)
{
	return rows[r];
}

V3 M33::operator*(V3 v)
{
	V3 r0 = rows[0];
	V3 r1 = rows[1];
	V3 r2 = rows[2];
	return V3(r0*v, r1*v, r2*v);
}

M33 M33::operator*(M33 m1)
{
	M33 m0 = *this, res;
	V3 c0 = m1.GetColumn(0);
	V3 c1 = m1.GetColumn(1);
	V3 c2 = m1.GetColumn(2);
	res.SetColumn(0, m0*c0);
	res.SetColumn(1, m0*c1);
	res.SetColumn(2, m0*c2);

	return res;
}

M33 M33::operator*(float scf)
{
	M33 m = *this;
	m[0] = m[0] * scf;
	m[1] = m[1] * scf;
	m[2] = m[2] * scf;
	return m;
}

M33 M33::operator/(float scf)
{
	M33 m = *this;
	m[0] = m[0] / scf;
	m[1] = m[1] / scf;
	m[2] = m[2] / scf;
	return m;
}

bool M33::operator==(M33 m1)
{
	M33 m0 = *this;
	return m0[0] == m1[0]
		&& m0[1] == m1[1]
		&& m0[2] == m1[2];
}

bool M33::operator!=(M33 m1)
{
	M33 m0 = *this;
	return !(m0 == m1);
}

istream& operator>>(istream& ist, M33 &m)
{
	return ist >> m[0] >> m[1] >> m[2];
}

ostream& operator<<(ostream& ost, M33 m)
{
	return ost << m[0] << m[1] << m[2] << endl;
}
