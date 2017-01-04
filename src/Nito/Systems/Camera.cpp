#include "Nito/Systems/Camera.hpp"

#include <map>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
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
    const Camera * camera;
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
        (Camera *)get_component(entity, "camera"),
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
    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const Camera * entity_camera = entity_state.camera;
        const Dimensions * entity_dimensions = entity_state.dimensions;
        const Transform * entity_transform = entity_state.transform;
        const float entity_width = entity_dimensions->width;
        const float entity_height = entity_dimensions->height;

        render(
            {
                entity_width,
                entity_height,
                entity_camera->z_near,
                entity_camera->z_far,
                calculate_view_matrix(
                    entity_width,
                    entity_height,
                    entity_dimensions->origin,
                    entity_transform->position,
                    entity_transform->scale,
                    entity_transform->rotation),
            });
    });

    cleanup_rendering();
}


} // namespace Nito
