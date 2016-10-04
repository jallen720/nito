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
#include "Cpp_Utils/Container.hpp"
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"
#include "Nito/ECS.hpp"
#include "Nito/Components.hpp"
#include "Nito/Systems/Renderer.hpp"


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

// Cpp_Utils/Map.hpp
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
};


static map<string, const Component_Handler> default_component_handlers
{
    {
        "transform",
        [](const JSON & component_data) -> Component
        {
            static const auto DEFAULT_POSITION_Z = 0.0f;
            static const auto DEFAULT_SCALE_Z = 1.0f;

            auto transform = new Transform;
            const JSON & position = component_data["position"];
            const JSON & scale = component_data["scale"];
            transform->position = vec3(position["x"], position["y"], DEFAULT_POSITION_Z);
            transform->scale = vec3(scale["x"], scale["y"], DEFAULT_SCALE_Z);
            return transform;
        }
    },
    {
        "sprite",
        [](const JSON & component_data) -> Component
        {
            auto sprite = new Sprite;
            sprite->texture_path = component_data["texture_path"];
            sprite->shader_pipeline_name = component_data["shader_pipeline_name"];
            return sprite;
        }
    },
};


static map<string, const System_Subscribe_Handler> default_system_subscribe_handlers
{
    { "renderer", renderer_subscribe },
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
    const JSON & glfw_context_version = window_config["glfw_context_version"];

    window =
        create_window(
            {
                window_config["width"],
                window_config["height"],
                window_config["title"],
                window_config["refresh_rate"],
                {
                    glfw_context_version["major"],
                    glfw_context_version["minor"],
                },
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
    int window_width;
    int window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    init_glew();

    configure_opengl(
        {
            window_width,
            window_height,
            opengl_config["pixels_per_unit"],
            {
                clear_color["red"],
                clear_color["green"],
                clear_color["blue"],
                clear_color["alpha"],
            },
            {
                blending["is_enabled"],
                blending["s_factor"],
                blending["d_factor"],
            },
        });


    // Load shader pipelines.
    const JSON shader_pipelines_data      = read_json_file("resources/data/shader_pipelines.json");
    const JSON shader_config              = read_json_file("resources/configs/shaders.json");
    const JSON shader_extensions          = shader_config["extensions"];
    const string version_source           = read_file("resources/shaders/shared/version.glsl");
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
    const JSON entities_data = read_json_file("resources/data/entities.json");
    const JSON systems_data  = read_json_file("resources/data/systems.json");

    for (const JSON & entity_data : entities_data)
    {
        Entity entity = create_entity();


        // Load components for entity.
        const JSON & components_data = entity_data["components"];

        for (const JSON & component_data : components_data)
        {
            add_component(entity, component_data["type"], component_data["data"]);
        }


        // Subscribe entity to systems.
        const JSON & entity_systems = entity_data["systems"];

        for (const string & system_name : entity_systems)
        {
            // Validate entity has components required by system.
            const JSON & required_components = systems_data[system_name]["required_components"];

            for (const string & required_component : required_components)
            {
                if (!has_component(entity, required_component))
                {
                    throw runtime_error(
                        "Entity " + to_string(entity) + " does not contain a " + required_component + " component " +
                        "required by the " + system_name + " system!");
                }
            }


            subscribe_to_system(entity, system_name);
        }
    }


    // Main loop
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
