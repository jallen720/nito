// Required before any other OpenGL includes
#include <GL/glew.h>

#include "Nito/Engine.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <glm/glm.hpp>
#include "Cpp_Utils/JSON.hpp"
#include "Cpp_Utils/File.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/String.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/Audio.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Resources.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/Systems/Button.hpp"
#include "Nito/Systems/Camera.hpp"
#include "Nito/Systems/Local_Transform.hpp"
#include "Nito/Systems/Renderer.hpp"
#include "Nito/Systems/Sprite_Dimensions_Handler.hpp"
#include "Nito/Systems/Text_Renderer.hpp"
#include "Nito/Systems/UI_Mouse_Event_Dispatcher.hpp"
#include "Nito/Systems/UI_Transform.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;
using Cpp_Utils::read_json_file;

// Cpp_Utils/File.hpp
using Cpp_Utils::read_file;
using Cpp_Utils::file_exists;
using Cpp_Utils::directify;

// Cpp_Utils/Container.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Fn.hpp
using Cpp_Utils::transform;

// Cpp_Utils/Map.hpp && Cpp_Utils/JSON.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string DEFAULT_SCENE_NAME = "default";


static const Component_Handlers TRANSFORM_COMPONENT_HANDLERS
{
    [](const JSON & data) -> Component
    {
        vec3 position;
        vec3 scale(1.0f);
        float rotation = 0.0f;

        if (contains_key(data, "position"))
        {
            const JSON & position_data = data["position"];
            position.x = position_data["x"];
            position.y = position_data["y"];

            if (contains_key(position_data, "z"))
            {
                position.z = position_data["z"];
            }
        }

        if (contains_key(data, "scale"))
        {
            const JSON & scale_data = data["scale"];
            scale.x = scale_data["x"];
            scale.y = scale_data["y"];
        }

        if (contains_key(data, "rotation"))
        {
            rotation = data["rotation"];
        }

        return new Transform
        {
            position,
            scale,
            rotation,
        };
    },
    get_component_deallocator<Transform>(),
};


static vector<Update_Handler> update_handlers;


static map<string, const System_Entity_Handlers> engine_system_entity_handlers
{
    {
        "button",
        {
            button_subscribe,
            button_unsubscribe,
        },
    },
    {
        "camera",
        {
            camera_subscribe,
            camera_unsubscribe,
        },
    },
    {
        "local_transform",
        {
            local_transform_subscribe,
            local_transform_unsubscribe,
        },
    },
    {
        "renderer",
        {
            renderer_subscribe,
            renderer_unsubscribe,
        },
    },
    {
        "text_renderer",
        {
            text_renderer_subscribe,
            text_renderer_unsubscribe,
        },
    },
    {
        "ui_mouse_event_dispatcher",
        {
            ui_mouse_event_dispatcher_subscribe,
            ui_mouse_event_dispatcher_unsubscribe,
        },
    },
    {
        "ui_transform",
        {
            ui_transform_subscribe,
            ui_transform_unsubscribe,
        },
    },
    {
        "sprite_dimensions_handler",
        {
            sprite_dimensions_handler_subscribe,
            sprite_dimensions_handler_unsubscribe,
        },
    },
};


static vector<Update_Handler> engine_update_handlers
{
    local_transform_update,
    renderer_update,
    text_renderer_update,
    camera_update,
};


static map<string, const Component_Handlers> engine_component_handlers
{
    {
        "transform",
        TRANSFORM_COMPONENT_HANDLERS
    },
    {
        "local_transform",
        TRANSFORM_COMPONENT_HANDLERS
    },
    {
        "ui_transform",
        {
            [](const JSON & data) -> Component
            {
                vec3 position;
                vec3 anchor;

                if (contains_key(data, "position"))
                {
                    const JSON & position_data = data["position"];
                    position.x = position_data["x"];
                    position.y = position_data["y"];
                }

                if (contains_key(data, "anchor"))
                {
                    const JSON & anchor_data = data["anchor"];
                    anchor.x = anchor_data["x"];
                    anchor.y = anchor_data["y"];
                }

                return new UI_Transform
                {
                    position,
                    anchor,
                };
            },
            get_component_deallocator<UI_Transform>(),
        }
    },
    {
        "sprite",
        {
            [](const JSON & data) -> Component
            {
                return new Sprite
                {
                    data["texture_path"],
                    data["shader_pipeline_name"],
                };
            },
            get_component_deallocator<Sprite>(),
        }
    },
    {
        "id",
        {
            get_component_allocator<string>(),
            get_component_deallocator<string>(),
        }
    },
    {
        "render_layer",
        {
            get_component_allocator<string>(),
            get_component_deallocator<string>(),
        }
    },
    {
        "viewport",
        {
            [](const JSON & data) -> Component
            {
                return new Viewport
                {
                    data["x"],
                    data["y"],
                    data["z_near"],
                    data["z_far"],
                };
            },
            get_component_deallocator<Viewport>(),
        }
    },
    {
        "dimensions",
        {
            [](const JSON & data) -> Component
            {
                auto dimensions = new Dimensions { 0.0f, 0.0f, vec3(0.0f) };

                if (contains_key(data, "width"))
                {
                    dimensions->width = data["width"].get<float>();
                }

                if (contains_key(data, "height"))
                {
                    dimensions->height = data["height"].get<float>();
                }

                if (contains_key(data, "origin"))
                {
                    const JSON & origin = data["origin"];
                    dimensions->origin.x = origin["x"];
                    dimensions->origin.y = origin["y"];
                }

                return dimensions;
            },
            get_component_deallocator<Dimensions>(),
        }
    },
    {
        "ui_mouse_event_handlers",
        {
            [](const JSON & /*data*/) -> Component
            {
                return new UI_Mouse_Event_Handlers;
            },
            get_component_deallocator<UI_Mouse_Event_Handlers>(),
        }
    },
    {
        "button",
        {
            [](const JSON & data) -> Component
            {
                return new Button
                {
                    data["hover_texture_path"],
                    data["pressed_texture_path"],
                    {},
                };
            },
            get_component_deallocator<Button>(),
        }
    },
    {
        "text",
        {
            [](const JSON & data) -> Component
            {
                vec3 color;

                if (contains_key(data, "color"))
                {
                    const JSON & color_data = data["color"];

                    if (contains_key(color_data, "r"))
                    {
                        color.x = color_data["r"];
                    }

                    if (contains_key(color_data, "g"))
                    {
                        color.y = color_data["g"];
                    }

                    if (contains_key(color_data, "b"))
                    {
                        color.z = color_data["b"];
                    }
                }

                return new Text
                {
                    data["font"],
                    color,
                    data["value"],
                };
            },
            get_component_deallocator<Text>(),
        }
    },
    {
        "parent_id",
        {
            get_component_allocator<string>(),
            get_component_deallocator<string>(),
        }
    }
};


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_resources(const string & root_path, const string & version_source, const string & vertex_attributes_source)
{
    // Load render layers
    const string render_layers_path = root_path + "resources/configs/render_layers.json";

    if (file_exists(render_layers_path))
    {
        const JSON render_layers_config = read_json_file(render_layers_path);

        for (const JSON & render_layer : render_layers_config)
        {
            load_render_layer(render_layer["name"], render_layer["space"]);
        }
    }


    // Load shader pipelines.
    const string shader_pipelines_path = root_path + "resources/data/shader_pipelines.json";

    if (file_exists(shader_pipelines_path))
    {
        const JSON shader_pipelines_data = read_json_file(shader_pipelines_path);
        vector<Shader_Pipeline> shader_pipelines;

        for (const JSON & shader_pipeline_data : shader_pipelines_data)
        {
            Shader_Pipeline shader_pipeline;
            shader_pipeline.name = shader_pipeline_data["name"];
            const JSON & shaders = shader_pipeline_data["shaders"];

            for_each(shaders, [&](const string & type, const string & source_path) -> void
            {
                vector<string> & shader_sources = shader_pipeline.shader_sources[type];


                // Load sources for shader into pipeline, starting with the version.glsl source, then the
                // vertex_attributes.glsl source if this is a vertex shader, then finally the shader source itself.
                shader_sources.push_back(version_source);

                if (type == "vertex")
                {
                    shader_sources.push_back(vertex_attributes_source);
                }

                shader_sources.push_back(read_file(root_path + source_path));
            });

            shader_pipelines.push_back(shader_pipeline);
        }

        load_shader_pipelines(shader_pipelines);
    }


    // Load control bindings.
    const string controls_path = root_path + "resources/data/controls.json";

    if (file_exists(controls_path))
    {
        const JSON controls = read_json_file(controls_path);

        for (const JSON & control_binding : controls)
        {
            const string & key = control_binding["key"];
            const string & action = control_binding["action"];
            const string & handler = control_binding["handler"];

            if (!contains_key(key_mappings, key))
            {
                throw runtime_error("ERROR: \"" + key + "\" is not a valid key!");
            }

            if (!contains_key(key_action_mappings, action))
            {
                throw runtime_error("ERROR: \"" + action + "\" is not a valid key action!");
            }

            add_control_binding(key_mappings.at(key), key_action_mappings.at(action), handler);
        }
    }


    // Load system requirements.
    const string system_requirements_path = root_path + "resources/data/system_requirements.json";

    if (file_exists(system_requirements_path))
    {
        for_each(read_json_file(system_requirements_path), set_system_requirements);
    }


    // Load component requirements.
    const string component_requirements_path = root_path + "resources/data/component_requirements.json";

    if (file_exists(component_requirements_path))
    {
        for_each(read_json_file(component_requirements_path), set_component_requirements);
    }


    // Load texture data.
    const string textures_path = root_path + "resources/data/textures.json";

    if (file_exists(textures_path))
    {
        for_each(read_json_file(textures_path).get<vector<JSON>>(), load_texture);
    }


    // Load font data.
    const string fonts_path = root_path + "resources/data/fonts.json";

    if (file_exists(fonts_path))
    {
        for_each(read_json_file(fonts_path).get<vector<JSON>>(), load_font);
    }


    // Load audio files.
    const string audio_files_path = root_path + "resources/data/audio_files.json";

    if (file_exists(audio_files_path))
    {
        for_each(read_json_file(audio_files_path).get<vector<string>>(), load_audio_file);
    }


    // Load scenes.
    const string scenes_path = root_path + "resources/data/scenes.json";

    if (file_exists(scenes_path))
    {
        for_each(read_json_file(scenes_path), set_scene);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void add_update_handler(const Update_Handler & update_handler)
{
    update_handlers.push_back(update_handler);
}


int run_engine()
{
    // Validate and load Nito installation root path from environment.
    const char * env_nito_path = getenv("NITO_PATH");

    if (env_nito_path == nullptr)
    {
        throw runtime_error(
            "ERROR: the environment variable NITO_PATH could not be found; please set an environment variable called "
            "NITO_PATH to the root directory of your Nito installation!");
    }

    const string NITO_PATH = directify(string(env_nito_path));


    // Load engine handlers.
    for_each(engine_update_handlers, add_update_handler);

    for_each(
        engine_system_entity_handlers,
        [](const string & name, const System_Entity_Handlers & system_entity_handlers) -> void
        {
            set_system_entity_handlers(name, system_entity_handlers.subscriber, system_entity_handlers.unsubscriber);
        });

    for_each(engine_component_handlers, [](const string & type, const Component_Handlers & component_handlers) -> void
    {
        set_component_handlers(type, component_handlers.allocator, component_handlers.deallocator);
    });


    // Initialize modules.
    input_init();


    // Initialize systems.
    ui_mouse_event_dispatcher_init();


    // Initalize 3rd-party libraries.
    init_glfw();
    init_freetype();


    // Create window.
    const JSON window_config = read_json_file("resources/configs/window.json");
    map<string, int> window_hints;

    for_each(window_config["hints"], [&](const string & hint_key, const int hint_value) -> void
    {
        window_hints[hint_key] = hint_value;
    });

    create_window(
        {
            window_config["width"],
            window_config["height"],
            window_config["title"],
            window_config["refresh_rate"],
            window_hints,
        });


    // Initialize GLEW after OpenGL context is created (when window is created).
    init_glew();


    // Initialize Graphics API.
    const JSON opengl_config = read_json_file(NITO_PATH + "resources/configs/opengl.json");
    const JSON clear_color = opengl_config["clear_color"];
    const JSON blending = opengl_config["blending"];

    configure_opengl(
        {
            opengl_config["pixels_per_unit"],
            opengl_config["capabilities"],
            opengl_config["clear_flags"],
            {
                clear_color["r"],
                clear_color["g"],
                clear_color["b"],
                clear_color["a"],
            },
            {
                blending["source_factor"],
                blending["destination_factor"],
            },
        });


    // !!! MUST COME AFTER GRAPHICS ENGINE AND WINDOW INITIALIZATION !!!
    ui_transform_init();


    // Initialize OpenAL and Audio API.
    init_openal();


    // Load vertex data.
    GLfloat sprite_vertex_data[]
    {
        // Position       // UV
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    GLuint sprite_index_data[]
    {
        0, 1, 2,
        0, 2, 3,
    };

    load_vertex_data(
        sprite_vertex_data,
        sizeof(sprite_vertex_data),
        sprite_index_data,
        sizeof(sprite_index_data));


    // Load engine resources first, then project resources.
    const string version_source = read_file(NITO_PATH + "resources/shaders/shared/version.glsl");
    const string vertex_attributes_source = read_file(NITO_PATH + "resources/shaders/shared/vertex_attributes.glsl");
    load_resources(NITO_PATH, version_source, vertex_attributes_source);
    load_resources("./", version_source, vertex_attributes_source);


    // Validate that a default scene was specified, then set that scene as the first scene to be loaded.
    if (!scene_exists(DEFAULT_SCENE_NAME))
    {
        throw runtime_error(
            "ERROR: a scene named \"" + DEFAULT_SCENE_NAME + "\" must be provided in resources/data/scenes.json!");
    }

    set_scene_to_load(DEFAULT_SCENE_NAME);


    // Main loop
    run_window_loop([&]() -> void
    {
        check_load_scene();

        for (const Update_Handler & update_handler : update_handlers)
        {
            update_handler();
        }
    });


    destroy_graphics();
    terminate_glfw();
    clean_openal();
    return 0;
}


} // namespace Nito
