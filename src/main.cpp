#include "rtweekend.h"

#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include "camera.h"
// #include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "cylinder.h"
#include "triangle.h"
#include "light.h"
#include "point_light.h"
#include "area_light.h"
#include "light_list.h"

#include "json.hpp"

using json = nlohmann::json;


void parse_scene(const json& scene_data, hittable_list& world, light_list& lights, camera& cam) {
    // Parse camera
    auto camera_data = scene_data["camera"];
    cam.image_width = camera_data["width"];
    cam.image_height = camera_data["height"];
    cam.lookfrom = point3(camera_data["position"][0], camera_data["position"][1], camera_data["position"][2]);
    cam.lookat = point3(camera_data["lookAt"][0], camera_data["lookAt"][1], camera_data["lookAt"][2]);
    cam.vup = vec3(camera_data["upVector"][0], camera_data["upVector"][1], camera_data["upVector"][2]);
    cam.vfov = camera_data["fov"];
    cam.background = color(scene_data["scene"]["backgroundcolor"][0], 
                           scene_data["scene"]["backgroundcolor"][1], 
                           scene_data["scene"]["backgroundcolor"][2]);
    cam.exposure = camera_data["exposure"];

    // Parse light sources
    for (const auto& light_data : scene_data["scene"]["lightsources"]) {
        std::string type = light_data["type"];
        if (type == "pointlight") {
            lights.add(make_shared<point_light>(
                point3(light_data["position"][0], light_data["position"][1], light_data["position"][2]),
                color(light_data["intensity"][0], light_data["intensity"][1], light_data["intensity"][2])
            ));
        }  else if (type == "arealight") {
            lights.add(make_shared<area_light>(
                point3(light_data["position"][0], light_data["position"][1], light_data["position"][2]),
                vec3(light_data["u"][0], light_data["u"][1], light_data["u"][2]),
                vec3(light_data["v"][0], light_data["v"][1], light_data["v"][2]),
                light_data["width"],
                light_data["height"],
                color(light_data["intensity"][0], light_data["intensity"][1], light_data["intensity"][2])
            ));
        }
    }

    // Parse shapes and materials
    for (const auto& shape_data : scene_data["scene"]["shapes"]) {
        std::string type = shape_data["type"];

        // Parse material
        auto mat_data = shape_data["material"];
        auto mat = new material();
        mat->kd = mat_data["kd"];
        mat->ks = mat_data["ks"];
        mat->specularexponent = mat_data["specularexponent"];
        mat->diffusecolor = color(mat_data["diffusecolor"][0], mat_data["diffusecolor"][1], mat_data["diffusecolor"][2]);
        mat->specularcolor = color(mat_data["specularcolor"][0], mat_data["specularcolor"][1], mat_data["specularcolor"][2]);
        mat->is_reflective = mat_data["isreflective"];
        mat->reflectivity = mat_data["reflectivity"];   
        mat->is_refractive = mat_data["isrefractive"];
        mat->refractiveindex = mat_data["refractiveindex"];
        if (mat_data.contains("texture")) {
            mat->load_texture(mat_data["texture"]);
            if (mat_data.contains("tile_factor_u")) {
                mat->tile_factor_u = mat_data["tile_factor_u"];
            if (mat_data.contains("tile_factor_v")) {
                mat->tile_factor_v = mat_data["tile_factor_v"];
                }
            }
        }

        if (mat_data.contains("is_microfacet")) {
            mat->is_microfacet = mat_data["is_microfacet"];

            if (mat->is_microfacet) {
                mat->base_color = color(mat_data["base_color"][0], mat_data["base_color"][1], mat_data["base_color"][2]);
                mat->roughness = mat_data["roughness"];
                mat->metalness = mat_data["metalness"];
                if (mat_data.contains("F0")) {
                    mat->F0 = color(mat_data["F0"][0], mat_data["F0"][1], mat_data["F0"][2]);
                } else {
                    mat->F0 = color(0.04, 0.04, 0.04); // default for non-metals
                }
            }
        }

        bool excluded = false;
        if (shape_data.contains("excluded")) {
            excluded = shape_data["excluded"];
        }

        if (type == "sphere") {

                world.add(make_shared<sphere>(
                    point3(shape_data["center"][0], shape_data["center"][1], shape_data["center"][2]),
                    shape_data["radius"],
                    mat,
                    excluded
                ));
          
        } else if (type == "cylinder") {
            world.add(make_shared<cylinder>(
                point3(shape_data["center"][0], shape_data["center"][1], shape_data["center"][2]),
                vec3(shape_data["axis"][0], shape_data["axis"][1], shape_data["axis"][2]),
                shape_data["radius"],
                shape_data["height"],
                mat,
                excluded
            ));
        } else if (type == "triangle") {
            // customized uv mapping
            if (mat_data.contains("uv0") && mat_data.contains("uv1") && mat_data.contains("uv2")) {
                vec3 uv0 = vec3(mat_data["uv0"][0], mat_data["uv0"][1], 0.0);
                vec3 uv1 = vec3(mat_data["uv1"][0], mat_data["uv1"][1], 0.0);
                vec3 uv2 = vec3(mat_data["uv2"][0], mat_data["uv2"][1], 0.0);
                world.add(make_shared<triangle>(
                    point3(shape_data["v0"][0], shape_data["v0"][1], shape_data["v0"][2]),
                    point3(shape_data["v1"][0], shape_data["v1"][1], shape_data["v1"][2]),
                    point3(shape_data["v2"][0], shape_data["v2"][1], shape_data["v2"][2]),
                    mat,
                    uv0, uv1, uv2,
                    excluded
                ));
            } else {
                world.add(make_shared<triangle>(
                    point3(shape_data["v0"][0], shape_data["v0"][1], shape_data["v0"][2]),
                    point3(shape_data["v1"][0], shape_data["v1"][1], shape_data["v1"][2]),
                    point3(shape_data["v2"][0], shape_data["v2"][1], shape_data["v2"][2]),
                    mat,
                    excluded
                )); 
            }
        }
        mat->print_material(); // Print material properties
    }
}


int main() {
    // Load JSON file
    std::string inputDir = "/home/jin/cgr/rt/data/jsons/";
    std::string jsonName = "area";
    std::string extension = ".ppm";
    std::ifstream input_file(inputDir+jsonName+".json");
    int output_id = 1;
    if (!input_file) {
        std::cerr << "Error: Could not open the JSON file." << std::endl;
        return 1;
    }

    json scene_data;
    input_file >> scene_data;

    hittable_list world;
    light_list lights;
    camera cam;

    // parse_scene_binary(scene_data, world, cam);

    parse_scene(scene_data, world, lights, cam);

    cam.filename = jsonName + std::to_string(output_id) + extension;

    cam.samples_per_pixel = 10;
    cam.aperture = 0.02;
    cam.focus_dist = 1.5;


    // // Build the BVH
    // world.build();
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();



    // cam.render_binary(world);
    cam.render(world, lights);

    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate elapsed time
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // Print runtime to terminal
    // Convert to seconds and print with 2 decimal places
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Rendering completed in " << duration / 1000.0 << " seconds." << std::endl;

    world.log_hit_counts();

    return 0;
}