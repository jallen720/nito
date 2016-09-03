// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>

#include "CppUtils/JSON/JSON.hpp"
#include "CppUtils/JSON/loadJSONFile.hpp"
#include "Nito/window.hpp"
#include "Nito/input.hpp"
#include "Nito/graphics.hpp"


using std::string;
using std::vector;
using CppUtils::JSON;
using CppUtils::loadJSONFile;

// Nito/window.hpp
using Nito::initGLFW;
using Nito::createWindow;
using Nito::terminateGLFW;

// Nito/input.hpp
using Nito::addControlBinding;
using Nito::setControlHandler;

// Nito/graphics.hpp
using Nito::initGLEW;
using Nito::configureOpenGL;


int main() {
    initGLFW();


    // Create window
    JSON windowConfig = loadJSONFile("configs/window.json");
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
    const vector<JSON> controls = loadJSONFile("configs/controls.json");

    for (const JSON & controlBinding : controls) {
        addControlBinding(
            controlBinding["key"],
            controlBinding["action"],
            controlBinding["handler"]);
    }


    // Initialize graphics engine
    const JSON openGLConfig = loadJSONFile("configs/opengl.json");
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


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    terminateGLFW();
    return 0;
}
