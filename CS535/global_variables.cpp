#include "global_variables.h"

bool use_HD = true;
int high_W = 1020;
int high_H = 720;
int res_W = 640;
int res_H = 480;
bool isRayTracing = false;
bool isDBGRaytracing = true;
bool isUseSBB = true;
bool is_print_FPS = true;

Scene*  cur_scene = nullptr;
bool use_projected_tex = true;
bool depth_test = true;
bool lod_texture = true;
string projected_tex_name = "images/projected.tiff";
string checker_box_tex_name = "images/Checkerboard_pattern.tiff";

bool is_dbg_zbuffer = false;
bool is_save_lod_tex = false;
bool isLight = true;
bool is_shadow = false;
bool isRefraction = false;
bool is_wireframe = false;
float refractRatio = 1.15f;
	
string cubemapFolder = "images/cubemaps/";

vector<string> cubemapFiles = {
	cubemapFolder + "right.tiff",
	cubemapFolder + "left.tiff",
	cubemapFolder + "ground.tiff",
	cubemapFolder + "top.tiff",
	cubemapFolder + "front.tiff",
	cubemapFolder + "back.tiff"
};

bool isCubemapMipmap = false;
int tmAnimationID = 0;
bool is_recording = true;
string recordName = "bug";
bool isOpenMP = true;
