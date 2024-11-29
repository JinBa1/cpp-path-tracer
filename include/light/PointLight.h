#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "util/Utilities.h"
#include "light/Light.h"

class PointLight : public Light {
    public:
        point3 position;
        Radiance intensity;

        double constant = 1.0; // Constant attenuation
        double linear = 0.0;   // Linear attenuation
        double quadratic = 0.0; // Quadratic attenuation

        double shadow_frac = 0.1;

        PointLight(const point3& position, const Radiance& intensity)
            : position(position), intensity(intensity) {}

        Radiance intensity_at(const point3& point, Vector3& direction) const {
            direction = unit_vector(position - point); // Direction from point to light
            Radiance intensity_n = intensity / dot(position - point, position - point);
            return intensity_n; // Falloff with distance squared
        }

        Radiance phong_shading(
            const point3& p,
            const Vector3& normal,
            const Vector3& view_dir,
            const IntersectionRecord& rec,
            const Object& world,
            const Radiance& ambient_light
        ) const override {
                Material mat = *rec.mat_ptr;
                Vector3 light_dir;
                Radiance light_intensity = intensity_at(p, light_dir) + ambient_light;
                // Shadow factor (0 for fully shadowed, 1 for fully lit)
                double shadow = shadow_factor(p, world);
                // double shadow = 1.0; // Disable shadows for testing
                // FALL BACK TO BLINN-PHONG SHADING

                // Sample texture color (if any) for diffuse and ambient contributions
                Radiance texture_color = mat.diffusecolor; // Default to diffusecolor
                if (mat.has_texture) {
                    texture_color = mat.get_texture_color(rec.u, rec.v); // Use texture color
                }
                // Diffuse contribution
                double diff = std::max(0.0, dot(normal, light_dir));
                Radiance diffuse = shadow * mat.kd * texture_color * light_intensity * diff;
                // Specular contribution
                Vector3 halfway = unit_vector(light_dir + view_dir);
                double spec = pow(std::max(0.0, dot(normal, halfway)), mat.specularexponent);
                Radiance specular = shadow * mat.ks * mat.specularcolor * light_intensity * spec;
                return diffuse + specular;
        }



        double shadow_factor(const point3& p, const Object& world) const {
            // Vector3 light_dir = unit_vector(position - p);
            // Ray shadow_ray(p + 0.001 * light_dir, light_dir);

            // IntersectionRecord shadow_rec;
            // bool is_shadowed = world.intersect(shadow_ray, Interval(0.001, infinity), shadow_rec);

            // return is_shadowed ? shadow_frac : 1.0; // Fully shadowed or fully illuminated

            Vector3 light_dir = unit_vector(position - p);
            Ray shadow_ray(p + 0.001 * light_dir, light_dir);

            IntersectionRecord shadow_rec;
            double shadow = 1.0; // Fully illuminated by default
            int depth = 0;       // Initialize recursion depth

            while (world.intersect(shadow_ray, Interval(0.001, infinity), shadow_rec)) {
                Material* mat = shadow_rec.mat_ptr;

                if (mat->is_refractive) {
                    // Calculate transmittance based on material properties
                    double transmittance = 1.0 / mat->refractiveindex; // Simplified model
                    shadow *= transmittance;

                    // Adjust the ray to continue through the refractive object
                    Vector3 refract_dir = refract(shadow_ray.direction(), shadow_rec.normal, mat->refractiveindex);
                    shadow_ray = Ray(shadow_rec.p + 0.001 * refract_dir, refract_dir);
                } else {
                    // Non-refractive object fully or partially blocks the light
                    shadow *= shadow_frac; // Allow partial shadowing for opaque materials
                    break;
                }

                depth++;
                if (depth >= 8) {
                    shadow *= shadow_frac; // Treat as partially shadowed
                    break;
                }
            }

            return shadow;
        }

        Radiance brdf_shading(            
            const point3& p,
            const Vector3& normal,
            const Vector3& view_dir,
            const FancyBRDF& brdf,
            const Object& world,
            const Vector3& coefficient
            ) const override  {
                
                double shadow = shadow_factor(p, world);
                if (shadow == shadow_frac){
                    return Radiance(0, 0, 0);
                }

                Vector3 light_dir;
                Radiance light_intensity = intensity_at(p, light_dir);

                Vector3 weight = brdf.Evaluate(light_dir, view_dir);

                // ASSERT(!ContainsNan(li));
			    // ASSERT(!ContainsNan(weight));

                return intensity * weight * coefficient;

        }





};

#endif // POINT_LIGHT_H