#ifndef CYLINDER_H
#define CYLINDER_H

#include "rtweekend.h"
#include "hittable.h"

class cylinder : public hittable {
  public:
    cylinder(const point3& center, const vec3& axis, double radius, double height, material* mat_ptr)
        : center(center), axis(unit_vector(axis)), radius(std::fmax(0, radius)), height(std::fmax(0, height)), mat_ptr(mat_ptr) {}

bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_t.max;

    // Check intersection with the side surface
    if (hit_cylinder_side(r, interval(ray_t.min, closest_so_far), temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec; // Update the hit_record with the closest hit
    }

    // Check intersection with the top and bottom caps
    if (hit_cylinder_caps(r, interval(ray_t.min, closest_so_far), temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec; // Update the hit_record with the closest hit
    }

    return hit_anything;
}

  private:

    point3 center;  // Center of the cylinder (midpoint along the height)
    vec3 axis;      // Unit vector along the cylinder's axis
    double radius;  // Radius of the cylinder
    double height;  // Total height of the cylinder
    material* mat_ptr;

    // Check intersection with the cylinder's side surface
    bool hit_cylinder_side(const ray& r, interval ray_t, hit_record& rec) const {
        vec3 oc = r.origin() - center;
        vec3 d_proj = r.direction() - dot(r.direction(), axis) * axis;  // Direction perpendicular to axis
        vec3 oc_proj = oc - dot(oc, axis) * axis;                      // Origin offset perpendicular to axis

        double a = d_proj.length_squared();
        double half_b = dot(d_proj, oc_proj);
        double c = oc_proj.length_squared() - radius * radius;

        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false;

        // Find the nearest root that lies in the acceptable range
        double sqrtd = std::sqrt(discriminant);
        double root = (-half_b - sqrtd) / a;

        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        // Compute the intersection point and project onto the cylinder's axis
        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 to_p = rec.p - center;
        double axis_proj = dot(to_p, axis);  // Projection of the hit point along the axis

        // Check if the intersection is within the height of the cylinder
        double half_height = height;
        if (axis_proj < -half_height || axis_proj > half_height)
            return false;

        // Compute the normal at the intersection point
        vec3 outward_normal = to_p - axis_proj * axis;  // Subtract the projection on the axis
        outward_normal = unit_vector(outward_normal);

        rec.set_face_normal(r, outward_normal);

        // Update the material in the hit record
        rec.mat_ptr = mat_ptr;

        return true;
    }

    // Check intersection with the cylinder's top and bottom caps
    bool hit_cylinder_caps(const ray& r, interval ray_t, hit_record& rec) const {
        double half_height = height;
        point3 top_center = center + half_height * axis;
        point3 bottom_center = center - half_height * axis;

        // Check intersection with the top cap
        if (hit_disk(r, top_center, ray_t, rec, true))
            return true;

        // Check intersection with the bottom cap
        return hit_disk(r, bottom_center, ray_t, rec, false);
    }

    // Check intersection with a disk (cap)
    bool hit_disk(const ray& r, const point3& disk_center, interval ray_t, hit_record& rec, bool is_top) const {
        vec3 normal = is_top ? axis : -axis; // Use inverted normal for the bottom cap
        double t = dot(disk_center - r.origin(), normal) / dot(r.direction(), normal);

        if (!ray_t.surrounds(t))
            return false;

        point3 p = r.at(t);  // Intersection point
        if ((p - disk_center).length_squared() > radius * radius)
            return false;

        // Update hit record
        rec.t = t;
        rec.p = p;
        rec.set_face_normal(r, normal);

        // Assign the material pointer
        rec.mat_ptr = mat_ptr;

        return true;
    }
};

#endif