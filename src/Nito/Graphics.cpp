#include "Nito/Graphics.hpp"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstddef>
#include <GLFW/glfw3.h>
#include "CppUtils/Fn/accumulate.hpp"
#include "CppUtils/MapUtils/containsKey.hpp"
#include "CppUtils/ContainerUtils/forEach.hpp"


using std::map;
using std::vector;
using std::string;
using std::runtime_error;
using std::size_t;
using CppUtils::accumulate;
using CppUtils::containsKey;
using CppUtils::forEach;


namespace Nito {


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexAttribute {
    struct Type {
        const GLenum glType;
        const size_t size;
    };

    using Types = const map<string, const Type>;

    static Types types;

    const Type & type;
    const GLint elementCount;
    const GLboolean isNormalized;
    const size_t size;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////
static const GLsizei VERTEX_ARRAY_COUNT  = 1;
static const GLsizei VERTEX_BUFFER_COUNT = 1;
static GLuint vertexArrayObjects[VERTEX_ARRAY_COUNT];
static GLuint vertexBufferObjects[VERTEX_BUFFER_COUNT];
static vector<GLuint> shaderPrograms;


VertexAttribute::Types VertexAttribute::types = {
    {
        "float",
        {
            GL_FLOAT,
            sizeof(GLfloat),
        },
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////
static VertexAttribute createVertexAttribute(
    const string & typeName,
    const GLint elementCount,
    const GLboolean isNormalized)
{
    if (!containsKey(VertexAttribute::types, typeName)) {
        throw runtime_error("ERROR: " + typeName + " is not a valid vertex attribute type!");
    }

    const VertexAttribute::Type type = VertexAttribute::types.at(typeName);

    return {
        type,
        elementCount,
        isNormalized,
        type.size * elementCount
    };
}


static void validateParameterIs(
    const GLuint shaderEntity,
    const GLenum parameter,
    const GLint expectedParameterValue,
    void (* getParameter)(GLuint, GLenum, GLint *),
    void (* getInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *))
{
    GLint parameterValue;
    getParameter(shaderEntity, parameter, &parameterValue);


    // Throw shader entity info log if parameterValue is not as expected.
    if (parameterValue != expectedParameterValue) {
        GLint infoLogLength;
        getParameter(shaderEntity, GL_INFO_LOG_LENGTH, &infoLogLength);
        vector<GLchar> infoLog(infoLogLength);
        getInfoLog(shaderEntity, infoLog.size(), nullptr, &infoLog[0]);
        throw runtime_error("ERROR: " + string(infoLog.begin(), infoLog.end()));
    }
}


static void compileShaderObject(const GLuint shaderObject, const GLchar * source) {
    // Attach source and compile shaderObject.
    glShaderSource(shaderObject, 1, &source, nullptr);
    glCompileShader(shaderObject);


    // Check for compile time errors.
    validateParameterIs(
        shaderObject,
        GL_COMPILE_STATUS,
        GL_TRUE,
        glGetShaderiv,
        glGetShaderInfoLog);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void initGLEW() {
    // Init GLEW in experimental mode.
    glewExperimental = GL_TRUE;


    // Validate GLEW initialized properly.
    if (glewInit() != GLEW_OK) {
        throw runtime_error("GLEW ERROR: Failed to initialize GLEW!");
    }
}


void configureOpenGL(const OpenGLConfig & openGLConfig) {
    // Configure viewport.
    int width, height;
    glfwGetFramebufferSize(openGLConfig.contextWindow, &width, &height);
    glViewport(0, 0, width, height);


    // Configure clear color.
    const Color & clearColor = openGLConfig.clearColor;

    glClearColor(
        clearColor.red,
        clearColor.green,
        clearColor.blue,
        clearColor.alpha);
}


void loadShaders(const GLchar * vertexShaderSource, const GLchar * fragmentShaderSource) {
    // Create and compile shader objects from sources.
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    compileShaderObject(vertexShaderObject, vertexShaderSource);
    compileShaderObject(fragmentShaderObject, fragmentShaderSource);


    // Create, attach shader objects to and link shader program.
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderObject);
    glAttachShader(shaderProgram, fragmentShaderObject);
    glLinkProgram(shaderProgram);


    // Check for linking errors.
    validateParameterIs(
        shaderProgram,
        GL_LINK_STATUS,
        GL_TRUE,
        glGetProgramiv,
        glGetProgramInfoLog);


    // If shader program linking was successful, track shader program.
    shaderPrograms.push_back(shaderProgram);


    // Detach and delete shaders, as they are no longer needed by anything.
    glDetachShader(shaderProgram, vertexShaderObject);
    glDetachShader(shaderProgram, fragmentShaderObject);
    glDeleteShader(vertexShaderObject);
    glDeleteShader(fragmentShaderObject);
}


void loadVertexData(const GLvoid * vertexData, const GLsizeiptr vertexDataSize) {
    // Vertex attribute specification
    static const vector<VertexAttribute> vertexAttributes {
        createVertexAttribute("float", 3, GL_FALSE), // Position
    };

    static const GLsizei vertexStride =
        accumulate(
            (GLsizei)0,
            vertexAttributes,
            [](const GLsizei total, const VertexAttribute & vertexAttribute) -> GLsizei {
                return total + vertexAttribute.size;
            });


    // Generate containers for vertex data.
    glGenVertexArrays(VERTEX_ARRAY_COUNT, vertexArrayObjects);
    glGenBuffers(VERTEX_BUFFER_COUNT, vertexBufferObjects);


    // Bind the Vertex Array Object first, then bind and set vertex buffer data.
    glBindVertexArray(vertexArrayObjects[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjects[0]);

    glBufferData(
        GL_ARRAY_BUFFER, // Target buffer to load data into
        vertexDataSize,  // Size of data
        vertexData,      // Pointer to data
        GL_STATIC_DRAW); // Usage


    // Define pointers to vertex attributes.
    size_t previousAttributeSize = 0;

    for (GLuint i = 0u; i < vertexAttributes.size(); i++) {
        const VertexAttribute & vertexAttribute = vertexAttributes[i];
        glEnableVertexAttribArray(i);

        glVertexAttribPointer(
            i,
            vertexAttribute.elementCount,
            vertexAttribute.type.glType,
            vertexAttribute.isNormalized,
            vertexStride,
            (GLvoid *)previousAttributeSize);

        previousAttributeSize = vertexAttribute.size;
    }


    // Note that this is allowed, the call to glVertexAttribPointer registered vertexBufferObjects
    // as the currently bound vertex buffer object so afterwards we can safely unbind.
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Unbind vertexArrayObjects (it's always a good thing to unbind any buffer/array to prevent
    // strange bugs).
    glBindVertexArray(0);
}


void renderGraphics() {
    // static const GLenum   MODE        = GL_TRIANGLES;
    // static const GLsizei  INDEX_COUNT = 4;
    // static const GLenum   TYPE        = GL_UNSIGNED_INT;
    // static const GLvoid * FIRST       = 0;


    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);


    // Bind shader program and vertex array for rendering.
    glUseProgram(shaderPrograms[0]);
    glBindVertexArray(vertexArrayObjects[0]);


    // Draw bound vertex data.
    // glDrawElements(MODE, INDEX_COUNT, TYPE, FIRST);
    glDrawArrays(GL_TRIANGLES, 0, 3);


    // Unbind shader program and vertex array.
    glBindVertexArray(0);
    glUseProgram(0);
}


void destroyGraphics() {
    // Delete all tracked graphics assets.
    glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertexArrayObjects);
    glDeleteBuffers(VERTEX_BUFFER_COUNT, vertexBufferObjects);
    forEach(shaderPrograms, glDeleteProgram);
}


} // namespace Nito
