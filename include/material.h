#ifndef MATERIAL_H
#define MATERIAL_H

#include "color.h"

class material {
    public:
        float ks = 0.1; // Specular reflection coefficient
        float kd = 0.9; // Diffuse reflection coefficient
        float specularexponent = 10; // Specular exponent
        color diffusecolor = color(0.5, 0.5, 0.5); // Diffuse color
        color specularcolor = color(1, 1, 1); // Specular color

        bool is_reflective = false; // Reflective material
        float reflectivity = 1.0; // Reflectivity coefficient

        float ka = 0.1; // Ambient reflection coefficient
        color ambientcolor = color(0.5, 0.5, 0.5); // Specular color

        bool is_refractive = false; // Refractive material
        float refractiveindex = 1.0; // Refractivity coefficient

};

#endif // MATERIAL_H