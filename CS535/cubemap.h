#pragma once
#include "framebuffer.h"

/*************************************/
// To initialize cubemp:
// Load cubemap in the same order with the ppcs
/*************************************/
class CubeMap
{
private:
	int last_fb_id;

public:
	shared_ptr<FrameBuffer> cubemapFB;
	vector<shared_ptr<PPC>> ppcs;
	vector<string> mapOrder;

	void LoadCubeMap(vector<string> fnames, vector<shared_ptr<PPC>> _ppcs);
	V3 LookupColor(V3 dir, int pixSz = -1);
	CubeMap();
	~CubeMap();
};

