#include "Nito/Systems/Circle_Collider.hpp"

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
struct Circle_Collider_State
{
    const Transform * transform;
    const Collider * collider;
    const Circle_Collider * circle_collider;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Circle_Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void circle_collider_subscribe(Entity entity)
{
    auto transform = (Transform *)get_component(entity, "transform");
    auto collider = (Collider *)get_component(entity, "collider");
    auto circle_collider = (Circle_Collider *)get_component(entity, "circle_collider");

    entity_states[entity] =
    {
        transform,
        collider,
        circle_collider,
    };

    load_circle_collider_data(entity, &collider->collision_handler, transform, circle_collider);
}


void circle_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    remove_circle_collider_data(entity);
}


void circle_collider_update()
{
    const float pixels_per_unit = get_pixels_per_unit();

    for_each(entity_states, [=](Entity /*entity*/, Circle_Collider_State & entity_state) -> void
    {
        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            static const string VERTEX_CONTAINER_ID("circle_collider");
            static const float ROTATION = 0.0f;

            const Transform * entity_transform = entity_state.transform;
            const float dimensional_size = entity_state.circle_collider->radius * pixels_per_unit * 2;
            vec3 position = entity_transform->position;
            position.z = -1.0f;

            load_render_data(
                {
                    Render_Modes::LINE_STRIP,
                    &Collider::LAYER_NAME,
                    nullptr,
                    &Collider::SHADER_PIPELINE_NAME,
                    &VERTEX_CONTAINER_ID,
                    &Collider::UNIFORMS,
                    calculate_model_matrix(
                        dimensional_size,
                        dimensional_size,
                        Collider::ORIGIN,
                        position,
                        entity_transform->scale,
                        ROTATION)
                });
        }
    });
}


} // namespace Nito
