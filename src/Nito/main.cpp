// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>
#include "CppUtils/JSON/JSON.hpp"
#include "CppUtils/JSON/readJSONFile.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"


using std::string;
using std::vector;
using CppUtils::JSON;
using CppUtils::readJSONFile;

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
using Nito::loadShaders;
using Nito::loadVertexData;
using Nito::renderGraphics;
using Nito::destroyGraphics;


int main() {
    initGLFW();


    // Create window
    JSON windowConfig = readJSONFile("configs/window.json");
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


    // Set control handlers
    auto exitHandler = [&](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    };

    auto printHandler = [](GLFWwindow * /*window*/, const int key, const int action) -> void {
        printf("key [%d] action [%d]\n", key, action);
    };

    setControlHandler("exit", exitHandler);
    setControlHandler("print", printHandler);


    // Load control bindings
    const vector<JSON> controls = readJSONFile("configs/controls.json");

    for (const JSON & controlBinding : controls) {
        addControlBinding(
            controlBinding["key"],
            controlBinding["action"],
            controlBinding["handler"]);
    }


    // Initialize graphics engine
    const JSON openGLConfig = readJSONFile("configs/opengl.json");
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
            }
        });


    // Load graphics data
    const GLchar * vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n"
        "out vec4 vertexColor;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
        "vertexColor = gl_Position;\n"
        "}\0";

    const GLchar * fragmentShaderSource =
        "#version 330 core\n"
        "in vec4 vertexColor;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "color = vertexColor;\n"
        "}\n\0";

    GLfloat spriteVertexData[] = {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        // -0.5f, -0.5f,  0.0f,
    };

    loadShaders(vertexShaderSource, fragmentShaderSource);
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
