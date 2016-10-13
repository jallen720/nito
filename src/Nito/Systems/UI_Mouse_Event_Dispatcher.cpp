#include "Nito/Systems/UI_Mouse_Event_Dispatcher.hpp"

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"
#include "Nito/Input.hpp"


using std::vector;
using std::map;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// glm/glm.hpp
using glm::vec3;
using glm::dvec2;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<bool> is_mouse_over_flags;
static vector<Sprite *> entity_sprites;
static vector<Transform *> entity_transforms;
static vector<UI_Mouse_Event_Handlers *> entity_ui_mouse_event_handlers;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool is_mouse_over(const dvec2 & mouse_position, const Sprite * sprite, const Transform * transform)
{
    const Dimensions & dimensions = sprite->dimensions;
    const float width = dimensions.width;
    const float height = dimensions.height;
    const vec3 sprite_origin_offset = sprite->dimensions.origin * vec3(width, height, 0.0f);
    const vec3 sprite_position = (transform->position * get_unit_scale()) - sprite_origin_offset;

    return mouse_position.x >= sprite_position.x && mouse_position.x < sprite_position.x + width &&
           mouse_position.y >= sprite_position.y && mouse_position.y < sprite_position.y + height;
}


void mouse_move_handler(const dvec2 & mouse_position)
{
    for (auto i = 0u; i < is_mouse_over_flags.size(); i++)
    {
        bool is_mouse_over_entity = is_mouse_over(mouse_position, entity_sprites[i], entity_transforms[i]);
        UI_Mouse_Event_Handlers * ui_mouse_event_handlers = entity_ui_mouse_event_handlers[i];


        // Check for mouse enter/exit events.
        if (is_mouse_over_flags[i])
        {
            if (!is_mouse_over_entity)
            {
                is_mouse_over_flags[i] = false;

                const UI_Mouse_Event_Handlers::Event_Handler & event_handler =
                    ui_mouse_event_handlers->mouse_exit_handler;

                if (event_handler)
                {
                    event_handler();
                }
            }
        }
        else
        {
            if (is_mouse_over_entity)
            {
                is_mouse_over_flags[i] = true;

                const UI_Mouse_Event_Handlers::Event_Handler & event_handler =
                    ui_mouse_event_handlers->mouse_enter_handler;

                if (event_handler)
                {
                    event_handler();
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ui_mouse_event_dispatcher_init()
{
    set_mouse_move_handler("ui_mouse_event_dispatcher", mouse_move_handler);
}


void ui_mouse_event_dispatcher_subscribe(const Entity entity)
{
    entity_sprites.push_back((Sprite *)get_component(entity, "sprite"));
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));

    entity_ui_mouse_event_handlers.push_back(
        (UI_Mouse_Event_Handlers *)get_component(entity, "ui_mouse_event_handlers"));

    is_mouse_over_flags.push_back(false);
}


} // namespace Nito
