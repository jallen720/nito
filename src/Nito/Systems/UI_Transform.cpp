#include "Nito/Systems/UI_Transform.hpp"

#include <map>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::map;

// glm/glm.hpp
using glm::vec3;

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
struct UI_Transform_State
{
    Transform * transform;
    UI_Transform * ui_transform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vec3 window_unit_size;
static map<Entity, UI_Transform_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ui_transform_init()
{
    window_unit_size = get_window_size() / get_pixels_per_unit();
}


void ui_transform_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        (UI_Transform *)get_component(entity, "ui_transform"),
    };
}


void ui_transform_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void ui_transform_update()
{
    for_each(entity_states, [](Entity /*entity*/, UI_Transform_State & entity_state) -> void
    {
        const UI_Transform * ui_transform = entity_state.ui_transform;
        entity_state.transform->position = ui_transform->position + (window_unit_size * ui_transform->anchor);
    });
}


} // namespace Nito
