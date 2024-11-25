#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "rtweekend.h"
#include "hittable.h"
#include "aabb.h"

#include <vector>
#include <memory>

// Forward declaration of bvh_node
class bvh_node;

class hittable_list : public hittable {
  public:
    std::vector<shared_ptr<hittable>> objects;
    std::shared_ptr<bvh_node> root;  // BVH root node

    hittable_list();
    hittable_list(std::shared_ptr<hittable> object);

    void clear();
    void add(std::shared_ptr<hittable> object);
    void build();

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override;
    aabb bounding_box() const override;

    void log_hit_counts() const;
};

#endif