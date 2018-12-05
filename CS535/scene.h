#pragma once
#include <vector>
#include <memory>
#include <chrono>

#include "CGInterface.h"
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "TM.h"
#include "cubemap.h"
#include "BillBoard.h"

using std::vector;
using std::unique_ptr;
using Clock = std::chrono::high_resolution_clock;

class Scene
{
public:
	GUI* gui;
	PPC* ppc, * third_view_ppc, * projectPPC;
	FrameBuffer* fb, *third_view_fb, *fbp, *gpufb;

	std::vector<shared_ptr<PPC>> light_ppcs;

	std::vector<shared_ptr<FrameBuffer>> textures;
	std::vector<shared_ptr<FrameBuffer>> shadow_maps;
	
	vector<shared_ptr<TM>> all_meshes;
	vector<shared_ptr<TM>> reflection_meshes;
	vector<shared_ptr<BillBoard>> billboards;
	shared_ptr<CubeMap> cubemap;
	
	float ka;
	float mf;

	Scene();
	void debug();
	void render();				// render all triangles in the scene
	void render_to_fb(PPC *currPPC, FrameBuffer *currFB);
	void render_wireframe();
	void render_zbuffer(PPC *currPPC, FrameBuffer *currFB);
	void update_sm();

	void render_GPU();
	void render_GPU_wireframe_mode();

	void update_billboards();
	void render_billboards(PPC *curPPC, FrameBuffer *curFB, shared_ptr<TM> reflectors);

	V3 calc_scene_center();
	~Scene();


private:
	void demo();
	void init_demo();
	void init_lights();
};

extern Scene* scene;
