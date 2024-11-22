#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"
#include "hittable.h"

class triangle : public hittable {
  public:
    triangle(const point3& v0, const point3& v1, const point3& v2)
        : v0(v0), v1(v1), v2(v2) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Using the Möller–Trumbore algorithm for ray-triangle intersection
        const vec3 edge1 = v1 - v0;
        const vec3 edge2 = v2 - v0;
        const vec3 h = cross(r.direction(), edge2);
        const double a = dot(edge1, h);

        // If a is close to 0, the ray is parallel to the triangle
        constexpr double epsilon = 1e-8;
        if (std::fabs(a) < epsilon)
            return false;

        const double f = 1.0 / a;
        const vec3 s = r.origin() - v0;
        const double u = f * dot(s, h);

        // Check if the intersection lies within the triangle
        if (u < 0.0 || u > 1.0)
            return false;

        const vec3 q = cross(s, edge1);
        const double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0)
            return false;

        // Compute t to find where the intersection occurs on the ray
        const double t = f * dot(edge2, q);

        if (!ray_t.surrounds(t))
            return false;

        // Fill hit record
        rec.t = t;
        rec.p = r.at(t);
        vec3 outward_normal = unit_vector(cross(edge1, edge2)); // Normalize the normal
        rec.set_face_normal(r, outward_normal);

        return true;
    }

  private:
    point3 v0, v1, v2; // Triangle vertices
};

#endif