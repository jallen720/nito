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
    const Collision_Handler * collision_handler;
    const bool * sends_collision;
    const bool * receives_collision;
    const float * radius;
    const vec3 * scale;
    vec3 * position;
};


struct Line_Collider_Data
{
    const Collision_Handler * collision_handler;
    const bool * sends_collision;
    const bool * receives_collision;
    const vec3 * begin;
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
static void trigger_collision_handlers(
    Entity collider_entity,
    const Collision_Handler * collider_entity_handler,
    const map<Entity, const Collision_Handler *> & collisions)
{
    // Trigger collider entity's collision handler if it is set.
    if (*collider_entity_handler)
    {
        for_each(collisions, [&](
            Entity collision_entity,
            const Collision_Handler * /*collision_entity_collider*/) -> void
        {
            (*collider_entity_handler)(collision_entity);
        });
    }


    // Trigger all collision entities' collision handlers if they are set.
    for_each(collisions, [=](Entity /*collision_entity*/, const Collision_Handler * collision_entity_handler) -> void
    {
        if (*collision_entity_handler)
        {
            (*collision_entity_handler)(collider_entity);
        }
    });
}


static vec3 get_intersection(
    const vec3 & line_a_begin,
    const vec3 & line_a_end,
    const vec3 & line_b_begin,
    const vec3 & line_b_end)
{
    // Source: https://en.wikipedia.org/wiki/Line-line_intersection

    const float x1 = line_a_begin.x;
    const float x2 = line_a_end.x;
    const float x3 = line_b_begin.x;
    const float x4 = line_b_end.x;
    const float y1 = line_a_begin.y;
    const float y2 = line_a_end.y;
    const float y3 = line_b_begin.y;
    const float y4 = line_b_end.y;
    const float denominator = ((x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4));

    const float intersection_x =
        ((((x1 * y2) - (y1 * x2)) * (x3 - x4)) - ((x1 - x2) * ((x3 * y4) - (y3 * x4)))) / denominator;

    const float intersection_y =
        ((((x1 * y2) - (y1 * x2)) * (y3 - y4)) - ((y1 - y2) * ((x3 * y4) - (y3 * x4)))) / denominator;

    return vec3(intersection_x, intersection_y, 0.0f);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_circle_collider_data(
    Entity entity,
    const Collision_Handler * collision_handler,
    const bool * sends_collision,
    const bool * receives_collision,
    const float * radius,
    vec3 * position,
    const vec3 * scale)
{
    circle_collider_datas[entity] =
    {
        collision_handler,
        sends_collision,
        receives_collision,
        radius,
        scale,
        position,
    };
}


void load_line_collider_data(
    Entity entity,
    const Collision_Handler * collision_handler,
    const bool * sends_collision,
    const bool * receives_collision,
    const vec3 * line_begin,
    const vec3 * line_end)
{
    line_collider_datas[entity] =
    {
        collision_handler,
        sends_collision,
        receives_collision,
        line_begin,
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
    map<vec3 *, vector<vec3>> collision_corrections;


    // Check for circle collider collisions.
    while (circles_iterator != circle_collider_datas.end())
    {
        const Entity circle_entity = circles_iterator->first;
        const Circle_Collider_Data & circle_data = circles_iterator->second;
        vec3 * circle_data_position = circle_data.position;
        const float circle_position_x = circle_data_position->x;
        const float circle_position_y = circle_data_position->y;
        const vec3 circle_position_2d(circle_position_x, circle_position_y, 0.0f);
        const float circle_radius = *circle_data.radius * circle_data.scale->x;
        const bool circle_sends_collision = *circle_data.sends_collision;
        const bool circle_receives_collision = *circle_data.receives_collision;
        map<Entity, const Collision_Handler *> collisions;


        // Check for collisions with other circle colliders.
        for_each(circle_collider_datas, ++circles_iterator, [&](
            Entity circle_b_entity,
            const Circle_Collider_Data & circle_b_data) -> void
        {
            vec3 * circle_b_data_position = circle_b_data.position;
            const vec3 circle_b_position_2d(circle_b_data_position->x, circle_b_data_position->y, 0.0f);
            const float actual_distance = distance(circle_position_2d, circle_b_position_2d);

            const float collision_distance =
                circle_radius + (*circle_b_data.radius * circle_b_data.scale->x);

            if (actual_distance <= collision_distance)
            {
                collisions[circle_b_entity] = circle_b_data.collision_handler;


                // Calculate collision corrections if necessary.
                const bool receiving = circle_sends_collision && *circle_b_data.receives_collision;
                const bool sending = *circle_b_data.sends_collision && circle_receives_collision;

                const vec3 correction =
                    normalize(circle_b_position_2d - circle_position_2d) *
                    (collision_distance - actual_distance);

                if (receiving && sending)
                {
                    const vec3 shared_correction = correction / 2.0f;
                    collision_corrections[circle_b_data_position].push_back(shared_correction);
                    collision_corrections[circle_data_position].push_back(-shared_correction);
                }
                else if (receiving)
                {
                    collision_corrections[circle_b_data_position].push_back(correction);
                }
                else if (sending)
                {
                    collision_corrections[circle_data_position].push_back(-correction);
                }
            }
        });


        // Check for collisions with line colliders.
        for_each(line_collider_datas, [&](Entity line_entity, Line_Collider_Data & line_data) -> void
        {
            const vec3 * line_begin = line_data.begin;
            const vec3 * line_end = line_data.end;
            const bool line_sends_collision = *line_data.sends_collision;
            const float line_begin_x = line_begin->x;
            const float line_begin_y = line_begin->y;
            const vec3 line_begin_2d(line_begin_x, line_begin_y, 0.0f);
            const float line_end_x = line_end->x;
            const float line_end_y = line_end->y;
            const vec3 line_end_2d(line_end_x, line_end_y, 0.0f);
            const float line_length = distance(line_begin_2d, line_end_2d);
            const float line_direction_x = line_end_x - line_begin_x;
            const float line_direction_y = line_end_y - line_begin_y;
            const vec3 line_normal = normalize(vec3(-line_direction_y, line_direction_x, 0.0f));
            const float line_begin_circle_offset_x = line_begin_x - circle_position_x;
            const float line_begin_circle_offset_y = line_begin_y - circle_position_y;
            const float A = (line_direction_x * line_direction_x) + (line_direction_y * line_direction_y);

            const float B =
                2 * ((line_direction_x * line_begin_circle_offset_x) + (line_direction_y * line_begin_circle_offset_y));

            const float C =
                (line_begin_circle_offset_x * line_begin_circle_offset_x) +
                (line_begin_circle_offset_y * line_begin_circle_offset_y) -
                (circle_radius * circle_radius);

            float discriminant = (B * B) - (4 * A * C);

            if (discriminant < 0)
            {
                // No intersection
            }
            else
            {
                // Ray didn't totally miss sphere, so there is a solution to the equation.

                discriminant = sqrtf(discriminant);
                const float intersection_a = (-B - discriminant) / (2 * A);
                const float intersection_b = (-B + discriminant) / (2 * A);


                // 3 HIT cases:
                //     --|-----|-->
                //     --|-->  |
                //       |   --|-->

                // 3 MISS cases:
                // --> |     |
                //     |     | -->
                //     | --> |
                const bool is_collision =
                    // intersection_a is the intersection, and it's closer than intersection_b (since intersection_a
                    // uses -B - discriminant).
                    (intersection_a >= 0.0f && intersection_a <= 1.0f) ||

                    // Here intersection_a didn't intersect so we are either started inside the sphere or completely
                    // past it.
                    (intersection_b >= 0.0f && intersection_b <= 1.0f);


                if (is_collision)
                {
                    collisions[line_entity] = line_data.collision_handler;


                    // Calculate collision corrections if necessary.
                    if (line_sends_collision && circle_receives_collision)
                    {
                        const float line_begin_circle_distance = distance(line_begin_2d, circle_position_2d);
                        const float line_end_circle_distance = distance(line_end_2d, circle_position_2d);

                        const vec3 circle_line_normal_intersection = get_intersection(
                            line_begin_2d,
                            line_end_2d,
                            circle_position_2d,
                            circle_position_2d - line_normal);


                        // Line begin is inside circle and off line.
                        if (line_begin_circle_distance < circle_radius &&
                            distance(line_end_2d, circle_line_normal_intersection) > line_length)
                        {
                            collision_corrections[circle_data_position].push_back(
                                normalize(circle_position_2d - line_begin_2d) *
                                (circle_radius - line_begin_circle_distance));
                        }
                        // Line end is inside circle and off line.
                        else if (line_end_circle_distance < circle_radius &&
                                 distance(line_begin_2d, circle_line_normal_intersection) > line_length)
                        {
                            collision_corrections[circle_data_position].push_back(
                                normalize(circle_position_2d - line_end_2d) *
                                (circle_radius - line_end_circle_distance));
                        }
                        // Line passes through circle.
                        else
                        {
                            const float x0 = circle_position_x;
                            const float x1 = line_begin_x;
                            const float x2 = line_end_x;
                            const float y0 = circle_position_y;
                            const float y1 = line_begin_y;
                            const float y2 = line_end_y;
                            const float xd = line_direction_x;
                            const float yd = line_direction_y;


                            // Source: https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
                            const float circle_line_normal_distance =
                                fabsf(((y2 - y1) * x0) - ((x2 - x1) * y0) + (x2 * y1) - (y2 * x1)) /
                                sqrtf((yd * yd) + (xd * xd));


                            const float correction_distance = circle_radius - circle_line_normal_distance;
                            collision_corrections[circle_data_position].push_back(correction_distance * line_normal);
                        }
                    }
                }
            }
        });


        trigger_collision_handlers(circle_entity, circle_data.collision_handler, collisions);
    };


    // Check for line collider collisions that have not already been checked.
    while (lines_iterator != line_collider_datas.end())
    {
        const Entity line_entity = lines_iterator->first;
        const Line_Collider_Data & line_data = lines_iterator->second;
        const vec3 * line_begin = line_data.begin;
        const vec3 * line_end = line_data.end;
        const float line_begin_x = line_begin->x;
        const float line_begin_y = line_begin->y;
        const float line_end_x = line_end->x;
        const float line_end_y = line_end->y;
        const vec3 r(line_end_x - line_begin_x, line_end_y - line_begin_y, 0.0f);
        map<Entity, const Collision_Handler *> collisions;


        // Check for collisions with other line colliders.
        for_each(line_collider_datas, ++lines_iterator, [&](
            Entity line_b_entity,
            const Line_Collider_Data & line_b_data) -> void
        {
            const Collision_Handler * line_b_collision_handler = line_b_data.collision_handler;
            const vec3 * line_b_begin = line_b_data.begin;
            const vec3 * line_b_end = line_b_data.end;
            const float line_b_begin_x = line_b_begin->x;
            const float line_b_begin_y = line_b_begin->y;
            const float line_begins_offset_x = line_b_begin_x - line_begin_x;
            const float line_begins_offset_y = line_b_begin_y - line_begin_y;
            const vec3 CmP(line_begins_offset_x, line_begins_offset_y, 0.0f);
            const vec3 s(line_b_end->x - line_b_begin_x, line_b_end->y - line_b_begin_y, 0.0f);
            const float CmPxr = (CmP.x * r.y) - (CmP.y * r.x);
            const float CmPxs = (CmP.x * s.y) - (CmP.y * s.x);
            const float rxs = (r.x * s.y) - (r.y * s.x);

            if (CmPxr == 0.0f)
            {
                // Lines are collinear, and so intersect if they have any overlap.

                if ((line_begins_offset_x < 0.0f) != (line_b_begin_x - line_end_x < 0.0f) ||
                    (line_begins_offset_y < 0.0f) != (line_b_begin_y - line_end_y < 0.0f))
                {
                    collisions[line_b_entity] = line_b_collision_handler;
                }
            }
            else if (rxs == 0.0f)
            {
                // Lines are parallel (no possible intersection).
            }
            else
            {
                const float rxsr = 1.0f / rxs;
                const float t = CmPxs * rxsr;
                const float u = CmPxr * rxsr;

                if (t >= 0.0f && t <= 1.0f &&
                    u >= 0.0f && u <= 1.0f)
                {
                    collisions[line_b_entity] = line_b_collision_handler;
                }
            }
        });


        trigger_collision_handlers(line_entity, line_data.collision_handler, collisions);
    }


    // Resolve collisions.
    for_each(collision_corrections, [](vec3 * position, const vector<vec3> & corrections) -> void
    {
        vec3 final_correction;

        for (const vec3 & correction : corrections)
        {
            if (fabsf(correction.x) > fabsf(final_correction.x))
            {
                final_correction.x = correction.x;
            }

            if (fabsf(correction.y) > fabsf(final_correction.y))
            {
                final_correction.y = correction.y;
            }
        }

        (*position) += final_correction;
    });
}


} // namespace Nito
