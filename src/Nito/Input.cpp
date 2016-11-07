#include "Nito/Input.hpp"

#include <vector>
#include <map>
#include <stdexcept>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Window.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;

// glm/glm.hpp
using glm::dvec2;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::at_value;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Control_Binding
{
    const int key;
    const int action;
    const Control_Handler & handler;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Control_Binding> control_bindings;
static map<string, Control_Handler> control_handlers;
static dvec2 mouse_position;

// Event handlers
static map<string, Mouse_Move_Handler> mouse_move_handlers;
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


static map<Key_Actions, const int> key_actions
{
    { Key_Actions::RELEASE , GLFW_RELEASE },
    { Key_Actions::PRESS   , GLFW_PRESS   },
    { Key_Actions::REPEAT  , GLFW_REPEAT  },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_key_handler(GLFWwindow * window, int key, int /*scan_code*/, int action, int /*mods*/)
{
    for (const Control_Binding & control_binding : control_bindings)
    {
        if (control_binding.key == key &&
            control_binding.action == action)
        {
            control_binding.handler(window, key, action);
        }
    }
}


void window_mouse_position_handler(GLFWwindow * /*window*/, double x_position, double y_position)
{
    mouse_position.x = x_position;

    // Invert y position to match coordinate system.
    mouse_position.y = get_window_size().y - y_position;

    // Trigger mouse move handlers with newly updated mouse position.
    for_each(mouse_move_handlers, [&](const string & /*name*/, const Mouse_Move_Handler & mouse_move_handler) -> void
    {
        mouse_move_handler(mouse_position);
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
        const string & /*name*/,
        const Mouse_Button_Handler & mouse_button_handler) -> void
    {
        mouse_button_handler(mouse_buttons.at(button), at_value(key_actions, action));
    });
}


void window_created_handler()
{
    set_window_key_handler(window_key_handler);
    set_window_mouse_position_handler(window_mouse_position_handler);
    set_window_mouse_button_handler(window_mouse_button_handler);
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


void add_control_binding(const string & key, const string & action, const string & handler)
{
    static const map<string, const Keys> key_mappings
    {
        { "space"             , Keys::SPACE           },
        { "apostrophe"        , Keys::APOSTROPHE      },
        { "comma"             , Keys::COMMA           },
        { "minus"             , Keys::MINUS           },
        { "period"            , Keys::PERIOD          },
        { "slash"             , Keys::SLASH           },
        { "num_0"             , Keys::NUM_0           },
        { "num_1"             , Keys::NUM_1           },
        { "num_2"             , Keys::NUM_2           },
        { "num_3"             , Keys::NUM_3           },
        { "num_4"             , Keys::NUM_4           },
        { "num_5"             , Keys::NUM_5           },
        { "num_6"             , Keys::NUM_6           },
        { "num_7"             , Keys::NUM_7           },
        { "num_8"             , Keys::NUM_8           },
        { "num_9"             , Keys::NUM_9           },
        { "semicolon"         , Keys::SEMICOLON       },
        { "equal"             , Keys::EQUAL           },
        { "a"                 , Keys::A               },
        { "b"                 , Keys::B               },
        { "c"                 , Keys::C               },
        { "d"                 , Keys::D               },
        { "e"                 , Keys::E               },
        { "f"                 , Keys::F               },
        { "g"                 , Keys::G               },
        { "h"                 , Keys::H               },
        { "i"                 , Keys::I               },
        { "j"                 , Keys::J               },
        { "k"                 , Keys::K               },
        { "l"                 , Keys::L               },
        { "m"                 , Keys::M               },
        { "n"                 , Keys::N               },
        { "o"                 , Keys::O               },
        { "p"                 , Keys::P               },
        { "q"                 , Keys::Q               },
        { "r"                 , Keys::R               },
        { "s"                 , Keys::S               },
        { "t"                 , Keys::T               },
        { "u"                 , Keys::U               },
        { "v"                 , Keys::V               },
        { "w"                 , Keys::W               },
        { "x"                 , Keys::X               },
        { "y"                 , Keys::Y               },
        { "z"                 , Keys::Z               },
        { "left_bracket"      , Keys::LEFT_BRACKET    },
        { "backslash"         , Keys::BACKSLASH       },
        { "right_bracket"     , Keys::RIGHT_BRACKET   },
        { "grave_accent"      , Keys::GRAVE_ACCENT    },
        { "world_1"           , Keys::WORLD_1         },
        { "world_2"           , Keys::WORLD_2         },
        { "escape"            , Keys::ESCAPE          },
        { "enter"             , Keys::ENTER           },
        { "tab"               , Keys::TAB             },
        { "backspace"         , Keys::BACKSPACE       },
        { "insert"            , Keys::INSERT          },
        { "delete"            , Keys::DELETE          },
        { "right"             , Keys::RIGHT           },
        { "left"              , Keys::LEFT            },
        { "down"              , Keys::DOWN            },
        { "up"                , Keys::UP              },
        { "page_up"           , Keys::PAGE_UP         },
        { "page_down"         , Keys::PAGE_DOWN       },
        { "home"              , Keys::HOME            },
        { "end"               , Keys::END             },
        { "caps_lock"         , Keys::CAPS_LOCK       },
        { "scroll_lock"       , Keys::SCROLL_LOCK     },
        { "num_lock"          , Keys::NUM_LOCK        },
        { "print_screen"      , Keys::PRINT_SCREEN    },
        { "pause"             , Keys::PAUSE           },
        { "f1"                , Keys::F1              },
        { "f2"                , Keys::F2              },
        { "f3"                , Keys::F3              },
        { "f4"                , Keys::F4              },
        { "f5"                , Keys::F5              },
        { "f6"                , Keys::F6              },
        { "f7"                , Keys::F7              },
        { "f8"                , Keys::F8              },
        { "f9"                , Keys::F9              },
        { "f10"               , Keys::F10             },
        { "f11"               , Keys::F11             },
        { "f12"               , Keys::F12             },
        { "f13"               , Keys::F13             },
        { "f14"               , Keys::F14             },
        { "f15"               , Keys::F15             },
        { "f16"               , Keys::F16             },
        { "f17"               , Keys::F17             },
        { "f18"               , Keys::F18             },
        { "f19"               , Keys::F19             },
        { "f20"               , Keys::F20             },
        { "f21"               , Keys::F21             },
        { "f22"               , Keys::F22             },
        { "f23"               , Keys::F23             },
        { "f24"               , Keys::F24             },
        { "f25"               , Keys::F25             },
        { "numpad_0"          , Keys::NUMPAD_0        },
        { "numpad_1"          , Keys::NUMPAD_1        },
        { "numpad_2"          , Keys::NUMPAD_2        },
        { "numpad_3"          , Keys::NUMPAD_3        },
        { "numpad_4"          , Keys::NUMPAD_4        },
        { "numpad_5"          , Keys::NUMPAD_5        },
        { "numpad_6"          , Keys::NUMPAD_6        },
        { "numpad_7"          , Keys::NUMPAD_7        },
        { "numpad_8"          , Keys::NUMPAD_8        },
        { "numpad_9"          , Keys::NUMPAD_9        },
        { "numpad_decimal"    , Keys::NUMPAD_DECIMAL  },
        { "numpad_divide"     , Keys::NUMPAD_DIVIDE   },
        { "numpad_multiply"   , Keys::NUMPAD_MULTIPLY },
        { "numpad_subtract"   , Keys::NUMPAD_SUBTRACT },
        { "numpad_add"        , Keys::NUMPAD_ADD      },
        { "numpad_enter"      , Keys::NUMPAD_ENTER    },
        { "numpad_equal"      , Keys::NUMPAD_EQUAL    },
        { "left_shift"        , Keys::LEFT_SHIFT      },
        { "left_control"      , Keys::LEFT_CONTROL    },
        { "left_alt"          , Keys::LEFT_ALT        },
        { "left_super"        , Keys::LEFT_SUPER      },
        { "right_shift"       , Keys::RIGHT_SHIFT     },
        { "right_control"     , Keys::RIGHT_CONTROL   },
        { "right_alt"         , Keys::RIGHT_ALT       },
        { "right_super"       , Keys::RIGHT_SUPER     },
        { "menu"              , Keys::MENU            },
    };

    static const map<string, const Key_Actions> key_action_mappings
    {
        { "release" , Key_Actions::RELEASE },
        { "press"   , Key_Actions::PRESS   },
        { "repeat"  , Key_Actions::REPEAT  },
    };


    // Validations
    if (!contains_key(key_mappings, key))
    {
        throw runtime_error("ERROR: \"" + key + "\" is not a valid key!");
    }

    if (!contains_key(key_action_mappings, action))
    {
        throw runtime_error("ERROR: \"" + action + "\" is not a valid key action!");
    }

    if (!contains_key(control_handlers, handler))
    {
        throw runtime_error("ERROR: \"" + handler + "\" is not a registered control handler!");
    }


    control_bindings.push_back(
        {
            keys.at(key_mappings.at(key)),
            key_actions.at(key_action_mappings.at(action)),
            control_handlers.at(handler),
        });
}


void set_control_handler(const string & name, const Control_Handler & control_handler)
{
    control_handlers[name] = control_handler;
}


void set_mouse_move_handler(const string & name, const Mouse_Move_Handler & mouse_move_handler)
{
    mouse_move_handlers[name] = mouse_move_handler;
}


void set_mouse_button_handler(const string & name, const Mouse_Button_Handler & mouse_button_handler)
{
    mouse_button_handlers[name] = mouse_button_handler;
}


Key_Actions get_key_action(const Keys key)
{
    return at_value(key_actions, glfwGetKey(*get_window(), keys.at(key)));
}


} // namespace Nito
