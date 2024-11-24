#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "rtweekend.h"
#include "light.h"

class point_light : public light {
    public:
        point3 position;
        color intensity;

        point_light(const point3& position, const color& intensity)
            : position(position), intensity(intensity) {}

        color intensity_at(const point3& point, vec3& direction) const override {
            direction = unit_vector(position - point); // Direction from point to light
            return intensity / dot(position - point, position - point); // Falloff with distance squared
        }

        color compute_lighting(
            const point3& p,
            const vec3& normal,
            const vec3& view_dir,
            const hit_record& rec,
            const hittable& world
        ) const override {
                material mat = *rec.mat_ptr;

                vec3 light_dir;
                color light_intensity = intensity_at(p, light_dir);

                // Shadow factor (0 for fully shadowed, 1 for fully lit)
                double shadow = shadow_factor(p, world);

                // Sample texture color (if any) for diffuse and ambient contributions
                color texture_color = mat.diffusecolor; // Default to diffusecolor
                if (mat.has_texture) {
                    texture_color = mat.get_texture_color(rec.u, rec.v); // Use texture color
                }

                // Diffuse contribution
                double diff = std::max(0.0, dot(normal, light_dir));
                color diffuse = shadow * mat.kd * texture_color * light_intensity * diff;

                // Specular contribution
                vec3 halfway = unit_vector(light_dir + view_dir);
                double spec = pow(std::max(0.0, dot(normal, halfway)), mat.specularexponent);
                color specular = shadow * mat.ks * mat.specularcolor * light_intensity * spec;

                return diffuse + specular;
        }

        double shadow_factor(const point3& p, const hittable& world) const override {
            vec3 light_dir = unit_vector(position - p);
            ray shadow_ray(p + 0.001 * light_dir, light_dir);

            hit_record shadow_rec;
            bool is_shadowed = world.hit(shadow_ray, interval(0.001, infinity), shadow_rec);

            return is_shadowed ? 0.2 : 1.0; // Fully shadowed or fully illuminated
        }
};

#endif // POINT_LIGHT_H