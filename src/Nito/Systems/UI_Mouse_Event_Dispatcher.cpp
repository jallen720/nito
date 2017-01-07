#include "Nito/Systems/UI_Mouse_Event_Dispatcher.hpp"

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Input.hpp"


using std::vector;
using std::map;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::remove;

// glm/glm.hpp
using glm::vec3;
using glm::dvec2;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct UI_Mouse_Event_Dispatcher_State
{
    bool is_mouse_over;
    const Transform * transform;
    const Dimensions * dimensions;
    UI_Mouse_Event_Handlers * ui_mouse_event_handlers;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, UI_Mouse_Event_Dispatcher_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool is_mouse_over(const dvec2 & mouse_position, const Dimensions * dimensions, const Transform * transform)
{
    const float width = dimensions->width;
    const float height = dimensions->height;
    const vec3 sprite_origin_offset = dimensions->origin * vec3(width, height, 0.0f);
    const vec3 sprite_position = (transform->position * get_pixels_per_unit()) - sprite_origin_offset;

    return mouse_position.x >= sprite_position.x && mouse_position.x < sprite_position.x + width &&
           mouse_position.y >= sprite_position.y && mouse_position.y < sprite_position.y + height;
}


static void check_call_handler(const UI_Mouse_Event_Handlers::Event_Handler & event_handler)
{
    if (event_handler)
    {
        event_handler();
    }
}


static void mouse_position_handler(const dvec2 & mouse_position)
{
    for_each(entity_states, [&](Entity /*entity*/, UI_Mouse_Event_Dispatcher_State & entity_state) -> void
    {
        bool is_mouse_currently_over_entity =
            is_mouse_over(mouse_position, entity_state.dimensions, entity_state.transform);

        bool & entity_is_mouse_over = entity_state.is_mouse_over;
        UI_Mouse_Event_Handlers * ui_mouse_event_handlers = entity_state.ui_mouse_event_handlers;


        // Check for mouse enter/exit events.
        if (entity_is_mouse_over)
        {
            if (!is_mouse_currently_over_entity)
            {
                entity_is_mouse_over = false;
                check_call_handler(ui_mouse_event_handlers->mouse_exit_handler);
            }
        }
        else
        {
            if (is_mouse_currently_over_entity)
            {
                entity_is_mouse_over = true;
                check_call_handler(ui_mouse_event_handlers->mouse_enter_handler);
            }
        }
    });
}


static void mouse_button_handler(Mouse_Buttons mouse_button, Button_Actions button_action)
{
    for_each(entity_states, [&](Entity /*entity*/, UI_Mouse_Event_Dispatcher_State & entity_state) -> void
    {
        // Only handle mouse button events if mouse is over entity.
        if (entity_state.is_mouse_over)
        {
            const UI_Mouse_Event_Handlers::Button_Handlers & button_handlers =
                entity_state.ui_mouse_event_handlers->mouse_button_handlers;

            if (contains_key(button_handlers, mouse_button))
            {
                const auto & mouse_button_handlers = button_handlers.at(mouse_button);

                if (contains_key(mouse_button_handlers, button_action))
                {
                    mouse_button_handlers.at(button_action)();
                }
            }
        }
    });
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ui_mouse_event_dispatcher_init()
{
    set_mouse_position_handler("ui_mouse_event_dispatcher", mouse_position_handler);
    set_mouse_button_handler("ui_mouse_event_dispatcher", mouse_button_handler);
}


void ui_mouse_event_dispatcher_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        false,
        (Transform *)get_component(entity, "transform"),
        (Dimensions *)get_component(entity, "dimensions"),
        (UI_Mouse_Event_Handlers *)get_component(entity, "ui_mouse_event_handlers"),
    };
}


void ui_mouse_event_dispatcher_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


} // namespace Nito
