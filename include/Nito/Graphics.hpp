#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>


struct GLFWwindow;


namespace Nito {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


using ShaderPipeline = std::map<std::string, std::vector<std::string>>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initGLEW();
void configureOpenGL(const OpenGLConfig & openGLConfig);
void loadShaderPipelines(const std::vector<ShaderPipeline> & shaderPipelines);


void loadVertexData(
    const GLvoid * vertexData,
    const GLsizeiptr vertexDataSize,
    const GLuint * indexData,
    const GLsizeiptr indexDataSize);


void renderGraphics();
void destroyGraphics();


} // namespace Nito
