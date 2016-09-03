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
void loadShaders(const GLchar * vertexShaderSource, const GLchar * fragmentShaderSource);
void loadVertexData(const GLvoid * vertexData, const GLsizeiptr vertexDataSize);
void renderGraphics();
void destroyGraphics();


} // namespace Nito
