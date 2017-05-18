#include "Nito/APIs/Input.hpp"

#include <vector>
#include <map>
#include <stdexcept>
#include <cstring>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/APIs/Window.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;
using std::function;

// glm/glm.hpp
using glm::dvec2;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::at_value;
using Cpp_Utils::remove;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Controller_State
{
    bool is_connected;
    int axis_count;
    int button_count;
    const float * axes;
    const unsigned char * buttons;
    unsigned char previous_buttons[16];
};


struct Button_Handler
{
    Button_Actions button_action;
    function<void()> callback;
};


struct Key_Handler
{
    Keys key;
    Button_Handler button_handler;
};


struct Controller_Button_Handler
{
    int controller;
    int button;
    Button_Handler button_handler;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static dvec2 mouse_position;
static vector<Controller_State> controller_states;

// Event handlers
static map<string, Key_Handler> key_handlers;
static map<string, Controller_Button_Handler> controller_button_handlers;
static map<string, Mouse_Position_Handler> mouse_position_handlers;
static map<string, Mouse_Button_Handler> mouse_button_handlers;


static const map<Keys, const int> KEYS
{
    { Keys::SPACE           , GLFW_KEY_SPACE         },
    { Keys::APOSTROPHE      , GLFW_KEY_APOSTROPHE    },
    { Keys::COMMA           , GLFW_KEY_COMMA         },
    { Keys::MINUS           , GLFW_KEY_MINUS         },
    { Keys::PERIOD          , GLFW_KEY_PERIOD        },
    { Keys::SLASH           , GLFW_KEY_SLASH         },
    { Keys::NUM_0           , GLFW_KEY_0             },
    { Keys::NUM_1           , GLFW_KEY_1             },
    { Keys::NUM_2           , GLFW_KEY_2             },
    { Keys::NUM_3           , GLFW_KEY_3             },
    { Keys::NUM_4           , GLFW_KEY_4             },
    { Keys::NUM_5           , GLFW_KEY_5             },
    { Keys::NUM_6           , GLFW_KEY_6             },
    { Keys::NUM_7           , GLFW_KEY_7             },
    { Keys::NUM_8           , GLFW_KEY_8             },
    { Keys::NUM_9           , GLFW_KEY_9             },
    { Keys::SEMICOLON       , GLFW_KEY_SEMICOLON     },
    { Keys::EQUAL           , GLFW_KEY_EQUAL         },
    { Keys::A               , GLFW_KEY_A             },
    { Keys::B               , GLFW_KEY_B             },
    { Keys::C               , GLFW_KEY_C             },
    { Keys::D               , GLFW_KEY_D             },
    { Keys::E               , GLFW_KEY_E             },
    { Keys::F               , GLFW_KEY_F             },
    { Keys::G               , GLFW_KEY_G             },
    { Keys::H               , GLFW_KEY_H             },
    { Keys::I               , GLFW_KEY_I             },
    { Keys::J               , GLFW_KEY_J             },
    { Keys::K               , GLFW_KEY_K             },
    { Keys::L               , GLFW_KEY_L             },
    { Keys::M               , GLFW_KEY_M             },
    { Keys::N               , GLFW_KEY_N             },
    { Keys::O               , GLFW_KEY_O             },
    { Keys::P               , GLFW_KEY_P             },
    { Keys::Q               , GLFW_KEY_Q             },
    { Keys::R               , GLFW_KEY_R             },
    { Keys::S               , GLFW_KEY_S             },
    { Keys::T               , GLFW_KEY_T             },
    { Keys::U               , GLFW_KEY_U             },
    { Keys::V               , GLFW_KEY_V             },
    { Keys::W               , GLFW_KEY_W             },
    { Keys::X               , GLFW_KEY_X             },
    { Keys::Y               , GLFW_KEY_Y             },
    { Keys::Z               , GLFW_KEY_Z             },
    { Keys::LEFT_BRACKET    , GLFW_KEY_LEFT_BRACKET  },
    { Keys::BACKSLASH       , GLFW_KEY_BACKSLASH     },
    { Keys::RIGHT_BRACKET   , GLFW_KEY_RIGHT_BRACKET },
    { Keys::GRAVE_ACCENT    , GLFW_KEY_GRAVE_ACCENT  },
    { Keys::WORLD_1         , GLFW_KEY_WORLD_1       },
    { Keys::WORLD_2         , GLFW_KEY_WORLD_2       },
    { Keys::ESCAPE          , GLFW_KEY_ESCAPE        },
    { Keys::ENTER           , GLFW_KEY_ENTER         },
    { Keys::TAB             , GLFW_KEY_TAB           },
    { Keys::BACKSPACE       , GLFW_KEY_BACKSPACE     },
    { Keys::INSERT          , GLFW_KEY_INSERT        },
    { Keys::DELETE          , GLFW_KEY_DELETE        },
    { Keys::RIGHT           , GLFW_KEY_RIGHT         },
    { Keys::LEFT            , GLFW_KEY_LEFT          },
    { Keys::DOWN            , GLFW_KEY_DOWN          },
    { Keys::UP              , GLFW_KEY_UP            },
    { Keys::PAGE_UP         , GLFW_KEY_PAGE_UP       },
    { Keys::PAGE_DOWN       , GLFW_KEY_PAGE_DOWN     },
    { Keys::HOME            , GLFW_KEY_HOME          },
    { Keys::END             , GLFW_KEY_END           },
    { Keys::CAPS_LOCK       , GLFW_KEY_CAPS_LOCK     },
    { Keys::SCROLL_LOCK     , GLFW_KEY_SCROLL_LOCK   },
    { Keys::NUM_LOCK        , GLFW_KEY_NUM_LOCK      },
    { Keys::PRINT_SCREEN    , GLFW_KEY_PRINT_SCREEN  },
    { Keys::PAUSE           , GLFW_KEY_PAUSE         },
    { Keys::F1              , GLFW_KEY_F1            },
    { Keys::F2              , GLFW_KEY_F2            },
    { Keys::F3              , GLFW_KEY_F3            },
    { Keys::F4              , GLFW_KEY_F4            },
    { Keys::F5              , GLFW_KEY_F5            },
    { Keys::F6              , GLFW_KEY_F6            },
    { Keys::F7              , GLFW_KEY_F7            },
    { Keys::F8              , GLFW_KEY_F8            },
    { Keys::F9              , GLFW_KEY_F9            },
    { Keys::F10             , GLFW_KEY_F10           },
    { Keys::F11             , GLFW_KEY_F11           },
    { Keys::F12             , GLFW_KEY_F12           },
    { Keys::F13             , GLFW_KEY_F13           },
    { Keys::F14             , GLFW_KEY_F14           },
    { Keys::F15             , GLFW_KEY_F15           },
    { Keys::F16             , GLFW_KEY_F16           },
    { Keys::F17             , GLFW_KEY_F17           },
    { Keys::F18             , GLFW_KEY_F18           },
    { Keys::F19             , GLFW_KEY_F19           },
    { Keys::F20             , GLFW_KEY_F20           },
    { Keys::F21             , GLFW_KEY_F21           },
    { Keys::F22             , GLFW_KEY_F22           },
    { Keys::F23             , GLFW_KEY_F23           },
    { Keys::F24             , GLFW_KEY_F24           },
    { Keys::F25             , GLFW_KEY_F25           },
    { Keys::NUMPAD_0        , GLFW_KEY_KP_0          },
    { Keys::NUMPAD_1        , GLFW_KEY_KP_1          },
    { Keys::NUMPAD_2        , GLFW_KEY_KP_2          },
    { Keys::NUMPAD_3        , GLFW_KEY_KP_3          },
    { Keys::NUMPAD_4        , GLFW_KEY_KP_4          },
    { Keys::NUMPAD_5        , GLFW_KEY_KP_5          },
    { Keys::NUMPAD_6        , GLFW_KEY_KP_6          },
    { Keys::NUMPAD_7        , GLFW_KEY_KP_7          },
    { Keys::NUMPAD_8        , GLFW_KEY_KP_8          },
    { Keys::NUMPAD_9        , GLFW_KEY_KP_9          },
    { Keys::NUMPAD_DECIMAL  , GLFW_KEY_KP_DECIMAL    },
    { Keys::NUMPAD_DIVIDE   , GLFW_KEY_KP_DIVIDE     },
    { Keys::NUMPAD_MULTIPLY , GLFW_KEY_KP_MULTIPLY   },
    { Keys::NUMPAD_SUBTRACT , GLFW_KEY_KP_SUBTRACT   },
    { Keys::NUMPAD_ADD      , GLFW_KEY_KP_ADD        },
    { Keys::NUMPAD_ENTER    , GLFW_KEY_KP_ENTER      },
    { Keys::NUMPAD_EQUAL    , GLFW_KEY_KP_EQUAL      },
    { Keys::LEFT_SHIFT      , GLFW_KEY_LEFT_SHIFT    },
    { Keys::LEFT_CONTROL    , GLFW_KEY_LEFT_CONTROL  },
    { Keys::LEFT_ALT        , GLFW_KEY_LEFT_ALT      },
    { Keys::LEFT_SUPER      , GLFW_KEY_LEFT_SUPER    },
    { Keys::RIGHT_SHIFT     , GLFW_KEY_RIGHT_SHIFT   },
    { Keys::RIGHT_CONTROL   , GLFW_KEY_RIGHT_CONTROL },
    { Keys::RIGHT_ALT       , GLFW_KEY_RIGHT_ALT     },
    { Keys::RIGHT_SUPER     , GLFW_KEY_RIGHT_SUPER   },
    { Keys::MENU            , GLFW_KEY_MENU          },
};


static map<Button_Actions, const int> button_actions
{
    { Button_Actions::RELEASE , GLFW_RELEASE },
    { Button_Actions::PRESS   , GLFW_PRESS   },
    { Button_Actions::REPEAT  , GLFW_REPEAT  },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void window_key_handler(GLFWwindow * /*window*/, int key, int /*scan_code*/, int action, int /*mods*/)
{
    for_each(key_handlers, [=](const string & /*id*/, const Key_Handler & key_handler) -> void
    {
        if (key_handler.key == at_value(KEYS, key))
        {
            const Button_Handler & button_handler = key_handler.button_handler;

            if (button_handler.button_action == at_value(button_actions, action))
            {
                button_handler.callback();
            }
        }
    });
}


static void window_mouse_position_handler(GLFWwindow * /*window*/, double x_position, double y_position)
{
    mouse_position.x = x_position;

    // Invert y position to match coordinate system.
    mouse_position.y = get_window_size().y - y_position;

    // Trigger mouse position handlers with newly updated mouse position.
    for_each(
        mouse_position_handlers,
        [&](const string & /*id*/, const Mouse_Position_Handler & mouse_position_handler) -> void
        {
            mouse_position_handler(mouse_position);
        });
}


static void window_mouse_button_handler(GLFWwindow * /*window*/, int button, int action, int /*mods*/)
{
    static map<int, const Mouse_Buttons> mouse_buttons
    {
        { GLFW_MOUSE_BUTTON_RIGHT  , Mouse_Buttons::RIGHT  },
        { GLFW_MOUSE_BUTTON_MIDDLE , Mouse_Buttons::MIDDLE },
        { GLFW_MOUSE_BUTTON_LEFT   , Mouse_Buttons::LEFT   },
    };

    for_each(
        mouse_button_handlers,
        [&](const string & /*id*/, const Mouse_Button_Handler & mouse_button_handler) -> void
        {
            mouse_button_handler(mouse_buttons.at(button), at_value(button_actions, action));
        });
}


static void window_created_handler()
{
    set_window_key_handler(window_key_handler);
    set_window_mouse_position_handler(window_mouse_position_handler);
    set_window_mouse_button_handler(window_mouse_button_handler);
}


template<typename T>
static void validate_handler_not_set(const map<string, T> & handlers, const string & name, const string & id)
{
    if (contains_key(handlers, id))
    {
        throw runtime_error("ERROR: " + name + " handlers already contains a handler with id \"" + id + "\"!");
    }
}


static void trigger_controller_button_handlers(int controller, int button, unsigned char action)
{
    for_each(
        controller_button_handlers,
        [=](const string & /*id*/, const Controller_Button_Handler & controller_button_handler) -> void
        {
            if (controller_button_handler.controller == controller &&
                controller_button_handler.button == button)
            {
                const Button_Handler & button_handler = controller_button_handler.button_handler;

                if (button_handler.button_action == at_value(button_actions, (int)action))
                {
                    button_handler.callback();
                }
            }
        });
}


static void validate_controller_is_connected(int controller)
{
    if (!controller_states[controller].is_connected)
    {
        throw runtime_error("ERROR: controller " + to_string(controller) + " is not connected!");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void input_api_init()
{
    // Add window_created_handler() so when the window is created it can set the window input handlers.
    add_window_created_handler(window_created_handler);

    for (int controller = GLFW_JOYSTICK_1; controller < GLFW_JOYSTICK_LAST; controller++)
    {
        controller_states.push_back({});
    }
}


void input_api_update()
{
    for (int controller = GLFW_JOYSTICK_1; controller < GLFW_JOYSTICK_LAST; controller++)
    {
        // Update controller states.
        Controller_State & controller_state = controller_states[controller];
        controller_state.is_connected = glfwJoystickPresent(controller);

        if (!controller_state.is_connected)
        {
            continue;
        }

        controller_state.axes = glfwGetJoystickAxes(controller, &controller_state.axis_count);
        controller_state.buttons = glfwGetJoystickButtons(controller, &controller_state.button_count);


        // Check for state changes and fire their associated handlers.
        const int button_count = controller_state.button_count;

        for (int button = 0; button < button_count; button++)
        {
            const unsigned char button_state = controller_state.buttons[button];

            if (button_state != controller_state.previous_buttons[button])
            {
                trigger_controller_button_handlers(controller, button, button_state);
            }
        }


        // Record current controller button states into controller_state.previous_buttons.
        memcpy(
            controller_state.previous_buttons,
            controller_state.buttons,
            sizeof(unsigned char) * button_count);
    }
}


void set_key_handler(const string & id, Keys key, Button_Actions button_action, const function<void()> & handler)
{
    validate_handler_not_set(key_handlers, "key", id);

    key_handlers[id] =
    {
        key,
        {
            button_action,
            handler,
        },
    };
}


void set_controller_button_handler(
    const string & id,
    int button,
    Button_Actions button_action,
    const function<void()> & handler,
    int controller)
{
    validate_handler_not_set(controller_button_handlers, "controller button", id);

    controller_button_handlers[id] =
    {
        controller,
        button,
        {
            button_action,
            handler,
        },
    };
}


void set_controller_button_handler(
    const string & id,
    DS4_Buttons button,
    Button_Actions button_action,
    const function<void()> & handler,
    int controller)
{
    set_controller_button_handler(id, (int)button, button_action, handler, controller);
}


void set_mouse_position_handler(const string & id, const Mouse_Position_Handler & mouse_position_handler)
{
    validate_handler_not_set(mouse_position_handlers, "mouse position", id);
    mouse_position_handlers[id] = mouse_position_handler;
}


void set_mouse_button_handler(const string & id, const Mouse_Button_Handler & mouse_button_handler)
{
    validate_handler_not_set(mouse_button_handlers, "mouse button", id);
    mouse_button_handlers[id] = mouse_button_handler;
}


void remove_key_handler(const std::string & id)
{
    remove(key_handlers, id);
}


void remove_controller_button_handler(const std::string & id)
{
    remove(controller_button_handlers, id);
}


void remove_mouse_position_handler(const std::string & id)
{
    remove(mouse_position_handlers, id);
}


void remove_mouse_button_handler(const std::string & id)
{
    remove(mouse_button_handlers, id);
}


Button_Actions get_key_button_action(Keys key)
{
    return at_value(button_actions, get_window_key_button_action(KEYS.at(key)));
}


Button_Actions get_controller_button_action(int controller_button, int controller)
{
    validate_controller_is_connected(controller);
    const Controller_State & controller_state = controller_states[controller];
    const int button_count = controller_state.button_count;

    if (controller_button >= button_count)
    {
        throw runtime_error(
            "ERROR: button " + to_string(controller_button) + " is out of range for the button count " +
            to_string(button_count) + " of controller " + to_string(controller) + "!");
    }

    return at_value(button_actions, (int)controller_state.buttons[controller_button]);
}


Button_Actions get_controller_button_action(DS4_Buttons controller_button, int controller)
{
    return get_controller_button_action((int)controller_button, controller);
}


float get_controller_axis(int controller_axis, int controller)
{
    if (!controller_states[controller].is_connected)
    {
        return 0.0f;
    }

    validate_controller_is_connected(controller);
    const Controller_State & controller_state = controller_states[controller];
    const int axis_count = controller_state.axis_count;

    if (controller_axis >= axis_count)
    {
        throw runtime_error(
            "ERROR: axis index " + to_string(controller_axis) + " is out of range for the axis count " +
            to_string(axis_count) + " of controller " + to_string(controller) + "!");
    }

    return controller_state.axes[controller_axis];
}


float get_controller_axis(DS4_Axes controller_axis, int controller)
{
    return get_controller_axis((int)controller_axis, controller);
}


const dvec2 & get_mouse_position()
{
    return mouse_position;
}


void set_mouse_visible(bool visible)
{
    set_input_mode(GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}


void debug_controllers()
{
    for (int controller = GLFW_JOYSTICK_1; controller < GLFW_JOYSTICK_LAST; controller++)
    {
        Controller_State & controller_state = controller_states[controller];

        if (!controller_state.is_connected)
        {
            continue;
        }

        printf("controller %d:\n", controller);

        for (int axis = 0; axis < controller_state.axis_count; axis++)
        {
            printf("    axis %d: %f\n", axis, controller_state.axes[axis]);
        }

        for (int button = 0; button < controller_state.button_count; button++)
        {
            printf("    button %d: %d\n", button, controller_state.buttons[button]);
        }

        puts("");
    }
}


} // namespace Nito
