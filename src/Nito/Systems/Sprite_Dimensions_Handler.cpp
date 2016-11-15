#include "Nito/Systems/Sprite_Dimensions_Handler.hpp"

#include <glm/glm.hpp>

#include "Nito/Components.hpp"
#include "Nito/APIs/Resources.hpp"


// glm/glm.hpp
using glm::vec3;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sprite_dimensions_handler_subscribe(const Entity entity)
{
    auto entity_sprite = (Sprite *)get_component(entity, "sprite");
    auto entity_dimensions = (Dimensions *)get_component(entity, "dimensions");
    const Dimensions & texture_dimensions = get_loaded_texture(entity_sprite->texture_path).dimensions;
    vec3 & entity_origin = entity_dimensions->origin;
    const vec3 & texture_origin = texture_dimensions.origin;

    if (entity_dimensions->width < 0.0f)
    {
        entity_dimensions->width = texture_dimensions.width;
    }

    if (entity_dimensions->height < 0.0f)
    {
        entity_dimensions->height = texture_dimensions.height;
    }

    if (entity_origin.x < 0.0f)
    {
        entity_origin.x = texture_origin.x;
    }

    if (entity_origin.y < 0.0f)
    {
        entity_origin.y = texture_origin.y;
    }
}


void sprite_dimensions_handler_unsubscribe(const Entity /*entity*/) {}


} // namespace Nito
