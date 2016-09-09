// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "CppUtils/JSON/JSON.hpp"
#include "CppUtils/JSON/readJSONFile.hpp"
#include "CppUtils/FileUtils/readFile.hpp"
#include "CppUtils/ContainerUtils/forEach.hpp"
#include "CppUtils/Fn/transform.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"


using std::string;
using std::vector;
using glm::vec3;
using CppUtils::JSON;
using CppUtils::readJSONFile;
using CppUtils::readFile;
using CppUtils::forEach;
using CppUtils::transform;

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
using Nito::add_entity;
using Nito::render_graphics;
using Nito::destroy_graphics;
using Nito::Shader_Pipeline;
using Nito::Texture;


int main()
{
    init_glfw();


    // Create window.
    JSON window_config = readJSONFile("resources/configs/window.json");
    const JSON & glfw_context_version = window_config["glfw-context-version"];

    GLFWwindow * window =
        create_window(
            {
                window_config["width"],
                window_config["height"],
                window_config["title"],
                window_config["refresh-rate"],
                {
                    glfw_context_version["major"],
                    glfw_context_version["minor"],
                },
            });


    // Set control handlers.
    auto exit_handler = [&](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void
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
    const JSON controls = readJSONFile("resources/data/controls.json");

    for (const JSON & control_binding : controls)
    {
        add_control_binding(
            control_binding["key"],
            control_binding["action"],
            control_binding["handler"]);
    }


    // Initialize graphics engine.
    const JSON opengl_config = readJSONFile("resources/configs/opengl.json");
    const JSON clear_color = opengl_config["clear-color"];
    const JSON blending = opengl_config["blending"];
    int window_width;
    int window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    init_glew();

    configure_opengl(
        {
            window_width,
            window_height,
            opengl_config["pixels-per-unit"],
            {
                clear_color["red"],
                clear_color["green"],
                clear_color["blue"],
                clear_color["alpha"],
            },
            {
                blending["is-enabled"],
                blending["s-factor"],
                blending["d-factor"],
            },
        });


    // Load shader pipelines.
    const JSON shader_pipelines_data     = readJSONFile("resources/data/shader-pipelines.json");
    const JSON shader_config             = readJSONFile("resources/configs/shaders.json");
    const JSON shader_extensions         = shader_config["extensions"];
    const string version_source          = readFile("resources/shaders/shared/version.glsl");
    const string vertex_attribute_source = readFile("resources/shaders/shared/vertex-attributes.glsl");
    vector<Shader_Pipeline> shader_pipelines;

    for (const JSON & shader_pipeline_data : shader_pipelines_data)
    {
        Shader_Pipeline shader_pipeline;
        shader_pipeline.name = shader_pipeline_data["name"];
        const JSON shaders = shader_pipeline_data["shaders"];

        for (const JSON & shader : shaders)
        {
            const string shader_type = shader["type"];

            // Load sources for shader into pipeline, starting with the version source, then the vertex attributes
            // source if this is a vertex shader, then finally the shader source itself.
            vector<string> & shader_sources = shader_pipeline.shader_sources[shader_type];
            shader_sources.push_back(version_source);

            if (shader_type == "vertex")
            {
                shader_sources.push_back(vertex_attribute_source);
            }

            shader_sources.push_back(readFile(
                "resources/shaders/" +
                shader["source-path"].get<string>() +
                shader_extensions[shader_type].get<string>()));
        }

        shader_pipelines.push_back(shader_pipeline);
    }

    load_shader_pipelines(shader_pipelines);


    // Load texture data.
    const JSON textures_data = readJSONFile("resources/data/textures.json");

    const vector<Texture> textures =
        transform<Texture>(textures_data, [](const JSON & texture_data) -> Texture
        {
            Texture::Options options;

            forEach(texture_data["options"], [&](const string & option_key, const string & option_value) -> void
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
    const JSON entities_data = readJSONFile("resources/data/entities.json");

    for (const JSON & entity_data : entities_data)
    {
        const JSON & position = entity_data["position"];

        add_entity(
            {
                entity_data["shader-pipeline-name"],
                entity_data["texture-path"],
                vec3(position["x"], position["y"], 0.0f),
            });
    }


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        render_graphics();
        glfwSwapBuffers(window);
    }


    destroy_graphics();
    terminate_glfw();
    return 0;
}
