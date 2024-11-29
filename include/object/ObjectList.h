#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "util/Utilities.h"
#include "object/Object.h"
#include "bvh/BoundingBox.h"

#include <vector>
#include <memory>

// Forward declaration of Node
class Node;

class ObjectList : public Object {
  public:
    std::vector<shared_ptr<Object>> objects;
    std::shared_ptr<Node> root;  // BVH root node

    UseBVH use_bvh = UseBVH::ENABLE;

    ObjectList();
    ObjectList(std::shared_ptr<Object> object);

    void clear();
    void add(std::shared_ptr<Object> object);
    void build();

    bool intersect(const Ray& r, Interval ray_t, IntersectionRecord& rec) const override {
      bool hit_anything = false;
      switch (use_bvh) {
          case UseBVH::ENABLE:
              hit_anything = intersect_bvh_enabled(r, ray_t, rec);
              break;
          case UseBVH::DISABLE:
              hit_anything = intersect_bvh_disabled(r, ray_t, rec);
              break;
      }
      return hit_anything;
    };

    bool intersect_bvh_enabled(const Ray& r, Interval ray_t, IntersectionRecord& rec) const;
    bool intersect_bvh_disabled(const Ray& r, Interval ray_t, IntersectionRecord& rec) const;

    BoundingBox bounding_box() const override;

    void log_hit_counts() const;
};

#endif