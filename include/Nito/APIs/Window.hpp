#pragma once


#include <string>
#include <map>
#include <functional>
#include <glm/glm.hpp>


struct GLFWwindow;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Window_Config
{
    const int width;
    const int height;
    const std::string title;
    const std::string refresh_rate;
    const std::map<std::string, int> hints;
};


using Window_Created_Handler = std::function<void()>;
using Window_Key_Handler = void (*)(GLFWwindow *, int, int, int, int);
using Window_Mouse_Position_Handler = void (*)(GLFWwindow *, double, double);
using Window_Mouse_Button_Handler = void (*)(GLFWwindow *, int, int, int);
using Window_Loop_Callback = std::function<void()>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_glfw();
void create_window(const Window_Config & window_config);
void close_window();
float get_time();
float get_delta_time();
const glm::vec3 & get_window_size();
int get_window_key_button_action(int key);
void add_window_created_handler(const Window_Created_Handler & window_created_handler);
void set_window_key_handler(const Window_Key_Handler window_key_handler);
void set_window_mouse_position_handler(const Window_Mouse_Position_Handler window_mouse_position_handler);
void set_window_mouse_button_handler(const Window_Mouse_Button_Handler & window_mouse_button_handler);
void set_input_mode(int mode, int value);
void run_window_loop(const Window_Loop_Callback & callback);
void terminate_glfw();


} // namespace Nito
