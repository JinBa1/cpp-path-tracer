#ifndef MATERIAL_H
#define MATERIAL_H

#include "color.h"
#include <vector>
#include <fstream>

// Define the custom clamp function here
template <typename T>
constexpr T clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : (value > max ? max : value);
}

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

        // TEXTURE MAPPING
        bool has_texture = false ; // flag for texture mapping
        // std::vector<color> texture_data; // texture pixel data
        std::vector<uint8_t> texture_data; // Compact texture data (R, G, B as uint8_t)
        int texture_width = 0; // texture width
        int texture_height = 0; // texture height
        float tile_factor_u = 1.0; // tiling factor for U coordinate
        float tile_factor_v = 1.0; // tiling factor for V coordinate

        // MICROFACET BRDF
        bool is_microfacet = false;
        float roughness = 0.5; // Roughness for NDF
        float metalness = 0.0; // 0 for dielectric, 1 for metal
        color base_color = color(0.8, 0.8, 0.8); // Base color for non-metals
        color F0 = color(0.04, 0.04, 0.04); // Fresnel reflectance at normal incidence



        color get_texture_color(double u, double v) const {
            if (!has_texture) return diffusecolor; // Return default color if no texture

            // Map (u, v) to texture pixel indices
            int i = clamp(static_cast<int>(u * texture_width), 0, texture_width - 1);
            int j = clamp(static_cast<int>(v * texture_height), 0, texture_height - 1);

            // Calculate the index in the 1D texture data array
            int index = (j * texture_width + i) * 3; // 3 channels (R, G, B)

            // Convert from uint8_t to float and return the color
            float r = texture_data[index] / 255.0f;
            float g = texture_data[index + 1] / 255.0f;
            float b = texture_data[index + 2] / 255.0f;

            return color(r, g, b);
        }


        void load_texture(const std::string& filename) {
            std::ifstream file(filename);
            if (!file) {
                std::cerr << "Failed to open texture file: " << filename << std::endl;
                return;
            }

            // Read PPM header
            std::string header;
            file >> header;
            if (header != "P3") {
                std::cerr << "Unsupported PPM format (only P3 supported): " << header << std::endl;
                return;
            }

            // Read texture dimensions and max value
            int max_val;
            file >> texture_width >> texture_height >> max_val;

            if (texture_width <= 0 || texture_height <= 0 || max_val != 255) {
                std::cerr << "Invalid PPM header in file: " << filename << std::endl;
                return;
            }

            // Resize the texture data array
            texture_data.resize(texture_width * texture_height * 3);

            // Read pixel data
            for (int i = 0; i < texture_data.size(); i++) {
                int pixel_value;
                file >> pixel_value;
                texture_data[i] = static_cast<uint8_t>(pixel_value);
            }

            has_texture = true; // Mark texture as loaded
            std::cout << "Texture loaded: " << filename 
                      << " (" << texture_width << "x" << texture_height << ")" << std::endl;
        }

        void print_material() const {
            std::cout << "Material:" << std::endl;
            std::cout << "  Diffuse Color: [" << diffusecolor.x() << ", " << diffusecolor.y() << ", " << diffusecolor.z() << "]" << std::endl;
            std::cout << "  Specular Color: [" << specularcolor.x() << ", " << specularcolor.y() << ", " << specularcolor.z() << "]" << std::endl;
            std::cout << "  Reflectivity: " << reflectivity << std::endl;
            std::cout << "  Refractive Index: " << refractiveindex << std::endl;
            std::cout << "  Is Reflective: " << is_reflective << std::endl;
            std::cout << "  Is Refractive: " << is_refractive << std::endl;
            std::cout << "  Is Microfacet: " << is_microfacet << std::endl;
            if (is_microfacet) {
                std::cout << "    Roughness: " << roughness << std::endl;
                std::cout << "    Metalness: " << metalness << std::endl;
                std::cout << "    F0: [" << F0.x() << ", " << F0.y() << ", " << F0.z() << "]" << std::endl;
                std::cout << "    Base Color: [" << base_color.x() << ", " << base_color.y() << ", " << base_color.z() << "]" << std::endl;
            }
            if (has_texture) {
                std::cout << "  Has Texture: Yes (Size: " << texture_width << "x" << texture_height << ")" << std::endl;
            } else {
                std::cout << "  Has Texture: No" << std::endl;
            }
        }        

};

inline double GGX_D(const vec3& h, const vec3& n, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH = std::max(dot(n, h), 0.0);
    float denom = (NdotH * NdotH * (alpha2 - 1) + 1);
    return alpha2 / (M_PI * denom * denom);
}

inline color fresnel_schlick(double cosTheta, const color& F0) {
    return F0 + (color(1.0, 1.0, 1.0) - F0) * pow(1.0 - cosTheta, 5);
}

inline double smith_G1(const vec3& v, const vec3& n, float roughness) {
    float NdotV = std::max(dot(n, v), 0.0);
    float k = (roughness + 1) * (roughness + 1) / 8.0;
    return NdotV / (NdotV * (1 - k) + k);
}

inline double smith_G(const vec3& wi, const vec3& wo, const vec3& n, float roughness) {
    return smith_G1(wi, n, roughness) * smith_G1(wo, n, roughness);
}

#endif // MATERIAL_H