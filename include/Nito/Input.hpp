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
using Control_Handler = std::function<void(GLFWwindow *, const int, const int)>;
using Mouse_Move_Handler = std::function<void(const glm::dvec2 &)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void add_control_binding(const std::string & key, const std::string & action, const std::string & handler);
void set_control_handler(const std::string & name, const Control_Handler & control_handler);
void key_callback(GLFWwindow * window, int key, int scan_code, int action, int mods);
void mouse_position_callback(GLFWwindow * /*window*/, double x_position, double y_position);
void set_mouse_move_handler(const std::string & name, const Mouse_Move_Handler & mouse_move_handler);


} // namespace Nito
