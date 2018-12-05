#include <fstream>
#include <algorithm>
#include <complex>
#include <GL/glext.h>

#include "TM.h"
#include "global_varibles.h"
#include "m33.h"

using namespace std;

void TM::SetRectangle(V3 O, float rw, float rh)
{
	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = O + V3(-rw / 2.0f, rh / 2.0f, 0.0f);
	verts[1] = O + V3(-rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[2] = O + V3(rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[3] = O + V3(rw / 2.0f, rh / 2.0f, 0.0f);

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	++tri;

	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;

	for (int vi = 0; vi < vertsN; ++vi)
	{
		colors[vi] = V3(0.0f);
	}
}

void TM::SetTriangle(vertex_properties p0, vertex_properties p1, vertex_properties p2)
{
	vertsN = 3;
	trisN = 1;
	Allocate();

	verts[0] = p0.p;
	verts[1] = p1.p;
	verts[2] = p2.p;

	colors[0] = p0.c;
	colors[1] = p1.c;
	colors[2] = p2.c;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
}

void TM::SetQuad(vertex_properties p0, vertex_properties p1, vertex_properties p2, vertex_properties p3)
{
	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = p0.p;
	verts[1] = p1.p;
	verts[2] = p2.p;
	verts[3] = p3.p;

	colors[0] = p0.c;
	colors[1] = p1.c;
	colors[2] = p2.c;
	colors[3] = p3.c;

	normals[0] = p0.n;
	normals[1] = p1.n;
	normals[2] = p2.n;
	normals[3] = p3.n;

	verts_st[0] = p0.s;
	verts_st[1] = p0.t;
	verts_st[2] = p1.s;
	verts_st[3] = p1.t;
	verts_st[4] = p2.s;
	verts_st[5] = p2.t;
	verts_st[6] = p3.s;
	verts_st[7] = p3.t;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;

	tris[3] = 2;
	tris[4] = 3;
	tris[5] = 0;
}

void TM::SetQuad(V3 O, V3 n, V3 up, float sz, float s, float t)
{
	up = up.UnitVector();
	V3 right = (up ^ n).UnitVector();
	V3 p0 = O + up * sz - right * sz;
	V3 p1 = O - up * sz - right * sz;
	V3 p2 = O - up * sz + right * sz;
	V3 p3 = O + up * sz + right * sz;
	V3 c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f);
	vertex_properties pp0(p0, c0, n, 0.0f, 0.0f);
	vertex_properties pp1(p1, c1, n, 0.0f, t);
	vertex_properties pp2(p2, c2, n, s, t);
	vertex_properties pp3(p3, c3, n, s, 0.0f);
	SetQuad(pp0, pp1, pp2, pp3);
}

void TM::set_bilboard(V3 O, V3 n, V3 up, float sz, float s, float t)
{
	V3 c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f);

	up = up.UnitVector();
	V3 right = (up ^ n).UnitVector();

	V3 p0 = O + up * sz - right * sz;
	V3 p1 = O - up * sz - right * sz;
	V3 p2 = O - up * sz + right * sz;
	V3 p3 = O + up * sz + right * sz;
	
	vertex_properties pp0(p0, c0, n, 0.0f, t);
	vertex_properties pp1(p1, c1, n, 0.0f, 0.0f);
	vertex_properties pp2(p2, c2, n, s, 0.0f);
	vertex_properties pp3(p3, c3, n, s, t);
	
	use_env_mapping = false;

	SetQuad(pp0, pp1, pp2, pp3);
}

void TM::set_unit_box()
{
	V3 v0 = V3(1.0f, 1.0f, 1.0f);
	V3 v1 = V3(1.0f, -1.0f, 1.0f);
	V3 v2 = V3(1.0f, -1.0f, -1.0f);
	V3 v3 = V3(1.0f, 1.0f, -1.0f);
	V3 v4 = V3(-1.0f, 1.0f, 1.0f);
	V3 v5 = V3(-1.0f, 1.0f, -1.0f);
	V3 v6 = V3(-1.0f, -1.0f, -1.0f);
	V3 v7 = V3(-1.0f, -1.0f, 1.0f);
	V3 color(0.0f);
	V3 origin(0.0f);
	int vertCount = 0;
	int trisCounter = 0;

	vertsN = 24;
	trisN = 12;
	Allocate();

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v0;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v1;
	
	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v2;
	
	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v3;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v4;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v5;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v6;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v7;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v1;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v7;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v6;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v2;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v0;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v3;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v5;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v4;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v0;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v4;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v7;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v1;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v3;

	verts_st[2 * vertCount + 0] = 1.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v2;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 0.0f;
	verts[vertCount++] = v6;

	verts_st[2 * vertCount + 0] = 0.0f;
	verts_st[2 * vertCount + 1] = 1.0f;
	verts[vertCount++] = v5;

	
	colors[0] = color;
	colors[1] = color;
	colors[2] = color;
	colors[3] = color;
	colors[4] = color;
	colors[5] = color;
	colors[6] = color;
	colors[7] = color;

	for (int vi = 0; vi < vertsN; ++vi)
		normals[vi] = (verts[vi] - origin).UnitVector();

	tris[trisCounter++] = 0;
	tris[trisCounter++] = 1;
	tris[trisCounter++] = 2;
	tris[trisCounter++] = 2;
	tris[trisCounter++] = 3;
	tris[trisCounter++] = 0;
	tris[trisCounter++] = 4;
	tris[trisCounter++] = 5;
	tris[trisCounter++] = 6;
	tris[trisCounter++] = 6;
	tris[trisCounter++] = 7;
	tris[trisCounter++] = 4;
	tris[trisCounter++] = 8;
	tris[trisCounter++] = 9;
	tris[trisCounter++] = 10;
	tris[trisCounter++] = 10;
	tris[trisCounter++] = 11;
	tris[trisCounter++] = 8;
	tris[trisCounter++] = 12;
	tris[trisCounter++] = 13;
	tris[trisCounter++] = 14;
	tris[trisCounter++] = 14;
	tris[trisCounter++] = 15;
	tris[trisCounter++] = 12;
	tris[trisCounter++] = 16;
	tris[trisCounter++] = 17;
	tris[trisCounter++] = 18;
	tris[trisCounter++] = 18;
	tris[trisCounter++] = 19;
	tris[trisCounter++] = 16;
	tris[trisCounter++] = 20;
	tris[trisCounter++] = 21;
	tris[trisCounter++] = 22;
	tris[trisCounter++] = 22;
	tris[trisCounter++] = 23;
	tris[trisCounter] = 20;
}

void TM::SetText(string tf)
{
	tex = tf;
}

void TM::Allocate()
{
	verts.resize(vertsN);
	colors.resize(vertsN);
	normals.resize(vertsN);
	verts_st.resize(2 * vertsN);
	tris.resize(3 * trisN); 
}

void TM::reset_color(V3 color)
{
	for(auto &c:colors)
	{
		c = color;
	}
}

tuple<V3, V3, V3, V3> TM::get_corners_axis()
{
	V3 corner, x, y, z;
	corner = verts[6];				
	x = verts[2] - verts[6];
	y = verts[5] - verts[6];
	z = verts[7] - verts[6];
	return tuple<V3, V3, V3, V3>(corner, x, y, z);
}

void TM::RenderPoints(PPC* ppc, FrameBuffer* fb)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		fb->Draw3DPoint(ppc, verts[vi], 0xFF000000, 7);
	}
}

void TM::RenderWireFrame(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		for (int ei = 0; ei < 3; ++ei)
		{
			int vi0 = tris[ti * 3 + ei];
			int vi1 = tris[ti * 3 + (ei + 1) % 3];
			fb->Draw3DSegment(ppc, verts[vi0], colors[vi0], verts[vi1], colors[vi1]);
		}
	}
}

void TM::RenderFill(PPC* ppc, FrameBuffer* fb)
{
	lod_size = -1;
	if (lod_texture)
	{
		auto aabb = ComputeAABB();
		V3 paabb0(0.0f), paabb1(0.0f);
		ppc->Project(aabb.corners[0], paabb0);
		ppc->Project(aabb.corners[1], paabb1);
		V3 paabbV = paabb1 - paabb0;
		lod_size = max(abs(paabbV[0]), abs(paabbV[1]));
		lod_size = clamp(lod_size, 0, fb->w);
	}

	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (verts_st.size() == verts.size() * 2) hasTexture = true;

		vertex_properties p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? verts_st[vi0 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi0 * 2 + 1] : 0.0f);
		vertex_properties p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? verts_st[vi1 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi1 * 2 + 1] : 0.0f);
		vertex_properties p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? verts_st[vi2 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi2 * 2 + 1] : 0.0f);

		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0) || !ppc->Project(p1.p, pp1) || !ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX || pp1[0] == FLT_MAX || pp2[0] == FLT_MAX)
			return;

		AABB bb_tri(pp0);
		bb_tri.AddPoint(pp1);
		bb_tri.AddPoint(pp2);
		if (!bb_tri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		int left = static_cast<int>(bb_tri.corners[0][0] + 0.5f), right = static_cast<int>(bb_tri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bb_tri.corners[0][1] + 0.5f), bottom = static_cast<int>(bb_tri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = same_side(uvP, pp0, pp1, pp2);
				bool s2 = same_side(uvP, pp1, pp2, pp0);
				bool s3 = same_side(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float div = (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f));

					if (equal(div, 0.0f))
						continue;

					div = 1.0f / div;

					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);

					if (depth_test && !fb->DepthTest(u, v, wv))
						continue;

					uvP[2] = wv;
					float k = V3(u, v, 1.0f) * qM[1] * div;
					float l = V3(u, v, 1.0f) * qM[2] * div;

					V3 p = ppc->Unproject(uvP);
					V3 pn = p0.n + (p1.n - p0.n) * k + (p2.n - p0.n) * l;
					V3 pc = p0.c + (p1.c - p0.c) * k + (p2.c - p0.c) * l;
					V3 st = st0 + (st1 - st0) * k + (st2 - st0) * l;
					V3 st0(p0.s, p0.t, 0.0f), st1(p1.s, p1.t, 0.0f), st2(p2.s, p2.t, 0.0f);

					vertex_properties pp(p, pc, pn, st[0], st[1]);
					auto [color , alpha] = shading(ppc, fb, u, v, wv, pp);

					if (!equal(alpha, 1.0f)) {
						V3 bgC(0.0f);
						bgC.SetColor(fb->Get(u, v));
						color = color * alpha + bgC * (1.0f - alpha);
					}

					fb->DrawPoint(u, v, color.GetColor());
				}
			}
		}
	}
}

void TM::RenderFillZ(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (verts_st.size() == verts.size() * 2) hasTexture = true;

		vertex_properties p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? verts_st[vi0 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi0 * 2 + 1] : 0.0f);
		vertex_properties p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? verts_st[vi1 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi1 * 2 + 1] : 0.0f);
		vertex_properties p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? verts_st[vi2 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi2 * 2 + 1] : 0.0f);

		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0))
			return;
		if (!ppc->Project(p1.p, pp1))
			return;
		if (!ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX ||
			pp1[0] == FLT_MAX ||
			pp2[0] == FLT_MAX)
			return;

		AABB bb_tri(pp0);
		bb_tri.AddPoint(pp1);
		bb_tri.AddPoint(pp2);
		if (!bb_tri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		int left = static_cast<int>(bb_tri.corners[0][0] + 0.5f), right = static_cast<int>(bb_tri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bb_tri.corners[0][1] + 0.5f), bottom = static_cast<int>(bb_tri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = same_side(uvP, pp0, pp1, pp2);
				bool s2 = same_side(uvP, pp1, pp2, pp0);
				bool s3 = same_side(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);

					if (depth_test && !fb->DepthTest(u, v, wv))
						continue;
				}
			}
		}
	}
}

void TM::RenderAABB(PPC* ppc, FrameBuffer* fb)
{
	V3 p0, p1, p2, p3;
	V3 p4, p5, p6, p7;
	auto aabb = ComputeAABB();
	V3 x(1.0f, 0.0f, 0.0f); 
	V3 y(0.0f, 1.0f, 0.0f); 
	V3 z(0.0f, 0.0f, 1.0f);
	V3 lc = aabb.corners[0];
	V3 rc = aabb.corners[1]; 
	
	float dx = (rc - lc) * x;
	float dy = (rc - lc) * y;
	float dz = (rc - lc) * z;
	
	p0 = lc;
	p1 = lc + x * dx;
	p2 = lc + y * dy;
	p3 = lc + x * dx + y * dy;
	p4 = p0 + z * dz;
	p5 = p1 + z * dz;
	p6 = p2 + z * dz;
	p7 = p3 + z * dz;

	V3 blue(0.0f, 0.0f, 1.0f);
	
	fb->Draw3DSegment(ppc, p4, blue, p5, blue);
	fb->Draw3DSegment(ppc, p4, blue, p6, blue);
	fb->Draw3DSegment(ppc, p5, blue, p7, blue);
	fb->Draw3DSegment(ppc, p6, blue, p7, blue);

	fb->Draw3DSegment(ppc, p0, blue, p4, blue);
	fb->Draw3DSegment(ppc, p1, blue, p5, blue);
	fb->Draw3DSegment(ppc, p2, blue, p6, blue);
	fb->Draw3DSegment(ppc, p3, blue, p7, blue);

	fb->Draw3DSegment(ppc, p0, blue, p1, blue);
	fb->Draw3DSegment(ppc, p0, blue, p2, blue);
	fb->Draw3DSegment(ppc, p1, blue, p3, blue);
	fb->Draw3DSegment(ppc, p2, blue, p3, blue);
}

void TM::render_billboard(PPC* ppc, FrameBuffer* fb, FrameBuffer* bbTexture)
{
	
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (verts_st.size() == verts.size() * 2) hasTexture = true;

		vertex_properties p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? verts_st[vi0 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi0 * 2 + 1] : 0.0f);
		vertex_properties p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? verts_st[vi1 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi1 * 2 + 1] : 0.0f);
		vertex_properties p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? verts_st[vi2 * 2] : 0.0f,
		                 hasTexture ? verts_st[vi2 * 2 + 1] : 0.0f);

		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0))
			return;
		if (!ppc->Project(p1.p, pp1))
			return;
		if (!ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX ||
			pp1[0] == FLT_MAX ||
			pp2[0] == FLT_MAX)
			return;

		AABB bb_tri(pp0);
		bb_tri.AddPoint(pp1);
		bb_tri.AddPoint(pp2);
		if (!bb_tri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		int left = static_cast<int>(bb_tri.corners[0][0] + 0.5f), right = static_cast<int>(bb_tri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bb_tri.corners[0][1] + 0.5f), bottom = static_cast<int>(bb_tri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = same_side(uvP, pp0, pp1, pp2);
				bool s2 = same_side(uvP, pp1, pp2, pp0);
				bool s3 = same_side(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float div = (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f));

					if (equal(div, 0.0f) || isnan(div))
						continue;

					div = 1.0f / div;
					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);

					if (depth_test && !fb->DepthTest(u, v, wv))
						continue;

					uvP[2] = wv;
					float k = V3(u, v, 1.0f) * qM[1] * div;
					float l = V3(u, v, 1.0f) * qM[2] * div;
					V3 st0(p0.s, p0.t, 0.0f), st1(p1.s, p1.t, 0.0f), st2(p2.s, p2.t, 0.0f);
					V3 st = st0 + (st1 - st0) * k + (st2 - st0) * l;

					float s = st[0], t = st[1], alpha = 0.0f;
					V3 color = bbTexture->BilinearLookupColor(s, t, alpha);

					if (!equal(alpha, 1.0f))
					{
						V3 bgC(0.0f);
						bgC.SetColor(fb->Get(u, v));
						color = color * alpha + bgC * (1.0f - alpha);
					}

					fb->DrawPoint(u, v, color.GetColor());
				}
			}
		}
	}
}

void TM::render_hardware(PPC *ppc, FrameBuffer *curfb)
{
	if (cgi == nullptr && soi == nullptr)
	{
		
		cgi = make_shared<CGInterface>();
		cgi->PerSessionInit();
		soi = make_shared<ShaderOneInterface>();
		soi->PerSessionInit(cgi.get(), shaderOneFile);

		if (!tex.empty())
			cur_scene->gpufb->LoadTextureGPU(tex);

		curfb->LoadTextureGPU("images/tex.tiff");
	}

	ppc->SetIntrinsicsHW();
	ppc->SetExtrinsicsHW();

	cgi->EnableProfiles();

	uniformVariables uniforms;
	uniforms.is_cube = is_cube;
	uniforms.is_cubemap = is_cubemap;
	uniforms.is_box = is_box;
	uniforms.is_gnd = is_ground;
	uniforms.tex = tex;
	vector<shared_ptr<TM>> otherTMs;
	for(auto t:cur_scene->meshes)
	{
		if(t->isBox == 0)
			continue;
		otherTMs.push_back(t);
	}
	uniforms.box0 = otherTMs[0];
	uniforms.box1 = otherTMs[1];
	uniforms.box2 = otherTMs[2];
	soi->PerFrameInit(uniforms);
	soi->BindPrograms();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, &verts[0]);
	glColorPointer(3, GL_FLOAT, 0, &colors[0]);
	glNormalPointer(GL_FLOAT, 0, &normals[0]);

	if (is_cube > 0)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &verts_st[0]);
	}

	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, &tris[0]);

	if(is_recording)
		curfb->SaveGPU2CPU();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	soi->PerFrameDisable();
	cgi->DisableProfiles();
}

void TM::render_hardware_wireframe(PPC *ppc, FrameBuffer *curfb)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	render_hardware(ppc, curfb);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
		normals[vi] = normals[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
	}
}

void TM::Translate(V3 tv)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] + tv;
	}
}

void TM::Scale(float scf)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] * scf;
	}
}

void TM::LoadModelBin(char* fname)
{
	ifstream ifs(fname, ios::binary);
	if (ifs.fail())
	{
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y')
	{
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (!verts.empty())
		verts.clear();
	verts.resize(vertsN);

	ifs.read(&yn, 1); // cols 3 floats
	if (!colors.empty())
		colors.clear();
	if (yn == 'y')
	{
		colors.resize(vertsN);
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (!normals.empty())
		normals.clear();
	if (yn == 'y')
	{
		normals.resize(vertsN);
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats

	if (!verts_st.empty())
		verts_st.clear();

	if (yn == 'y')
	{
		verts_st.resize(vertsN * 2);
	}

	ifs.read((char*)&verts[0], vertsN * 3 * sizeof(float)); // load verts

	if (colors.size() == vertsN)
	{
		ifs.read((char*)&colors[0], vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals.size() == vertsN)
		ifs.read((char*)&normals[0], vertsN * 3 * sizeof(float)); // load normals

	if (verts_st.size() != 0)
		ifs.read((char*)&verts_st[0], vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris.size() != 0)
		tris.clear();
	tris.resize(trisN * 3);
	ifs.read((char*)&tris[0], trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors.size() == 0) ? "rgb " : "") << ((normals.size() == 0) ? "nxnynz " : "") << (
		(verts_st.size() == 0) ? "tcstct " : "") << endl;
}

AABB TM::ComputeAABB()
{
	AABB aabb(verts[0]);
	for (int vi = 0; vi < vertsN; ++vi)
	{
		aabb.AddPoint(verts[vi]);
	}
	return aabb;
}

void TM::PositionAndSize(V3 tmC, float tmSize)
{
	AABB aabb = ComputeAABB();
	V3 oldC = aabb.GetCenter();
	float oldSize = aabb.GetDiagnoalLength();

	Translate(V3(0.0f) - oldC);
	Scale(tmSize / oldSize);
	Translate(tmC);
}

V3 TM::GetCenter()
{
	AABB aabb(verts[0]);
	aabb = ComputeAABB();
	return aabb.GetCenter();
}

tuple<V3, float> TM::shading(PPC* ppc, FrameBuffer* fb, int u, int v, float w, vertex_properties& pp, V3 dn)
{
	V3 color(0.0f); float alpha = 1.0f;
	if (fb->textures.find(tex) != fb->textures.end())
	{
		float s = clamp(Fract(pp.s), 0.0f, 1.0f);
		float t = clamp(Fract(pp.t), 0.0f, 1.0f);

		pp.c = fb->LookupColor(tex, s, t, alpha, lod_size);
	}

	if (use_env_mapping)
	{
		auto [envColor, envEffect] = env_mapping(ppc, fb, cur_scene->cubemap.get(), pp.p, pp.n, dn);

		envEffect = use_obj_color ? envEffect : 1.0f;
		pp.c = clamp_color(pp.c * (1.0f - envEffect) + envColor * envEffect);
	}

	if (use_projected_tex)
	{
		V3 c(0.0f);
		float a = 0.0f;
		if (calculate_projective_texture(u, v, w, c, a))
			pp.c = pp.c * (1.0f - a) + c * a;
	}

	if (isLight)
		color = lighting_shading(ppc, pp, u, v, w);
	else
		color = pp.c;

	return tuple<V3, float>(color, alpha);
}

void TM::lighting_shading(V3 mc, V3 L, PPC* ppc)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		float ka = 0.5f;
		float kd = (L - verts[vi]).UnitVector() * normals[vi].UnitVector();
		float ks = (ppc->C - verts[vi]).UnitVector() * (L - verts[vi]).UnitVector().Reflect(normals[vi].UnitVector());
		
		kd = max(kd, 0.0f);
		ks = pow(max(ks, 0.0f), 8);
		colors[vi] = mc * (ka + (1.0f - ka) * kd) + ks;
	}
}

V3 TM::lighting_shading(PPC* ppc, vertex_properties& pp, int u, int v, float w)
{
	V3 ret(0.0f);
	
	if (cur_scene->lightPPCs.empty())
		return pp.c;

	float ka = 0.2f;
	float kd = 0.0f, ks = 0.0f;
	float sd = 1.0f;
	for (size_t li = 0; li < cur_scene->lightPPCs.size(); ++li)
	{
		auto ppc2 = cur_scene->lightPPCs[li];

		kd += max((ppc2->C - pp.p).UnitVector() * pp.n.UnitVector(), 0.0f);
		float liks = (ppc->C - pp.p).UnitVector() * (ppc2->C - pp.p).UnitVector().Reflect(pp.n.UnitVector());
		ks += pow(max(liks, 0.0f), 200);

		if (is_shadow && !cur_scene->shadow_maps.empty())
		{
			auto SM = cur_scene->shadow_maps[li];
			float uf = static_cast<float>(u), vf = static_cast<float>(v), z = w;
			V3 v2 = homograph_mapping(V3(uf, vf, z), ppc, ppc2.get());
			if (v2[2] < 0.0f)
				continue;

			float eps = 0.15f;
			if (SM->GetZ(v2[0], v2[1]) - v2[2] > eps)
			{
				sd *= 0.2f;
			}
		}
	}

	ka = clamp(ka, 0.0f, 1.0f);
	kd = clamp(kd, 0.0f, 1.0f);
	ks = clamp(ks, 0.0f, 1.0f);

	ret = pp.c * (ka + (1.0f - ka) * kd) + V3(1.0f) * ks;
	ret = ret * sd; 
	return ret;
}

bool TM::calculate_shadow(PPC* ppc, int u, int v, float z, float& shadow_effect)
{
	bool is_in_shadow = false;
	shadow_effect = 1.0f; 
	
	if (cur_scene->shadow_maps.empty())
		return is_in_shadow;

	float uf = static_cast<float>(u) + 0.5f, vf = static_cast<float>(v) + 0.5f;
	for (size_t li = 0; li < cur_scene->lightPPCs.size(); ++li)
	{
		auto ppc1 = ppc;
		auto ppc2 = cur_scene->lightPPCs[li];
		auto SM = cur_scene->shadow_maps[li];

		V3 v2 = homograph_mapping(V3(uf, vf, z), ppc1, ppc2.get());

		if (v2[2] < 0.0f)
			continue;

		float eps = 0.15f;
		if (SM->GetZ(v2[0], v2[1]) - v2[2] > eps)
		{
			is_in_shadow = true;
			shadow_effect *= 0.2f;
		}
	}

	return is_in_shadow;
}

bool TM::calculate_projective_texture(int u, int v, float z, V3& color, float& alpha)
{
	float uf = static_cast<float>(u) + 0.5f, vf = static_cast<float>(v) + 0.5f;
	auto ppc1 = cur_scene->ppc;
	auto ppc2 = cur_scene->projectPPC;
	auto projFB = cur_scene->fbp;
	string projTexName = projected_tex_name;

	if (!ppc1 || !ppc1 || !projFB)
		return false;

	V3 v2 = homograph_mapping(V3(uf, vf, z), ppc1, ppc2);

	if (v2[2] < 0.0f)
		return false;

	AABB aabb(v2);
	if (!aabb.Clip2D(0, projFB->w - 1, 0, projFB->h - 1))
		return false;

	float eps = 0.05f;

	if (projFB->GetZ(v2[0], v2[1]) - v2[2] <= eps)
	{
		unsigned int c = projFB->Get(v2[0], v2[1]);
		color.SetColor(c);
		unsigned char* rgba = (unsigned char*)&c;
		alpha = static_cast<float>(rgba[3]) / 255.0f;
		return true;
	}

	return false;
}

tuple<V3, float> TM::env_mapping(PPC* ppc, FrameBuffer* fb, CubeMap* cubemap, V3 p, V3 n, V3 dn)
{
	V3 c(0.0f);
	float envEffect = 0.0f;
	auto sceneBBs = cur_scene->sceneBillboard;
	float distance = 0.0f;
	V3 bbColor(0.0f);
	float alpha = 0.0f;

	if (!cubemap)
		return tuple<V3, float>(c,envEffect);

	envEffect = 0.4f;
	
	V3 viewDir = (ppc->C - p).UnitVector();

	if (isRefraction)
		viewDir = viewDir.Refract(n, refractRatio);
	else
		viewDir = viewDir.Reflect(n);

	env_billboard_intersection(sceneBBs, p, viewDir, distance, bbColor, alpha);
	env_billboard_intersection(reflector_billboards, p, viewDir, distance, bbColor, alpha);
	bbColor = bbColor * alpha;

	if (!equal(distance, 0.0f))
	{
		distance = 1.0f / distance;
		float disAttenauation = max(pow(distance * 10.0f, 2), 1.0f);
		return tuple<V3, float>(bbColor,0.3f);
	}
	else {
		float pxSz = dn != V3(0.0f) ? abs((n + dn).UnitVector() * n.UnitVector()) * static_cast<float>(ppc->h) * 0.5f : -1;
		V3 cubemap_color = cubemap->LookupColor(viewDir, pxSz);
		float env_effect = envEffect;
		return tuple<V3, float>(cubemap_color, env_effect);
	}
}

int TM::env_billboard_intersection(vector<shared_ptr<BillBoard>> bbs, V3 p, V3 viewDir, float& distance, V3& bbColor,
                          float& alpha)
{
	int ret = 0;
	for (auto b : bbs)
	{
		float t = 0.0f;

		if (!b->Intersect(p, viewDir, t))
			continue;

		t = 1.0 / t;
		ret = 1;

		if (distance < t)
		{
			distance = t;

			t = 1.0f / t;
			V3 pBB = p + viewDir * t;
			bbColor = b->GetColor(b->fbTexture.get(), pBB, alpha);
			if (b->mesh->tex.empty())
				b->GetColor(pBB, alpha);

			if (equal(alpha, 0.0f))
				distance = 0.0f;
		}
	}

	return ret;
}

V3 TM::clamp_color(V3 color)
{
	V3 ret(0.0f);
	ret[0] = clamp(color[0], 0.0f, 1.0f);
	ret[1] = clamp(color[1], 0.0f, 1.0f);
	ret[2] = clamp(color[2], 0.0f, 1.0f);
	return ret;
}

V3 TM::homograph_mapping(V3 uvw, PPC* ppc1, PPC* ppc2)
{
	M33 abc1;
	abc1.SetColumn(0, ppc1->a);
	abc1.SetColumn(1, ppc1->b);
	abc1.SetColumn(2, ppc1->c);

	M33 abc2;
	abc2.SetColumn(0, ppc2->a);
	abc2.SetColumn(1, ppc2->b);
	abc2.SetColumn(2, ppc2->c);
	auto abc2Inv = abc2.Inverse();

	auto qC = abc2Inv * (ppc1->C - ppc2->C);
	auto qM = abc2Inv * abc1;

	float w1 = 1.0f / uvw[2];
	V3 px = V3(uvw[0], uvw[1], 1.0f) * w1;
	float w2 = 1.0f / (qC[2] + qM[2] * px);
	float u2 = (qC[0] + qM[0] * px) * w2;
	float v2 = (qC[1] + qM[1] * px) * w2;

	return V3(u2, v2, w2);
}

void TM::SetAllPointsColor(V3 color)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		colors[vi] = color;
	}
}


void TM::morph_to_sphere(V3 c, float r, float fract)
{
	fract = clamp(fract, 0.0f, 1.0f);
	if (ori_verts.empty())
		ori_verts = verts;

	if (ori_norms.empty())
		ori_norms = normals;

	for (int vi = 0; vi < vertsN; ++vi)
	{
		V3 vp = ori_verts[vi];
		V3 dis = vp - c;
		dis = dis.UnitVector() * r;
		verts[vi] = vp * (1.0f - fract) + (c + dis) * fract;

		V3 n = ori_norms[vi];
		V3 newn = dis.UnitVector();
		normals[vi] = n * (1.0f - fract) + newn * fract;
	}
}



TM::~TM()
{
}
