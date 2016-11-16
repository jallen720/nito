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


    // Default to texture width & height if not set by user.
    if (entity_dimensions->width == 0.0f)
    {
        entity_dimensions->width = texture_dimensions.width;
    }

    if (entity_dimensions->height == 0.0f)
    {
        entity_dimensions->height = texture_dimensions.height;
    }
}


void sprite_dimensions_handler_unsubscribe(const Entity /*entity*/) {}


} // namespace Nito
