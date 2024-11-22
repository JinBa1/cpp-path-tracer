#include "rtweekend.h"

#include <string>
#include <fstream>
#include <vector>
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "cylinder.h"
#include "triangle.h"

#include "json.hpp"

using json = nlohmann::json;

void parse_scene(const json& scene_data, hittable_list& world, camera& cam) {
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

    // Parse shapes
    for (const auto& shape : scene_data["scene"]["shapes"]) {
        std::string type = shape["type"];
        if (type == "sphere") {
            world.add(make_shared<sphere>(
                point3(shape["center"][0], shape["center"][1], shape["center"][2]),
                shape["radius"]
            ));
        } else if (type == "cylinder") {
            world.add(make_shared<cylinder>(
                point3(shape["center"][0], shape["center"][1], shape["center"][2]),
                vec3(shape["axis"][0], shape["axis"][1], shape["axis"][2]),
                shape["radius"],
                shape["height"]
            ));
        } else if (type == "triangle") {
            world.add(make_shared<triangle>(
                point3(shape["v0"][0], shape["v0"][1], shape["v0"][2]),
                point3(shape["v1"][0], shape["v1"][1], shape["v1"][2]),
                point3(shape["v2"][0], shape["v2"][1], shape["v2"][2])
            ));
        }
    }
}


int main() {


    // hittable_list world;

    // world.add(make_shared<sphere>(point3(-0.3,0.19,1), 0.2));
    // world.add(make_shared<cylinder>(point3(-0.3,-0.2,1), vec3(1,0,0), 0.15, 0.2));
    // world.add(make_shared<triangle>(point3(0,0,1), point3(0.5,0,1), point3(0.25,0.25,1)));


    // camera cam;

    // // cam.aspect_ratio = 16.0 / 9.0;
    // cam.image_width  = 1200;
    // cam.image_height = 800;

    // cam.vfov = 45;
    // cam.lookfrom = point3(0,0,0);
    // cam.lookat   = point3(0,0,1);
    // cam.vup      = vec3(0,1,0);

    // cam.background = color(0.25, 0.25, 0.25);

    // cam.render(world);


    // Load JSON file
    std::string inputDir = "/home/jin/cgr/rt/data/jsons/";
    std::string jsonName = "binary_primitives.json";

    std::ifstream input_file(inputDir+jsonName);
    if (!input_file) {
        std::cerr << "Error: Could not open the JSON file." << std::endl;
        return 1;
    }

    json scene_data;
    input_file >> scene_data;

    hittable_list world;
    camera cam;

    parse_scene(scene_data, world, cam);

    cam.filename = "output10.ppm";

    cam.render(world);


    return 0;
}