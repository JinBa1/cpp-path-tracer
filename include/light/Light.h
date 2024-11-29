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
      const point3& p,
      const Vector3& normal,
      const Vector3& view_dir,
      const IntersectionRecord& rec,
      const Object& world,
      const Radiance& ambient_light
     ) const = 0;

     virtual Radiance brdf_shading(
      const point3& p,
      const Vector3& normal,
      const Vector3& view_dir,
      const FancyBRDF& brdf,
      const Object& world,
      const Vector3& coefficient
     ) const = 0;

};

inline Radiance calculate_ambient(Material& mat, Radiance ambient_light) {
   if (mat.has_texture) {
       return Radiance(0, 0, 0);
   } 
   Radiance diffuse = mat.kd * mat.diffusecolor * ambient_light;
   Radiance specular = mat.ks * mat.specularcolor * ambient_light;
   Radiance ambient = mat.ka * diffuse + specular;
   return ambient;
}

#endif // LIGHT_H