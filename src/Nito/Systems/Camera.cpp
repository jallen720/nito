#include "Nito/Systems/Camera.hpp"

#include <map>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::map;

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
struct Entity_State
{
    const Viewport * viewport;
    const Dimensions * dimensions;
    const Transform * transform;
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
void camera_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (Viewport *)get_component(entity, "viewport"),
        (Dimensions *)get_component(entity, "dimensions"),
        (Transform *)get_component(entity, "transform"),
    };
}


void camera_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void camera_update()
{
    init_rendering();

    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const Viewport * entity_viewport = entity_state.viewport;
        const Dimensions * entity_dimensions = entity_state.dimensions;
        const Transform * entity_transform = entity_state.transform;

        render(
            {
                entity_viewport->x,
                entity_viewport->y,
                entity_viewport->z_near,
                entity_viewport->z_far,
                {
                    entity_dimensions->width,
                    entity_dimensions->height,
                    &entity_dimensions->origin,
                    &entity_transform->position,
                    &entity_transform->scale,
                    entity_transform->rotation,
                },
            });
    });

    cleanup_rendering();
}


} // namespace Nito
