#ifndef OBJECT_H
#define OBJECT_H

#include "util/Utilities.h"
#include "bvh/BoundingBox.h"
#include "Material.h"
#include <algorithm>



class IntersectionRecord {
  public:
    point3 p;
    Vector3 normal;
    double t;
    bool front_face;

    double u, v; // UV coordinates for texture mapping

    Material* mat_ptr; // Pointer to the material of the object

    Vector3 dpdu = Vector3(1, 0, 0);
    Vector3 dpdv = Vector3(0, 1, 0); // Tangent vectors

    void set_face_normal(const Ray& r, const Vector3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }

};

class Object {
  public:
    virtual ~Object() = default;

    virtual bool intersect(const Ray& r, Interval ray_t, IntersectionRecord& rec) const = 0;

    virtual BoundingBox bounding_box() const = 0;

    virtual bool exclude_from_bvh() const { return false; }
};

#endif