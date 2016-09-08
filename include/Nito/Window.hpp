#pragma once


#include <string>


struct GLFWwindow;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ContextVersion
{
    const int major;
    const int minor;
};


struct WindowConfig
{
    const int width;
    const int height;
    const std::string title;
    const std::string refreshRate;
    const ContextVersion contextVersion;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initGLFW();
GLFWwindow * createWindow(const WindowConfig & windowConfig);
void terminateGLFW();


} // namespace Nito
