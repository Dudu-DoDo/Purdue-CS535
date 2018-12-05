#include "CubeMap.h"
#include <fstream>

void CubeMap::LoadCubeMap(vector<string> fnames, vector<shared_ptr<PPC>> _ppcs)
{
	bool isSuccess = true;

	if(!cubemapFB)
		cubemapFB = make_shared<FrameBuffer>(0,0,0,0);

	ppcs = _ppcs;
	cubemapFB->textures.clear();
	mapOrder.clear();
	for(auto f:fnames)
	{
		if (!cubemapFB->LoadTexture(f)) isSuccess = false;
		mapOrder.push_back(f);
	}

	if (isSuccess)
		cerr << "Succesfully load cube maps! \n";
}

V3 CubeMap::LookupColor(V3 dir, int pixSz)
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

CubeMap::CubeMap():last_fb_id(0)
{
}


CubeMap::~CubeMap()
{
}
