#ifndef LIGHT_H
#define LIGHT_H

#include "rtweekend.h"
#include "hittable.h"

class light {
    public:
     virtual ~light() = default;

     virtual color intensity_at(const point3& point, vec3& direction) const = 0;

     virtual color compute_lighting(
        const point3& p,
        const vec3& normal,
        const vec3& view_dir,
        const hit_record& rec,
        const hittable& world
     ) const = 0;

    // New method to determine shadow contribution
    virtual double shadow_factor(const point3& p, const hittable& world) const = 0;
};



#endif // LIGHT_H