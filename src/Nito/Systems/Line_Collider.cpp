#include "Nito/Systems/Line_Collider.hpp"

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Physics.hpp"


using std::map;
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
struct Line_Collider_State
{
    const Transform * transform;
    const Collider * collider;
    const Line_Collider * line_collider;
    vec3 world_start;
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
        &line_collider_state.world_start,
        &line_collider_state.world_end);
}


void line_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    remove_line_collider_data(entity);
}


void line_collider_update()
{
    static float pixels_per_unit = get_pixels_per_unit();

    for_each(entity_states, [=](Entity /*entity*/, Line_Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Line_Collider * entity_line_collider = entity_state.line_collider;
        const vec3 & entity_line_collider_start = entity_line_collider->start;
        const vec3 & entity_line_collider_end = entity_line_collider->end;
        vec3 & entity_world_start = entity_state.world_start;


        // Update world start and end positions for line collider.
        entity_world_start = get_child_world_position(entity_transform, entity_line_collider_start);
        entity_state.world_end = get_child_world_position(entity_transform, entity_line_collider_end);


        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            static const string VERTEX_CONTAINER_ID("line_collider");
            static const vec3 BASE_ANGLE_VECTOR(1.0f, 0.0f, 0.0f);
            vec3 position = entity_world_start;
            position.z = -1.0f;

            float rotation =
                degrees(angle(BASE_ANGLE_VECTOR, normalize(entity_line_collider_end - entity_line_collider_start)));

            load_render_data(
                {
                    Render_Modes::LINES,
                    &Collider::LAYER_NAME,
                    nullptr,
                    &Collider::SHADER_PIPELINE_NAME,
                    &VERTEX_CONTAINER_ID,
                    &Collider::UNIFORMS,
                    calculate_model_matrix(
                        distance(entity_line_collider_start, entity_line_collider_end) * pixels_per_unit,
                        pixels_per_unit,
                        Collider::ORIGIN,
                        position,
                        entity_transform->scale,
                        entity_transform->rotation + rotation)
                });
        }
    });
}


} // namespace Nito
