#pragma once


#include <functional>
#include <string>
#include <GLFW/glfw3.h>


namespace Nito {


using ControlHandler = std::function<void(GLFWwindow *, const int, const int)>;


void addControlBinding(
    const std::string & key,
    const std::string & action,
    const std::string & handler);

void setControlHandler(const std::string & name, const ControlHandler & controlHandler);
void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);


} // namespace Nito
