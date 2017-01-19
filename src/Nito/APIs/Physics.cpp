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
    const Collider * collider;
    const vec3 * start;
    const vec3 * end;
};


using Circle_Collider_Data_Iterator = map<Entity, Circle_Collider_Data>::const_iterator;
using Line_Collider_Data_Iterator = map<Entity, Line_Collider_Data>::const_iterator;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Circle_Collider_Data> circle_collider_datas;
static map<Entity, Line_Collider_Data> line_collider_datas;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void trigger_collision_handlers(
    Entity collider_entity,
    const function<void(Entity)> & collider_entity_handler,
    const map<Entity, const Collider *> & collisions)
{
    // Trigger collider entity's collision handler if it is set.
    if (collider_entity_handler)
    {
        for_each(collisions, [&](Entity collision_entity, const Collider * /*collision_entity_collider*/) -> void
        {
            collider_entity_handler(collision_entity);
        });
    }


    // Trigger all collision entities' collision handlers if they are set.
    for_each(collisions, [=](Entity /*collision_entity*/, const Collider * collision_entity_collider) -> void
    {
        const function<void(Entity)> & collision_entity_handler = collision_entity_collider->collision_handler;

        if (collision_entity_handler)
        {
            collision_entity_handler(collider_entity);
        }
    });
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_circle_collider_data(
    Entity entity,
    const Collider * collider,
    const Transform * transform,
    const Circle_Collider * circle_collider)
{
    circle_collider_datas[entity] =
    {
        transform,
        collider,
        circle_collider,
    };
}


void load_line_collider_data(Entity entity, const Collider * collider, const vec3 * line_start, const vec3 * line_end)
{
    line_collider_datas[entity] =
    {
        collider,
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
    Circle_Collider_Data_Iterator circles_iterator = circle_collider_datas.begin();
    Line_Collider_Data_Iterator lines_iterator = line_collider_datas.begin();


    // Check for circle collider collisions.
    while (circles_iterator != circle_collider_datas.end())
    {
        const Entity circle_entity = circles_iterator->first;
        const Circle_Collider_Data & circle_data = circles_iterator->second;
        const Transform * circle_transform = circle_data.transform;
        const vec3 & circle_position = circle_transform->position;
        const vec3 & circle_scale = circle_transform->scale;
        const float circle_radius = circle_data.circle_collider->radius * circle_scale.x;
        const function<void(Entity)> & circle_collision_handler = circle_data.collider->collision_handler;
        map<Entity, const Collider *> collisions;


        // Check for collisions with other circle colliders.
        for_each(circle_collider_datas, ++circles_iterator, [&](
            Entity circle_b_entity,
            const Circle_Collider_Data & circle_b_data) -> void
        {
            const Transform * circle_b_transform = circle_b_data.transform;

            const float collision_distance =
                circle_radius + (circle_b_data.circle_collider->radius * circle_b_transform->scale.x);

            if (distance((vec2)circle_position, (vec2)circle_b_transform->position) <= collision_distance)
            {
                collisions[circle_b_entity] = circle_b_data.collider;
            }
        });


        // Check for collisions with line colliders.
        for_each(line_collider_datas, [&](Entity line_entity, Line_Collider_Data & line_data) -> void
        {
            const Collider * line_collider = line_data.collider;
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
            const float start_circle_offset_x = start_x - circle_x;
            const float start_circle_offset_y = start_y - circle_y;
            const float A = (direction_x * direction_x) + (direction_y * direction_y);
            const float B = 2 * ((direction_x * start_circle_offset_x) + (direction_y * start_circle_offset_y));

            const float C =
                (start_circle_offset_x * start_circle_offset_x) +
                (start_circle_offset_y * start_circle_offset_y) -
                (circle_radius * circle_radius);

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
                // BOTH discriminant and A are non-negative.
                float t1 = (-B - discriminant) / (2 * A);
                float t2 = (-B + discriminant) / (2 * A);

                // 3 HIT cases:
                // --|-----|-->            --|-->  |             |   --|-->
                // Impale(t1 hit, t2 hit)  Poke(t1 hit, t2 > 1)  ExitWound(t1 < 0, t2 hit)

                // 3 MISS cases:
                // --> |     |                |     | -->           | --> |
                // FallShort(t1 > 1, t2 > 1)  Past(t1 < 0, t2 < 0)  CompletelyInside(t1 < 0, t2 > 1)

                if (t1 >= 0.0f && t1 <= 1.0f)
                {
                    // t1 is the intersection, and it's closer than t2 (since t1 uses -B - discriminant) Impale, Poke
                    collisions[line_entity] = line_collider;
                }
                else if (t2 >= 0.0f && t2 <= 1.0f)
                {
                    // here t1 didn't intersect so we are either started inside the sphere or completely past it
                    collisions[line_entity] = line_collider;
                }
            }
        });


        trigger_collision_handlers(circle_entity, circle_collision_handler, collisions);
    };


    // Check for line collider collisions that have not already been checked.
    while (lines_iterator != line_collider_datas.end())
    {
        const Entity line_entity = lines_iterator->first;
        const Line_Collider_Data & line_data = lines_iterator->second;
        const function<void(Entity)> & line_collision_handler = line_data.collider->collision_handler;
        const vec3 * line_start = line_data.start;
        const vec3 * line_end = line_data.end;
        const float line_start_x = line_start->x;
        const float line_start_y = line_start->y;
        const float line_end_x = line_end->x;
        const float line_end_y = line_end->y;
        const vec3 r(line_end_x - line_start_x, line_end_y - line_start_y, 0.0f);
        map<Entity, const Collider *> collisions;


        // Check for collisions with other line colliders.
        for_each(line_collider_datas, ++lines_iterator, [&](
            Entity line_b_entity,
            const Line_Collider_Data & line_b_data) -> void
        {
            const Collider * line_b_collider = line_b_data.collider;
            const vec3 * line_b_start = line_b_data.start;
            const vec3 * line_b_end = line_b_data.end;
            const float line_b_start_x = line_b_start->x;
            const float line_b_start_y = line_b_start->y;
            const vec3 CmP(line_b_start_x - line_start_x, line_b_start_y - line_start_y, 0.0f);
            const vec3 s(line_b_end->x - line_b_start_x, line_b_end->y - line_b_start_y, 0.0f);
            const float CmPxr = (CmP.x * r.y) - (CmP.y * r.x);
            const float CmPxs = (CmP.x * s.y) - (CmP.y * s.x);
            const float rxs = (r.x * s.y) - (r.y * s.x);

            if (CmPxr == 0.0f)
            {
                // Lines are collinear, and so intersect if they have any overlap.

                if ((line_b_start_x - line_start_x < 0.0f) != (line_b_start_x - line_end_x < 0.0f) ||
                    (line_b_start_y - line_start_y < 0.0f) != (line_b_start_y - line_end_y < 0.0f))
                {
                    collisions[line_b_entity] = line_b_collider;
                }
            }
            else if (rxs == 0.0f)
            {
                // Lines are parallel (no possible intersection).
            }
            else
            {
                float rxsr = 1.0f / rxs;
                float t = CmPxs * rxsr;
                float u = CmPxr * rxsr;

                if (t >= 0.0f && t <= 1.0f &&
                    u >= 0.0f && u <= 1.0f)
                {
                    collisions[line_b_entity] = line_b_collider;
                }
            }
        });


        trigger_collision_handlers(line_entity, line_collision_handler, collisions);
    }
}


} // namespace Nito
