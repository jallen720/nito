#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>


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
    const GLsizei windowWidth;
    const GLsizei windowHeight;
    const Color clearColor;
    const unsigned int pixelsPerUnit;
};


using ShaderPipeline = std::map<std::string, std::vector<std::string>>;


struct Texture {
    using Options = std::map<std::string, std::string>;

    std::string path;
    std::string format;
    Options options;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initGLEW();
void configureOpenGL(const OpenGLConfig & openGLConfig);
void loadShaderPipelines(const std::vector<ShaderPipeline> & shaderPipelines);
void loadTextures(const std::vector<Texture> & textures);


void loadVertexData(
    const GLvoid * vertexData,
    const GLsizeiptr vertexDataSize,
    const GLuint * indexData,
    const GLsizeiptr indexDataSize);


void renderGraphics();
void destroyGraphics();


} // namespace Nito
