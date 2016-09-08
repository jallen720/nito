#include "Nito/Input.hpp"

#include <vector>
#include <map>
#include <stdexcept>
#include "CppUtils/MapUtils/containsKey.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;
using CppUtils::containsKey;


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void add_control_binding(const string & key, const string & action, const string & handler)
{
    static const map<string, const int> glfw_key_codes
    {
        { "space"         , GLFW_KEY_SPACE         },
        { "apostrophe"    , GLFW_KEY_APOSTROPHE    },
        { "comma"         , GLFW_KEY_COMMA         },
        { "minus"         , GLFW_KEY_MINUS         },
        { "period"        , GLFW_KEY_PERIOD        },
        { "slash"         , GLFW_KEY_SLASH         },
        { "0"             , GLFW_KEY_0             },
        { "1"             , GLFW_KEY_1             },
        { "2"             , GLFW_KEY_2             },
        { "3"             , GLFW_KEY_3             },
        { "4"             , GLFW_KEY_4             },
        { "5"             , GLFW_KEY_5             },
        { "6"             , GLFW_KEY_6             },
        { "7"             , GLFW_KEY_7             },
        { "8"             , GLFW_KEY_8             },
        { "9"             , GLFW_KEY_9             },
        { "semicolon"     , GLFW_KEY_SEMICOLON     },
        { "equal"         , GLFW_KEY_EQUAL         },
        { "a"             , GLFW_KEY_A             },
        { "b"             , GLFW_KEY_B             },
        { "c"             , GLFW_KEY_C             },
        { "d"             , GLFW_KEY_D             },
        { "e"             , GLFW_KEY_E             },
        { "f"             , GLFW_KEY_F             },
        { "g"             , GLFW_KEY_G             },
        { "h"             , GLFW_KEY_H             },
        { "i"             , GLFW_KEY_I             },
        { "j"             , GLFW_KEY_J             },
        { "k"             , GLFW_KEY_K             },
        { "l"             , GLFW_KEY_L             },
        { "m"             , GLFW_KEY_M             },
        { "n"             , GLFW_KEY_N             },
        { "o"             , GLFW_KEY_O             },
        { "p"             , GLFW_KEY_P             },
        { "q"             , GLFW_KEY_Q             },
        { "r"             , GLFW_KEY_R             },
        { "s"             , GLFW_KEY_S             },
        { "t"             , GLFW_KEY_T             },
        { "u"             , GLFW_KEY_U             },
        { "v"             , GLFW_KEY_V             },
        { "w"             , GLFW_KEY_W             },
        { "x"             , GLFW_KEY_X             },
        { "y"             , GLFW_KEY_Y             },
        { "z"             , GLFW_KEY_Z             },
        { "left-bracket"  , GLFW_KEY_LEFT_BRACKET  },
        { "backslash"     , GLFW_KEY_BACKSLASH     },
        { "right-bracket" , GLFW_KEY_RIGHT_BRACKET },
        { "grave-accent"  , GLFW_KEY_GRAVE_ACCENT  },
        { "world-1"       , GLFW_KEY_WORLD_1       },
        { "world-2"       , GLFW_KEY_WORLD_2       },
        { "escape"        , GLFW_KEY_ESCAPE        },
        { "enter"         , GLFW_KEY_ENTER         },
        { "tab"           , GLFW_KEY_TAB           },
        { "backspace"     , GLFW_KEY_BACKSPACE     },
        { "insert"        , GLFW_KEY_INSERT        },
        { "delete"        , GLFW_KEY_DELETE        },
        { "right"         , GLFW_KEY_RIGHT         },
        { "left"          , GLFW_KEY_LEFT          },
        { "down"          , GLFW_KEY_DOWN          },
        { "up"            , GLFW_KEY_UP            },
        { "page_up"       , GLFW_KEY_PAGE_UP       },
        { "page_down"     , GLFW_KEY_PAGE_DOWN     },
        { "home"          , GLFW_KEY_HOME          },
        { "end"           , GLFW_KEY_END           },
        { "caps-lock"     , GLFW_KEY_CAPS_LOCK     },
        { "scroll-lock"   , GLFW_KEY_SCROLL_LOCK   },
        { "num-lock"      , GLFW_KEY_NUM_LOCK      },
        { "print-screen"  , GLFW_KEY_PRINT_SCREEN  },
        { "pause"         , GLFW_KEY_PAUSE         },
        { "f1"            , GLFW_KEY_F1            },
        { "f2"            , GLFW_KEY_F2            },
        { "f3"            , GLFW_KEY_F3            },
        { "f4"            , GLFW_KEY_F4            },
        { "f5"            , GLFW_KEY_F5            },
        { "f6"            , GLFW_KEY_F6            },
        { "f7"            , GLFW_KEY_F7            },
        { "f8"            , GLFW_KEY_F8            },
        { "f9"            , GLFW_KEY_F9            },
        { "f10"           , GLFW_KEY_F10           },
        { "f11"           , GLFW_KEY_F11           },
        { "f12"           , GLFW_KEY_F12           },
        { "f13"           , GLFW_KEY_F13           },
        { "f14"           , GLFW_KEY_F14           },
        { "f15"           , GLFW_KEY_F15           },
        { "f16"           , GLFW_KEY_F16           },
        { "f17"           , GLFW_KEY_F17           },
        { "f18"           , GLFW_KEY_F18           },
        { "f19"           , GLFW_KEY_F19           },
        { "f20"           , GLFW_KEY_F20           },
        { "f21"           , GLFW_KEY_F21           },
        { "f22"           , GLFW_KEY_F22           },
        { "f23"           , GLFW_KEY_F23           },
        { "f24"           , GLFW_KEY_F24           },
        { "f25"           , GLFW_KEY_F25           },
        { "kp-0"          , GLFW_KEY_KP_0          },
        { "kp-1"          , GLFW_KEY_KP_1          },
        { "kp-2"          , GLFW_KEY_KP_2          },
        { "kp-3"          , GLFW_KEY_KP_3          },
        { "kp-4"          , GLFW_KEY_KP_4          },
        { "kp-5"          , GLFW_KEY_KP_5          },
        { "kp-6"          , GLFW_KEY_KP_6          },
        { "kp-7"          , GLFW_KEY_KP_7          },
        { "kp-8"          , GLFW_KEY_KP_8          },
        { "kp-9"          , GLFW_KEY_KP_9          },
        { "kp-decimal"    , GLFW_KEY_KP_DECIMAL    },
        { "kp-divide"     , GLFW_KEY_KP_DIVIDE     },
        { "kp-multiply"   , GLFW_KEY_KP_MULTIPLY   },
        { "kp-subtract"   , GLFW_KEY_KP_SUBTRACT   },
        { "kp-add"        , GLFW_KEY_KP_ADD        },
        { "kp-enter"      , GLFW_KEY_KP_ENTER      },
        { "kp-equal"      , GLFW_KEY_KP_EQUAL      },
        { "left-shift"    , GLFW_KEY_LEFT_SHIFT    },
        { "left-control"  , GLFW_KEY_LEFT_CONTROL  },
        { "left-alt"      , GLFW_KEY_LEFT_ALT      },
        { "left-super"    , GLFW_KEY_LEFT_SUPER    },
        { "right-shift"   , GLFW_KEY_RIGHT_SHIFT   },
        { "right-control" , GLFW_KEY_RIGHT_CONTROL },
        { "right-alt"     , GLFW_KEY_RIGHT_ALT     },
        { "right-super"   , GLFW_KEY_RIGHT_SUPER   },
        { "menu"          , GLFW_KEY_MENU          },
    };

    static const map<string, const int> glfw_key_actions
    {
        { "release" , GLFW_RELEASE },
        { "press"   , GLFW_PRESS   },
        { "repeat"  , GLFW_REPEAT  },
    };


    // Validations
    if (!containsKey(glfw_key_codes, key))
    {
        throw runtime_error("ERROR: \"" + key + "\" is not a valid key!");
    }

    if (!containsKey(glfw_key_actions, action))
    {
        throw runtime_error("ERROR: \"" + action + "\" is not a valid action!");
    }

    if (!containsKey(control_handlers, handler))
    {
        throw runtime_error("ERROR: \"" + handler + "\" is not a registered control handler!");
    }


    control_bindings.push_back(
        {
            glfw_key_codes.at(key),
            glfw_key_actions.at(action),
            control_handlers.at(handler),
        });
}


void set_control_handler(const string & name, const Control_Handler & control_handler)
{
    control_handlers[name] = control_handler;
}


void key_callback(GLFWwindow * window, int key, int /*scan_code*/, int action, int /*mods*/)
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


} // namespace Nito
