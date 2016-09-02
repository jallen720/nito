#include "Nito/window.hpp"

#include <map>
#include <vector>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "CppUtils/StringUtils/toString.hpp"
#include "CppUtils/MapUtils/containsKey.hpp"

#include "Nito/input.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;
using CppUtils::toString;
using CppUtils::containsKey;

// Nito/input.hpp
using Nito::keyCallback;


namespace Nito
{


static vector<GLFWwindow *> windows;


static void errorCallback(int error, const char * description)
{
    throw runtime_error((string)"GLFW ERROR [" + toString(error) + "]: " + description + "!");
}


void initGLFW()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        throw runtime_error("ERROR: Failed to initialize GLFW!");
    }
}


GLFWwindow * createWindow(const WindowConfig & windowConfig)
{
    static const map<string, const int> swapIntervals
    {
        { "every-update", 1 },
        { "every-other-update", 2 },
    };

    if (!containsKey(swapIntervals, windowConfig.refreshRate))
    {
        throw runtime_error(
            "ERROR: \"" + windowConfig.refreshRate + "\" is not a valid refresh rate!");
    }


    // Window pre-configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, windowConfig.contextVersion.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, windowConfig.contextVersion.minor);


    // Window creation
    GLFWwindow * window =
        glfwCreateWindow(
            windowConfig.width,
            windowConfig.height,
            windowConfig.title.c_str(),
            nullptr,
            nullptr);

    if (window == nullptr)
    {
        throw runtime_error("GLFW ERROR: Failed to create GLFW window!");
    }

    windows.push_back(window);


    // Window post-configuration
    glfwMakeContextCurrent(window);
    glfwSwapInterval(swapIntervals.at(windowConfig.refreshRate));
    glfwSetKeyCallback(window, keyCallback);


    return window;
}


void terminateGLFW()
{
    // TODO: Add map function to CppUtils/VectorUtils
    for (GLFWwindow * window : windows)
    {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
}


} // namespace Nito
