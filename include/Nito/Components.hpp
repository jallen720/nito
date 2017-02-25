#pragma once


#include <string>
#include <map>
#include <functional>
#include <glm/glm.hpp>

#include "Nito/APIs/ECS.hpp"


namespace Nito
{


enum class Mouse_Buttons;
enum class Button_Actions;


struct Transform
{
    glm::vec3 position;
    glm::vec3 scale;
    float rotation;
};


struct UI_Transform
{
    glm::vec3 position;
    glm::vec3 anchor;
};


struct Camera
{
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
    bool render;
    std::string texture_path;
    std::string shader_pipeline_name;
};


struct UI_Mouse_Event_Handlers
{
    using Event_Handler = std::function<void()>;
    using Button_Handlers = std::map<Mouse_Buttons, std::map<Button_Actions, Event_Handler>>;

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
    glm::vec3 color;
    std::string value;
};


struct Circle_Collider
{
    float radius;
};


struct Line_Collider
{
    glm::vec3 begin;
    glm::vec3 end;
};


struct Polygon_Collider
{
    std::vector<glm::vec3> points;
    bool wrap;
};


} // namespace Nito
