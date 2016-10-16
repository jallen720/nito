#pragma once


#include <functional>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Mouse_Buttons
{
    LEFT,
    MIDDLE,
    RIGHT,
};


enum class Key_Actions
{
    PRESS,
    REPEAT,
    RELEASE,
};


using Control_Handler = std::function<void(GLFWwindow *, const int, const int)>;
using Mouse_Move_Handler = std::function<void(const glm::dvec2 &)>;
using Mouse_Button_Handler = std::function<void(const Mouse_Buttons, const Key_Actions)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void input_init();
void add_control_binding(const std::string & key, const std::string & action, const std::string & handler);
void set_control_handler(const std::string & name, const Control_Handler & control_handler);
void set_mouse_move_handler(const std::string & name, const Mouse_Move_Handler & mouse_move_handler);
void set_mouse_button_handler(const std::string & name, const Mouse_Button_Handler & mouse_button_handler);


} // namespace Nito
