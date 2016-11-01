#include "Nito/Engine.hpp"

// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Cpp_Utils/JSON.hpp"
#include "Cpp_Utils/File.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/String.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"
#include "Nito/ECS.hpp"
#include "Nito/Resources.hpp"
#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/Systems/Renderer.hpp"
#include "Nito/Systems/Camera.hpp"
#include "Nito/Systems/UI_Transform.hpp"
#include "Nito/Systems/UI_Mouse_Event_Dispatcher.hpp"
#include "Nito/Systems/Button.hpp"
#include "Nito/Systems/Text_Renderer.hpp"


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
static vector<Update_Handler> update_handlers;


static map<string, const System_Subscribe_Handler> default_system_subscribe_handlers
{
    { "renderer"                  , renderer_subscribe                  },
    { "camera"                    , camera_subscribe                    },
    { "ui_transform"              , ui_transform_subscribe              },
    { "ui_mouse_event_dispatcher" , ui_mouse_event_dispatcher_subscribe },
    { "button"                    , button_subscribe                    },
    { "text_renderer"             , text_renderer_subscribe             },
};


static vector<Update_Handler> default_update_handlers
{
    renderer_update,
    text_renderer_update,
    camera_update,
};


static map<string, const Component_Handler> default_component_handlers
{
    {
        "transform",
        [](const JSON & data) -> Component
        {
            vec3 position;
            vec3 scale(1.0f);

            if (contains_key(data, "position"))
            {
                const JSON & position_data = data["position"];
                position.x = position_data["x"];
                position.y = position_data["y"];
            }

            if (contains_key(data, "scale"))
            {
                const JSON & scale_data = data["scale"];
                scale.x = scale_data["x"];
                scale.y = scale_data["y"];
            }

            return new Transform
            {
                position,
                scale,
            };
        }
    },
    {
        "ui_transform",
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
        }
    },
    {
        "sprite",
        [](const JSON & data) -> Component
        {
            const string texture_path = data["texture_path"];

            // Use texture dimensions as default dimensions for sprite.
            Dimensions dimensions = get_loaded_texture(texture_path).dimensions;

            // If dimensions field is present, overwrite texture dimensions with provided fields.
            if (contains_key(data, "dimensions"))
            {
                const JSON & dimensions_data = data["dimensions"];

                if (contains_key(dimensions_data, "width"))
                {
                    dimensions.width = dimensions_data["width"].get<float>();
                }

                if (contains_key(dimensions_data, "height"))
                {
                    dimensions.height = dimensions_data["height"].get<float>();
                }

                if (contains_key(dimensions_data, "origin"))
                {
                    const JSON & origin = dimensions_data["origin"];
                    dimensions.origin.x = origin["x"];
                    dimensions.origin.y = origin["y"];
                }
            }

            return new Sprite
            {
                texture_path,
                data["shader_pipeline_name"],
                dimensions,
            };
        }
    },
    {
        "id",
        string_component_handler
    },
    {
        "render_layer",
        string_component_handler
    },
    {
        "viewport",
        [](const JSON & data) -> Component
        {
            return new Viewport
            {
                data["x"],
                data["y"],
                data["z_near"],
                data["z_far"],
            };
        }
    },
    {
        "dimensions",
        [](const JSON & data) -> Component
        {
            vec3 origin;

            if (contains_key(data, "origin"))
            {
                const JSON & origin_data = data["origin"];
                origin = vec3(origin_data["x"], origin_data["y"], 0.0f);
            }

            return new Dimensions
            {
                data["width"],
                data["height"],
                origin,
            };
        }
    },
    {
        "ui_mouse_event_handlers",
        [](const JSON & /*data*/) -> Component
        {
            return new UI_Mouse_Event_Handlers;
        }
    },
    {
        "button",
        [](const JSON & data) -> Component
        {
            return new Button
            {
                data["hover_texture_path"],
                data["pressed_texture_path"],
                {},
            };
        }
    },
    {
        "text",
        [](const JSON & data) -> Component
        {
            const JSON & color_data = data["color"];
            vec3 color;

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

            return new Text
            {
                data["font"],
                color,
                data["value"],
            };
        }
    }
};


static map<string, const Control_Handler> default_control_handlers
{
    {
        "exit",
        [](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
string get_system_requirement_message(
    const Entity entity,
    const string & system_name,
    const string & requirement_name,
    const string & requirement_type)
{
    return "ERROR: entity " + to_string(entity) + " does not contain a " + requirement_name + " " +
           requirement_type + " required by the " + system_name + " system!";
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
    // Load default handlers.
    for_each(default_update_handlers, add_update_handler);
    for_each(default_component_handlers, set_component_handler);
    for_each(default_system_subscribe_handlers, set_system_subscribe_handler);
    for_each(default_control_handlers, set_control_handler);


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


    // Initialize graphics engine.
    const JSON opengl_config = read_json_file("resources/configs/opengl.json");
    const JSON clear_color = opengl_config["clear_color"];
    const JSON blending = opengl_config["blending"];

    configure_opengl(
        {
            opengl_config["pixels_per_unit"],
            opengl_config["capabilities"],
            opengl_config["clear_flags"],
            {
                clear_color["red"],
                clear_color["green"],
                clear_color["blue"],
                clear_color["alpha"],
            },
            {
                blending["source_factor"],
                blending["destination_factor"],
            },
        });


    // !!! MUST COME AFTER GRAPHICS ENGINE AND WINDOW INITIALIZATION !!!
    ui_transform_init();


    // Load render layers
    const JSON render_layers_config = read_json_file("resources/configs/render_layers.json");

    for (const JSON & render_layer : render_layers_config)
    {
        load_render_layer(render_layer["name"], render_layer["render_space"]);
    }


    // Load shader pipelines.
    const JSON shader_pipelines_data = read_json_file("resources/data/shader_pipelines.json");
    const JSON shader_config = read_json_file("resources/configs/shaders.json");
    const JSON shader_extensions = shader_config["extensions"];
    const string version_source = read_file("resources/shaders/shared/version.glsl");
    const string vertex_attributes_source = read_file("resources/shaders/shared/vertex_attributes.glsl");
    vector<Shader_Pipeline> shader_pipelines;

    for (const JSON & shader_pipeline_data : shader_pipelines_data)
    {
        Shader_Pipeline shader_pipeline;
        shader_pipeline.name = shader_pipeline_data["name"];
        const JSON shaders = shader_pipeline_data["shaders"];

        for (const JSON & shader : shaders)
        {
            const string shader_type = shader["type"];
            vector<string> & shader_sources = shader_pipeline.shader_sources[shader_type];


            // Load sources for shader into pipeline, starting with the version.glsl source, then the
            // vertex_attributes.glsl source if this is a vertex shader, then finally the shader source itself.
            shader_sources.push_back(version_source);

            if (shader_type == "vertex")
            {
                shader_sources.push_back(vertex_attributes_source);
            }

            shader_sources.push_back(read_file(
                "resources/shaders/" +
                shader["source_path"].get<string>() +
                shader_extensions[shader_type].get<string>()));
        }

        shader_pipelines.push_back(shader_pipeline);
    }

    load_shader_pipelines(shader_pipelines);


    // Load texture data.
    for_each(read_json_file("resources/data/textures.json").get<vector<JSON>>(), load_texture);


    // Load font data.
    for_each(read_json_file("resources/data/fonts.json").get<vector<JSON>>(), load_font);


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


    // Load control bindings.
    const JSON controls = read_json_file("resources/data/controls.json");

    for (const JSON & control_binding : controls)
    {
        add_control_binding(
            control_binding["key"],
            control_binding["action"],
            control_binding["handler"]);
    }


    // Load entities.
    const vector<JSON> entities_data = read_json_file("resources/data/entities.json");

    const vector<Entity> entities = transform<Entity>(entities_data, [](const JSON & /*entity_data*/) -> Entity
    {
        return create_entity();
    });


    // Add components to entities.
    for (auto i = 0u; i < entities.size(); i++)
    {
        for_each(entities_data[i]["components"], [&](const string & component, const JSON & data) -> void
        {
            add_component(entities[i], component, data);
        });
    }


    // Subscribe entities to systems.
    const JSON systems_data = read_json_file("resources/data/systems.json");

    for (auto i = 0u; i < entities.size(); i++)
    {
        const Entity entity = entities[i];
        const vector<string> & entity_systems = entities_data[i]["systems"];


        // Validate all system and component requirements are met for all entity systems, then subscribe entity to them.
        for (const string & system_name : entity_systems)
        {
            // Defining system and component requirements for systems is optional, so make sure requirements are defined
            // before checking them.
            if (contains_key(systems_data, system_name))
            {
                const JSON & system_data = systems_data[system_name];

                if (contains_key(system_data, "required_components"))
                {
                    for (const string & required_component : system_data["required_components"])
                    {
                        if (!has_component(entity, required_component))
                        {
                            throw runtime_error(
                                get_system_requirement_message(entity, system_name, required_component, "component"));
                        }
                    }
                }

                if (contains_key(system_data, "required_systems"))
                {
                    for (const string & required_system : system_data["required_systems"])
                    {
                        if (!contains(entity_systems, required_system))
                        {
                            throw runtime_error(
                                get_system_requirement_message(entity, system_name, required_system, "system"));
                        }
                    }
                }
            }


            // If entity meets all system and component requirements for this system, subscribe entity to it.
            subscribe_to_system(entity, system_name);
        }
    }


    // Main loop
    run_window_loop([&]() -> void
    {
        for (const Update_Handler & update_handler : update_handlers)
        {
            update_handler();
        }
    });


    destroy_graphics();
    terminate_glfw();
    return 0;
}


} // namespace Nito
