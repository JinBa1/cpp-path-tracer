#ifndef BVH_H
#define BVH_H
#include "rtweekend.h"

#include "aabb.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
  public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;  // ADD GETTER FUNCTION AND MOVE TO PRIVATE


    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        // RANDOM SPLITS
        // // Check if no objects
        // if (objects.empty()) return;
        // // Check if only one object (leaf node)
        // size_t object_span = end - start;
        // if (object_span == 1) {
        //     left = right = objects[start];
        //     bbox = objects[start]->bounding_box();
        //     return;
        // }
        // // Split objects by their centroids along an axis
        // int axis = random_int(0, 2);  // Choose splitting axis
        // auto comparator = [axis](const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
        //     return a->bounding_box().axis_interval(axis).min < b->bounding_box().axis_interval(axis).min;
        // };
        // std::sort(objects.begin() + start, objects.begin() + end, comparator);
        // size_t mid = start + object_span / 2;
        // // Recursively create left and right nodes
        // left = make_shared<bvh_node>(objects, start, mid);
        // right = make_shared<bvh_node>(objects, mid, end);
        // // Compute the bounding box of this node
        // bbox = aabb(left->bounding_box(), right->bounding_box());

        // LONGEST AXIS SPLITS
        // if (objects.empty()) return;
        // size_t object_span = end - start;
        // if (object_span == 1) {
        //     left = right = objects[start];
        //     bbox = objects[start]->bounding_box();
        //     return;
        // }
        // // Calculate the bounding box for all objects in this range
        // aabb overall_bbox;
        // for (size_t i = start; i < end; i++) {
        //     overall_bbox = aabb(overall_bbox, objects[i]->bounding_box());
        // }
        // // Choose the axis with the largest extent
        // vec3 extents = vec3(overall_bbox.x.size(), overall_bbox.y.size(), overall_bbox.z.size());
        // int axis = extents[0] > extents[1] ? (extents[0] > extents[2] ? 0 : 2) : (extents[1] > extents[2] ? 1 : 2);
        // auto comparator = [axis](const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
        //     return a->bounding_box().axis_interval(axis).min < b->bounding_box().axis_interval(axis).min;
        // };
        // std::sort(objects.begin() + start, objects.begin() + end, comparator);
        // size_t mid = start + object_span / 2;
        // left = make_shared<bvh_node>(objects, start, mid);
        // right = make_shared<bvh_node>(objects, mid, end);
        // bbox = aabb(left->bounding_box(), right->bounding_box());
        
        // SAH SPLITS
        if (objects.empty()) return;
        size_t object_span = end - start;
        if (object_span == 1) {
            left = right = objects[start];
            bbox = objects[start]->bounding_box();
            return;
        }
        // Compute overall bounding box
        aabb overall_bbox;
        for (size_t i = start; i < end; i++) {
            overall_bbox = aabb(overall_bbox, objects[i]->bounding_box());
        }
        double best_cost = std::numeric_limits<double>::infinity();
        int best_axis = -1;
        size_t best_split = start;
        // Iterate over all axes
        for (int axis = 0; axis < 3; axis++) {
            // Sort objects along the current axis
            auto comparator = [axis](const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
                return a->bounding_box().axis_interval(axis).min < b->bounding_box().axis_interval(axis).min;
            };
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            // Precompute bounding boxes for left and right groups
            std::vector<aabb> left_boxes(object_span), right_boxes(object_span);
            aabb left_bbox, right_bbox;
            for (size_t i = start; i < end; i++) {
                left_bbox = aabb(left_bbox, objects[i]->bounding_box());
                left_boxes[i - start] = left_bbox;
            }
            for (size_t i = end - 1; i > start; i--) {
                right_bbox = aabb(right_bbox, objects[i - 1]->bounding_box());
                right_boxes[i - start - 1] = right_bbox;
            }
            // Compute SAH cost for each split point
            for (size_t i = start + 1; i < end; i++) {
                double left_area = left_boxes[i - start - 1].surface_area();
                double right_area = right_boxes[i - start - 1].surface_area();
                size_t left_count = i - start;
                size_t right_count = end - i;
                double cost = left_area * left_count + right_area * right_count;
                if (cost < best_cost) {
                    best_cost = cost;
                    best_axis = axis;
                    best_split = i;
                }
            }
        }
        // Sort objects along the best axis
        auto comparator = [best_axis](const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
            return a->bounding_box().axis_interval(best_axis).min < b->bounding_box().axis_interval(best_axis).min;
        };
        std::sort(objects.begin() + start, objects.begin() + end, comparator);
        // Split at the best position
        left = make_shared<bvh_node>(objects, start, best_split);
        right = make_shared<bvh_node>(objects, best_split, end);
        bbox = aabb(left->bounding_box(), right->bounding_box());


    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t)) {
            return false; }

        // visualizing
        // rec.t = ray_t.min; // Set a dummy hit record
        // rec.p = r.at(ray_t.min);
        // rec.normal = vec3(1, 0, 0); // Arbitrary normal
        // rec.mat_ptr = nullptr; // No material for bounding box visualization
        // return true;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);


        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    // shared_ptr<hittable> left;
    // shared_ptr<hittable> right;
    aabb bbox;
};

inline void print_bvh_structure(const std::shared_ptr<bvh_node>& node, int depth = 0) {
    if (!node) return;

    // Print current node's bounding box
    std::cout << std::string(depth * 2, ' ') << "Node at depth " << depth << ": "
              << "Bounding Box [X: (" << node->bounding_box().x.min << ", " << node->bounding_box().x.max
              << "), Y: (" << node->bounding_box().y.min << ", " << node->bounding_box().y.max
              << "), Z: (" << node->bounding_box().z.min << ", " << node->bounding_box().z.max << ")]\n";

    if (!node->left && !node->right) {
        std::cout << std::string(depth * 2, ' ') << "  Leaf Node\n";
        return;
    }

    // Recursively print children
    if (node->left) {
        std::cout << std::string(depth * 2, ' ') << "  Left Child:\n";
        print_bvh_structure(std::dynamic_pointer_cast<bvh_node>(node->left), depth + 1);
    }
    if (node->right) {
        std::cout << std::string(depth * 2, ' ') << "  Right Child:\n";
        print_bvh_structure(std::dynamic_pointer_cast<bvh_node>(node->right), depth + 1);
    }
}

#endif