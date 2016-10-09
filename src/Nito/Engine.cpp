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
#include "Cpp_Utils/String.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"
#include "Nito/ECS.hpp"
#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/Systems/Renderer.hpp"
#include "Nito/Systems/Camera.hpp"


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

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static GLFWwindow * window;
static vector<Update_Handler> update_handlers;


static vector<Update_Handler> default_update_handlers
{
    renderer_update,
    camera_update,
};


static map<string, const Component_Handler> default_component_handlers
{
    {
        "transform",
        [](const JSON & component_data) -> Component
        {
            const JSON & position = component_data["position"];
            const JSON & scale = component_data["scale"];
            vec3 origin;

            if (contains_key(component_data, "origin"))
            {
                const JSON & origin_data = component_data["origin"];
                origin = vec3(origin_data["x"], origin_data["y"], 0.0f);
            }

            return new Transform
            {
                vec3(position["x"], position["y"], 0.0f),
                vec3(scale["x"], scale["y"], 1.0f),
                origin,
            };
        }
    },
    {
        "sprite",
        [](const JSON & component_data) -> Component
        {
            return new Sprite
            {
                component_data["texture_path"],
                component_data["shader_pipeline_name"],
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
        [](const JSON & component_data) -> Component
        {
            return new Viewport
            {
                component_data["x"],
                component_data["y"],
                component_data["width"],
                component_data["height"],
                component_data["z_near"],
                component_data["z_far"],
            };
        }
    },
};


static map<string, const System_Subscribe_Handler> default_system_subscribe_handlers
{
    { "renderer" , renderer_subscribe },
    { "camera"   , camera_subscribe   },
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
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLFWwindow ** get_window()
{
    return &window;
}


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


    // Initalize GLFW.
    init_glfw();


    // Create window.
    const JSON window_config = read_json_file("resources/configs/window.json");
    map<string, int> window_hints;

    for_each(window_config["hints"], [&](const string & hint_key, const int hint_value) -> void
    {
        window_hints[hint_key] = hint_value;
    });

    window = create_window(
        {
            window_config["width"],
            window_config["height"],
            window_config["title"],
            window_config["refresh_rate"],
            window_hints,
        });


    // Load control bindings.
    const JSON controls = read_json_file("resources/data/controls.json");

    for (const JSON & control_binding : controls)
    {
        add_control_binding(
            control_binding["key"],
            control_binding["action"],
            control_binding["handler"]);
    }


    // Initialize graphics engine.
    const JSON opengl_config = read_json_file("resources/configs/opengl.json");
    const JSON clear_color = opengl_config["clear_color"];
    const JSON blending = opengl_config["blending"];
    init_glew();

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
    const JSON textures_data = read_json_file("resources/data/textures.json");

    const vector<Texture> textures =
        transform<Texture>(textures_data, [](const JSON & texture_data) -> Texture
        {
            Texture::Options options;

            for_each(texture_data["options"], [&](const string & option_key, const string & option_value) -> void
            {
                options[option_key] = option_value;
            });

            return
            {
                "resources/textures/" + texture_data["path"].get<string>(),
                texture_data["format"],
                options,
            };
        });

    load_textures(textures);


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


    // Load entities.
    const vector<JSON> entities_data = read_json_file("resources/data/entities.json");

    const vector<Entity> entities = transform<Entity>(entities_data, [](const JSON & /*entity_data*/) -> Entity
    {
        return create_entity();
    });


    // Add components to entities.
    for (auto i = 0u; i < entities.size(); i++)
    {
        const JSON & components_data = entities_data[i]["components"];

        for (const JSON & component_data : components_data)
        {
            add_component(entities[i], component_data["type"], component_data["data"]);
        }
    }

    // Subscribe entities to systems.
    const JSON required_components_data = read_json_file("resources/data/required_components.json");

    for (auto i = 0u; i < entities.size(); i++)
    {
        const Entity entity = entities[i];
        const JSON & entity_systems = entities_data[i]["systems"];

        for (const string & system_name : entity_systems)
        {
            // Validate entity has components required by system if system's required components are specified.
            if (contains_key(required_components_data, system_name))
            {
                const JSON & required_components = required_components_data[system_name];

                for (const string & required_component : required_components)
                {
                    if (!has_component(entity, required_component))
                    {
                        throw runtime_error(
                            "ERROR: entity " + to_string(entity) + " does not contain a " + required_component + " " +
                            "component required by the " + system_name + " system!");
                    }
                }
            }


            // If entity has all components required by system, subscribe entity to that system.
            subscribe_to_system(entity, system_name);
        }
    }


    // Main loop

    // TODO: actually calculate delta time.
    static const float delta_time = 0.02f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        for (const Update_Handler & update_handler : update_handlers)
        {
            update_handler(delta_time);
        }

        glfwSwapBuffers(window);
    }


    destroy_graphics();
    terminate_glfw();
    return 0;
}


} // namespace Nito
