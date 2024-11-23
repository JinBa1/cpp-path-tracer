#ifndef LIGHT_LIST_H
#define LIGHT_LIST_H

#include "rtweekend.h"
#include "light.h"

#include <vector>

class light_list : public light {
    public:
        std::vector<shared_ptr<light>> lights;

        light_list() {}
        light_list(shared_ptr<light> light) { add(light); }

        void clear() { lights.clear(); }

        void add(shared_ptr<light> light) {
            lights.push_back(light);
        }

        color intensity_at(const point3& point, vec3& direction) const override {
            // Combine intensity of all lights
            color total_intensity(0, 0, 0);
            for (const auto& l : lights) {
                vec3 light_dir;
                total_intensity += l->intensity_at(point, light_dir);
            }
            direction = vec3(0, 0, 0); // Not meaningful for combined light
            return total_intensity;
        }

        color compute_lighting(
            const point3& p, 
            const vec3& normal, 
            const vec3& view_dir, 
            const material& mat, 
            const hittable& world
        ) const override {

            color result(0, 0, 0);

            for (const auto& l : lights) {
                result += l->compute_lighting(p, normal, view_dir, mat, world);
            }

            return result;
        }

        double shadow_factor(const point3& p, const hittable& world) const override {
            double total_factor = 0.0;

            for (const auto& l : lights) {
                total_factor += l->shadow_factor(p, world);
            }

            // Average the shadow contributions
            return total_factor / lights.size();
        }
};


#endif // LIGHT_LIST_H