#include "v3.h"

#include "m33.h"

V3::V3(float x)
{
	xyz[0] = xyz[1] = xyz[2] = x;
}

V3::V3(float x, float y, float z) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

}

V3 V3::UnitVector()
{
	V3 v = *this;
	return (equal(Length(), 0.0f)) ? 0.0f : v / Length();
}

float V3::Length()
{
	V3 v = *this;
	return std::sqrt(v*v);
}

void V3::SetColor(unsigned int color)
{
	V3 &v = *this;
	unsigned char*rgba = (unsigned char*)&color;
	v[0] = static_cast<float>(rgba[0]) / 255.0f;
	v[1] = static_cast<float>(rgba[1]) / 255.0f;
	v[2] = static_cast<float>(rgba[2]) / 255.0f;
}

unsigned int V3::GetColor()
{
	// clip to (0.0f,1.0f)
	V3 v = *this;
	unsigned int ret = 0xFF000000;
	unsigned char *rgba = (unsigned char*)&ret;
	for(int i = 0; i < 3; ++i)
	{
		int ichan = static_cast<int>(v[i] * 255.0f);
		ichan = std::max(0, ichan);
		ichan = std::min(ichan, 255);
		rgba[i] = ichan;
	}
	return ret;
}


V3 V3::cross(V3 v1)
{
	V3 v0 = *this;
	return V3(
		v0[1] * v1[2] - v0[2] * v1[1],
		v0[2] * v1[0] - v0[0] * v1[2],
		v0[0] * v1[1] - v0[1] * v1[0]);
}

V3 V3::Rotate(V3 a, float angled)
{
	V3 v = *this, x(1.0f,0.0f,0.0f),y(0.0f,1.0f,0.0f),b,c, ret;
	auto xa = abs(x*a);
	auto ya = abs(y*a);
	V3 aux = x;
	if(xa > ya)
	{
		aux = y;
	}
	aux = aux.UnitVector();

	V3 a0 = (aux^a).UnitVector();
	V3 a2 = (a0^a).UnitVector();

	// from origin to new coord
	auto rad = Deg2Rad(angled);
	M33 lcs(a0, a, a2);
	M33 rot;
	rot.SetRotate(1, angled);
	M33 iao = lcs.Inverse();
	return iao * rot * lcs * v;
}

V3 V3::RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	V3 p = *this;
	V3 OP = p - O;
	if (OP == V3(0.0f))
	{
		cerr << "Rotation is too close to rotation center! \n";
		return p;
	}

	OP = OP.Rotate(a, angled);
	return OP + O;
}

V3 V3::Reflect(V3 n)
{
	V3 v = *this, ret(0.0f);
	n = n.UnitVector();
	ret  = n * (v*n)*2.0f - v;
	return ret;
}

V3 V3::Refract(V3 n, float ratio)
{
	// default is eye vector
	V3 alpha = (V3(0.0f) - *this).UnitVector();
	n = n.UnitVector();
	float cosi = alpha * n;
	V3 alphaY = n * (alpha * n);
	V3 alphaX = alpha - alphaY;
	float n1 = 1.0f, n2 = ratio;

	cosi < 0.0f ? cosi = -cosi : swap(n1, n2);

	float n1n2 = n1 / n2;
	float k = 1.0f - n1n2 * n1n2 * (1.0f - cosi * cosi);

	return k < 0.0f ? V3(0.0f) : alphaX * n1n2 + alphaY * (sqrt(k) / cosi);
}

float& V3::operator[](int i)
{
	return xyz[i];
}

float V3::operator*(V3 v1)
{
	V3 v0 = *this;
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

V3 V3::operator*(float scf)
{
	V3 v = *this;
	return V3(v[0] * scf, v[1] * scf, v[2] * scf);
}

V3 V3::operator/(float scf)
{
	V3 v = *this;
	return V3(v[0] / scf, v[1] / scf, v[2] / scf);
}

V3 V3::operator+(V3 v1)
{
	V3 v0 = *this, ret;
	ret[0] = v0[0] + v1[0];
	ret[1] = v0[1] + v1[1];
	ret[2] = v0[2] + v1[2];
	return ret;
}

V3 V3::operator+(float f)
{
	V3 &v = *this;
	v[0] += f;
	v[1] += f;
	v[2] += f;
	return v;
}

V3 V3::operator-(V3 v1)
{
	V3 v0 = *this;
	return V3(v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]);
}

V3 V3::operator^(V3 v1)
{
	return cross(v1);
}

bool V3::operator==(V3 v1)
{
	auto equal = [&](float a, float b)
	{
		return std::abs(a - b) < 1e-6;
	};

	V3 v0 = *this;
	return equal(v0[0], v1[0])
		&& equal(v0[1], v1[1])
		&& equal(v0[2], v1[2]);
}

bool V3::operator!=(V3 v1)
{
	V3 v0 = *this;
	return !(v0 == v1);
}

istream& operator>>(istream& ist, V3 &v)
{
	return ist >> v[0] >> v[1] >> v[2];
}

ostream& operator<<(ostream& ost, V3 v)
{
	return ost << v[0] << " " << v[1] << " " << v[2] << endl;
}
