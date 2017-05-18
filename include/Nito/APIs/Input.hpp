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


enum class DS4_Axes
{
    LEFT_STICK_X,
    LEFT_STICK_Y,
    RIGHT_STICK_X,
    L2,
    R2,
    RIGHT_STICK_Y,
    D_PAD_X,
    D_PAD_Y,
    UNKNOWN_8,
    TOUCHPAD_X,
    TOUCHPAD_Y,
    UNKNOWN_11,
};


enum class DS4_Buttons
{
    SQUARE,
    X,
    CIRCLE,
    TRIANGLE,
    L1,
    R1,
    L2,
    R2,
    SHARE,
    START,
    L3,
    R3,
    PS,
    TOUCHPAD,
};


using Mouse_Position_Handler = std::function<void(const glm::dvec2 &)>;
using Mouse_Button_Handler = std::function<void(Mouse_Buttons, Button_Actions)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void input_api_init();
void input_api_update();

void set_key_handler(
    const std::string & id,
    Keys key,
    Button_Actions button_action,
    const std::function<void()> & handler);

void set_controller_button_handler(
    const std::string & id,
    int button,
    Button_Actions button_action,
    const std::function<void()> & handler,
    int controller = GLFW_JOYSTICK_1);

void set_controller_button_handler(
    const std::string & id,
    DS4_Buttons button,
    Button_Actions button_action,
    const std::function<void()> & handler,
    int controller = GLFW_JOYSTICK_1);

void set_mouse_position_handler(const std::string & id, const Mouse_Position_Handler & mouse_position_handler);
void set_mouse_button_handler(const std::string & id, const Mouse_Button_Handler & mouse_button_handler);
void remove_key_handler(const std::string & id);
void remove_controller_button_handler(const std::string & id);
void remove_mouse_position_handler(const std::string & id);
void remove_mouse_button_handler(const std::string & id);
Button_Actions get_key_button_action(Keys key);
Button_Actions get_controller_button_action(int controller_button, int controller = GLFW_JOYSTICK_1);
Button_Actions get_controller_button_action(DS4_Buttons controller_button, int controller = GLFW_JOYSTICK_1);
float get_controller_axis(int controller_axis, int controller = GLFW_JOYSTICK_1);
float get_controller_axis(DS4_Axes controller_axis, int controller = GLFW_JOYSTICK_1);
const glm::dvec2 & get_mouse_position();
void set_mouse_visible(bool visible);
void debug_controllers();


} // namespace Nito
