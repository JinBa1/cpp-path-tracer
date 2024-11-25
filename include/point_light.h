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
                // double shadow = 1.0; // Disable shadows for testing

                if (mat.is_microfacet) {  // BRDF for microfacet materials
                vec3 h = unit_vector(light_dir + view_dir); // Halfway vector
                double NdotL = std::max(0.0, dot(normal, light_dir));
                double NdotV = std::max(0.0, dot(normal, view_dir));
                double NdotH = std::max(0.0, dot(normal, h));
                double VdotH = std::max(0.0, dot(view_dir, h));

                // Normal Distribution Function (NDF)
                double D = GGX_D(h, normal, mat.roughness);

                // Fresnel term
                color F = fresnel_schlick(VdotH, mat.F0 + (mat.base_color - mat.F0) * mat.metalness);

                // Geometry function
                double G = smith_G(light_dir, view_dir, normal, mat.roughness);

                // Microfacet BRDF
                color numerator = D * F * G;
                double denominator = 4.0 * NdotL * NdotV + 0.001; // Avoid division by zero
                color specular = numerator / denominator;

                // Diffuse term (Lambertian), scaled by (1 - metalness) since metals have no diffuse component
                color diffuse = (1.0 - mat.metalness) * (color(1.0, 1.0, 1.0) - F) * mat.base_color / M_PI;

                // Combine contributions
                return shadow * light_intensity * NdotL * (diffuse + specular);
                }

                // FALL BACK TO BLINN-PHONG SHADING

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