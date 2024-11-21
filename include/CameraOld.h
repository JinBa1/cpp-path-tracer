// #ifndef CAMERA_H
// #define CAMERA_H

// #include <string>
// #include <vector>
// #include "color.h"
// #include "vec3.h"

// class Camera {

// public:
//     Camera(std::string type,
//             int width,
//             int height,
//             const vec3& position,
//             const vec3& lookAt,
//             const vec3& upVector,
//             float fov,
//             float exposure);

//     void render(std::string outputDir) const;

// private:
//     // Camera specifications, read from the input file 
//     std::string type;
//     int width;
//     int height;
//     vec3 position;
//     vec3 lookAt;
//     vec3 upVector;
//     float fov;
//     float exposure;

//     // Camera basis vectors
//     float focalLength;
//     vec3 u, v, w;

//     // Viewport
//     float viewportWidth;
//     float viewportHeight;
//     vec3 horizontal;
//     vec3 vertical;
//     vec3 topRightCorner;

//     void computeCameraBasis();
//     void computeViewport();

// };;

// // Vector3 ray_color(const Ray& r);

// // bool hit_sphere(const Point3& center, double radius, const Ray& r);

// #endif // CAMERA_H