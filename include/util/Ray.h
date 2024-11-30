#ifndef RAY_H
#define RAY_H

#include "util/Vector3.h"

class Ray {
  public:
    Ray() {}

    Ray(const Point3& rayOrigin, const Vector3& rayDirection) : originPoint(rayOrigin), vectorDirection(rayDirection) {}

    Point3 at(double t) const {
      // Returns the point at parameter t along the ray
      return  t*vectorDirection + originPoint;
    }

    const Point3& origin() const  { 
      // Returns the origin of the ray
      return originPoint; 
    }
    const Vector3& direction() const {
      // Returns the direction of the ray
      return vectorDirection; 
    }

  private:
    Point3 originPoint;
    Vector3 vectorDirection;
};

#endif