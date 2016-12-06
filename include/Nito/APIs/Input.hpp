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


enum class Keys
{
    SPACE,
    APOSTROPHE,
    COMMA,
    MINUS,
    PERIOD,
    SLASH,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    SEMICOLON,
    EQUAL,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LEFT_BRACKET,
    BACKSLASH,
    RIGHT_BRACKET,
    GRAVE_ACCENT,
    WORLD_1,
    WORLD_2,
    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,
    INSERT,
    DELETE,
    RIGHT,
    LEFT,
    DOWN,
    UP,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    CAPS_LOCK,
    SCROLL_LOCK,
    NUM_LOCK,
    PRINT_SCREEN,
    PAUSE,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    NUMPAD_0,
    NUMPAD_1,
    NUMPAD_2,
    NUMPAD_3,
    NUMPAD_4,
    NUMPAD_5,
    NUMPAD_6,
    NUMPAD_7,
    NUMPAD_8,
    NUMPAD_9,
    NUMPAD_DECIMAL,
    NUMPAD_DIVIDE,
    NUMPAD_MULTIPLY,
    NUMPAD_SUBTRACT,
    NUMPAD_ADD,
    NUMPAD_ENTER,
    NUMPAD_EQUAL,
    LEFT_SHIFT,
    LEFT_CONTROL,
    LEFT_ALT,
    LEFT_SUPER,
    RIGHT_SHIFT,
    RIGHT_CONTROL,
    RIGHT_ALT,
    RIGHT_SUPER,
    MENU,
};


enum class Button_Actions
{
    PRESS,
    REPEAT,
    RELEASE,
};


enum class Controller_Axes
{
    LEFT_STICK_X,
    LEFT_STICK_Y,
    RIGHT_STICK_X,
    RIGHT_STICK_Y,
};


using Mouse_Position_Handler = std::function<void(const glm::dvec2 &)>;
using Mouse_Button_Handler = std::function<void(const Mouse_Buttons, const Button_Actions)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void input_init();
void set_mouse_position_handler(const std::string & name, const Mouse_Position_Handler & mouse_position_handler);
void set_mouse_button_handler(const std::string & name, const Mouse_Button_Handler & mouse_button_handler);
Button_Actions get_key_button_action(const Keys key);
Button_Actions get_controller_button_action(const int controller_button, const int controller = GLFW_JOYSTICK_1);
float get_controller_axis(const Controller_Axes controller_axis, const int controller = GLFW_JOYSTICK_1);
void debug_controllers();


} // namespace Nito
