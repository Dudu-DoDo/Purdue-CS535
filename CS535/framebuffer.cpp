#include <iostream>
#include <algorithm>
#include <fstream>
#include "scene.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <GL/glext.h>

#include "framebuffer.h"
#include "v3.h"
#include "tiffio.h"
#include "AABB.h"
#include "m33.h"
#include "global_variables.h"

#define PERSPECTIVE_CORRECT_INTERPOLATION

unordered_map<std::string, vector<shared_ptr<TextureInfo>>> FrameBuffer::textures;
unordered_map<std::string, GLuint> FrameBuffer::gpu_tex_ID;

float get_alpha(unsigned int c) {
	float alpha = 0.0f;
	unsigned char* rgba = (unsigned char*)&c;
	alpha = static_cast<float>(rgba[3]) / 255.0f;
	return alpha;
}

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h)
	: Fl_Gl_Window(u0, v0, _w, _h, nullptr)
{
	w = _w;
	h = _h;
	isgpu = false;
	pix = new unsigned int[w * h];
	zb = new float[w * h];
}


void FrameBuffer::draw()
{
	if (isgpu)
	{
		if (is_wireframe) {
			cur_scene->render_GPU();
		}
		else {
			cur_scene->render_GPU_wireframe_mode();
		}
	}
	else
	{
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	}
}

int FrameBuffer::handle(int event)
{
	switch (event)
	{
	case FL_KEYBOARD:
		{
			KeyboardHandle();
			return 0;
		}
	default:
		break;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle()
{
	
	int key = Fl::event_key();
	cerr << "Pressed " << (char)key << endl;
	switch (key)
	{
	default:
		cerr << "INFO: do not understand keypress" << endl;
		break;
	}

	cerr << cur_scene->ppc->C;
}


void FrameBuffer::SetBGR(unsigned int bgr)
{
	//	memset(pix, bgr, sizeof(unsigned int) * w * h);

	for (int v = 0; v < h - 1; ++v)
	{
		for (int u = 0; u < w - 1; ++u)
		{
			pix[(h - 1 - v) * w + u] = bgr;
		}
	}
}

void FrameBuffer::Set(int u, int v, int color)
{
	pix[(h - 1 - v) * w + u] = color;
}

void FrameBuffer::SetGuarded(int u, int v, unsigned int color)
{
	// clip to window 
	if (!IsInScreen(u, v))
		return;

	Set(u, v, color);
}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(const char* fname)
{
	TIFF* in = TIFFOpen(fname, "r");
	if (in == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height)
	{
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w * h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0)
	{
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::SaveAsTiff(const char* fname)
{
	TIFF* out = TIFFOpen(fname, "w");

	if (out == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++)
	{
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}

void FrameBuffer::SaveTextureAsTiff(string fname, const string textureName, int loD)
{
	if (textures.find(textureName) == textures.end())
	{
		cerr << "Cannot find texture: " << textureName << endl;
		return;
	}

	auto tex = textures[textureName][loD];

	TIFF* out = TIFFOpen(fname.c_str(), "w");
	if (out == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, tex->w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, tex->h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)tex->h; row++)
	{
		TIFFWriteScanline(out, &tex->texture[(tex->h - row - 1) * tex->w], row);
	}

	TIFFClose(out);
}

int FrameBuffer::ClipToScreen(int& u0, int& v0, int& u1, int& v1)
{
	// Out of screen 
	if (u0 > w || v0 > h || u1 < 0 || v1 < 0)
		return 0;

	u0 = max(0, u0);
	v0 = max(0, v0);
	u1 = min(u1, w - 1);
	v1 = min(v1, h - 1);
	return 1;
}

int FrameBuffer::ClipToScreen(float& u0, float& v0, float& u1, float& v1)
{
	// Out of screen 
	if (u0 > w || v0 > h || u1 < 0 || v1 < 0)
		return 0;

	if (u0 < 0.0f)
		u0 = 0.0f;
	if (v0 < 0.0f)
		v0 = 0.0f;
	if (u1 > static_cast<float>(w - 1))
		u1 = static_cast<float>(w - 1);
	if (v1 > static_cast<float>(h - 1))
		v1 = static_cast<float>(h - 1);

	return 1;
}

bool FrameBuffer::IsInScreen(int u, int v)
{
	if (u < 0)
		return false;
	if (u > w - 1)
		return false;
	if (v < 0)
		return false;
	if (v > h - 1)
		return false;

	return true;
}

void FrameBuffer::ClearBGRZ(unsigned bgr, float z0)
{
	SetBGR(bgr);
	ClearZ(z0);
}

void FrameBuffer::ClearZ(float z0)
{
	memset(zb, z0, sizeof(float) * w * h);
}

bool FrameBuffer::depth_test(int u, int v, float curz)
{
	// First, u, v should be in screen
	if (!IsInScreen(u, v))
		return false;

	int uv = (h - 1 - v) * w + u;
	if (zb[uv] > curz)
		return false;

	zb[uv] = curz;
	return true;
}


float FrameBuffer::GetZ(int u, int v)
{
	if (!IsInScreen(u, v))
		return 0.0f;

	return zb[(h - 1 - v) * w + u];
}

unsigned FrameBuffer::Get(int u, int v)
{
	if (!IsInScreen(u, v))
		return 0xFF000000;

	return pix[(h - 1 - v) * w + u];
}

bool FrameBuffer::LoadTexture(const std::string texFile)
{
	const char* fname = texFile.c_str();
	TIFF* in = TIFFOpen(fname, "r");
	if (in == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return false;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	vector<unsigned int> texMemory(width * height);

	if (TIFFReadRGBAImage(in, width, height, &texMemory[0], 0) == 0)
	{
		cerr << "failed to load " << fname << endl;
		return false;
	}

	// commit changes
	if (textures.find(texFile) != textures.end())
	{
		textures.erase(texFile);
	}
	shared_ptr<TextureInfo> newTex = make_shared<TextureInfo>();
	newTex->texture = texMemory;
	newTex->w = width;
	newTex->h = height;
	textures[texFile].push_back(newTex);
	TIFFClose(in);

	// Preprocess Lod textures
	if (lod_texture)
		PrepareTextureLoD(texFile);

	return true;
}

bool FrameBuffer::load_texture_GPU(const std::string texFile)
{
	const char* fname = texFile.c_str();
	shared_ptr<TextureInfo> new_tex = make_shared<TextureInfo>();
	int width, height;
	GLuint tex_id = 0;
	vector<unsigned int> tex_memory(width * height);

	TIFF* in = TIFFOpen(fname, "r");
	if (in == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return false;
	}

	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);

	if (TIFFReadRGBAImage(in, width, height, &tex_memory[0], 0) == 0)
	{
		cerr << "failed to load " << fname << endl;
		return false;
	}

	// commit changes
	if (textures.find(texFile) != textures.end())
	{
		textures.erase(texFile);
	}
	new_tex->texture = tex_memory;
	new_tex->w = width;
	new_tex->h = height;
	textures[texFile].push_back(new_tex);
	TIFFClose(in);

	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &new_tex->texture[0]);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	gpu_tex_ID[texFile] = tex_id;

	return true;
}

bool FrameBuffer::load_cubemap_to_GPU(vector<string> cubemaps)
{
	shared_ptr<TextureInfo> new_tex = make_shared<TextureInfo>();

	int width, height;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for(int i = 0; i < 6; ++i)
	{
		auto texFile = cubemaps[i];
		const char* fname = texFile.c_str();
		TIFF* in = TIFFOpen(fname, "r");
		if(in == nullptr)
		{
			cerr << fname << " could not be opened" << endl;
			return false;
		}
		TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
		vector<unsigned int> texMemory(width * height);

		if (TIFFReadRGBAImage(in, width, height, &texMemory[0], 0) == 0)
		{
			cerr << "failed to load " << fname << endl;
			return false;
		}

		if (textures.find(texFile) != textures.end()) {
			textures.erase(texFile);
		}

		new_tex->texture = texMemory;
		new_tex->w = width;
		new_tex->h = height;
		textures[texFile].push_back(new_tex);
		TIFFClose(in);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(new_tex->texture[0]));
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		gpu_tex_ID[texFile] = textureID;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return true;
}

void FrameBuffer::DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1)
{
	V3 v2v1 = p1 - p0;
	V3 c2c1 = c1 - c0;
	int pixelN;
	if (fabsf(v2v1[0]) > fabsf(v2v1[1]))
	{
		// Horizontal
		pixelN = static_cast<int>(fabs(v2v1[0])) + 1;
	}
	else
	{
		// Vertical
		pixelN = static_cast<int>(fabs(v2v1[1])) + 1;
	}

	for (int stepi = 0; stepi < pixelN + 1; ++stepi)
	{
		float fract = static_cast<float>(stepi) / static_cast<float>(pixelN);
		V3 point = p0 + v2v1 * fract;
		V3 color = c0 + c2c1 * fract;

		int u = static_cast<int>(point[0]);
		int v = static_cast<int>(point[1]);

		// Depth test
		if (depth_test(u, v, point[2]))
			SetGuarded(u, v, color.GetColor());
	}
}

void FrameBuffer::Draw3DSegment(PPC* ppc, V3 p1, V3 c1, V3 p2, V3 c2)
{
	V3 pp1, pp2;

	if (!ppc->Project(p1, pp1))
		return;
	if (!ppc->Project(p2, pp2))
		return;
	DrawSegment(pp1, c1, pp2, c2);
}

void FrameBuffer::DrawRectangle(int u0, int v0, int u1, int v1, unsigned color)
{
	if (!ClipToScreen(u0, v0, u1, v1))
		return;

	for (int i = u0; i <= u1; ++i)
	{
		for (int j = v0; j <= v1; ++j)
		{
			SetGuarded(i, j, color);
		}
	}
}

void FrameBuffer::DrawCircle(int u0, int v0, int r, unsigned int color)
{
	// bounding box, then iterate the bounding box area
	int u1 = u0 - r, v1 = v0 - r, u2 = u0 + r, v2 = v0 + r;
	if (!ClipToScreen(u1, v1, u2, v2))
		return;

	for (int u = u1; u <= u2; ++u)
	{
		for (int v = v1; v <= v2; ++v)
		{
			if ((u - u0) * (u - u0) + (v - v0) * (v - v0) <= r * r)
				SetGuarded(u, v, color);
		}
	}
}

void FrameBuffer::DrawEllipse(int u0, int v0, float r0, float r1, unsigned color)
{
	int u1 = u0 - r0, v1 = v0 - r1, u2 = u0 + r0, v2 = u0 + r1;
	if (!ClipToScreen(u1, v1, u2, v2))
		return;

	for (int u = u1; u <= u2; ++u)
	{
		for (int v = v1; v <= v2; ++v)
		{
			if ((u - u0) * (u - u0) / (r0 * r0) + (v - v0) * (v - v0) / (r1 * r1) <= 1.0f)
			{
				SetGuarded(u, v, color);
			}
		}
	}
}

void FrameBuffer::DrawPoint(int u, int v, unsigned color)
{
	SetGuarded(u, v, color);
}

void FrameBuffer::Draw3DPoint(PPC* camera, V3 p, unsigned color, int pointSize)
{
	V3 pp;
	if (!camera->Project(p, pp) || pp[0] < 0 || pp[1] < 0 ||
		pp[0] >= static_cast<float>(w) || pp[1] >= static_cast<float>(h))
		return;

	int u = static_cast<int>(pp[0]);
	int v = static_cast<int>(pp[1]);
	int halfPointSize = pointSize / 2;
	DrawRectangle(u - halfPointSize, v - halfPointSize, u + halfPointSize, v + halfPointSize, color);
}

void FrameBuffer::Draw3DTriangle(PPC* camera, V3 p1, V3 p2, V3 p3, V3 color)
{
	V3 pp0, pp1, pp2;
	if (!camera->Project(p1, pp0) || !camera->Project(p2, pp1) || !camera->Project(p3, pp2))
		return;

	if (pp0[0] == FLT_MAX || pp1[0] == FLT_MAX || pp2[0] == FLT_MAX)
		return;

	AABB bb_tri(pp0);
	bb_tri.AddPoint(pp1);
	bb_tri.AddPoint(pp2);
	if (!ClipToScreen(bb_tri.corners[0][0], bb_tri.corners[0][1], bb_tri.corners[1][0], bb_tri.corners[1][1]))
		return;

	// Rasterize bbox 
	int left = static_cast<int>(bb_tri.corners[0][0] + 0.5f), right = static_cast<int>(bb_tri.corners[1][0] - 0.5f);
	int top = static_cast<int>(bb_tri.corners[0][1] + 0.5f), bottom = static_cast<int>(bb_tri.corners[1][1] - 0.5f);

	for (int i = top; i <= bottom; ++i)
	{
		for (int j = left; j <= right; ++j)
		{
			V3 p(j, i, 0.0f);
			bool s1 = same_side(p, pp0, pp1, pp2);
			bool s2 = same_side(p, pp1, pp2, pp0);
			bool s3 = same_side(p, pp2, pp0, pp1);
			if (s1 && s2 && s3)
			{
				// DrawPoint(j, i, color.GetColor());
				int u = j, v = i;
				M33 m; // Project of P based on A, B, C
				m.SetColumn(0, V3(pp0[0], pp0[1], 1.0f));
				m.SetColumn(1, V3(pp0[0], pp0[1], 1.0f));
				m.SetColumn(2, V3(pp1[0], pp1[1], 1.0f));
				V3 ABC = m.Inverse() * p;

				// Depth test
				float ppz = ABC * V3(pp0[2], pp0[2], pp1[2]);
				if (depth_test(u, v, ppz))
					DrawPoint(u, v, color.GetColor());
			}
		}
	}
}

void FrameBuffer::Draw3DTriangle(PPC* ppc, V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2)
{
	V3 pp0, pp1, pp2;
	if (!ppc->Project(p0, pp0) || !ppc->Project(p1, pp1) || !ppc->Project(p2, pp2))
		return;

	if (pp0[0] == FLT_MAX || pp1[0] == FLT_MAX || pp2[0] == FLT_MAX)
		return;

	AABB bb_tri(pp0);
	bb_tri.AddPoint(pp1);
	bb_tri.AddPoint(pp2);
	if (!ClipToScreen(bb_tri.corners[0][0], bb_tri.corners[0][1], bb_tri.corners[1][0], bb_tri.corners[1][1]))
		return;

	// Rasterize bbox 
	int left = static_cast<int>(bb_tri.corners[0][0] + 0.5f), right = static_cast<int>(bb_tri.corners[1][0] - 0.5f);
	int top = static_cast<int>(bb_tri.corners[0][1] + 0.5f), bottom = static_cast<int>(bb_tri.corners[1][1] - 0.5f);

	// Q matrix
	M33 abcM;
	abcM.SetColumn(0, ppc->a);
	abcM.SetColumn(1, ppc->b);
	abcM.SetColumn(2, ppc->c);
	M33 vcM; // V1-C, V2-C, V3-C
	vcM.SetColumn(0, p0 - ppc->C);
	vcM.SetColumn(1, p1 - ppc->C);
	vcM.SetColumn(2, p2 - ppc->C);
	M33 qM = vcM.Inverse() * abcM;
	for (int i = top; i <= bottom; ++i)
	{
		for (int j = left; j <= right; ++j)
		{
			V3 pp(static_cast<float>(j) + 0.5f, static_cast<float>(i) + 0.5f, 1.0f);
			bool s1 = same_side(pp, pp0, pp1, pp2);
			bool s2 = same_side(pp, pp1, pp2, pp0);
			bool s3 = same_side(pp, pp2, pp0, pp1);
			if (s1 == true && s2 == true && s3 == true)
			{
				// Perspective correct interpolation
				int u = j, v = i;
				float k = V3(u, v, 1.0f) * qM[1] / (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.
					GetColumn(2) * V3(1.0f));
				float l = V3(u, v, 1.0f) * qM[2] / (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.
					GetColumn(2) * V3(1.0f));
				float w = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) * V3(1.0f);
				if (depth_test(u, v, w))
				{
					V3 c = c0 + (c1 - c0) * k + (c2 - c0) * l;
					DrawPoint(u, v, c.GetColor());
				}
			}
		}
	}
}

void FrameBuffer::DrawTexture(const std::string texFile, int LoD)
{
	if (textures.find(texFile) == textures.end())
		return;

	auto tex = textures.at(texFile).back();
	delete[] pix;
	w = tex->w;
	h = tex->h;
	pix = new unsigned int[w * h];
	copy(tex->texture.begin(), tex->texture.end(), pix);
}

void FrameBuffer::DrawCubeMap(PPC* ppc, CubeMap* cubemap)
{
	for (int v = 0; v < h; ++v)
	{
		for (int u = 0; u < w; ++u)
		{
			V3 eyeRay = ppc->GetRay(u, v);
			V3 c = cubemap->LookupColor(eyeRay);
			DrawPoint(u, v, c.GetColor());
		}
	}
}


void FrameBuffer::DrawPPC(PPC* wPPC, PPC* tPPC, float vf)
{
	float f = tPPC->GetFocal();
	float scf = vf / f;

	V3 vd = tPPC->GetVD();
	V3 tC = tPPC->C;
	V3 ta = tPPC->a * scf, tb = tPPC->b * scf, tc = tPPC->c * scf;
	float w = tPPC->w, h = tPPC->h;
	V3 c = V3(0.0f, 1.0f, 0.0f);

	Draw3DSegment(wPPC, tC, c, tC + tc, c);
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + ta * w, c);
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + ta * w, c, tC + tc + ta * w + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + tb * h, c, tC + tc + ta * w + tb * h, c);
}

void FrameBuffer::VisualizeCurrView(PPC* ppc0, float currf, PPC* ppc1, FrameBuffer* fb1)
{
	V3 cv;

	for (int v = 0; v < h; ++v)
	{
		for (int u = 0; u < w; ++u)
		{
			float z = GetZ(u, v);
			if (equal(z, 0.0f))
				continue;
			V3 pP(0.5f + static_cast<float>(u), 0.5f + static_cast<float>(v), z);
			V3 pixP = ppc0->UnprojectPixel(pP[0], pP[1], currf);
			cv.SetColor(Get(u, v));
			fb1->Draw3DPoint(ppc1, pixP, cv.GetColor(), 1);
		}
	}
}

void FrameBuffer::VisualizeCurrView3D(PPC* ppc0, PPC* ppc1, FrameBuffer* fb1)
{
	V3 cv;

	for (int v = 0; v < h; ++v)
	{
		for (int u = 0; u < w; ++u)
		{
			float z = GetZ(u, v);
			if (equal(z, 0.0f))
				continue;
			V3 pP(0.5f + static_cast<float>(u), 0.5f + static_cast<float>(v), z);
			V3 pixP = ppc0->Unproject(pP);
			cv.SetColor(Get(u, v));
			fb1->Draw3DPoint(ppc1, pixP, cv.GetColor(), 1);
		}
	}
}

V3 FrameBuffer::lookup_color(std::string texFile, float s, float t, float& alpha, int pixelSz)
{
	int curLoD = 0;
	int nextLoD = 0;


	// Default, look up highest quality texture
	int maxLoD = static_cast<int>(textures[texFile].size() - 1);
	if (pixelSz == -1)
	{
		curLoD = maxLoD;
		nextLoD = min(curLoD + 1, maxLoD);
	}
	else
	{
		curLoD = clamp(static_cast<int>(log2(pixelSz)), 1, maxLoD);
		nextLoD = min(curLoD + 1, maxLoD);
	}

	if (nextLoD == curLoD)
		return bilinear_interpolation(textures[texFile][curLoD], s, t, alpha);

	V3 c0 = bilinear_interpolation(textures[texFile][curLoD], s, t, alpha);
	V3 c1 = bilinear_interpolation(textures[texFile][nextLoD], s, t, alpha);
	float fract = static_cast<float>(log2(pixelSz) - curLoD);

	return c0 * (1.0f - fract) + c1 * fract;
}

V3 FrameBuffer::bilinear_interpolation(float s, float t, float& alpha)
{
	V3 c0, c1, c2, c3;
	int texW = w, texH = h;
	float textS = s * static_cast<float>(texW - 1);
	float textT = t * static_cast<float>(texH - 1);
	float a0, a1, a2, a3; // alpha

	int u0 = clamp(static_cast<int>(textS - 0.5f), 0, texW - 1);
	int v0 = clamp(static_cast<int>(textT - 0.5f), 0, texH - 1);
	int u1 = clamp(static_cast<int>(textS + 0.5f), 0, texW - 1);
	int v1 = clamp(static_cast<int>(textT + 0.5f), 0, texH - 1);

	unsigned int ori0 = pix[(texH - 1 - v0) * texW + u0];
	unsigned int ori1 = pix[(texH - 1 - v0) * texW + u1];
	unsigned int ori2 = pix[(texH - 1 - v1) * texW + u0];
	unsigned int ori3 = pix[(texH - 1 - v1) * texW + u1];
	c0.SetColor(ori0);
	c1.SetColor(ori1);
	c2.SetColor(ori2);
	c3.SetColor(ori3);

	a0 = get_alpha(ori0);
	a1 = get_alpha(ori1);
	a2 = get_alpha(ori2);
	a3 = get_alpha(ori3);

	float uf0 = static_cast<float>(u0) + 0.5f, vf0 = static_cast<float>(v0) + 0.5f;
	float intpS = clamp(textS - uf0, 0.0f, 1.0f), intpT = clamp(textT - vf0, 0.0f, 1.0f);

	// commit result
	alpha = a0 * (1.0f - intpS) * (1.0f - intpT) + a1 * intpS * (1.0f - intpT) + a2 * (1.0f - intpS) * intpT + a3 *
		intpS * intpT;
	return c0 * (1.0f - intpS) * (1.0f - intpT) + c1 * intpS * (1.0f - intpT) + c2 * (1.0f - intpS) * intpT + c3 * intpS
		* intpT;
}

V3 FrameBuffer::bilinear_interpolation(shared_ptr<TextureInfo> tex, float s, float t)
{
	V3 c0, c1, c2, c3;
	int texW = tex->w, texH = tex->h;
	float textS = s * static_cast<float>(texW - 1);
	float textT = t * static_cast<float>(texH - 1);

	int u0 = clamp(static_cast<int>(textS - 0.5f), 0, texW - 1);
	int v0 = clamp(static_cast<int>(textT - 0.5f), 0, texH - 1);
	int u1 = clamp(static_cast<int>(textS + 0.5f), 0, texW - 1);
	int v1 = clamp(static_cast<int>(textT + 0.5f), 0, texH - 1);

	unsigned int ori0 = tex->texture[(texH - 1 - v0) * texW + u0];
	unsigned int ori1 = tex->texture[(texH - 1 - v0) * texW + u1];
	unsigned int ori2 = tex->texture[(texH - 1 - v1) * texW + u0];
	unsigned int ori3 = tex->texture[(texH - 1 - v1) * texW + u1];

	c0.SetColor(ori0);
	c1.SetColor(ori1);
	c2.SetColor(ori2);
	c3.SetColor(ori3);

	float uf0 = static_cast<float>(u0) + 0.5f, vf0 = static_cast<float>(v0) + 0.5f;
	float interpolation_s = clamp(textS - uf0, 0.0f, 1.0f);
	float interpolation_t = clamp(textT - vf0, 0.0f, 1.0f);

	return c0 * (1.0f - interpolation_s) * (1.0f - interpolation_t) + c1 * interpolation_s * (1.0f - interpolation_t) + c2 * (1.0f - interpolation_s) * interpolation_t + c3 * interpolation_s
		* interpolation_t;
}


V3 FrameBuffer::bilinear_interpolation(shared_ptr<TextureInfo> tex, float s, float t, float& alpha)
{
	V3 c0, c1, c2, c3;

	int texW = tex->w, texH = tex->h;
	float textS = s * static_cast<float>(texW - 1);
	float textT = t * static_cast<float>(texH - 1);
	float uf0 = static_cast<float>(u0) + 0.5f;
	float vf0 = static_cast<float>(v0) + 0.5f;

	float a0, a1, a2, a3;

	int u0 = clamp(static_cast<int>(textS - 0.5f), 0, texW - 1);
	int v0 = clamp(static_cast<int>(textT - 0.5f), 0, texH - 1);
	int u1 = clamp(static_cast<int>(textS + 0.5f), 0, texW - 1);
	int v1 = clamp(static_cast<int>(textT + 0.5f), 0, texH - 1);

	unsigned int ori0 = tex->texture[(texH - 1 - v0) * texW + u0];
	unsigned int ori1 = tex->texture[(texH - 1 - v0) * texW + u1];
	unsigned int ori2 = tex->texture[(texH - 1 - v1) * texW + u0];
	unsigned int ori3 = tex->texture[(texH - 1 - v1) * texW + u1];

	c0.SetColor(ori0);
	c1.SetColor(ori1);
	c2.SetColor(ori2);
	c3.SetColor(ori3);

	a0 = get_alpha(ori0);
	a1 = get_alpha(ori1);
	a2 = get_alpha(ori2);
	a3 = get_alpha(ori3);
	
	float interpolation_s = clamp(textS - uf0, 0.0f, 1.0f);
	float interpolation_t = clamp(textT - vf0, 0.0f, 1.0f);

	alpha = a0 * (1.0f - interpolation_s) * (1.0f - interpolation_t) + a1 * interpolation_s * (1.0f - interpolation_t) + a2 * (1.0f - interpolation_s) * interpolation_t + a3 *
		interpolation_s * interpolation_t;
	return c0 * (1.0f - interpolation_s) * (1.0f - interpolation_t) + c1 * interpolation_s * (1.0f - interpolation_t) + c2 * (1.0f - interpolation_s) * interpolation_t + c3 * interpolation_s
		* interpolation_t;
}

V3 FrameBuffer::Light(vertex_properties pp, V3 L, PPC* ppc)
{
	V3 ret(0.0f);
	float ka = 0.5f;
	float kd = max((L - pp.p).UnitVector() * pp.n.UnitVector(), 0.0f);
	float ks = (ppc->C - pp.p).UnitVector() * (L - pp.p).UnitVector().Reflect(pp.n.UnitVector());
	ks = 0.5f * pow(max(ks, 0.0f), 32);
	ret = pp.c * (ka + (1.0f - ka) * kd) + ks;
	return ret;
}

void FrameBuffer::PrepareTextureLoD(const string texFile)
{
	if (textures.find(texFile) == textures.end())
		return;

	auto curTex = textures[texFile][0];
	int loDMax = static_cast<int>(log2(curTex->w)), curLoD = loDMax;
	textures[texFile].clear();
	textures[texFile].resize(loDMax + 1);
	textures[texFile][curLoD] = curTex;

	while (curTex->w >= 2)
	{
		int nextW = curTex->w / 2, newLoD = curLoD - 1;
		shared_ptr<TextureInfo> newTex = make_shared<TextureInfo>();
		newTex->w = nextW;
		newTex->h = nextW;
		newTex->texture.resize(nextW * nextW);

		auto GetPixColor = [&](vector<unsigned int>& pix,int w, int h, int u, int v)
		{
			return pix[(h - 1 - v) * w + u];
		};

		auto SetPixColor = [&](vector<unsigned int>& pix, V3 c, int w, int h, int u, int v)
		{
			pix[(h - 1 - v) * w + u] = c.GetColor();
		};

		for (int ustep = 0; ustep < curTex->w / 2; ++ustep)
		{
			for (int vstep = 0; vstep < curTex->h / 2; ++vstep)
			{
				int u0 = ustep * 2 + 0, u1 = ustep * 2 + 1;
				int v0 = vstep * 2 + 0, v1 = vstep * 2 + 1;
				V3 c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f);
				c0.SetColor(GetPixColor(curTex->texture, curTex->w, curTex->h, u0, v0));
				c1.SetColor(GetPixColor(curTex->texture, curTex->w, curTex->h, u0, v1));
				c2.SetColor(GetPixColor(curTex->texture, curTex->w, curTex->h, u1, v0));
				c3.SetColor(GetPixColor(curTex->texture, curTex->w, curTex->h, u1, v1));
				V3 c = (c0 + c1 + c2 + c3) * 0.25f;
				SetPixColor(newTex->texture, c, newTex->w, newTex->h, ustep, vstep);
			}
		}

		textures[texFile][newLoD] = newTex;
		curTex = newTex;
		curLoD = newLoD;

		if (is_save_lod_tex)
		{
			const string texSaveName = texFile + to_string(newLoD) + ".tiff";
			SaveTextureAsTiff(texSaveName, texFile, newLoD);
			cerr << "Current tex: " << texFile << " While count: " << newLoD << endl;
		}
	}
}

void FrameBuffer::SetupGPU()
{
	isgpu = true;
}

void FrameBuffer::SaveGPUAsTiff(const string saveFile)
{
	SaveGPU2CPU();
	SaveAsTiff(saveFile.c_str());
}

GLuint FrameBuffer::SaveCPU2GPUtexture()
{
	GLuint texID = 0;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}
