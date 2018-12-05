
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <random>

#include "AABB.h"
#include "TM.h"
#include "global_variables.h"
#include "scene.h"
#include "v3.h"
#include "m33.h"

Scene* scene;
int TM::ID = 0;

Scene::Scene()
{
	int u0 = 20;
	int v0 = 20;
	int w = res_W;
	int h = res_H;
	int fovf = 70.0f;

	gui = new GUI();
	fb = new FrameBuffer(u0, v0, w, h);
	gpufb = new FrameBuffer(u0, v0, w, h);
	third_view_fb = new FrameBuffer(u0 + fb->w + 30, v0, 512, 512);
	ppc = new PPC(fb->w, fb->h, fovf);
	third_view_ppc = new PPC(third_view_fb->w, third_view_fb->h, 35.0f);

	gui->show();
	gpufb->SetupGPU();
	gpufb->show();
	gui->uiw->position(u0, v0 + fb->h + 100);
	init_demo();
}

void Scene::render()
{
	float currf = 40.0f;

	render_to_fb(ppc, fb);
	third_view_fb->ClearBGRZ(0xFFFFFFFF, 0.0f);
	third_view_fb->DrawPPC(third_view_ppc, ppc, currf);
	fb->VisualizeCurrView(ppc, currf, third_view_ppc, third_view_fb); 
	fb->VisualizeCurrView3D(ppc, third_view_ppc, third_view_fb);

	for (auto l : light_ppcs)
	{
		third_view_fb->Draw3DPoint(third_view_ppc, l->C, 0xFFFFFF00, 10);
	}

	third_view_fb->redraw();
}

void Scene::render_to_fb(PPC* currPPC, FrameBuffer* currFB)
{
	if (currFB)
	{
		currFB->ClearBGRZ(0xFF999999, 0.0f);

		if(cubemap)
			currFB->DrawCubeMap(currPPC, cubemap.get());
			
		for (auto t : all_meshes) {
			t->RenderFill(currPPC, currFB);
		}

		for (auto r : reflection_meshes) {
			r->RenderFill(currPPC, currFB);
		}
		
		currFB->redraw();
	}
}

void Scene::render_wireframe()
{
	fb->ClearBGRZ(0xFFFFFFFF, 0.0f);

	for (auto t : all_meshes) {
		t->RenderWireFrame(ppc, fb);
	}

	fb->redraw();
}

void Scene::render_zbuffer(PPC* currPPC, FrameBuffer* currFB)
{
	if (currFB)
	{
		currFB->ClearZ(0.0f);

		for (auto m : all_meshes)
		{
			m->RenderFillZ(currPPC, currFB);
		}

		currFB->redraw();
	}
}

void Scene::update_sm()
{
	for (size_t li = 0; li < light_ppcs.size(); ++li)
	{
		render_zbuffer(light_ppcs[li].get(), shadow_maps[li].get());
	}
}

void Scene::render_GPU()
{
	glEnable(GL_DEPTH_TEST | GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto cube_map_file = cubemapFiles;
	if (FrameBuffer::gpu_tex_ID.find(cube_map_file[0]) == FrameBuffer::gpu_tex_ID.end()) {
		gpufb->load_cubemap_to_GPU(cube_map_file);
	}

	for (auto t : all_meshes) {
		t->render_hardware(ppc, gpufb);
	}
}

void Scene::render_GPU_wireframe_mode() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ppc->SetIntrinsicsHW();
	ppc->SetExtrinsicsHW();

	for (auto t : all_meshes)
	{
		t->render_hardware_wireframe(ppc, gpufb);
	}
}

void Scene::update_billboards()
{
	int w = res_W;
	int h = res_H;

	for (auto r : reflection_meshes)
	{
		auto tmp_id = r->id;

		auto ref_center = r->GetCenter();
		auto r_size = r->ComputeAABB().GetDiagnoalLength();

		r->reflector_billboards.clear();
		r->reflector_billboards.reserve(reflection_meshes.size() - 1);

		for (auto other_tm : reflection_meshes)
		{
			if (other_tm->id == tmp_id)
				continue;

			auto other_tm_center = other_tm->GetCenter();
			shared_ptr<BillBoard> bb = make_shared<BillBoard>();
			V3 n = (ref_center - other_tm_center).UnitVector();
			
			float fovf = 55.0f;
			shared_ptr<PPC> ppc = make_shared<PPC>(w, h, fovf);
			shared_ptr<FrameBuffer> bbFB = make_shared<FrameBuffer>(0, 0, w, h);

			bb->set_billboard(ref_center, n, V3(0.0f, 1.0f, 0.0f), r_size);
			ppc->PositionAndOrient(ref_center, other_tm_center, V3(0.0f, 1.0f, 0.0f));

			render_billboards(ppc.get(), bbFB.get(), other_tm);

			bb->texture = bbFB;
			bb->mesh->PositionAndSize(other_tm_center, r_size);

			r->reflector_billboards.push_back(bb);
		}
	}
}

void Scene::render_billboards(PPC* currPPC, FrameBuffer* currFB, shared_ptr<TM> reflector) {
	currFB->ClearBGRZ(0xFFFFFFFF, 0.0f);
	reflector->RenderFill(currPPC, currFB);
	currFB->redraw();
}

V3 Scene::calc_scene_center()
{
	V3 ret(0.0f);
	for (auto m : all_meshes) {
		ret = ret + m->GetCenter();
	}
	ret = ret / static_cast<float>(all_meshes.size());
	return ret;
}

Scene::~Scene()
{
	if (ppc != nullptr)
		delete ppc;
	if (third_view_ppc != nullptr)
		delete third_view_ppc;
	if (fb != nullptr)
		delete fb;
	if (third_view_fb != nullptr)
		delete third_view_fb;
}

void Scene::debug()
{
	demo();
	fb->redraw();
}

void Scene::init_lights()
{
	int u0 = 20, v0 = 20;
	float fovf = 80.0f;
	int w = 640;
	int h = 640;
	V3 y(0.0f, 1.0f, 0.0f), gColor(0.5f);
	float lightsz = 1.0f, height = 0.0f;
	V3 light_pos = all_meshes[0]->GetCenter() + V3(0.0f, 100.0f, 100.0f);

	shared_ptr<PPC> light_ppc = make_shared<PPC>(w, h, fovf);
	shared_ptr<FrameBuffer> light_sm = make_shared<FrameBuffer>(u0 + fb->w * 2 + 30, v0, w, h);

	light_sm->ClearBGRZ(0xFFFFFFFF, 0.0f);
	light_ppc->PositionAndOrient(light_pos, all_meshes[0]->GetCenter(), V3(0.0f, 1.0f, 0.0f));

	shadow_maps.push_back(light_sm);
	light_ppcs.push_back(light_ppc);
	update_sm();
}

void Scene::init_demo() {
	int w = 800;
	int h = 800;
	int len = 100;

	float tm_size = 100.0f;
	float box_fract = 0.65f;
	
	ka = 0.5f;
	mf = 0.0f;

	V3 tmC = ppc->C + ppc->GetVD() * 150.0f;

	shared_ptr<TM> cubemap = make_shared<TM>();
	shared_ptr<TM> box0 = make_shared<TM>();
	shared_ptr<TM> box1 = make_shared<TM>();
	shared_ptr<TM> box2 = make_shared<TM>();
	shared_ptr<TM> ground = make_shared<TM>();

	cubemap->set_unit_box();
	box0->set_unit_box();
	box1->set_unit_box();
	box2->set_unit_box();

	cubemap->set_shader_one("CG/shaderOne.cg");
	box0->set_shader_one("CG/shaderOne.cg");
	box1->set_shader_one("CG/shaderOne.cg");
	box2->set_shader_one("CG/shaderOne.cg");
	
	cubemap->is_cube = 0;
	cubemap->is_cubemap = 1;
	box0->is_box = 1;
	box1->is_box = 1;
	box2->is_box = 1;
	
	box0->is_cube = 1;
	box1->is_cube = 1;
	box2->is_cube = 1;

	box0->tex = "images/tex.tiff";
	box1->tex = "images/tex.tiff";
	box2->tex = "images/tex.tiff";

	ground->SetQuad(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f,0.0f,-1.0f), 1.0f);
	ground->set_shader_one("CG/shaderOne.cg");
	ground->is_ground = 1;
	
	cubemap->PositionAndSize(V3(0.0f), tm_size * 17.0f);
	box0->PositionAndSize(tmC, tm_size * box_fract);
	box1->PositionAndSize(tmC + V3(-1.0f,0.0f,-1.0f) * tm_size * box_fract * 0.4f, tm_size* box_fract);
	box2->PositionAndSize(tmC + V3(1.0f, 0.0f, -1.0f) * tm_size* box_fract * 0.4f, tm_size* box_fract);
	ground->PositionAndSize(tmC + V3(0.0f,-1.0f,0.0f) * tm_size* box_fract * 0.9f, tm_size * 100.0f);

	box0->reset_color(V3(1.0f, 0.0f, 0.0f));
	box1->reset_color(V3(0.0f, 1.0f, 0.0f));
	box2->reset_color(V3(0.0f, 0.0f, 1.0f));
	ground->reset_color(V3(0.75f));

	all_meshes.push_back(box0);
	all_meshes.push_back(box1);
	all_meshes.push_back(box2);
	all_meshes.push_back(ground);
	all_meshes.push_back(cubemap);

	V3 light_center = all_meshes[0]->GetCenter() + V3(0.0f,90.0f, 90.0f);
	shared_ptr<PPC> light_ppc_ = make_shared<PPC>(w, h, 90.0f);
	light_ppc_->PositionAndOrient(light_center, all_meshes[0]->GetCenter(), V3(0.0f, 1.0f, 0.0f));
	light_ppcs.push_back(light_ppc_);

	ppc->PositionAndOrient(V3(0.0f, tm_size, -5.0f), calc_scene_center(), V3(0.0f, 1.0f, 0.0f));
	ppc->RevolveH(all_meshes[0]->GetCenter(), 60.0f);

	third_view_fb->ClearBGRZ(0xFFFFFFFF, 0.0f);
	third_view_fb->DrawEllipse(w / 2, h / 2, 50, 50, 0xFF000000);
	
	third_view_fb->redraw();
	third_view_fb->SaveAsTiff("images/tex.tiff");
}

void Scene::demo()
{
	int count = 0;
	int framesN = 720;
	PPC ppc0 = *ppc, ppc1 = *ppc;

	ppc1.C = ppc1.C + V3(40.0f, 100.0f, 0.0f);
	ppc1.PositionAndOrient(ppc1.C, all_meshes[1]->GetCenter(), V3(0.0f, 1.0f, 0.0f));
	ppc1 = *ppc;
		
	for(int i = 0; i < framesN; ++i) {
		light_ppcs[0]->RevolveH(all_meshes[1]->GetCenter(), 1.0f);
		mf = static_cast<float>(i) / static_cast<float>(framesN - 1);

		gpufb->redraw();
		Fl::check();

		if(true) {
			char buffer[50];
			sprintf_s(buffer,"images/%03d.tiff", i);
			gpufb->SaveGPUAsTiff(buffer);
		}
	}
	*ppc = ppc0;
	return;
}
