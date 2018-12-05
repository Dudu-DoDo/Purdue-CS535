#include <fstream>
#include "PPC.h"
#include "m33.h"
#include "framebuffer.h"
#include "global_variables.h"


PPC::PPC(int _w, int _h, float hfov):w(_w), h(_h)
{
	C = V3(0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(-0.5f*w, 0.5f*h, -0.5f*w / tan(deb2rad(hfov/2.0f)));
}

PPC::PPC(int _w, int _h, float hfov, float aperture) :w(_w), h(_h), lens_radius(aperture/2.0f)
{
	C = V3(0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(-0.5f*w, 0.5f*h, -0.5f*w / tan(deb2rad(hfov / 2.0f)));
}

void PPC::Translate(V3 v)
{
	C = C + v;
}

int PPC::Project(V3 P, V3& ProjP)
{
	M33 proj;
	proj.SetColumn(0, a);
	proj.SetColumn(1, b);
	proj.SetColumn(2, c);
	ProjP[0] = FLT_MAX;

	V3 PPixel = proj.Inverse()*(P - C);
	if (w <= 0.0f)
		return 0;

	auto w = PPixel[2];
	auto u = PPixel[0] / w;
	auto v = PPixel[1] / w;

	ProjP = V3(u, v, 1.0f / w);
	return 1;
}

bool PPC::inside_image_plane(V3 pp)
{
	int u = static_cast<int>(pp[0]);
	int v = static_cast<int>(pp[1]);

	if (pp[2] <= 0.0f)
		return false;

	if (u < 0 || u > w - 1)
		return false;

	if (v < 0 || v > h - 1)
		return false;

	return true;
}

V3 PPC::GetVD()
{
	return (a^b).UnitVector();
}

float PPC::GetFocal()
{
	return (a^b).UnitVector() * c;
}

float PPC::GetHorizontalFOV()
{
	return 2.0f * Rad2Deg(atan(static_cast<float>(w) / 2.0f * a.Length() / GetFocal()));
}

float PPC::GetVerticalFOV()
{
	return 2.0f * Rad2Deg(atan(static_cast<float>(h) / 2.0f * b.Length() / GetFocal()));
}

V3 PPC::GetRay(int u, int v)
{
	return c + a * (static_cast<float>(u) + 0.5f) + b * (static_cast<float>(v) + 0.5f);
}

V3 PPC::GetRay(float u, float v)
{
	return c + a * u + b * v;
}

ray PPC::GetRayWithAperture(float u, float v)
{
	V3 rd = random_in_unit_disk() * lens_radius;
	V3 offset = a.UnitVector() * rd.x() + b.UnitVector() * rd.y();
	return ray(C + offset, c + a * u + b * v - offset);
}

V3 PPC::GetRayCenter(int u, int v)
{
	return C + GetRay(u, v);
}

V3 PPC::Unproject(V3 pp)
{
	V3 vec = (a*pp[0] + b * pp[1] + c);
	float w = 1.0f / pp[2];
	return C + vec * w;
}

V3 PPC::UnprojectPixel(float uf, float vf, float currf)
{
	return C + (a*uf + b * vf + c)* currf * (1.0f / GetFocal());
}

void PPC::Pan(float theta)
{
	a = a.Rotate(b.UnitVector()*(-1.0f), theta);
	c = c.Rotate(b.UnitVector()*(-1.0f), theta);
}

void PPC::Tilt(float theta)
{
	b = b.Rotate(a.UnitVector(), theta);
	c = c.Rotate(a.UnitVector(), theta);
}

void PPC::Roll(float theta)
{
	V3 dir = (a ^ b).UnitVector();
	a = a.Rotate(dir, theta);
	b = b.Rotate(dir, theta);
	c = c.Rotate(dir, theta);
}

void PPC::RevolveH(V3 p, float theta)
{
	V3 rC = C.RotateThisPointAboutArbitraryAxis(p, V3(0.0f, 1.0f, 0.0f), theta);
	
	PositionAndOrient(rC, p, V3(0.0f, 1.0f, 0.0f));
}

void PPC::RevolveV(V3 p, float theta)
{
	V3 rC = C.RotateThisPointAboutArbitraryAxis(p, V3(1.0f, 0.0f, 0.0f), theta);
	
	V3 newa, newb, newc;
	V3 newvd = (p - rC).UnitVector();
	float f = GetFocal();
	newb = (newvd ^ V3(1.0f, 0.0f, 0.0f)).UnitVector() * b.Length();
	newa = (newb ^ newvd).UnitVector() * a.Length();
	newc = (newvd * f) - newa * static_cast<float>(w) / 2.0f - newb * static_cast<float>(h) / 2.0f;

	a = newa;
	b = newb;
	c = newc;
	C = rC;
}

void PPC::PositionAndOrient(V3 newC, V3 lap, V3 up)
{
	V3 newa, newb, newc;
	V3 newvd = (lap - newC).UnitVector();
	float f = GetFocal();
	newa = (newvd ^ up).UnitVector() * a.Length();
	newb = (newvd ^ newa).UnitVector() * b.Length();
	newc = (newvd * f) - newa * static_cast<float>(w) / 2.0f - newb * static_cast<float>(h) / 2.0f;

	a = newa;
	b = newb;
	c = newc;
	C = newC;
}

void PPC::PositionAndOrient(V3 newC, V3 lap, V3 up, float aperture, float focal)
{
	V3 newa, newb, newc;
	V3 newvd = (lap - newC).UnitVector();
	float scaf = focal / GetFocal();
	float f = GetFocal() * scaf;
	newa = (newvd ^ up).UnitVector() * a.Length() * scaf;
	newb = (newvd ^ newa).UnitVector() * b.Length() * scaf;
	newc = (newvd * f) - newa * static_cast<float>(w) / 2.0f  - newb * static_cast<float>(h) / 2.0f;

	a = newa;
	b = newb;
	c = newc;
	C = newC;
	lens_radius = aperture /2.0f;
}

void PPC::Zoom(float scf)
{
	V3 vd = GetVD();
	float f = GetFocal();
	float newf = f * scf;
	c = c + vd * (newf-f);
}

void PPC::SetInterpolated(PPC* ppc0, PPC* ppc1, float fract)
{
	*this = *ppc0;

	V3 newC = ppc0->C + (ppc1->C - ppc0->C) * fract;
	V3 newvd = ppc0->GetVD() + (ppc1->GetVD() - ppc0->GetVD())*fract;
	V3 up = (ppc0->b + (ppc1->b - ppc0->b)*fract)*-1.0f;
	PositionAndOrient(newC, newC + newvd, up);
}

void PPC::SaveBin(std::string fname)
{
	ofstream of(fname.c_str(), ofstream::out | ostream::binary);
	if (of.is_open())
	{
		of.write((char *)&C, sizeof(float) * 3);
		of.write((char *)&a, sizeof(float) * 3);
		of.write((char *)&b, sizeof(float) * 3);
		of.write((char *)&c, sizeof(float) * 3);
		of.write((char *)&w, sizeof(int));
		of.write((char *)&h, sizeof(int));
	}
	else
		cerr << "File " << fname.c_str() << " cannot open to write!\n";
	
	cerr << "File " << fname.c_str() << " written!\n";
	of.close();
}

void PPC::LoadBin(std::string fname)
{
	ifstream input(fname.c_str(),ifstream::in | ifstream::binary);
	if (input.is_open())
	{
		input.read((char*)&C, sizeof(float) * 3);
		input.read((char*)&a, sizeof(float) * 3);
		input.read((char*)&b, sizeof(float) * 3);
		input.read((char*)&c, sizeof(float) * 3);
		input.read((char*)&w, sizeof(int));
		input.read((char*)&h, sizeof(int));
	}
	else
		cerr << "File " << fname.c_str() << " cannot open!\n";
	
	input.close();
}

void PPC::MoveForward(float delta)
{
	V3 vd = GetVD();
	C = C + vd * delta;
}

void PPC::MoveLeft(float delta)
{
	C = C + a * delta;
}

void PPC::MoveDown(float delta)
{
	C = C + b * delta;
}

void PPC::SetIntrinsicsHW()
{
	glViewport(0, 0, w, h);
	
	float left = -a.Length() * static_cast<float>(w) / 2.0f * scf;
	float right = -left;
	float top = b.Length() * static_cast<float>(h) / 2.0f * scf;
	float bottom = -top;

	float zNear = 1.0f;
	float zFar = 100.0f;
	float scf = zNear / GetFocal();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
}

void PPC::SetExtrinsicsHW()
{
	V3 L = C + GetVD() * 100.0f;
	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], L[0], L[1], L[2], -b[0], -b[1], -b[2]);
}

