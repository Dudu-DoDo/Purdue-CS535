# Purdue CS535 
This repo has the source codes and some notes for CS535. 

# Notes for each homework 
Here are some key implementations for each homework.

## HW3 Texture Mapping 
### Billinear Interpolation 
```c++
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
	float intpS = clamp(textS - uf0, 0.0f, 1.0f), intpT = clamp(textT - vf0, 0.0f, 1.0f);

	// commit result
	return c0 * (1.0f - intpS) * (1.0f - intpT) + c1 * intpS * (1.0f - intpT) + c2 * (1.0f - intpS) * intpT + c3 * intpS
		* intpT;
}
```

### LoD Trillinear Interpolation
```c++
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
```

## HW4 Shaodw Mapping and Projective Texture Mapping
### Homograph Mapping 
```c++
V3 homograph_mapping(V3 uvw, PPC* ppc1, PPC* ppc2)
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
```

### Invisible Projective Texture Mapping 
Key idea: render 2 different zbuffers. One default zbuffer and one for background. When rendering the projective texture mapping, we should use the second zbuffer as the w in the homograph mapping function so that the object is invisible.

## HW5 Environment Mapping
### Cubemap Lookup Function 
```c++
V3 LookupColor(V3 dir, int pixSz)
{
	V3 color(0.0f);
	if (ppcs.size() != 6)
		return color;

	dir = dir.UnitVector();
	V3 p = dir;
	V3 pp(0.0f);

	for(int i = 0; i < ppcs.size(); ++i)
	{
		auto curPPC = ppcs[last_fb_id];
		curPPC->Project(p, pp);
		if (curPPC->inside_image_plane(pp))
		{
			// catched by one ppc
			float s = pp[0] / static_cast<float>(curPPC->w-1), t = pp[1] / static_cast<float>(curPPC->h-1);			
			float a = 0.0f;
			color = cubemapFB->lookup_color(mapOrder[last_fb_id], s, t, a, pixSz);
			break;
		}

		last_fb_id = (last_fb_id + 1) % 6;
	}
	
	return color;
}
```

### Billboard for Specular Reflection
Class design:
```c++
class BillBoard
{
public:
	shared_ptr<TM> mesh;
	shared_ptr<FrameBuffer> texture;
	GLuint texID;

	BillBoard();
	~BillBoard();

	void set_billboard(V3 O, V3 n, V3 up, float sz, float s = 1.0f, float t = 1.0f);
	bool intersect(V3 p, V3 d, float & t);
	bool inside_billboard(V3 p);

	M33 get_corners();
	V3 color(V3 p, float &alpha);
	V3 color(FrameBuffer *fb, V3 p);
	V3 color(FrameBuffer *fb, V3 p, float &alpha);

	void render(PPC *ppc, FrameBuffer *fb);
private:
	void get_st(V3 p, float &s, float &t);
};

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

```

During rendering, we need to update the billboard everytime when any object in the scene moves.
After the updates, we can call the RenderFill for each triangle mesh.
The key in RenderFill is to use EnvMapping for surface shading.

```c++

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
```

## HW6 Hardware Rendering
The code implement the soft shadow rendering part codes.
The key feature here is to setup the scene using several billboards. For example, each face of the cube is a billboard, each face of the cubemap is a billboard, the ground can also be a billboard. 

The ray-billboard intersection is fast. We can first do the ray-plane intersection, then test if the intersection point is inside the billboard or not. 

So the key function is ray-billboard test: 

```c++
bool intersect_billboard(float3x3 bb, float3 ro, float3 rd, out float ot)
{
	float3 p0 = bb[0];
	float3 p1 = bb[1];
	float3 p2 = bb[2];
	float3 n = normalize(cross(p1-p0,p2-p0)); 
	
	float t = -dot(ro - p0,n)/dot(rd,n);
	float3 p = ro + t * rd - p0;
	float3 x = p2 - p0;
	float3 y = p1 - p0;
	float u = dot(p,normalize(x)) / length(x);
	float v = dot(p,normalize(y)) / length(y);
	
	if(u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0)
	{
		return false;
	}

	if(tex2D(texture, float2(u,v)).r > 0.0)
		return false;

	ot = t;
	return true;
}
```

The billboard can be definied by 3 corners. 

To render the transparency, we also need to shoot the ray such that the ray will continue to intersect with the scene. So the ray-scene intersection is the key: 

```c++

void ray_scene_intersect(float3 ro, float3 rd, out float t, out float3 color)
{
	float eps = 1e-2;
	float tmp_t;
	color = 0.8;

	all_faces box0Faces, box1Faces, box2Faces;
	box0Faces = get_all_faces(gb_box0);
	box1Faces = get_all_faces(gb_box1);
	box2Faces = get_all_faces(gb_box2);

	t = (gnd_plane_height - ro.y) / rd.y;

	for(int i = 0; i < 6; ++i)
	{
		if(intersect_billboard(box0Faces.face[i], ro, rd, tmp_t))
		{
			if(tmp_t > 0.0 && tmp_t < t && tmp_t > eps)
			{
				t = tmp_t;
				color = gb_box0_color;
			}
		}
		
		if(intersect_billboard(box1Faces.face[i], ro, rd, tmp_t))
		{
			if(tmp_t > 0.0 && tmp_t < t && tmp_t > eps)
			{
				t = tmp_t;
				color = gb_box1_color;
			}
		}

		if(intersect_billboard(box2Faces.face[i], ro, rd, tmp_t))
		{
			if(tmp_t > 0.0 && tmp_t < t && tmp_t > eps)
			{
				t = tmp_t;
				color = gb_box2_color;
			}
		}
	}
}
```
So after the ray-scene intersection, we will replace the transparent cube surface color with the ray-scene intersected surface color.
