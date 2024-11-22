#ifndef CAMERA_H
#define CAMERA_H
#include "rtweekend.h"

#include "hittable.h"
#include "ImageWriter.h"

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

    std::string filename = "output.ppm";    // Output filename

    void render(const hittable& world) {
        initialize();

        std::vector<color> pixelData(image_width * image_height);

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                auto ray_direction = pixel_center - center;
                ray r(center, ray_direction);

                color pixel_color = ray_color(r, world);
                pixelData[j * image_width + i] = pixel_color;
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


    void initialize() {
        aspect_ratio = double(image_width) / double(image_height);
        // image_height = int(image_width / aspect_ratio);
        // image_height = (image_height < 1) ? 1 : image_height;

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

    color ray_color(const ray& r, const hittable& world) const {
        hit_record rec;

        if (world.hit(r, interval(0, infinity), rec)) {
            // return 0.5 * (rec.normal + color(1,1,1));
            return color(1,0,0);
        }

        // vec3 unit_direction = unit_vector(r.direction());
        // auto a = 0.5*(unit_direction.y() + 1.0);
        // return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        return background;
    }
};

#endif