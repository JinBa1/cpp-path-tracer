#ifndef LIGHT_H
#define LIGHT_H

#include "util/Utilities.h"
#include "object/Object.h"
#include "brdf/BRDF.h"

#include "Material.h"

class Light {
    public:

     virtual ~Light() = default;

     virtual Radiance phong_shading(
      const Point3& p,                // Point of intersection
      const Vector3& normal,          // Normal at the point of intersection
      const Vector3& view_dir,        // Direction of the view
      const IntersectionRecord& rec,  // Intersection record
      const Object& world,            // World object
      const Radiance& ambient_light   // Ambient light
     ) const = 0;

     virtual Radiance brdf_shading(
      const Point3& p,                // Point of intersection
      const Vector3& normal,          // Normal at the point of intersection
      const Vector3& view_dir,        // Direction of the view
      const FancyBRDF& brdf,          // BRDF of the material
      const Object& world,            // World object
      const Vector3& coefficient      // Coefficient for the BRDF contribution
     ) const = 0;

};

inline Radiance calculate_ambient(Material& mat, Radiance ambient_light) {
    // Calculate ambient light contribution
    if (mat.has_texture) {
       return Radiance(0, 0, 0); // Skip ambient light for textured materials
    } 
    // use blinn-phong shading model
    Radiance diffuse = mat.kd * mat.diffusecolor * ambient_light;
    Radiance specular = mat.ks * mat.specularcolor * ambient_light;
    Radiance ambient = mat.ka * diffuse + specular;
    return ambient;
}

#endif // LIGHT_H