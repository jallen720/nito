#include <string>
#include <vector>
#include <cstdio>
#include <GLFW/glfw3.h>

#include "CppUtils/JSON/JSON.hpp"
#include "CppUtils/JSON/loadJSONFile.hpp"
#include "Nito/input.hpp"


using std::string;
using std::vector;
using CppUtils::JSON;
using CppUtils::loadJSONFile;

// Nito/input.hpp
using Nito::keyCallback;
using Nito::addControlBinding;
using Nito::setControlHandler;


static void errorCallback(int error, const char * description)
{
    printf("GLFW Error [%d]: %s\n", error, description);
}


int main()
{
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        puts("Failed to initialize GLFW!");
        return 1;
    }


    // Create window
    JSON windowConfig = loadJSONFile("configs/window.json");
    const JSON & glfwContextVersion = windowConfig["glfw-context-version"];
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfwContextVersion["major"]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfwContextVersion["minor"]);

    GLFWwindow * window =
        glfwCreateWindow(
            windowConfig["width"],
            windowConfig["height"],
            windowConfig["title"].get<string>().c_str(),
            nullptr,
            nullptr);

    if (window == nullptr)
    {
        puts("Failed to create GLFW window!");
        glfwTerminate();
        return 1;
    }


    // Configure window
    glfwSetKeyCallback(window, keyCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    // Set control handlers
    auto exitHandler = [&](GLFWwindow * window, const int /*key*/, const int /*action*/) -> void
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    };

    auto printHandler = [](GLFWwindow * /*window*/, const int key, const int action) -> void
    {
        printf("key [%d] action [%d]\n", key, action);
    };

    setControlHandler("exit", exitHandler);
    setControlHandler("print", printHandler);


    // Load control bindings
    const vector<JSON> controls = loadJSONFile("configs/controls.json");

    for (const JSON & controlBinding : controls)
    {
        addControlBinding(
            controlBinding["key"],
            controlBinding["action"],
            controlBinding["handler"]);
    }


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}
