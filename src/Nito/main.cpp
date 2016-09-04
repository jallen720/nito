// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>
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
using CppUtils::JSON;
using CppUtils::readJSONFile;
using CppUtils::readFile;
using CppUtils::forEach;
using CppUtils::transform;

// Nito/Window.hpp
using Nito::initGLFW;
using Nito::createWindow;
using Nito::terminateGLFW;

// Nito/Input.hpp
using Nito::addControlBinding;
using Nito::setControlHandler;

// Nito/Graphics.hpp
using Nito::initGLEW;
using Nito::configureOpenGL;
using Nito::loadShaderPipelines;
using Nito::loadVertexData;
using Nito::renderGraphics;
using Nito::destroyGraphics;
using Nito::ShaderPipeline;


int main() {
    initGLFW();


    // Create window.
    JSON windowConfig = readJSONFile("resources/configs/window.json");
    const JSON & glfwContextVersion = windowConfig["glfw-context-version"];

    GLFWwindow * window =
        createWindow(
            {
                windowConfig["width"],
                windowConfig["height"],
                windowConfig["title"],
                windowConfig["refresh-rate"],
                {
                    glfwContextVersion["major"],
                    glfwContextVersion["minor"],
                }
            });


    // Set control handlers.
    auto exitHandler = [&](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    };

    auto printHandler = [](GLFWwindow * /*window*/, const int key, const int action) -> void {
        printf("key [%d] action [%d]\n", key, action);
    };

    setControlHandler("exit", exitHandler);
    setControlHandler("print", printHandler);


    // Load control bindings.
    const vector<JSON> controls = readJSONFile("resources/data/controls.json");

    for (const JSON & controlBinding : controls) {
        addControlBinding(
            controlBinding["key"],
            controlBinding["action"],
            controlBinding["handler"]);
    }


    // Initialize graphics engine.
    const JSON openGLConfig = readJSONFile("resources/configs/opengl.json");
    const JSON clearColor = openGLConfig["clear-color"];
    initGLEW();

    configureOpenGL(
        {
            window,
            {
                clearColor["red"],
                clearColor["green"],
                clearColor["blue"],
                clearColor["alpha"],
            },
        });


    // Load shader pipelines.
    const JSON shaderPipelinesData      = readJSONFile("resources/data/shader-pipelines.json");
    const JSON shadersConfig            = readJSONFile("resources/configs/shaders.json");
    const JSON shaderExtensions         = shadersConfig["extensions"];
    const string versionSource          = readFile("resources/shaders/shared/version.glsl");
    const string vertexAttributesSource = readFile("resources/shaders/shared/vertex-attributes.glsl");

    const vector<ShaderPipeline> shaderPipelines =
        transform<ShaderPipeline>(shaderPipelinesData, [&](const JSON & shaderPipelineData) -> ShaderPipeline {
            ShaderPipeline shaderPipeline;

            forEach(shaderPipelineData, [&](const string & shaderType, const string & shaderSource) -> void {
                // Load sources for shader into pipeline, starting with the version source, then the vertex attributes
                // source if this is a vertex shader, then finally the shader source itself.
                vector<string> & shaderSources = shaderPipeline[shaderType];
                shaderSources.push_back(versionSource);

                if (shaderType == "vertex") {
                    shaderSources.push_back(vertexAttributesSource);
                }

                shaderSources.push_back(readFile(
                    "resources/shaders/" +
                    shaderSource +
                    shaderExtensions[shaderType].get<string>()));
            });

            return shaderPipeline;
        });

    loadShaderPipelines(shaderPipelines);


    // Load vertex data.
    GLfloat spriteVertexData[] = {
        // Position          // Color
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,
        // -0.5f, -0.5f,  0.0f,
    };

    loadVertexData(spriteVertexData, sizeof(spriteVertexData));


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderGraphics();
        glfwSwapBuffers(window);
    }


    destroyGraphics();
    terminateGLFW();
    return 0;
}
