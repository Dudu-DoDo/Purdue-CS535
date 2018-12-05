#pragma once
#include "v3.h"

class AABB
{
public:
	V3 corners[2];
	AABB(V3 firstPoint);
	void AddPoint(V3 p);
	V3 GetCenter();
	V3 GetDiagnolVector();
	float GetDiagnoalLength();
	int clip(float left, float right, float top, float bottom);

	friend ostream& operator<<(ostream& ist, AABB bbox);
};
