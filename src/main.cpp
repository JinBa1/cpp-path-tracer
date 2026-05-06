#include "util/Utilities.h"

#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "Camera.h"
// #include "object/Object.h"
#include "object/ObjectList.h"
#include "object/Sphere.h"
#include "object/Cylinder.h"
#include "object/Triangle.h"
#include "light/Light.h"
#include "light/PointLight.h"
#include "light/RectangularLight.h"
#include "light/LightList.h"
#include "Material.h"
#include "Parser.h"


int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <jsonName> [outputDir] [outputName]" << std::endl;
        return 1;
    }

    // Get the JSON name and optional output ID from the command line
    std::string jsonName = argv[1];
    std::string outputDir = (argc > 2) ? argv[2] : ""; // Default output directory is empty
    std::string imageName = (argc > 3) ? argv[3] : "rendered"; // Default output ID is 102

    std::string extension = ".ppm";

    ObjectList world;
    LightList lights;
    Camera cam;

    Parser parser;

    try {
        parser.parse_scene(jsonName, jsonName, world, lights, cam);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    cam.filename = outputDir + imageName + extension;


    // // Build the BVH
    if (world.use_bvh == UseBVH::ENABLE) {
        world.build();
    }

    cam.print_specs();
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