#include "hittable_list.h"
#include "bvh_node.h"

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
    if (!objects.empty()) {
        root = std::make_shared<bvh_node>(objects, 0, objects.size());
    }
}

bool hittable_list::hit(const ray& r, interval ray_t, hit_record& rec) const {
    if (root) {
        return root->hit(r, ray_t, rec);
    }
    return false;
}

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