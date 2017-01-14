#include "Nito/Systems/Line_Collider.hpp"

#include <map>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Physics.hpp"


using std::map;
using std::string;
using std::function;

// glm/glm.hpp
using glm::vec3;
using glm::vec4;

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
    auto transform = (Transform *)get_component(entity, "transform");
    auto collider = (Collider *)get_component(entity, "collider");
    auto line_collider = (Line_Collider *)get_component(entity, "line_collider");

    entity_states[entity] =
    {
        transform,
        collider,
        line_collider,
    };

    // load_collider_data(entity, transform, collider, line_collider);
}


void line_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    // remove_collider_data(entity);
}


void line_collider_update()
{
    static float pixels_per_unit = get_pixels_per_unit();

    for_each(entity_states, [=](Entity /*entity*/, Line_Collider_State & entity_state) -> void
    {
        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            static const string VERTEX_CONTAINER_ID("line_collider");

            const Transform * entity_transform = entity_state.transform;
            const Line_Collider * entity_line_collider = entity_state.line_collider;
            vec3 local_position = entity_line_collider->offset;
            local_position.z = -1.0f;

            load_render_data(
                {
                    Render_Modes::LINES,
                    &Collider::LAYER_NAME,
                    nullptr,
                    &Collider::SHADER_PIPELINE_NAME,
                    &VERTEX_CONTAINER_ID,
                    &Collider::UNIFORMS,
                    calculate_model_matrix(
                        entity_line_collider->size * pixels_per_unit,
                        pixels_per_unit,
                        Collider::ORIGIN,
                        get_child_world_position(entity_transform, local_position),
                        entity_transform->scale,
                        entity_transform->rotation)
                });
        }
    });
}


} // namespace Nito
