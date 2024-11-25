#ifndef CYLINDER_H
#define CYLINDER_H

#include "rtweekend.h"
#include "hittable.h"

class cylinder : public hittable {

  public:

  mutable int hit_call_count = 0;
    cylinder(const point3& center, const vec3& axis, double radius, double height, material* mat_ptr, bool exclude_from_bvh = false)
        : center(center), axis(unit_vector(axis)), radius(std::fmax(0, radius)),
         height(std::fmax(0, height)), mat_ptr(mat_ptr), excluded(exclude_from_bvh) {}

    bool exclude_from_bvh() const override { return excluded; }

bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

    ++hit_call_count;

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

    bool excluded;

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

        if (mat_ptr->has_texture) {
            // double theta = std::atan2(outward_normal.z(), outward_normal.x());
            // rec.u = 0.5 + theta / (2 * M_PI);
            // rec.v = 0.5 + axis_proj / height; // Map height proportionally to [0, 1]
            // Compute base UV coordinates
            double theta = std::atan2(outward_normal.z(), outward_normal.x());
            double base_u = 0.5 + theta / (2 * M_PI);  // Map to [0, 1]
            double base_v = 0.5 + axis_proj / height;  // Map height proportionally to [0, 1]

            // Dynamic scaling based on texture aspect ratio
            double ideal_aspect_ratio = (2.0 * M_PI * radius) / height;
            double texture_aspect_ratio = static_cast<double>(mat_ptr->texture_width) / mat_ptr->texture_height;

            double scale_factor_u = 1.0;
            double scale_factor_v = 1.0;

            if (texture_aspect_ratio > ideal_aspect_ratio) {
                scale_factor_v = texture_aspect_ratio / ideal_aspect_ratio;
            } else if (texture_aspect_ratio < ideal_aspect_ratio) {
                scale_factor_u = ideal_aspect_ratio / texture_aspect_ratio;
            }

            // Apply scaling and tiling

            double tiled_u = fmod(base_u * mat_ptr->tile_factor_u * scale_factor_u, 1.0);
            double tiled_v = fmod(base_v * mat_ptr->tile_factor_v * scale_factor_v, 1.0);

            // Ensure UV values are in the range [0, 1]
          rec.u = tiled_u >= 0 ? tiled_u : tiled_u + 1;
          rec.v = tiled_v >= 0 ? tiled_v : tiled_v + 1;
        }

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

        // Compute UV for disk caps
        if (mat_ptr->has_texture) {
            // vec3 p_local = rec.p - disk_center; // Point relative to the disk center
            // rec.u = 0.5 + p_local.x() / (2 * radius);
            // rec.v = 0.5 + p_local.z() / (2 * radius);
            vec3 p_local = rec.p - disk_center; // Point relative to the disk center

            // Compute base UV coordinates
            double base_u = 0.5 + p_local.x() / (2 * radius);
            double base_v = 0.5 + p_local.z() / (2 * radius);



            double tiled_u = fmod(base_u * mat_ptr->tile_factor_u, 1.0);
            double tiled_v = fmod(base_v * mat_ptr->tile_factor_v, 1.0);

            // Ensure UV values are in the range [0, 1]
            rec.u = tiled_u >= 0 ? tiled_u : tiled_u + 1;
            rec.v = tiled_v >= 0 ? tiled_v : tiled_v + 1;
        }

        return true;
    }

aabb bounding_box() const override {
    // Half-axis vector scaled to half the cylinder height
    vec3 half_axis = (height / 2) * axis;

    // Calculate the top and bottom centers of the cylinder
    point3 top_center = center + half_axis;
    point3 bottom_center = center - half_axis;

    // A radius vector orthogonal to the cylinder's axis
    vec3 radius_vector = radius * vec3(1, 1, 1); // Uniform scaling in all directions

    // Adjust bounding box to cover the maximum extents
    // The bounding box must encompass the radius at all points
    point3 min_point = point3(
        std::fmin(top_center.x() - radius, bottom_center.x() - radius),
        std::fmin(top_center.y() - radius, bottom_center.y() - radius),
        std::fmin(top_center.z() - radius, bottom_center.z() - radius)
    );

    point3 max_point = point3(
        std::fmax(top_center.x() + radius, bottom_center.x() + radius),
        std::fmax(top_center.y() + radius, bottom_center.y() + radius),
        std::fmax(top_center.z() + radius, bottom_center.z() + radius)
    );

    return aabb(min_point, max_point);
}

};

#endif