#include "Nito/Systems/Polygon_Collider.hpp"

#include <map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Physics.hpp"


using std::map;
using std::vector;
using std::string;

// glm/glm.hpp
using glm::distance;
using glm::degrees;
using glm::normalize;
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
struct Polygon_Collider_State
{
    Transform * transform;
    const Collider * collider;
    const Polygon_Collider * polygon_collider;
    vector<vec3> line_begins;
    vector<vec3> line_ends;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Polygon_Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void polygon_collider_subscribe(Entity entity)
{
    auto collider = (Collider *)get_component(entity, "collider");
    auto transform = (Transform *)get_component(entity, "transform");
    auto polygon_collider = (Polygon_Collider *)get_component(entity, "polygon_collider");
    Polygon_Collider_State & polygon_collider_state = entity_states[entity];
    polygon_collider_state.transform = transform;
    polygon_collider_state.collider = collider;
    polygon_collider_state.polygon_collider = polygon_collider;


    // Populate line begin and end vectors.
    vector<vec3> & line_begins = polygon_collider_state.line_begins;
    vector<vec3> & line_ends = polygon_collider_state.line_ends;
    const int line_count = polygon_collider->points.size() + (polygon_collider->wrap ? 0 : -1);

    for (int i = 0; i < line_count; i++)
    {
        line_begins.emplace_back();
        line_ends.emplace_back();
    }


    load_polygon_collider_data(
        entity,
        &collider->collision_handler,
        &collider->sends_collision,
        &collider->receives_collision,
        &line_begins,
        &line_ends,
        &transform->position);
}


void polygon_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    remove_polygon_collider_data(entity);
}


void polygon_collider_update()
{
    for_each(entity_states, [=](Entity /*entity*/, Polygon_Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Polygon_Collider * entity_polygon_collider = entity_state.polygon_collider;
        vector<vec3> & line_begins = entity_state.line_begins;
        vector<vec3> & line_ends = entity_state.line_ends;
        const vector<vec3> & points = entity_polygon_collider->points;
        const int line_count = line_begins.size();
        const int point_count = points.size();


        // Update world begin and end positions for each line in polygon collider.
        for (int curr = 0, next = 1; curr < line_count; curr++, next++)
        {
            if (next >= point_count)
            {
                next = 0;
            }

            line_begins[curr] = get_child_world_position(entity_transform, points[curr]);
            line_ends[curr] = get_child_world_position(entity_transform, points[next]);
        }


        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            for (int i = 0; i < line_count; i++)
            {
                draw_line_collider(line_begins[i], line_ends[i], entity_transform->scale);
            }
        }
    });
}


} // namespace Nito
