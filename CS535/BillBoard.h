#pragma once
#include "TM.h"
#include "m33.h"

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

