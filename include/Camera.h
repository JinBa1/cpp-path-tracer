#ifndef CAMERA_H
#define CAMERA_H
#include "util/Utilities.h"

#include "object/Object.h"
#include "ImageWriter.h"
#include "light/Light.h"
#include "light/LightList.h"

class Camera {
  public:
    /* Public Camera Parameters Here */
    // double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width  = 100;  // Rendered image width in pixel count  from width
    int    image_height = 100;        // Rendered image height in pixel count from height
    point3 lookfrom = point3(0,0,0);   // from position
    point3 lookat   = point3(0,0,-1);  // from lookAt
    Vector3   vup      = Vector3(0,1,0);     // from upVector
    double vfov     = 90;              // from fov (vertical)
    Radiance background = Radiance(0,0,0);   // Background color
    Radiance ambient_light = Radiance(0.3, 0.3, 0.3); // Example ambient light

    int nbounces = 8;  // Number of bounces for ray tracing
    double exposure = 0.1;  // Exposure value for tone mapping
    std::string filename = "output.ppm";    // Output filename
    //Antialiasing
    int samples_per_pixel = 10; // Number of samples per pixel
    //Defocus blur
    double aperture = 0.02;  // Diameter of the lens aperture
    double focus_dist = 1.5;  // Distance from the camera to the focal plane
    // RENDER MODE
    RenderMode render_mode = RenderMode::PATH;

    bool allow_brdf = false;

    void print_specs() {
        std::cout << "Specifications:" << std::endl;
        std::cout << "Number of Bounces: " << nbounces << std::endl;
        std::cout << "Exposure: " << exposure << std::endl;
        std::cout << "Output Filename: " << filename << std::endl;
        std::cout << "Samples Per Pixel: " << samples_per_pixel << std::endl;
        std::cout << "Aperture: " << aperture << std::endl;
        std::cout << "Focus Distance: " << focus_dist << std::endl;
        std::cout << "Render Mode(0-binary 1-phong 2-path): " << static_cast<int>(render_mode) << std::endl;
        std::cout << "Allow BRDF: " << (allow_brdf ? "true" : "false") << std::endl;
    }


    // Tone mapping selector
    Radiance tone_map(const Vector3& hdr_color) const {
        double exp_exposure = 1.0 - exposure;
        // return linear_tone_map(hdr_color, exp_exposure);
        // return reinhard_tone_map(hdr_color, exp_exposure);
        // return filmic_tone_map(hdr_color, exp_exposure);
        return luminance_based_scaling(hdr_color, exp_exposure);
        // return hdr_color;
    }

    void render(const Object& world,  Light& lights) {
        switch (render_mode) {
            case RenderMode::BINARY:
                output_image(render_binary(world));
                break;
            case RenderMode::PHONG:
                output_image(render_phong(world, lights));
                break;
            case RenderMode::PATH:
                output_image(render_path(world, lights));
                break;
        }
    }



  private:
    /* Private Camera Variables Here */
    // int    image_height;   // Rendered image height
    double aspect_ratio;  // Ratio of image width over height
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    Vector3   pixel_delta_u;  // Offset to pixel to the right
    Vector3   pixel_delta_v;  // Offset to pixel below
    Vector3   u, v, w;              // Camera frame basis vectors

    Vector3 defocus_disk_u, defocus_disk_v; // Defocus disk basis vectors

    void initialize() ;

    // ANTI-ALIASING + DEFOUCS BLUR
    Ray get_sampled_ray(int i, int j) const ;
    // PIXEL SAMPLING
    Vector3 sample_offset() const {
        return sample_square(); // random point in the pixel
    }
    // LENS SAMPLING
    Vector3 random_point_in_aperture() const {
        Vector3 point = random_in_unit_disk() * (aperture / 2.0);
        return point.x() * defocus_disk_u + point.y() * defocus_disk_v;
    }

    Radiance trace_binary(const Ray& r, const Object& world) const;

    // HANDLE 3 CASE, USE APPROXIMATION AS LONG AS IS REFRATIVE
    Radiance trace_phong(const Ray& r, const Object& world, const Light& lights, int depth) const;

    Radiance schlick_blend(Radiance locals, Radiance reflected, Radiance refracted, double eta, double cos_theta_i, double R0) const {
        double fresnel_reflectance = R0 + (1 - R0) * pow(1 - cos_theta_i, 5);
        double fresnel_transmittance = 1 - fresnel_reflectance;
        double local_weight = std::max(0.0, 1 - fresnel_reflectance - fresnel_transmittance);
        return  local_weight * locals + fresnel_reflectance * reflected + fresnel_transmittance * refracted;
    }

    // PATH TRACING
    Radiance trace_path(const Ray& r, const Object& world, const Light& lights, int depth) const {
        if(allow_brdf){
            return trace_path_brdf(r, world, lights, depth);
        }else{
            return trace_path_phong(r, world, lights, depth);
        }
    }

    Radiance trace_path_brdf(const Ray& r, const Object& world, const Light& lights, int depth) const;
    Radiance trace_path_phong(const Ray& r, const Object& world, const Light& lights, int depth) const;




    // three rendering modes
    std::vector<Radiance> render_binary(const Object& world) ;
    std::vector<Radiance> render_phong(const Object& world, const Light& lights) ;
    std::vector<Radiance> render_path(const Object& world,  Light& lights) ;

    void output_image(const std::vector<Radiance>& pixelData) const {
        std::clog << "\rDone.                 \n";
        ImageWriter writer(pixelData, image_width, image_height, filename);
        // Write the PPM file
        if (writer.writePPM()) {
            std::cout << "Image written successfully to " << filename << std::endl;
        } else {
            std::cerr << "Failed to write image." << std::endl;
        }
    }

};



#endif