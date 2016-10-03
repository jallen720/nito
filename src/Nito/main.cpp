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
#include "Nito/Systems/Controller.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;
using std::runtime_error;
using glm::vec3;
using Cpp_Utils::JSON;
using Cpp_Utils::read_json_file;
using Cpp_Utils::read_file;
using Cpp_Utils::for_each;
using Cpp_Utils::transform;
using Cpp_Utils::contains_key;
using Cpp_Utils::to_string;

// Nito/Window.hpp
using Nito::init_glfw;
using Nito::create_window;
using Nito::terminate_glfw;

// Nito/Input.hpp
using Nito::add_control_binding;
using Nito::set_control_handler;

// Nito/Graphics.hpp
using Nito::init_glew;
using Nito::configure_opengl;
using Nito::load_shader_pipelines;
using Nito::load_textures;
using Nito::load_vertex_data;
using Nito::destroy_graphics;
using Nito::Shader_Pipeline;
using Nito::Texture;

// Nito/ECS.hpp
using Nito::create_entity;
using Nito::get_entities;
using Nito::add_component;
using Nito::has_component;
using Nito::get_component;
using Nito::Entity;
using Nito::Component;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;

// Nito/Systems/Renderer.hpp
using Nito::renderer_subscribe;
using Nito::renderer_update;

// Nito/Systems/Controller.hpp
using Nito::controller_subscribe;
using Nito::controller_update;
using Nito::controller_init;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Component_Handler = function<Component(const JSON &)>;


struct System_Handler
{
    using Subscribe_Handler = void (*)(const Entity);
    using Update_Handler = void (*)();

    Subscribe_Handler subscribe_handler;
    Update_Handler update_handler;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const map<string, const Component_Handler> component_handlers
{
    {
        "transform",
        [](const JSON & component_config) -> Component
        {
            static const auto DEFAULT_POSITION_Z = 0.0f;
            static const auto DEFAULT_SCALE_Z = 1.0f;

            auto transform = new Transform;
            const JSON & position = component_config["position"];
            const JSON & scale = component_config["scale"];
            transform->position = vec3(position["x"], position["y"], DEFAULT_POSITION_Z);
            transform->scale = vec3(scale["x"], scale["y"], DEFAULT_SCALE_Z);
            return transform;
        }
    },
    {
        "sprite",
        [](const JSON & component_config) -> Component
        {
            auto sprite = new Sprite;
            sprite->texture_path = component_config["texture_path"];
            sprite->shader_pipeline_name = component_config["shader_pipeline_name"];
            return sprite;
        }
    },
};


const map<string, const System_Handler> system_handlers
{
    {
        "renderer",
        {
            renderer_subscribe,
            renderer_update,
        },
    },
    {
        "controller",
        {
            controller_subscribe,
            controller_update,
        },
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    init_glfw();


    // Create window.
    const JSON window_config = read_json_file("resources/configs/window.json");
    const JSON & glfw_context_version = window_config["glfw_context_version"];

    GLFWwindow * window =
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


    // !!! TEMP !!!
    controller_init(window);


    // Set control handlers.
    auto exit_handler = [](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    };

    auto print_handler = [](GLFWwindow * /*window*/, const int key, const int action) -> void
    {
        printf("key [%d] action [%d]\n", key, action);
    };

    set_control_handler("exit", exit_handler);
    set_control_handler("print", print_handler);


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
            const string component_type = component_data["type"];

            if (!contains_key(component_handlers, component_type))
            {
                throw runtime_error("\"" + component_type + "\" is not a supported component type!");
            }

            add_component(
                entity,
                component_type,
                component_handlers.at(component_type)(component_data["config"]));
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


            system_handlers.at(system_name).subscribe_handler(entity);
        }
    }


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        for_each(system_handlers, [](const string & /*system_name*/, const System_Handler & system_handler) -> void
        {
            system_handler.update_handler();
        });

        glfwSwapBuffers(window);
    }


    destroy_graphics();
    terminate_glfw();
    return 0;
}
