#ifndef LIGHT_H
#define LIGHT_H

#include "rtweekend.h"

class light {
    public:
     virtual ~light() = default;

     virtual color intensity_at(const point3& point, vec3& direction) const = 0;

     virtual color compute_lighting(
        const point3& p,
        const vec3& normal,
        const vec3& view_dir,
        const material& mat
     ) const = 0;
};

#endif // LIGHT_H