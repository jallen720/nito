#include "Nito/APIs/Physics.hpp"

#include <map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::map;
using std::vector;
using std::function;

// glm/glm.hpp
using glm::distance;
using glm::vec2;
using glm::vec3;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Collider_Data
{
    const Transform * transform;
    const Collider * collider;
};


struct Circle_Collider_Data
{
    Collider_Data collider_data;
    const Circle_Collider * circle_collider;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Circle_Collider_Data> circle_collider_datas;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_collider_data(
    Entity entity,
    const Transform * transform,
    const Collider * collider,
    const Circle_Collider * circle_collider)
{
    circle_collider_datas[entity] =
    {
        {
            transform,
            collider,
        },
        circle_collider,
    };
}


void remove_collider_data(Entity entity)
{
    if (contains_key(circle_collider_datas, entity))
    {
        remove(circle_collider_datas, entity);
    }
}


void physics_api_update()
{
    // TODO: could be optimized with an overload of for_each().
    for_each(circle_collider_datas, [=](Entity a_entity, Circle_Collider_Data & a_data) -> void
    {
        const Collider_Data & a_collider_data = a_data.collider_data;
        const Transform * a_transform = a_collider_data.transform;
        const vec3 & a_position = a_transform->position;
        const vec3 & a_scale = a_transform->scale;
        const float a_radius = a_data.circle_collider->radius;
        const function<void(Entity)> & a_collision_handler = a_collider_data.collider->collision_handler;
        vector<Entity> collisions;


        // Check for collisions with other colliders.
        for_each(circle_collider_datas, [&](Entity b_entity, Circle_Collider_Data & b_data) -> void
        {
            // Don't check for collisions against self.
            if (b_entity == a_entity)
            {
                return;
            }


            const Collider_Data & b_collider_data = b_data.collider_data;
            const Transform * b_transform = b_collider_data.transform;

            const float collision_distance =
                (a_radius * a_scale.x) + (b_data.circle_collider->radius * b_transform->scale.x);

            if (distance((vec2)a_position, (vec2)b_transform->position) <= collision_distance)
            {
                collisions.push_back(b_entity);
            }
        });


        // Trigger collision handler for all collisions if one was set.
        if (a_collision_handler)
        {
            for_each(collisions, a_collision_handler);
        }
    });
}


} // namespace Nito
