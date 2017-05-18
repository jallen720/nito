#include "Nito/APIs/Window.hpp"

#include <vector>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Container.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Window_Created_Handler> window_created_handlers;
static GLFWwindow * window;
static float delta_time;
static vec3 window_size;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char * description)
{
    throw runtime_error("GLFW ERROR [" + to_string(error) + "]: " + description + "!");
}


static void window_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    window_size.x = width;
    window_size.y = height;
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
        throw runtime_error("GLFW ERROR: failed to initialize GLFW!");
    }
}


void create_window(const Window_Config & window_config)
{
    static const map<string, const int> SWAP_INTERVALS
    {
        { "every_update"       , 1 },
        { "every_other_update" , 2 },
    };

    static const map<string, const int> HINT_KEYS
    {
        // Window related hints
        { "resizable", GLFW_RESIZABLE },

        // Context related hints
        { "context_version_major" , GLFW_CONTEXT_VERSION_MAJOR },
        { "context_version_minor" , GLFW_CONTEXT_VERSION_MINOR },
    };


    // TODO: Add support for multiple windows.
    // Ensure a window does not already exist.
    if (window != nullptr)
    {
        throw runtime_error(
            "ERROR: a window has already been created, and support for multiple windows has not been implemented!");
    }


    // Configure window hints.
    for_each(window_config.hints, [&](const string & hint_key, int hint_value) -> void
    {
        glfwWindowHint(HINT_KEYS.at(hint_key), hint_value);
    });


    // Window creation
    int window_width = window_config.width;
    int window_height = window_config.height;

    window = glfwCreateWindow(
        window_width,
        window_height,
        window_config.title.c_str(),
        nullptr,
        nullptr);

    if (window == nullptr)
    {
        throw runtime_error("GLFW ERROR: failed to create window!");
    }

    window_size.x = window_width;
    window_size.y = window_height;


    // Window post-configuration
    if (!contains_key(SWAP_INTERVALS, window_config.refresh_rate))
    {
        throw runtime_error("ERROR: \"" + window_config.refresh_rate + "\" is not a valid refresh rate!");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(SWAP_INTERVALS.at(window_config.refresh_rate));
    glfwSetWindowSizeCallback(window, window_size_callback);


    // Trigger window created handlers.
    for (const Window_Created_Handler & window_created_handler : window_created_handlers)
    {
        window_created_handler();
    }
}


void close_window()
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}


float get_time()
{
    return glfwGetTime();
}


float get_delta_time()
{
    return delta_time;
}


const vec3 & get_window_size()
{
    return window_size;
}


int get_window_key_button_action(int key)
{
    return glfwGetKey(window, key);
}


void add_window_created_handler(const Window_Created_Handler & window_created_handler)
{
    window_created_handlers.push_back(window_created_handler);
}


void set_window_key_handler(const Window_Key_Handler window_key_handler)
{
    glfwSetKeyCallback(window, window_key_handler);
}


void set_window_mouse_position_handler(const Window_Mouse_Position_Handler window_mouse_position_handler)
{
    glfwSetCursorPosCallback(window, window_mouse_position_handler);
}


void set_window_mouse_button_handler(const Window_Mouse_Button_Handler & window_mouse_button_handler)
{
    glfwSetMouseButtonCallback(window, window_mouse_button_handler);
}


void set_input_mode(int mode, int value)
{
    glfwSetInputMode(window, mode, value);
}


void run_window_loop(const Window_Loop_Callback & callback)
{
    delta_time = 0.02f;
    // float frame_start_time = get_time();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        callback();
        glfwSwapBuffers(window);
        // const float current_time = get_time();
        // delta_time = current_time - frame_start_time;
        // frame_start_time = current_time;
    }
}


void terminate_glfw()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}


} // namespace Nito
