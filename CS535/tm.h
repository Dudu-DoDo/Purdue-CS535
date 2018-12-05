#pragma once
#include <vector>

#include "v3.h"
#include "framebuffer.h"
#include "AABB.h"
#include "cubemap.h"
#include "CGInterface.h"

using std::vector;
class BillBoard;

struct vertex_properties
{
	V3 p, c, n;		// position, color, normal
	float s, t;		// texture s, texture t
	vertex_properties(V3 _p, V3 _c, V3 _n, float _s, float _t) :p(_p), c(_c), n(_n), s(_s), t(_t) {};
};

// Triangle Mesh
class TM
{
public:
	vector<V3> verts, colors, normals, ori_verts, ori_norms;		// vertices and colors
	vector<float> verts_st;
	vector<unsigned int>  tris;		// indices

	int vertsN, trisN;
	int id;
	int is_cube;
	int is_cubemap;
	int is_box;
	int is_ground;

	vector<shared_ptr<BillBoard>> reflector_billboards;	

	shared_ptr<CGInterface> cgi;
	shared_ptr<ShaderOneInterface> soi;
	string shaderOneFile;

	std::string tex;
	int lod_size;		
	bool use_env_mapping;
	bool use_obj_color;
	bool is_refract;

	TM() :vertsN(0), trisN(0), lod_size(0), use_env_mapping(false), 
		use_obj_color(true),id(ID++), is_refract(false),
		is_cube(0), is_cubemap(0), is_ground(0) {};

	void SetRectangle(V3 O, float rw, float rh);
	void SetTriangle(vertex_properties p0, vertex_properties p1, vertex_properties p2);
	void SetQuad(vertex_properties p0, vertex_properties p1, vertex_properties p2, vertex_properties p3);
	void SetQuad(V3 O, V3 n, V3 up, float sz, float s = 1.0f, float t = 1.0f);
	void set_bilboard(V3 O, V3 n, V3 up, float sz, float s = 1.0f, float t = 1.0f);
	void set_unit_box();
	void SetText(std::string tf);
	void Allocate();
	void SetAllPointsColor(V3 color);	// set color to verts
	void set_shader_one(const string shaderFile) { shaderOneFile = shaderFile; };
	void reset_color(V3 color);
	tuple<V3, V3, V3, V3> get_corners_axis();

	// Rasterization
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void RenderFill(PPC *ppc, FrameBuffer *fb);
	void RenderFillZ(PPC *ppc, FrameBuffer *fb); // only draw z buffer
	void RenderAABB(PPC *ppc, FrameBuffer *fb);
	void render_billboard(PPC *ppc, FrameBuffer *fb, FrameBuffer *bbTexture);
	void render_hardware(PPC *ppc, FrameBuffer *curfb);
	void render_hardware_wireframe(PPC *ppc, FrameBuffer *curfb);

	// Transformation
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void Translate(V3 tv);
	void Scale(float scf);		// normalize size to some scf
	void LoadModelBin(char *fname);
	AABB ComputeAABB();
	float ComputeSBBR(V3 c);	// given c, compute r
	void PositionAndSize(V3 tmC, float tmSize);
	V3 GetCenter();
	
	tuple<V3, float> shading(PPC* ppc, FrameBuffer* fb, int u, int v, float w, vertex_properties& pp, V3 dn = V3(0.0f));
	void lighting_shading(V3 mc, V3 L, PPC *ppc);	  // Per vertex light
	V3 lighting_shading(PPC *ppc, vertex_properties& pp, int u, int v, float w); // Per pixel  light 
	bool calculate_shadow(PPC* ppc, int u, int v, float z, float &shadow_effect);
	bool calculate_projective_texture(int u, int v, float z, V3 &color, float &alpha);
	V3 homograph_mapping(V3 uvw, PPC* ppc1, PPC* ppc2);
	V3 clamp_color(V3 color);
	tuple<V3, float> env_mapping(PPC *ppc, FrameBuffer *fb, CubeMap *cubemap, V3 p, V3 n, V3 dn = V3(0.0f));
	int env_billboard_intersection(vector<shared_ptr<BillBoard>> bbs, V3 p, V3 viewDir, float &distance, V3 &color, float &alpha);

	// Morphing
	void morph_to_sphere(V3 c,float r, float fract);

	
	~TM();
	static int ID;
};
 
