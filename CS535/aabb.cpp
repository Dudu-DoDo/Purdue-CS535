#include <algorithm>

#include "AABB.h"

AABB::AABB(V3 firstPoint)
{
	corners[0] = firstPoint;
	corners[1] = firstPoint;
}

void AABB::AddPoint(V3 p)
{
	for(int i = 0; i <3; ++i)
	{
		corners[0][i] = std::min(p[i], corners[0][i]);
		corners[1][i] = std::max(p[i], corners[1][i]);
	}
}

V3 AABB::GetCenter()
{
	return (corners[0] + corners[1]) * 0.5f;
}

V3 AABB::GetDiagnolVector()
{
	return corners[1] - corners[0];
}

float AABB::GetDiagnoalLength()
{
	V3 diagonalV = corners[1] - corners[0];
	return diagonalV.Length();
}

int AABB::clip(float left, float right, float top, float bottom)
{
	V3 &c0 = corners[0];
	V3 &c1 = corners[1];
	if (c0[0] > right || c1[0] < left || c0[1] > bottom || c1[1] < top)
		return 0;
	if (c0[0] < left)
		c0[0] = left;
	if (c1[0] > right)
		c1[0] = right;
	if (c0[1] < top)
		c0[1] = top;
	if (c1[1] > bottom)
		c1[1] = bottom;
	return 1;
}

ostream& operator<<(ostream& ist, AABB bbox)
{
	return ist << "BBox corners: " << bbox.corners[0] << bbox.corners[1];
}
