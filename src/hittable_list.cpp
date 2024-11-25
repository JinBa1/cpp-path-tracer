#include "hittable_list.h"
#include "bvh_node.h"
#include "sphere.h"
#include "triangle.h"
#include "cylinder.h"

hittable_list::hittable_list() {}

hittable_list::hittable_list(std::shared_ptr<hittable> object) {
    add(object);
}

void hittable_list::clear() {
    objects.clear();
}

void hittable_list::add(std::shared_ptr<hittable> object) {
    objects.push_back(object);
}

void hittable_list::build() {
    std::vector<std::shared_ptr<hittable>> bvh_objects;

    for (const auto& obj : objects) {
        if (!obj->exclude_from_bvh()) {
            bvh_objects.push_back(obj);
        }
    }
    if (!bvh_objects.empty()) {
        root = std::make_shared<bvh_node>(bvh_objects, 0, bvh_objects.size());
        std::cout << "BVH Structure:\n";
        print_bvh_structure(root);
    }
    // if (!objects.empty()) {
    //     root = std::make_shared<bvh_node>(objects, 0, objects.size());
    //     std::cout << "BVH Structure:\n";
    //     print_bvh_structure(root);
    // }
}

bool hittable_list::hit(const ray& r, interval ray_t, hit_record& rec) const {
    // if (root) {
    //     return root->hit(r, ray_t, rec);
    // }
    // return false;
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_t.max;

    // Test BVH
    if (root && root->hit(r, ray_t, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
    }

    // Test excluded objects
    for (const auto& object : objects) {
        if (object->exclude_from_bvh() && object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

// NOT USING BVH
// bool hittable_list::hit(const ray& r, interval ray_t, hit_record& rec) const {
//     hit_record temp_rec;
//     bool hit_anything = false;
//     auto closest_so_far = ray_t.max;

//     for (const auto& object : objects) {
//         if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
//             hit_anything = true;
//             closest_so_far = temp_rec.t;
//             rec = temp_rec;
//         }
//     }

//     return hit_anything;
// }


aabb hittable_list::bounding_box() const {
    return root ? root->bounding_box() : aabb();
}

void hittable_list::log_hit_counts() const {
    for (const auto& object : objects) {
        // Check if the object is a sphere
        if (auto sphere_ptr = std::dynamic_pointer_cast<sphere>(object)) {
            std::cout << "Sphere hit_call_count: " << sphere_ptr->hit_call_count << std::endl;
        }
        // Check if the object is a triangle
        else if (auto triangle_ptr = std::dynamic_pointer_cast<triangle>(object)) {
            std::cout << "Triangle hit_call_count: " << triangle_ptr->hit_call_count << std::endl;
        }
        // Check if the object is a cylinder
        else if (auto cylinder_ptr = std::dynamic_pointer_cast<cylinder>(object)) {
            std::cout << "Cylinder hit_call_count: " << cylinder_ptr->hit_call_count << std::endl;
        }
        // Handle other hittable objects
        else {
            std::cout << "Unknown object type, skipping." << std::endl;
        }
    }
}