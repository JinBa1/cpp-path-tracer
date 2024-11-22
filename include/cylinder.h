#ifndef CYLINDER_H
#define CYLINDER_H

#include "rtweekend.h"
#include "hittable.h"

class cylinder : public hittable {
  public:
    point3 center;  // Center of the cylinder (midpoint along the height)
    vec3 axis;      // Unit vector along the cylinder's axis
    double radius;  // Radius of the cylinder
    double height;  // Total height of the cylinder

    cylinder(const point3& center, const vec3& axis, double radius, double height)
        : center(center), axis(unit_vector(axis)), radius(std::fmax(0, radius)), height(std::fmax(0, height)) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        bool hit_side = hit_cylinder_side(r, ray_t, rec);
        bool hit_caps = hit_cylinder_caps(r, ray_t, rec);
        return hit_side || hit_caps;
    }

  private:
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
        return true;
    }

    // Check intersection with the cylinder's top and bottom caps
    bool hit_cylinder_caps(const ray& r, interval ray_t, hit_record& rec) const {
        double half_height = height;
        point3 top_center = center + half_height * axis;
        point3 bottom_center = center - half_height * axis;

        // Check intersection with the top cap
        if (hit_disk(r, top_center, ray_t, rec))
            return true;

        // Check intersection with the bottom cap
        return hit_disk(r, bottom_center, ray_t, rec);
    }

    // Check intersection with a disk (cap)
    bool hit_disk(const ray& r, const point3& disk_center, interval ray_t, hit_record& rec) const {
        vec3 normal = axis;  // Normal to the disk
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
        return true;
    }
};

#endif