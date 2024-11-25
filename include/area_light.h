#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#include "light.h"

class area_light : public light {
public:
    point3 position;  // Center of the area light
    vec3 u, v;        // Basis vectors defining the area light plane
    double width, height;  // Dimensions of the area light
    color intensity;  // Light intensity

    int num_samples;  // Number of samples for soft shadows

    area_light(
        const point3& position,
        const vec3& u,
        const vec3& v,
        double width,
        double height,
        const color& intensity,
        int num_samples = 16
    ) : position(position), u(u), v(v), width(width), height(height), intensity(intensity), num_samples(num_samples) {}

    color intensity_at(const point3& point, vec3& direction) const override {
        direction = unit_vector(position - point);  // Approximation for now
        return intensity / dot(position - point, position - point);
    }

color compute_lighting(
    const point3& p,
    const vec3& normal,
    const vec3& view_dir,
    const hit_record& rec,
    const hittable& world
) const override {
    material mat = *rec.mat_ptr;
    color total_result(0, 0, 0); // Accumulate results from all samples

    for (int i = 0; i < num_samples; ++i) {
        // Sample a point on the area light's surface
        point3 sample_point = random_sample_on_surface();
        vec3 light_dir = unit_vector(sample_point - p);

        // Shadow factor (0 for fully shadowed, 1 for fully lit)
        double shadow = shadow_factor_from_sample(p, sample_point, world);
        if (shadow == 0.0) continue; // Skip if fully shadowed

        // Light intensity from the sampled point
        color light_intensity = intensity / dot(sample_point - p, sample_point - p); // Falloff with distance^2

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

            // Combine contributions and accumulate
            total_result += shadow * light_intensity * NdotL * (diffuse + specular);
            continue;
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

        // Combine diffuse and specular and accumulate
        total_result += diffuse + specular;
    }

    // Average the accumulated contributions from all samples
    return total_result / num_samples;
}

    double shadow_factor(const point3& p, const hittable& world) const override {
        int unblocked = 0;

        for (int i = 0; i < num_samples; ++i) {
            point3 sample_point = random_sample_on_surface();
            vec3 light_dir = unit_vector(sample_point - p);
            ray shadow_ray(p + 0.001 * light_dir, light_dir);

            hit_record shadow_rec;
            if (!world.hit(shadow_ray, interval(0.001, infinity), shadow_rec)) {
                unblocked++;
            }
        }

        return double(unblocked) / num_samples;
    }

private:
    point3 random_sample_on_surface() const {
        // Uniform sampling on a rectangle
        double s = random_double(-0.5, 0.5);
        double t = random_double(-0.5, 0.5);
        return position + (s * width * u) + (t * height * v);
    }

    double shadow_factor_from_sample(const point3& p, const point3& sample_point, const hittable& world) const {
        vec3 light_dir = unit_vector(sample_point - p);
        ray shadow_ray(p + 0.001 * light_dir, light_dir);

        hit_record shadow_rec;
        bool is_shadowed = world.hit(shadow_ray, interval(0.001, infinity), shadow_rec);

        return is_shadowed ? 0.0 : 1.0;  // Fully blocked or fully illuminated
    }
};

#endif // AREA_LIGHT_H