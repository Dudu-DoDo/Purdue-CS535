#pragma once
#include "hitable.h"
#include <vector>

class hitable_list :
	public hitable
{
public:
	std::vector<shared_ptr<hitable>> list;

	hitable_list() {};
	hitable_list(std::vector<shared_ptr<hitable >> &l) :list(l) {};

	bool hit(ray& r, float t_min, float t_max, hit_record& rec) const override;
};

inline bool hitable_list::hit(ray& r, float t_min, float t_max, hit_record& rec) const
{
	hit_record tmp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for(auto h : list)
	{
		if (h->hit(r, t_min, t_max, tmp_rec))
		{
			hit_anything = true;
			if (closest_so_far > tmp_rec.t)
			{
				closest_so_far = tmp_rec.t;
				rec = tmp_rec;
			}
		}
	}
	return hit_anything;
}

