#ifndef BVH_H
#define BVH_H
#include "rtweekend.h"

#include "aabb.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
  public:
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        // Check if no objects
        if (objects.empty()) return;

        

        // Check if only one object (leaf node)
        size_t object_span = end - start;
        if (object_span == 1) {
            left = right = objects[start];
            bbox = objects[start]->bounding_box();
            return;
        }

        // Split objects by their centroids along an axis
        int axis = random_int(0, 2);  // Choose splitting axis
        auto comparator = [axis](const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
            return a->bounding_box().axis_interval(axis).min < b->bounding_box().axis_interval(axis).min;
        };

        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        size_t mid = start + object_span / 2;

        // Recursively create left and right nodes
        left = make_shared<bvh_node>(objects, start, mid);
        right = make_shared<bvh_node>(objects, mid, end);

        // Compute the bounding box of this node
        bbox = aabb(left->bounding_box(), right->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t)) {
            return false; }

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);


        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;
};

#endif