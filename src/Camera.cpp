#include "Camera.h"

#include "brdf/FancyBRDF.h"

void Camera::initialize() {
    aspect_ratio = double(image_width) / double(image_height);
    center = lookfrom;

    // Determine viewport dimensions.
    auto focal_length = (lookfrom - lookat).length();
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta/2);
    auto viewport_height = 2 * h * focal_length;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    // std::cout << "image_width: " << image_width  << "\n" 
    //             << "image_height: " << image_height << "\n"
    //             << "aspect_ratio: " << aspect_ratio << "\n"
    //             << "viewport_width: " << viewport_width  << "\n" 
    //             << "viewport_height: " << viewport_height <<std::endl;

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    defocus_disk_u = u;
    defocus_disk_v = v;

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    Vector3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
    Vector3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
}


// ANTI-ALIASING
Ray Camera::get_sampled_ray(int i, int j) const {
    // // Generate a random offset within the [-0.5, +0.5] range
    // Vector3 offset = sample_square();

    // // Compute the sampled pixel position
    // Vector3 pixel_sample = pixel00_loc
    //                 + ((i + offset.x()) * pixel_delta_u)
    //                 + ((j + offset.y()) * pixel_delta_v);

    // Vector3 ray_origin = center;
    // Vector3 ray_direction = pixel_sample - ray_origin;

    // return ray(ray_origin, ray_direction);
    Vector3 offset = sample_offset();
    Vector3 lens_sample = random_point_in_aperture();

    Vector3 offset_origin = center + lens_sample;

    Vector3 pixel_sample = pixel00_loc
                    + ((i + offset.x()) * pixel_delta_u)
                    + ((j + offset.y()) * pixel_delta_v);

    Vector3 focal_target = center + focus_dist * unit_vector(pixel_sample - center);
    Vector3 ray_direction = focal_target - offset_origin;

    return Ray(offset_origin, ray_direction);
}


Radiance Camera::trace_binary(const Ray& r, const Object& world) const {
    IntersectionRecord rec;
    if (world.intersect(r, Interval(0, infinity), rec)) {
        return Radiance(1,0,0);
    }
    return Radiance(0,0,0);
}


// HANDLE 3 CASE, USE APPROXIMATION AS LONG AS IS REFRATIVE
Radiance Camera::trace_phong(const Ray& r, const Object& world, const Light& lights, int depth) const {
    IntersectionRecord rec;
    Radiance result;

    // If the ray hits nothing, return the background color.
    if (!world.intersect(r, Interval(0.001, infinity), rec)) {
        return background;
    }

    // return color(1, 0, 0);

    // Compute Blinn-Phong shading at the intersection point
    Vector3 view_dir = unit_vector(-r.direction());
    Radiance local_shading = lights.phong_shading(rec.p, rec.normal, view_dir, rec, world, ambient_light);

    Radiance ambient = calculate_ambient(*rec.mat_ptr, ambient_light);

    // Stop recursion if max depth is reached
    if (depth <= 0) {
        return ambient + local_shading;
    }

    // Initialize reflection and refraction contributions
    Radiance reflected_color(0, 0, 0);
    Radiance refracted_color(0, 0, 0);

    // Reflective only case
    if (rec.mat_ptr->is_reflective && !rec.mat_ptr->is_refractive) {
        Vector3 reflect_dir = reflect_pm(r.direction(), rec.normal);
        Ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
        reflected_color = trace_phong(reflect_ray, world, lights, depth - 1);
        result = (1 - rec.mat_ptr->reflectivity) * local_shading +
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
            Vector3 reflect_dir = reflect_pm(r.direction(), rec.normal);
            Ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
            refracted_color = trace_phong(reflect_ray, world, lights, depth - 1);
        } else {
            // Compute refraction direction using Snell's law
            Vector3 refract_dir = refract(unit_vector(r.direction()), rec.normal, eta);
            Ray refract_ray(rec.p + 0.001 * refract_dir, refract_dir);
            refracted_color = trace_phong(refract_ray, world, lights, depth - 1);
        }

        // Reflective contribution
        if (rec.mat_ptr->is_reflective) {
            Vector3 reflect_dir = reflect_pm(r.direction(), rec.normal);
            Ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
            reflected_color = trace_phong(reflect_ray, world, lights, depth - 1);
        }

        // Combine contributions
        // Use Schlick approximation for Fresnel effect
        double R0 = pow((1.0 - rec.mat_ptr->refractiveindex) / (1.0 + rec.mat_ptr->refractiveindex), 2);
        result = schlick_blend((ambient + local_shading), reflected_color, refracted_color, eta, cos_theta_i, R0);
        // result = ambient + local_shading + reflected_color + refracted_color;
        return result;
    }

    // Local shading only case
    result = ambient + local_shading;
    return result;
}


Radiance Camera::trace_path_phong(const Ray& r, const Object& world, const Light& lights, int depth) const {
    // Base case: terminate recursion if depth is zero
    if (depth <= 0) {
        return Radiance(0, 0, 0);  // No contribution beyond max depth
    }
    IntersectionRecord rec;
    // If the ray misses, return the background color
    if (!world.intersect(r, Interval(0.001, infinity), rec)) {
        return background;
    }
    // Extract material properties
    Material* mat = rec.mat_ptr;
    // Emitted light from the surface (e.g., for light sources)
    // Radiance emitted = mat->emission;
    Radiance emitted = Radiance(0,0,0);


    // Ideal reflection and refraction
    Radiance reflection_color(0, 0, 0);
    Radiance refraction_color(0, 0, 0);
    // Reflection
    if (mat->is_reflective) {
        Vector3 reflect_dir = reflect_pm(r.direction(), rec.normal);
        Ray reflect_ray(rec.p + 0.001 * reflect_dir, reflect_dir);
        reflection_color = trace_path(reflect_ray, world, lights, depth - 1);
    }
    // Refraction
    if (mat->is_refractive) {
        double eta = rec.front_face ? (1.0 / mat->refractiveindex) : mat->refractiveindex;
        // Calculate refraction direction
        Vector3 refract_dir = refract(unit_vector(r.direction()), rec.normal, eta);
        Ray refract_ray(rec.p + 0.001 * refract_dir, refract_dir);
        // Handle total internal reflection
        if (refract_dir != Vector3(0, 0, 0)) {
            refraction_color = trace_path(refract_ray, world, lights, depth - 1);
        }
    }
    // Combine reflection and refraction using Fresnel weights
    Vector3 view_dir = unit_vector(-r.direction());
    Radiance surface_color = lights.phong_shading(rec.p, rec.normal, view_dir, rec, world, ambient_light);  // Surface color or texture
    Radiance result = emitted;
    if (mat->is_reflective && mat->is_refractive) {
        // Schlick's approximation for Fresnel reflectance
        double cos_theta = std::fabs(dot(-unit_vector(r.direction()), rec.normal));
        double R0 = pow((1.0 - mat->refractiveindex) / (1.0 + mat->refractiveindex), 2);
        double reflectance = R0 + (1.0 - R0) * pow(1.0 - cos_theta, 5);
        result += reflectance * reflection_color + (1.0 - reflectance) * refraction_color;
    } else if (mat->is_reflective) {
        result += reflection_color;
    } else if (mat->is_refractive) {
        result += refraction_color;
    } else {
        // Diffuse surfaces (cosine-weighted hemisphere sampling for indirect lighting)
        Vector3 scatter_dir = random_in_hemisphere(rec.normal);
        Ray scattered_ray(rec.p + 0.001 * scatter_dir, scatter_dir);
        result += surface_color * trace_path(scattered_ray, world, lights, depth - 1);
    }


    // Russian Roulette termination
    if (depth > 3) {
        double termination_probability = 0.8;  // Adjust as needed
        if (random_double() > termination_probability) {
            return result;  // Terminate ray tracing
        }
        result /= termination_probability;  // Weight for unbiased result
    }
    return result;
}


//brdf supported
Radiance Camera::trace_path_brdf(const Ray& r, const Object& world, const Light& lights, int depth) const {
    IntersectionRecord rec;
    Radiance result;
    Vector3 coefficient = Vector3(1, 1, 1);

    Vector3 view_dir = -r.direction();

    if(depth <=0) {
        return Radiance(0, 0, 0);
    }
    // If the ray hits nothing, return the background color.
    if (!world.intersect(r, Interval(0.001, infinity), rec)) {
        return background;
    }


    FancyBRDF brdf = rec.mat_ptr->build_brdf( rec.normal, rec.dpdu, rec.dpdv);

    Radiance direct = lights.brdf_shading(rec.p, rec.normal, view_dir, brdf, world, coefficient);

    Vector3 l;
    Vector3 sample = Vector3(random_double(), random_double(), 0);
    Vector3 weight = brdf.Sample(l,view_dir, sample);

    // If the sample weight is negligible, terminate

    coefficient *= weight;

    // Compute the reflected ray
    Ray scattered_ray(rec.p, l);

    // Recursively calculate the indirect contribution
    Radiance indirect = trace_path(scattered_ray, world, lights, depth - 1);

    // Combine direct and indirect lighting contributions
    result = direct + coefficient * indirect;

    return result;
}


// RENDERING MODES

std::vector<Radiance> Camera::render_path(const Object& world, Light& lights) {
    initialize();
    std::vector<Radiance> pixelData(image_width * image_height);
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            Radiance pixel_color(0, 0, 0);
            // Multi-sampling loop
            for (int sample = 0; sample < samples_per_pixel; ++sample) {
                Ray r = get_sampled_ray(i, j);  // Generate a ray with random sampling
                pixel_color += trace_path(r, world, lights, nbounces);
            }
            // Average the color of the sampled rays
            pixel_color *= (1.0 / samples_per_pixel);
            // Apply tone mapping
            pixelData[j * image_width + i] = tone_map(pixel_color);
        }
    }
    return pixelData;
}


std::vector<Radiance> Camera::render_binary(const Object& world) {
    initialize();
    std::vector<Radiance> pixelData(image_width * image_height);
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - center;
            Ray r(center, ray_direction);
            // Pass `nbounces` to control recursive depth
            Radiance pixel_color = trace_binary(r, world);
            pixelData[j * image_width + i] = pixel_color;
        }
    }
    return pixelData;
}

std::vector<Radiance> Camera::render_phong(const Object& world, const Light& lights) {
    initialize();
    std::vector<Radiance> pixelData(image_width * image_height);
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - center;
            Ray r(center, ray_direction);
            // Pass `nbounces` to control recursive depth
            Radiance pixel_color = trace_phong(r, world, lights, nbounces);
            pixelData[j * image_width + i] = tone_map(pixel_color);
        }
    }
    return pixelData;
}