#ifndef SPHERE_H
#define SPHERE_H

#include "rtweekend.h"
#include "hittable.h"

class sphere : public hittable {
  public:
    sphere(const point3& center, double radius, material* mat_ptr)
     : center(center), radius(std::fmax(0,radius)), mat_ptr(mat_ptr) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);

        // Set the matreial pointer in hit_record
        rec.mat_ptr = mat_ptr;

        // Compute UV coordinates for texture mapping
        if (mat_ptr->has_texture) {
          // rec.u = 0.5 + std::atan2(outward_normal.z(), outward_normal.x()) / (2 * M_PI);
          // rec.v = 1.0 - std::asin(outward_normal.y()) / M_PI; // could clamp to [0,1], floating point errors
            // Calculate base UV coordinates (equirectangular mapping)
          double base_u = 0.5 + std::atan2(outward_normal.z(), outward_normal.x()) / (2 * M_PI);
          double base_v = 1.0 - std::asin(outward_normal.y()) / M_PI;

          // Adjust for texture scaling and tiling
          double scale_factor_u = 1.0;
          double scale_factor_v = 1.0;

          if (mat_ptr->texture_width > 0 && mat_ptr->texture_height > 0) {
              // Compute aspect ratio scaling (if needed)
              double texture_aspect_ratio = double(mat_ptr->texture_width) / mat_ptr->texture_height;
              double sphere_aspect_ratio = 2.0; // Ideal aspect ratio for a sphere's UV map

              scale_factor_u = sphere_aspect_ratio / texture_aspect_ratio;
          }

          // Apply tiling and scaling
          double tiled_u = fmod(base_u * mat_ptr->tile_factor_u * scale_factor_u, 1.0);
          double tiled_v = fmod(base_v * mat_ptr->tile_factor_v * scale_factor_v, 1.0);

          // Ensure UVs remain in the [0, 1] range
          rec.u = tiled_u >= 0 ? tiled_u : tiled_u + 1;
          rec.v = tiled_v >= 0 ? tiled_v : tiled_v + 1;
        }
        return true;
    }

  private:
    point3 center;
    double radius;
    material* mat_ptr;
};

#endif