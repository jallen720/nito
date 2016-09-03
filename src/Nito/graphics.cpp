#include "Nito/graphics.hpp"

#include <stdexcept>
#include <GLFW/glfw3.h>


using std::runtime_error;


namespace Nito {


void initGLEW() {
    // Init GLEW in experimental mode
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        throw runtime_error("GLEW ERROR: Failed to initialize GLEW!");
    }
}


void configureOpenGL(const OpenGLConfig & openGLConfig) {
    int width, height;
    glfwGetFramebufferSize(openGLConfig.contextWindow, &width, &height);
    glViewport(0, 0, width, height);

    const Color & clearColor = openGLConfig.clearColor;

    glClearColor(
        clearColor.red,
        clearColor.green,
        clearColor.blue,
        clearColor.alpha);
}


} // namespace Nito
