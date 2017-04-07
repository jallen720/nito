#include "Nito/Systems/Line_Collider.hpp"

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Physics.hpp"


using std::map;
using std::string;

// glm/glm.hpp
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
struct Line_Collider_State
{
    Transform * transform;
    const Collider * collider;
    const Line_Collider * line_collider;
    vec3 world_begin;
    vec3 world_end;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Line_Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void line_collider_subscribe(Entity entity)
{
    auto collider = (Collider *)get_component(entity, "collider");
    Line_Collider_State & line_collider_state = entity_states[entity];
    line_collider_state.transform = (Transform *)get_component(entity, "transform");
    line_collider_state.collider = collider;
    line_collider_state.line_collider = (Line_Collider *)get_component(entity, "line_collider");

    load_line_collider_data(
        entity,
        &collider->collision_handler,
        &collider->sends_collision,
        &collider->receives_collision,
        &collider->enabled,
        &line_collider_state.world_begin,
        &line_collider_state.world_end);
}


void line_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    remove_line_collider_data(entity);
}


void line_collider_update()
{
    for_each(entity_states, [=](Entity /*entity*/, Line_Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Line_Collider * entity_line_collider = entity_state.line_collider;
        vec3 & entity_world_begin = entity_state.world_begin;
        vec3 & entity_world_end = entity_state.world_end;


        // Update world begin and end positions for line collider.
        entity_world_begin = get_child_world_position(entity_transform, entity_line_collider->begin);
        entity_world_end = get_child_world_position(entity_transform, entity_line_collider->end);


        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            draw_line_collider(entity_world_begin, entity_world_end, entity_transform->scale);
        }
    });
}


} // namespace Nito
