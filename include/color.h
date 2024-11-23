#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <sstream>

using color = vec3;

// Linear Tone Mapping
inline color linear_tone_map(const color& hdr_color, double exposure) {
    color mapped_color = exposure * hdr_color;
    return color(
        std::min(1.0, mapped_color.x()),
        std::min(1.0, mapped_color.y()),
        std::min(1.0, mapped_color.z())
    );
}

// Reinhard Tone Mapping
inline vec3 reinhard_tone_map(const vec3& hdr_color, double exposure) {
    vec3 mapped_color = exposure * hdr_color;
    return vec3(
        mapped_color.x() / (1.0 + mapped_color.x()),
        mapped_color.y() / (1.0 + mapped_color.y()),
        mapped_color.z() / (1.0 + mapped_color.z())
    );

}

// Filmic Tone Mapping (ACES Approximation)
inline vec3 filmic_tone_map(const vec3& hdr_color, double exposure) {
    vec3 mapped_color = exposure * hdr_color;

    const double a = 2.51;
    const double b = 0.03;
    const double c = 2.43;
    const double d = 0.59;
    const double e = 0.14;

    auto filmic_curve = [&](double x) {
        return (x * (a * x + b)) / (x * (c * x + d) + e);
    };

    return vec3(
        std::min(1.0, filmic_curve(mapped_color.x())),
        std::min(1.0, filmic_curve(mapped_color.y())),
        std::min(1.0, filmic_curve(mapped_color.z()))
    );
}

inline color luminance_based_scaling(const color& hdr_color, double exposure) {
    // Calculate luminance
    double luminance = 0.2126 * hdr_color.x() + 0.7152 * hdr_color.y() + 0.0722 * hdr_color.z();

    // Scale luminance by exposure
    double scaled_luminance = exposure * luminance;

    // Avoid division by zero for black pixels
    if (luminance == 0.0) {
        return vec3(0.0, 0.0, 0.0); // No scaling needed for black
    }

    // Scale RGB values proportionally
    color scaled_color = (scaled_luminance / luminance) * hdr_color;

    // Clamp to [0, 1] to ensure valid color range
    return color(
        std::min(1.0, std::max(0.0, scaled_color.x())),
        std::min(1.0, std::max(0.0, scaled_color.y())),
        std::min(1.0, std::max(0.0, scaled_color.z()))
    );
}

inline std::string get_color_string(const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Translate the [0,1] component values to the byte range [0,255].
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    // Create a string stream to format the color components.
    std::ostringstream oss;
    oss << rbyte << ' ' << gbyte << ' ' << bbyte;
    return oss.str();
}

#endif