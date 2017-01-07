#include "Nito/Systems/Camera.hpp"

#include <map>
#include <stdexcept>
#include <glm/glm.hpp>

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Window.hpp"


using std::map;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const Camera * entity_camera;
static const Dimensions * entity_dimensions;
static const Transform * entity_transform;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool entity_subscribed()
{
    return entity_camera != nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void camera_subscribe(Entity entity)
{
    if (entity_subscribed())
    {
        throw runtime_error("ERROR: only one entity can be subscribed to the camera system per scene!");
    }

    entity_camera = (Camera *)get_component(entity, "camera");
    entity_dimensions = (Dimensions *)get_component(entity, "dimensions");
    entity_transform = (Transform *)get_component(entity, "transform");
}


void camera_unsubscribe(Entity /*entity*/)
{
    entity_camera = nullptr;
    entity_dimensions = nullptr;
    entity_transform = nullptr;
}


void camera_update()
{
    static const vec3 & window_size = get_window_size();

    if (!entity_subscribed())
    {
        return;
    }

    const float entity_width = window_size.x;
    const float entity_height = window_size.y;

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

    cleanup_rendering();
}


} // namespace Nito
