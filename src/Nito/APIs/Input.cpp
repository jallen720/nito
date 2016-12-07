#include "Nito/APIs/Input.hpp"

#include <vector>
#include <map>
#include <stdexcept>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/APIs/Window.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;

// glm/glm.hpp
using glm::dvec2;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::at_value;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static dvec2 mouse_position;

// Event handlers
static map<string, Key_Handler> key_handlers;
static map<string, Mouse_Position_Handler> mouse_position_handlers;
static map<string, Mouse_Button_Handler> mouse_button_handlers;


static const map<Keys, const int> keys
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


static map<Controller_Axes, const int> ds4_axes
{
    { Controller_Axes::LEFT_STICK_X  , 0 },
    { Controller_Axes::LEFT_STICK_Y  , 1 },
    { Controller_Axes::RIGHT_STICK_X , 2 },
    { Controller_Axes::RIGHT_STICK_Y , 5 },
};


static map<Controller_Axes, const string> controller_axis_names
{
    { Controller_Axes::LEFT_STICK_X  , "Controller_Axes::LEFT_STICK_X"  },
    { Controller_Axes::LEFT_STICK_Y  , "Controller_Axes::LEFT_STICK_Y"  },
    { Controller_Axes::RIGHT_STICK_X , "Controller_Axes::RIGHT_STICK_X" },
    { Controller_Axes::RIGHT_STICK_Y , "Controller_Axes::RIGHT_STICK_Y" },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_key_handler(GLFWwindow * /*window*/, int key, int /*scan_code*/, int action, int /*mods*/)
{
    for_each(key_handlers, [=](const string & /*id*/, const Key_Handler & key_handler) -> void
    {
        if (key_handler.key == at_value(keys, key) &&
            key_handler.button_action == at_value(button_actions, action))
        {
            key_handler.handler();
        }
    });
}


void window_mouse_position_handler(GLFWwindow * /*window*/, double x_position, double y_position)
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


void window_mouse_button_handler(GLFWwindow * /*window*/, int button, int action, int /*mods*/)
{
    static map<int, const Mouse_Buttons> mouse_buttons
    {
        { GLFW_MOUSE_BUTTON_RIGHT  , Mouse_Buttons::RIGHT  },
        { GLFW_MOUSE_BUTTON_MIDDLE , Mouse_Buttons::MIDDLE },
        { GLFW_MOUSE_BUTTON_LEFT   , Mouse_Buttons::LEFT   },
    };

    for_each(mouse_button_handlers, [&](
        const string & /*id*/,
        const Mouse_Button_Handler & mouse_button_handler) -> void
    {
        mouse_button_handler(mouse_buttons.at(button), at_value(button_actions, action));
    });
}


void window_created_handler()
{
    set_window_key_handler(window_key_handler);
    set_window_mouse_position_handler(window_mouse_position_handler);
    set_window_mouse_button_handler(window_mouse_button_handler);
}


template<typename T>
void validate_handler_not_set(const map<string, T> & handlers, const string & name, const string & id)
{
    if (contains_key(handlers, id))
    {
        throw runtime_error("ERROR: " + name + " handlers already contains a handler with id \"" + id + "\"!");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void input_init()
{
    add_window_created_handler(window_created_handler);
}


void set_key_handler(const string & id, const Key_Handler & key_handler)
{
    validate_handler_not_set(key_handlers, "key", id);
    key_handlers[id] = key_handler;
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


Button_Actions get_key_button_action(const Keys key)
{
    return at_value(button_actions, get_window_key_button_action(keys.at(key)));
}


Button_Actions get_controller_button_action(const int controller_button, const int controller)
{
    int button_count;
    const unsigned char * buttons = glfwGetJoystickButtons(controller, &button_count);

    if (controller_button >= button_count)
    {
        throw runtime_error(
            "ERROR: button " + to_string(controller_button) + " is out of range for the button count " +
            to_string(button_count) + " of controller " + to_string(controller) + "!");
    }

    return at_value(button_actions, (int)buttons[controller_button]);
}


float get_controller_axis(const Controller_Axes controller_axis, const int controller)
{
    int axis_count;
    const int axis_index = ds4_axes.at(controller_axis);
    const float * axes = glfwGetJoystickAxes(controller, &axis_count);

    if (axis_index >= axis_count)
    {
        throw runtime_error(
            "ERROR: axis index " + to_string(axis_index) + " for axis " + controller_axis_names.at(controller_axis) +
            " is out of range for the axis count " + to_string(axis_count) + " of controller " + to_string(controller) +
            "!");
    }

    return axes[axis_index];
}


void debug_controllers()
{
    vector<int> connected_controllers;

    for (int controller = GLFW_JOYSTICK_1; controller < GLFW_JOYSTICK_LAST; controller++)
    {
        if (glfwJoystickPresent(controller))
        {
            connected_controllers.push_back(controller);
        }
    }

    for (const int connected_controller : connected_controllers)
    {
        int axis_count;
        int button_count;
        const float * axes = glfwGetJoystickAxes(connected_controller, &axis_count);
        const unsigned char * buttons = glfwGetJoystickButtons(connected_controller, &button_count);
        printf("controller %d:\n", connected_controller);

        for (int axis = 0; axis < axis_count; axis++)
        {
            printf("    axis %d: %f\n", axis, axes[axis]);
        }

        for (int button = 0; button < button_count; button++)
        {
            printf("    button %d: %d\n", button, buttons[button]);
        }

        puts("");
    }
}


} // namespace Nito
