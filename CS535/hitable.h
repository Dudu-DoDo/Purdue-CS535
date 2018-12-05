#pragma once
#include "ray.h"

class material;

struct hit_record
{
	float t;
	V3 p, n;
	shared_ptr<material> mat_ptr;
};

class hitable
{
public:
	virtual bool hit(ray& r, float t_min, float t_max, hit_record &rec) const = 0;
};

