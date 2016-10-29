#pragma once


#include <string>
#include <map>
#include <functional>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace Nito
{


enum class Mouse_Buttons;
enum class Key_Actions;


struct Transform
{
    glm::vec3 position;
    glm::vec3 scale;
};


struct UI_Transform
{
    glm::vec3 position;
    glm::vec3 anchor;
};


struct Viewport
{
    GLint x;
    GLint y;
    float z_near;
    float z_far;
};


struct Dimensions
{
    float width;
    float height;
    glm::vec3 origin;
};


struct Sprite
{
    std::string texture_path;
    std::string shader_pipeline_name;
    Dimensions dimensions;
};


struct UI_Mouse_Event_Handlers
{
    using Event_Handler = std::function<void()>;
    using Button_Handlers = std::map<Mouse_Buttons, std::map<Key_Actions, Event_Handler>>;

    Event_Handler mouse_enter_handler;
    Event_Handler mouse_exit_handler;
    Button_Handlers mouse_button_handlers;
};


struct Button
{
    std::string hover_texture_path;
    std::string pressed_texture_path;
    std::function<void()> click_handler;
};


struct Text
{
    std::string font;
    std::string value;
};


} // namespace Nito
