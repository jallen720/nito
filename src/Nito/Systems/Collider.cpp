#include "Nito/Systems/Collider.hpp"

#include <map>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::map;
using std::string;
using std::function;

// glm/glm.hpp
using glm::distance;
using glm::vec2;
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
struct Collider_State
{
    const Collider * collider;
    const Transform * transform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void collider_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        (Collider *)get_component(entity, "collider"),
        (Transform *)get_component(entity, "transform"),
    };
}


void collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void collider_update()
{
    static float pixels_per_unit = get_pixels_per_unit();


    // TODO: could be optimized with an overload of for_each().
    for_each(entity_states, [=](Entity entity, Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Collider * entity_collider = entity_state.collider;
        const vec3 & entity_position = entity_transform->position;
        const vec3 & entity_scale = entity_transform->scale;
        const float entity_radius = entity_collider->radius;


        // Check for collisions with other colliders.
        for_each(entity_states, [&](Entity other_entity, Collider_State & other_entity_state) -> void
        {
            // Don't check for collisions against self.
            if (other_entity == entity)
            {
                return;
            }

            const Transform * other_entity_transform = other_entity_state.transform;

            const float collision_distance =
                (entity_radius * entity_scale.x) +
                (other_entity_state.collider->radius * other_entity_transform->scale.x);

            if (distance((vec2)entity_position, (vec2)other_entity_transform->position) <= collision_distance)
            {
                const function<void(Entity)> & entity_collision_handler = entity_collider->collision_handler;

                if (entity_collision_handler)
                {
                    entity_collision_handler(other_entity);
                }
            }
        });


        // Render collider if flagged.
        if (entity_collider->render)
        {
            static const vec4 COLOR(0.0f, 0.7f, 0.0f, 1.0f);

            static const Render_Data::Uniforms UNIFORMS
            {
                { "color", Uniform { Uniform::Types::VEC4, &COLOR } },
            };

            static const string LAYER_NAME("world");
            static const string SHADER_PIPELINE_NAME("color");
            static const string VERTEX_CONTAINER_ID("collider");
            static const vec3 ORIGIN(0.0f);
            static const float ROTATION = 0.0f;

            const float dimensional_size = entity_radius * pixels_per_unit * 2;

            load_render_data(
                {
                    Render_Modes::LINE_STRIP,
                    &LAYER_NAME,
                    nullptr,
                    &SHADER_PIPELINE_NAME,
                    &VERTEX_CONTAINER_ID,
                    &UNIFORMS,
                    calculate_model_matrix(
                        dimensional_size,
                        dimensional_size,
                        ORIGIN,
                        entity_position,
                        entity_scale,
                        ROTATION)
                });
        }
    });
}


} // namespace Nito
