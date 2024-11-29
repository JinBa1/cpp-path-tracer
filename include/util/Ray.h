#ifndef RAY_H
#define RAY_H

#include "util/Vector3.h"

class Ray {
  public:
    Ray() {}

    Ray(const point3& rayOrigin, const Vector3& rayDirection) : originPoint(rayOrigin), vectorDirection(rayDirection) {}

    point3 at(double t) const {
        return  t*vectorDirection + originPoint;
    }

    const point3& origin() const  { return originPoint; }
    const Vector3& direction() const { return vectorDirection; }

  private:
    point3 originPoint;
    Vector3 vectorDirection;
};

#endif