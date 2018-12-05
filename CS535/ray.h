#pragma once
#include "v3.h"

class ray
{
public:
	// Ro and rd
	V3 ro, rd;

	ray() {};
	ray(const V3 &a, const V3 &b) { ro = a; rd = b; }
	V3 origin() const { return ro; }
	V3 direction() const { return rd; }
	V3 point_at_parameter(float t) { return ro + rd * t; }
};

