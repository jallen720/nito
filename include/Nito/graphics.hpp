#pragma once


#include <GL/glew.h>


struct GLFWwindow;


namespace Nito {


struct Color {
    const GLfloat red;
    const GLfloat green;
    const GLfloat blue;
    const GLfloat alpha;
};


struct OpenGLConfig {
    GLFWwindow * contextWindow;
    const Color clearColor;
};


void initGLEW();
void configureOpenGL(const OpenGLConfig & openGLConfig);


} // namespace Nito
