#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"
#include "hittable.h"

class triangle : public hittable {
  public:

mutable int hit_call_count = 0;

    triangle(const point3& v0, const point3& v1, const point3& v2, material* mat_ptr, bool exclude_from_bvh = false)
        : v0(v0), v1(v1), v2(v2), mat_ptr(mat_ptr),
        uv0(vec3(0,0,0)), uv1(1,0,0), uv2(0,1,0), excluded(exclude_from_bvh) {

        }

    triangle(const point3& v0, const point3& v1, const point3& v2, material* mat_ptr,
             const vec3& uv0, const vec3& uv1, const vec3& uv2, bool exclude_from_bvh = false)
        : v0(v0), v1(v1), v2(v2), mat_ptr(mat_ptr),
        uv0(uv0), uv1(uv1), uv2(uv2), excluded(exclude_from_bvh) {}

    bool exclude_from_bvh() const override { return excluded; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        ++hit_call_count;

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

        // Set the material pointer in hit_record
        rec.mat_ptr = mat_ptr;

        if (mat_ptr->has_texture) {
            // Interpolate UV coordinates using barycentric coordinates
            rec.u = (1 - u - v) * uv0.x() + u * uv1.x() + v * uv2.x();
            rec.v = (1 - u - v) * uv0.y() + u * uv1.y() + v * uv2.y();
        }

        return true;
    }

    aabb bounding_box() const override {
        point3 min = point3(
            std::min({v0.x(), v1.x(), v2.x()}),
            std::min({v0.y(), v1.y(), v2.y()}),
            std::min({v0.z(), v1.z(), v2.z()})
        );
        point3 max = point3(
            std::max({v0.x(), v1.x(), v2.x()}),
            std::max({v0.y(), v1.y(), v2.y()}),
            std::max({v0.z(), v1.z(), v2.z()})
        );
        return aabb(min, max);
    }

  private:
    point3 v0, v1, v2; // Triangle vertices
    material* mat_ptr;
    vec3 uv0, uv1, uv2; // UV coordinates for texture mapping, ignore z componenet

    bool excluded;
};

#endif