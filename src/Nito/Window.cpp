#include "Nito/Window.hpp"

#include <map>
#include <vector>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "CppUtils/StringUtils/toString.hpp"
#include "CppUtils/MapUtils/containsKey.hpp"
#include "CppUtils/ContainerUtils/forEach.hpp"

#include "Nito/Input.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;
using CppUtils::toString;
using CppUtils::containsKey;
using CppUtils::forEach;

// Nito/Input.hpp
using Nito::key_callback;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<GLFWwindow *> windows;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char * description)
{
    throw runtime_error((string)"GLFW ERROR [" + toString(error) + "]: " + description + "!");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_glfw()
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        throw runtime_error("ERROR: Failed to initialize GLFW!");
    }
}


GLFWwindow * create_window(const Window_Config & window_config)
{
    static const map<string, const int> swap_intervals
    {
        { "every_update"       , 1 },
        { "every_other_update" , 2 },
    };

    if (!containsKey(swap_intervals, window_config.refresh_rate))
    {
        throw runtime_error("ERROR: \"" + window_config.refresh_rate + "\" is not a valid refresh rate!");
    }


    // Window pre-configuration
    const Window_Config::Context_Version & context_version = window_config.context_version;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, context_version.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, context_version.minor);


    // Window creation
    GLFWwindow * window =
        glfwCreateWindow(
            window_config.width,
            window_config.height,
            window_config.title.c_str(),
            nullptr,
            nullptr);

    if (window == nullptr)
    {
        throw runtime_error("GLFW ERROR: Failed to create GLFW window!");
    }

    windows.push_back(window);


    // Window post-configuration
    glfwMakeContextCurrent(window);
    glfwSwapInterval(swap_intervals.at(window_config.refresh_rate));
    glfwSetKeyCallback(window, key_callback);


    return window;
}


void terminate_glfw()
{
    // Destroy all windows.
    forEach(windows, glfwDestroyWindow);
    windows.clear();


    glfwTerminate();
}


} // namespace Nito
