#pragma once
#include "v3.h"
#include "ray.h"

class PPC
{
public:
	V3 a, b, c, C;	
	int w, h;
	float lens_radius;

	PPC(int _w, int _h, float hfov);
	PPC(int _w, int _h, float hfov, float aperture);
	void Translate(V3 v);
	int Project(V3 P, V3 &ProjP);
	bool inside_image_plane(V3 pp);

	void Roll(float theta);
	void Pan(float theta);
	void Tilt(float theta);
	void RevolveV(V3 p, float theta);
	void RevolveH(V3 p, float theta);
	void Zoom(float theta);
	void MoveForward(float delta);
	void MoveLeft(float delta);
	void MoveDown(float delta);

	V3 GetVD();							
	float GetFocal();
	float GetVerticalFOV();
	float GetHorizontalFOV();
	V3 GetRay(int u, int v);			
	V3 GetRay(float u, float v);		
	V3 GetRayCenter(int u, int v);		
	V3 Unproject(V3 pp);				
	V3 UnprojectPixel(float uf, float vf, float currf);	
	ray GetRayWithAperture(float u, float v);

	void PositionAndOrient(V3 newC, V3 lap, V3 up);		
	void PositionAndOrient(V3 newC, V3 lap, V3 up, float aperture, float focal);

	void SaveBin(std::string fname);
	void LoadBin(std::string fname);

	void SetInterpolated(PPC* ppc0, PPC* ppc1, float fract);
	void SetIntrinsicsHW();
	void SetExtrinsicsHW();
};

