#include "Nito/Systems/Button.hpp"

#include <vector>
#include <string>

#include "Nito/Components.hpp"
#include "Nito/Input.hpp"


using std::vector;
using std::string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void button_subscribe(const Entity entity)
{
    auto button = (Button *)get_component(entity, "button");
    auto sprite = (Sprite *)get_component(entity, "sprite");
    auto ui_mouse_event_handlers = (UI_Mouse_Event_Handlers *)get_component(entity, "ui_mouse_event_handlers");
    const string default_button_texture_path = sprite->texture_path;


    // Settup ui_mouse_event_handlers to update sprite texture path from button; make left mouse release event trigger
    // Button's click handler.
    ui_mouse_event_handlers->mouse_enter_handler = [=]() -> void
    {
        sprite->texture_path = button->hover_texture_path;
    };

    ui_mouse_event_handlers->mouse_exit_handler = [=]() -> void
    {
        sprite->texture_path = default_button_texture_path;
    };

    ui_mouse_event_handlers->mouse_button_handlers[Mouse_Buttons::LEFT][Key_Actions::PRESS] = [=]() -> void
    {
        sprite->texture_path = button->pressed_texture_path;
    };

    ui_mouse_event_handlers->mouse_button_handlers[Mouse_Buttons::LEFT][Key_Actions::RELEASE] = [=]() -> void
    {
        sprite->texture_path = button->hover_texture_path;

        if (button->click_handler)
        {
            button->click_handler();
        }
    };
}


} // namespace Nito
