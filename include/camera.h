#ifndef CAMERA_H
#define CAMERA_H
#include "rtweekend.h"

#include "hittable.h"
#include "ImageWriter.h"
#include "light.h"

class camera {
  public:
    /* Public Camera Parameters Here */
    // double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width  = 100;  // Rendered image width in pixel count  from width
    int    image_height = 100;        // Rendered image height in pixel count from height
    point3 lookfrom = point3(0,0,0);   // from position
    point3 lookat   = point3(0,0,-1);  // from lookAt
    vec3   vup      = vec3(0,1,0);     // from upVector
    double vfov     = 90;              // from fov (vertical)

    color background = color(0,0,0);   // Background color

    //Actually use default values
    color ambient_light = color(0.1, 0.1, 0.1); // Example ambient light

    int nbounces = 8;  // Number of bounces for ray tracing

    double exposure = 1.0;  // Exposure value for tone mapping

    std::string filename = "output.ppm";    // Output filename

    //Antialiasing
    int samples_per_pixel = 10; // Number of samples per pixel

    //Defocus blur
    double aperture = 0.1;  // Diameter of the lens aperture
    double focus_dist = 10.0;  // Distance from the camera to the focal plane


    // Tone mapping selector
    color tone_map(const vec3& hdr_color) const {
        double exp_exposure = 1.0 + exposure;
        return linear_tone_map(hdr_color, exp_exposure);
        // return reinhard_tone_map(hdr_color, exp_exposure);
        // return filmic_tone_map(hdr_color, exp_exposure);
        // return luminance_based_scaling(hdr_color, exp_exposure);
        // return hdr_color;
    }

    void render(const hittable& world, const light& lights) {
        initialize();

        std::vector<color> pixelData(image_width * image_height);

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                // auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                // auto ray_direction = pixel_center - center;
                // ray r(center, ray_direction);

                // // Pass `nbounces` to control recursive depth
                // color pixel_color = ray_color_phong(r, world, lights, nbounces);
                // pixelData[j * image_width + i] = tone_map(pixel_color);
                color pixel_color(0, 0, 0);

                // Multi-sampling loop
                for (int sample = 0; sample < samples_per_pixel; ++sample) {
                    ray r = get_sampled_ray(i, j);  // Generate a ray with random sampling
                    pixel_color += ray_color_phong(r, world, lights, nbounces);
                }

                // Average the color of the sampled rays
                pixel_color *= (1.0 / samples_per_pixel);

                // Apply tone mapping
                pixelData[j * image_width + i] = tone_map(pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";

        std::string outputDir = "/home/jin/cgr/rt/output/";
        ImageWriter writer(pixelData, image_width, image_height, outputDir + filename);

        // Write the PPM file
        if (writer.writePPM()) {
            std::cout << "Image written successfully to " << filename << std::endl;
        } else {
            std::cerr << "Failed to write image." << std::endl;
        }
    }



  private:
    /* Private Camera Variables Here */
    // int    image_height;   // Rendered image height
    double aspect_ratio;  // Ratio of image width over height
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors

    vec3 defocus_disk_u, defocus_disk_v; // Defocus disk basis vectors


    void initialize() {
        aspect_ratio = double(image_width) / double(image_height);
        center = lookfrom;

        // Determine viewport dimensions.
        auto focal_length = (lookfrom - lookat).length();
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focal_length;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        std::cout << "image_width: " << image_width  << "\n" 
                  << "image_height: " << image_height << "\n"
                  << "aspect_ratio: " << aspect_ratio << "\n"
                  << "viewport_width: " << viewport_width  << "\n" 
                  << "viewport_height: " << viewport_height <<std::endl;

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        defocus_disk_u = u;
        defocus_disk_v = v;

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    // ANTI-ALIASING
    ray get_sampled_ray(int i, int j) const {
        // // Generate a random offset within the [-0.5, +0.5] range
        // vec3 offset = sample_square();

        // // Compute the sampled pixel position
        // vec3 pixel_sample = pixel00_loc
        //                 + ((i + offset.x()) * pixel_delta_u)
        //                 + ((j + offset.y()) * pixel_delta_v);

        // vec3 ray_origin = center;
        // vec3 ray_direction = pixel_sample - ray_origin;

        // return ray(ray_origin, ray_direction);
        vec3 offset = sample_square();
        vec3 lens_sample = random_point_in_aperture();

        vec3 offset_origin = center + lens_sample;

        vec3 pixel_sample = pixel00_loc
                        + ((i + offset.x()) * pixel_delta_u)
                        + ((j + offset.y()) * pixel_delta_v);

        vec3 focal_target = center + focus_dist * unit_vector(pixel_sample - center);
        vec3 ray_direction = focal_target - offset_origin;

        return ray(offset_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns a random point in the [-0.5, -0.5] to [+0.5, +0.5] range
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    } // END ANTI-ALIASING

vec3 random_point_in_aperture() const {
    vec3 point = random_in_unit_disk() * (aperture / 2.0);
    return point.x() * defocus_disk_u + point.y() * defocus_disk_v;
}

vec3 random_in_unit_disk() const {
    while (true) {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() < 1) return p;
    }
}

    // HANDLE 3 CASE, USE APPROXIMATION AS LONG AS IS REFRATIVE
    color ray_color_phong(const ray& r, const hittable& world, const light& lights, int depth) const {
        hit_record rec;
        color result;

        // If the ray hits nothing, return the background color.
        if (!world.hit(r, interval(0.001, infinity), rec)) {
            return background;
        }

        // Compute Blinn-Phong shading at the intersection point
        vec3 view_dir = unit_vector(-r.direction());
        color local_shading = lights.compute_lighting(rec.p, rec.normal, view_dir, rec, world);

        // Add ambient light contribution
        color ambient = rec.mat_ptr->ka * rec.mat_ptr->ambientcolor * ambient_light;

        // Stop recursion if max depth is reached
        if (depth <= 0) {
            return ambient + local_shading;
        }

        // Initialize reflection and refraction contributions
        color reflected_color(0, 0, 0);
        color refracted_color(0, 0, 0);

        // Reflective only case
        if (rec.mat_ptr->is_reflective && !rec.mat_ptr->is_refractive) {
            vec3 reflect_dir = unit_vector(r.direction() - 2 * dot(r.direction(), rec.normal) * rec.normal);
            ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
            reflected_color = ray_color_phong(reflect_ray, world, lights, depth - 1);
            result = ambient + (1 - rec.mat_ptr->reflectivity) * local_shading +
                    rec.mat_ptr->reflectivity * reflected_color;
            // result = ambient +  local_shading + reflected_color;
            return result;
        }

        // Combined refractive and reflective case
        if (rec.mat_ptr->is_refractive) {
            double eta = rec.front_face ? (1.0 / rec.mat_ptr->refractiveindex) : rec.mat_ptr->refractiveindex;

            // Calculate cos(theta) for the incident ray
            double cos_theta_i = std::fabs(dot(-unit_vector(r.direction()), rec.normal));
            double sin_theta_t = eta * std::sqrt(1 - cos_theta_i * cos_theta_i);

            if (sin_theta_t > 1.0) {
                // Total Internal Reflection: Treat as reflective
                vec3 reflect_dir = unit_vector(r.direction() - 2 * dot(r.direction(), rec.normal) * rec.normal);
                ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
                refracted_color = ray_color_phong(reflect_ray, world, lights, depth - 1);
            } else {
                // Compute refraction direction using Snell's law
                vec3 refract_dir = refract(unit_vector(r.direction()), rec.normal, eta);
                ray refract_ray(rec.p + 0.001 * refract_dir, refract_dir);
                refracted_color = ray_color_phong(refract_ray, world, lights, depth - 1);
            }

            // Use Schlick approximation for Fresnel effect
            double R0 = pow((1.0 - rec.mat_ptr->refractiveindex) / (1.0 + rec.mat_ptr->refractiveindex), 2);
            double fresnel_reflectance = R0 + (1 - R0) * pow(1 - cos_theta_i, 5);
            double fresnel_transmittance = 1 - fresnel_reflectance;

            // Reflective contribution
            if (rec.mat_ptr->is_reflective) {
                vec3 reflect_dir = unit_vector(r.direction() - 2 * dot(r.direction(), rec.normal) * rec.normal);
                ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
                reflected_color = ray_color_phong(reflect_ray, world, lights, depth - 1);
            }

            // Combine contributions
            double local_weight = std::max(0.0, 1 - fresnel_reflectance - fresnel_transmittance);
            result = local_weight * (ambient + local_shading) +
                    fresnel_reflectance * reflected_color +
                    fresnel_transmittance * refracted_color;
            // result = ambient + local_shading + reflected_color + refracted_color;

            return result;
        }

        // Local shading only case
        result = ambient + local_shading;
        return result;
    }




};

#endif