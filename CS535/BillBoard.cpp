#include <fstream>
#include "BillBoard.h"
#include "global_variables.h"

BillBoard::BillBoard()
{
}

BillBoard::~BillBoard()
{
}

void BillBoard::set_billboard(V3 O, V3 n, V3 up, float sz, float s, float t) {
	mesh = make_shared<TM>();
	mesh->set_bilboard(O, n, up, sz, s, t);
}

bool BillBoard::intersect(V3 p, V3 d, float &t)
{
	V3 b0 = mesh->verts[0];
	V3 bn = (mesh->normals[0]).UnitVector();

	t = (b0 - p) * bn / (d * bn);
	if (t < 0.0f)
		return false;

	V3 pb = p + d * t; 
	return inside_billboard(pb);
}

bool BillBoard::inside_billboard(V3 p)
{
	V3 p0 = mesh->verts[0];
	V3 p1 = mesh->verts[1];
	V3 p3 = mesh->verts[3];
	V3 n = mesh->normals[0];

	if(!equal((p-p0) * n,0.0f))
		return false;

	V3 v = p - p0;
	V3 x = p3 - p0;
	V3 y = p1 - p0;
	float xf = x * v, yf = y * v;
	return !(xf < 0.0f || yf < 0.0f || xf / x.Length() > x.Length() || yf / y.Length() > y.Length());
}

M33 BillBoard::get_corners()
{
	M33 corners;
	corners[0] = mesh->verts[0];
	corners[1] = mesh->verts[1];
	corners[2] = mesh->verts[3];
	return corners;
}

V3 BillBoard::color(V3 p, float& alpha)
{
	float s, t;

	if (!texture || !inside_billboard(p))
	{
		alpha = 0.0f;
		return V3(0.0f);
	}

	get_st(p, s, t);

	return texture->bilinear_interpolation(s, t, alpha);
}

V3 BillBoard::color(FrameBuffer *fb, V3 p)
{
	float s, t;

	if (!inside_billboard(p))
		return V3(0.0f);

	get_st(p, s, t);

	auto &tex = fb->textures.at(mesh->tex).back();
	return fb->bilinear_interpolation(tex, s, t);
}

V3 BillBoard::color(FrameBuffer* fb, V3 p, float& alpha)
{
	float s, t;

	if (!inside_billboard(p))
	{
		alpha = 0.0f;
		return V3(0.0f);
	}
	get_st(p, s, t);

	return  fb->lookup_color(mesh->tex, s, t, alpha);
}

void BillBoard::render(PPC* ppc, FrameBuffer* fb)
{
	mesh->render_billboard(ppc, fb, texture.get());
}

void BillBoard::get_st(V3 p, float& s, float& t)
{
	V3 p0 = mesh->verts[0];
	V3 p1 = mesh->verts[1];
	V3 p3 = mesh->verts[3];

	V3 x = p3 - p0;
	V3 y = p1 - p0;
	V3 v = p - p0;
	
	float xf = x * v;
	float yf = y * v;

	s = xf / (x.Length() * x.Length());
	t = yf / (y.Length() * y.Length());
}
