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
            const material& mat
        ) const override {
            vec3 light_dir;
            color light_intensity = intensity_at(p, light_dir);

            // Diffuse contribution
            double diff = std::max(0.0, dot(normal, light_dir));
            color diffuse = mat.kd * mat.diffusecolor * light_intensity * diff;

            // Specular contribution
            vec3 halfway = unit_vector(light_dir + view_dir);
            double spec = pow(std::max(0.0, dot(normal, halfway)), mat.specularexponent);
            color specular = mat.ks * mat.specularcolor * light_intensity * spec;

            return diffuse + specular;
        }
};

#endif // POINT_LIGHT_H