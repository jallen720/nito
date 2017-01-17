#include "Nito/APIs/Physics.hpp"

#include <map>
#include <vector>
#include <functional>
#include <cmath>
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

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Circle_Collider_Data
{
    const Transform * transform;
    const Collider * collider;
    const Circle_Collider * circle_collider;
};


struct Line_Collider_Data
{
    const vec3 * start;
    const vec3 * end;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Circle_Collider_Data> circle_collider_datas;
static map<Entity, Line_Collider_Data> line_collider_datas;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_circle_collider_data(
    Entity entity,
    const Transform * transform,
    const Collider * collider,
    const Circle_Collider * circle_collider)
{
    circle_collider_datas[entity] =
    {
        transform,
        collider,
        circle_collider,
    };
}


void load_line_collider_data(Entity entity, const vec3 * line_start, const vec3 * line_end)
{
    line_collider_datas[entity] =
    {
        line_start,
        line_end,
    };
}


void remove_circle_collider_data(Entity entity)
{
    remove(circle_collider_datas, entity);
}


void remove_line_collider_data(Entity entity)
{
    remove(line_collider_datas, entity);
}


void physics_api_update()
{
    // TODO: could be optimized with an overload of for_each().
    for_each(circle_collider_datas, [=](Entity circle_entity, Circle_Collider_Data & circle_data) -> void
    {
        const Transform * circle_transform = circle_data.transform;
        const vec3 & circle_position = circle_transform->position;
        const vec3 & circle_scale = circle_transform->scale;
        const float circle_radius = circle_data.circle_collider->radius;
        const function<void(Entity)> & circle_collision_handler = circle_data.collider->collision_handler;
        vector<Entity> collisions;


        // Check for collisions with other circle colliders.
        for_each(circle_collider_datas, [&](Entity circle_b_entity, Circle_Collider_Data & circle_b_data) -> void
        {
            // Don't check for collisions against self.
            if (circle_b_entity == circle_entity)
            {
                return;
            }


            const Transform * circle_b_transform = circle_b_data.transform;

            const float collision_distance =
                (circle_radius * circle_scale.x) +
                (circle_b_data.circle_collider->radius * circle_b_transform->scale.x);

            if (distance((vec2)circle_position, (vec2)circle_b_transform->position) <= collision_distance)
            {
                collisions.push_back(circle_b_entity);
            }
        });


        // Check for collisions with line colliders.
        for_each(line_collider_datas, [&](Entity line_entity, Line_Collider_Data & line_data) -> void
        {
            const vec3 * start = line_data.start;
            const vec3 * end = line_data.end;
            const float start_x = start->x;
            const float start_y = start->y;
            const float end_x = end->x;
            const float end_y = end->y;
            const float circle_x = circle_position.x;
            const float circle_y = circle_position.y;
            const float direction_x = end_x - start_x;
            const float direction_y = end_y - start_y;
            const float A = direction_x * direction_x + direction_y * direction_y;
            const float B = 2 * (direction_x * (start_x - circle_x) + direction_y * (start_y - circle_y));

            const float C =
                (start_x - circle_x) * (start_x - circle_x) +
                (start_y - circle_y) * (start_y - circle_y) -
                circle_radius * circle_radius;

            float discriminant = B * B - 4 * A * C;

            if (discriminant < 0)
            {
                // No intersection
            }
            else
            {
                // Ray didn't totally miss sphere, so there is a solution to the equation.

                discriminant = sqrt(discriminant);

                // Either solution may be on or off the ray so need to test both t1 is always the smaller value, because
                // BOTH discriminant and A are nonnegative.
                float t1 = (-B - discriminant) / (2 * A);
                float t2 = (-B + discriminant) / (2 * A);

                // 3x HIT cases:
                //          -o->             --|-->  |            |  --|->
                // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit),

                // 3x MISS cases:
                //       ->  o                     o ->              | -> |
                // FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

                if (t1 >= 0 && t1 <= 1)
                {
                    // t1 is the intersection, and it's closer than t2 (since t1 uses -B - discriminant) Impale, Poke
                    collisions.push_back(line_entity);
                }

                // here t1 didn't intersect so we are either started inside the sphere or completely past it
                if (t2 >= 0 && t2 <= 1)
                {
                    // ExitWound
                    collisions.push_back(line_entity);
                }
            }
        });


        // Trigger collision handler for all collisions if one was set.
        if (circle_collision_handler)
        {
            for_each(collisions, circle_collision_handler);
        }
    });
}


} // namespace Nito
