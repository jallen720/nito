// Required before any other OpenGL includes
#include <GL/glew.h>

#include "Nito/Engine.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Cpp_Utils/JSON.hpp"
#include "Cpp_Utils/File.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/String.hpp"

#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Audio.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Resources.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Physics.hpp"
#include "Nito/Systems/Button.hpp"
#include "Nito/Systems/Camera.hpp"
#include "Nito/Systems/Local_Transform.hpp"
#include "Nito/Systems/Renderer.hpp"
#include "Nito/Systems/Sprite_Dimensions_Handler.hpp"
#include "Nito/Systems/Text_Renderer.hpp"
#include "Nito/Systems/UI_Mouse_Event_Dispatcher.hpp"
#include "Nito/Systems/UI_Transform.hpp"
#include "Nito/Systems/Circle_Collider.hpp"
#include "Nito/Systems/Line_Collider.hpp"
#include "Nito/Systems/Polygon_Collider.hpp"


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
static float time_scale;
static vector<Update_Handler> update_handlers;


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

            if (contains_key(position_data, "x"))
            {
                position.x = position_data["x"];
            }

            if (contains_key(position_data, "y"))
            {
                position.y = position_data["y"];
            }

            if (contains_key(position_data, "z"))
            {
                position.z = position_data["z"];
            }
        }

        if (contains_key(data, "scale"))
        {
            const JSON & scale_data = data["scale"];

            if (contains_key(scale_data, "x"))
            {
                scale.x = scale_data["x"];
            }

            if (contains_key(scale_data, "y"))
            {
                scale.y = scale_data["y"];
            }
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


static const vector<Update_Handler> ENGINE_UPDATE_HANDLERS
{
    input_api_update,
    physics_api_update,
    ui_transform_update,
    local_transform_update,
    renderer_update,
    text_renderer_update,
    circle_collider_update,
    line_collider_update,
    polygon_collider_update,

    // Should come after all update handlers that will affect renderable data (renderers, colliders, etc.).
    camera_update,
};


static const map<string, const System_Entity_Handlers> ENGINE_SYSTEM_ENTITY_HANDLERS
{
    NITO_SYSTEM_ENTITY_HANDLERS(button),
    NITO_SYSTEM_ENTITY_HANDLERS(camera),
    NITO_SYSTEM_ENTITY_HANDLERS(local_transform),
    NITO_SYSTEM_ENTITY_HANDLERS(renderer),
    NITO_SYSTEM_ENTITY_HANDLERS(text_renderer),
    NITO_SYSTEM_ENTITY_HANDLERS(ui_mouse_event_dispatcher),
    NITO_SYSTEM_ENTITY_HANDLERS(ui_transform),
    NITO_SYSTEM_ENTITY_HANDLERS(sprite_dimensions_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(circle_collider),
    NITO_SYSTEM_ENTITY_HANDLERS(line_collider),
    NITO_SYSTEM_ENTITY_HANDLERS(polygon_collider),
};


static const map<string, const Component_Handlers> ENGINE_COMPONENT_HANDLERS
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
                    contains_key(data, "render") ? data["render"].get<bool>() : true,
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
        "camera",
        {
            [](const JSON & data) -> Component
            {
                return new Camera
                {
                    data["z_near"],
                    data["z_far"],
                };
            },
            get_component_deallocator<Camera>(),
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
    },
    {
        "collider",
        {
            [](const JSON & data) -> Component
            {
                return new Collider
                {
                    contains_key(data, "render") ? data["render"].get<bool>() : false,
                    contains_key(data, "send_collision") ? data["send_collision"].get<bool>() : false,
                    contains_key(data, "receives_collision") ? data["receives_collision"].get<bool>() : false,
                    {},
                };
            },
            get_component_deallocator<Collider>(),
        }
    },
    {
        "circle_collider",
        {
            [](const JSON & data) -> Component
            {
                return new Circle_Collider
                {
                    data["radius"],
                };
            },
            get_component_deallocator<Circle_Collider>(),
        }
    },
    {
        "line_collider",
        {
            [](const JSON & data) -> Component
            {
                const JSON & begin_data = data["begin"];
                const JSON & end_data = data["end"];

                return new Line_Collider
                {
                    vec3(begin_data["x"], begin_data["y"], 0.0f),
                    vec3(end_data["x"], end_data["y"], 0.0f),
                };
            },
            get_component_deallocator<Line_Collider>(),
        }
    },
    {
        "polygon_collider",
        {
            [](const JSON & data) -> Component
            {
                auto polygon_collider = new Polygon_Collider;
                vector<vec3> & points = polygon_collider->points;
                polygon_collider->wrap = contains_key(data, "wrap") ? data["wrap"].get<bool>() : false;

                for (const JSON & point_data : data["points"])
                {
                    points.emplace_back(point_data["x"], point_data["y"], 0.0f);
                }

                return polygon_collider;
            },
            get_component_deallocator<Polygon_Collider>(),
        }
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void load_resources(
    const string & root_path,
    const string & version_source,
    const string & vertex_attributes_source)
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
        for_each(read_json_file(textures_path).get<vector<JSON>>(), load_textures);
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


    // Load blueprints.
    const string blueprints_path = root_path + "resources/data/blueprints.json";

    if (file_exists(blueprints_path))
    {
        set_blueprints(read_json_file(blueprints_path));
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
    // Initialize time scale to 1.
    set_time_scale(1.0f);


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
    for_each(ENGINE_UPDATE_HANDLERS, add_update_handler);

    for_each(
        ENGINE_SYSTEM_ENTITY_HANDLERS,
        [](const string & name, const System_Entity_Handlers & system_entity_handlers) -> void
        {
            set_system_entity_handlers(name, system_entity_handlers.subscriber, system_entity_handlers.unsubscriber);
        });

    for_each(ENGINE_COMPONENT_HANDLERS, [](const string & type, const Component_Handlers & component_handlers) -> void
    {
        set_component_handlers(type, component_handlers.allocator, component_handlers.deallocator);
    });


    // Initialize APIs.
    input_api_init();


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
            opengl_config["default_vertex_container_id"],
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


    // Load default vertex data.
    const vector<GLfloat> default_vertex_data
    {
        // Position       // UV
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    const vector<GLuint> default_index_data
    {
        0, 1, 2,
        0, 2, 3,
    };

    load_vertex_data(get_default_vertex_container_id(), default_vertex_data, default_index_data);


    // Load circle collider vertex data.
    const float PI = 3.14159;
    vector<GLfloat> circle_collider_vertex_data;
    vector<GLuint> circle_collider_index_data;
    float radius = 0.5f;
    int index = 0;

    for (float angle = 0.0f; angle <= 2 * PI; angle += 0.25f)
    {
        circle_collider_vertex_data.push_back(radius * cos(angle));
        circle_collider_vertex_data.push_back(radius * sin(angle));
        circle_collider_vertex_data.push_back(0.0f);
        circle_collider_vertex_data.push_back(0.0f);
        circle_collider_vertex_data.push_back(0.0f);
        circle_collider_index_data.push_back(index++);
    }

    circle_collider_index_data.push_back(0);
    load_vertex_data("circle_collider", circle_collider_vertex_data, circle_collider_index_data);


    // Load line collider vertex data.
    const vector<GLfloat> line_collider_vertex_data
    {
        // Position        // UV
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.1f, 0.0f, 0.0f, 0.0f,
    };

    const vector<GLuint> line_collider_index_data { 0, 1, 2, 3 };
    load_vertex_data("line_collider", line_collider_vertex_data, line_collider_index_data);


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
        delete_flagged_entities();
        check_load_scene();

        for (const Update_Handler & update_handler : update_handlers)
        {
            update_handler();
        }
    });


    // Cleanup
    destroy_graphics();
    terminate_glfw();
    clean_openal();


    return 0;
}


float get_time_scale()
{
    return time_scale;
}


void set_time_scale(float value)
{
    if (value < 0.0f || value > 1.0f)
    {
        throw runtime_error("ERROR: time scale must be set to a value between 0 and 1!");
    }

    time_scale = value;
}


} // namespace Nito
