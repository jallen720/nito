// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>
#include "CppUtils/JSON/JSON.hpp"
#include "CppUtils/JSON/readJSONFile.hpp"
#include "CppUtils/FileUtils/readFile.hpp"

#include "Nito/Window.hpp"
#include "Nito/Input.hpp"
#include "Nito/Graphics.hpp"


using std::string;
using std::vector;
using CppUtils::JSON;
using CppUtils::readJSONFile;
using CppUtils::readFile;

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
    const vector<JSON> controls = readJSONFile("resources/data/controls.json");

    for (const JSON & controlBinding : controls) {
        addControlBinding(
            controlBinding["key"],
            controlBinding["action"],
            controlBinding["handler"]);
    }


    // Initialize graphics engine
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


    // Load graphics data
    const string vertexShaderSource = readFile("resources/shaders/simple.vert");
    const string fragmentShaderSource = readFile("resources/shaders/simple.frag");

    GLfloat spriteVertexData[] = {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        // -0.5f, -0.5f,  0.0f,
    };

    loadShaders(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
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
