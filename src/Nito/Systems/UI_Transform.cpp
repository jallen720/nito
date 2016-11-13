#include "Nito/Systems/UI_Transform.hpp"

#include <glm/glm.hpp>

#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Graphics.hpp"


// glm/glm.hpp
using glm::vec3;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 window_unit_size;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ui_transform_init()
{
    window_unit_size = get_window_size() / get_unit_scale();
}


void ui_transform_subscribe(const Entity entity)
{
    auto transform = (Transform *)get_component(entity, "transform");
    auto ui_transform = (UI_Transform *)get_component(entity, "ui_transform");
    transform->position = ui_transform->position + (window_unit_size * ui_transform->anchor);
}


void ui_transform_unsubscribe(const Entity /*entity*/)
{
    // TODO: Fix ui transform so it unapplies position.
}


} // namespace Nito
