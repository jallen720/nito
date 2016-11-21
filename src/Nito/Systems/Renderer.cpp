#include "Nito/Systems/Renderer.hpp"

#include <map>
#include <string>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::map;
using std::string;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Entity_State
{
    const string * render_layer;
    const Sprite * sprite;
    const Transform * transform;
    const Dimensions * dimensions;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void renderer_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (string *)get_component(entity, "render_layer"),
        (Sprite *)get_component(entity, "sprite"),
        (Transform *)get_component(entity, "transform"),
        (Dimensions *)get_component(entity, "dimensions"),
    };
}


void renderer_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void renderer_update()
{
    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const Sprite * entity_sprite = entity_state.sprite;
        const Transform * entity_transform = entity_state.transform;
        const Dimensions * entity_dimensions = entity_state.dimensions;

        load_render_data(
            {
                entity_state.render_layer,
                &entity_sprite->texture_path,
                &entity_sprite->shader_pipeline_name,
                nullptr,
                calculate_model_matrix(
                    entity_dimensions->width,
                    entity_dimensions->height,
                    entity_dimensions->origin,
                    entity_transform->position,
                    entity_transform->scale,
                    entity_transform->rotation),
            });
    });
}


} // namespace Nito
