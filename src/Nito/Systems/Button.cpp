#include "Nito/Systems/Button.hpp"

#include <map>
#include <string>
#include "Cpp_Utils/Map.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"


using std::map;
using std::string;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, UI_Mouse_Event_Handlers *> entity_ui_mouse_event_handlers;


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
    entity_ui_mouse_event_handlers[entity] = ui_mouse_event_handlers;


    // Settup ui_mouse_event_handlers to update sprite texture path from button; make left mouse release event trigger
    // Button's click handler.
    const string default_button_texture_path = sprite->texture_path;

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


void button_unsubscribe(const Entity entity)
{
    static const auto dud = []() -> void {};


    // Unsubscribe entity's event handlers.
    UI_Mouse_Event_Handlers * ui_mouse_event_handlers = entity_ui_mouse_event_handlers[entity];
    ui_mouse_event_handlers->mouse_enter_handler = dud;
    ui_mouse_event_handlers->mouse_exit_handler = dud;
    ui_mouse_event_handlers->mouse_button_handlers[Mouse_Buttons::LEFT][Key_Actions::PRESS] = dud;
    ui_mouse_event_handlers->mouse_button_handlers[Mouse_Buttons::LEFT][Key_Actions::RELEASE] = dud;
    remove(entity_ui_mouse_event_handlers, entity);
}


} // namespace Nito
