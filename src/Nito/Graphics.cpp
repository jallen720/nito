#include "Nito/Graphics.hpp"

#include <stdexcept>
#include <cstddef>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include "CppUtils/Fn/accumulate.hpp"
#include "CppUtils/MapUtils/containsKey.hpp"
#include "CppUtils/ContainerUtils/forEach.hpp"


using std::map;
using std::vector;
using std::string;
using std::runtime_error;
using std::size_t;
using glm::vec4;
using CppUtils::accumulate;
using CppUtils::containsKey;
using CppUtils::forEach;


namespace Nito {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const GLsizei VERTEX_ARRAY_COUNT  = 1;
static const GLsizei VERTEX_BUFFER_COUNT = 1;
static const GLsizei INDEX_BUFFER_COUNT  = 1;
static GLuint vertexArrayObjects[VERTEX_ARRAY_COUNT];
static GLuint vertexBufferObjects[VERTEX_BUFFER_COUNT];
static GLuint indexBufferObjects[INDEX_BUFFER_COUNT];
static vector<GLuint> shaderPrograms;


VertexAttribute::Types VertexAttribute::types {
    {
        "float",
        {
            GL_FLOAT,
            sizeof(GLfloat),
        },
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static VertexAttribute createVertexAttribute(
    const string & typeName,
    const GLint elementCount,
    const GLboolean isNormalized)
{
    if (!containsKey(VertexAttribute::types, typeName)) {
        throw runtime_error("ERROR: " + typeName + " is not a valid vertex attribute type!");
    }

    const VertexAttribute::Type & type = VertexAttribute::types.at(typeName);

    return {
        type,
        elementCount,
        isNormalized,
        type.size * elementCount,
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


static void compileShaderObject(const GLuint shaderObject, const vector<string> sources) {
    // Attach sources and compile shaderObject.
    const size_t sourceCount = sources.size();
    const GLchar * sourceCode[sourceCount];

    for (size_t i = 0; i < sourceCount; i++) {
        sourceCode[i] = sources[i].c_str();
    }

    glShaderSource(shaderObject, sourceCount, sourceCode, nullptr);
    glCompileShader(shaderObject);


    // Check for compile time errors.
    validateParameterIs(
        shaderObject,
        GL_COMPILE_STATUS,
        GL_TRUE,
        glGetShaderiv,
        glGetShaderInfoLog);
}


static void setUniform(const GLuint shaderProgram, const GLchar * uniformName, const vec4 & value) {
    glUniform4f(
        glGetUniformLocation(shaderProgram, uniformName),
        value.x,
        value.y,
        value.z,
        value.w);
}


static void validateNoOpenGLError(const string & functionName) {
    static const map<GLenum, const string> openGLErrorMessages {
        { GL_INVALID_ENUM                  , "Invalid enum"                  },
        { GL_INVALID_VALUE                 , "Invalid value"                 },
        { GL_INVALID_OPERATION             , "Invalid operation"             },
        { GL_INVALID_FRAMEBUFFER_OPERATION , "Invalid framebuffer operation" },
        { GL_OUT_OF_MEMORY                 , "Out of memory"                 },
        { GL_STACK_UNDERFLOW               , "Stack underflow"               },
        { GL_STACK_OVERFLOW                , "Stack overflow"                },
    };

    const GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        string errorMessage =
            containsKey(openGLErrorMessages, error)
            ? openGLErrorMessages.at(error)
            : "An unknown OpenGL error occurred!";

        throw runtime_error("OPENGL ERROR: in " + functionName + "(): " + errorMessage);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("configureOpenGL");
}


void loadShaderPipelines(const vector<ShaderPipeline> & shaderPipelines) {
    static const map<string, const GLenum> shaderTypes {
        { "vertex"   , GL_VERTEX_SHADER   },
        { "fragment" , GL_FRAGMENT_SHADER },
    };

    GLuint shaderProgram;
    vector<GLuint> shaderObjects;

    const auto processShaderObjects = [&](void (* processShaderObject)(GLuint, GLuint)) -> void {
        for (const GLuint shaderObject : shaderObjects) {
            processShaderObject(shaderProgram, shaderObject);
        }
    };


    // Process shader pipelines.
    for (const ShaderPipeline & shaderPipeline : shaderPipelines) {
        // Create and compile shader objects from sources.
        forEach(shaderPipeline, [&](const string & shaderType, const vector<string> & sources) -> void {
            if (!containsKey(shaderTypes, shaderType)) {
                throw runtime_error("ERROR: " + shaderType + " is not a valid shader type!");
            }

            GLuint shaderObject = glCreateShader(shaderTypes.at(shaderType));
            compileShaderObject(shaderObject, sources);
            shaderObjects.push_back(shaderObject);
        });


        // Create, attach shader objects to and link shader program.
        shaderProgram = glCreateProgram();
        processShaderObjects(glAttachShader);
        glLinkProgram(shaderProgram);


        // Track shader program if it linked successfully.
        validateParameterIs(
            shaderProgram,
            GL_LINK_STATUS,
            GL_TRUE,
            glGetProgramiv,
            glGetProgramInfoLog);

        shaderPrograms.push_back(shaderProgram);


        // Detach and delete shaders, as they are no longer needed by anything.
        processShaderObjects(glDetachShader);
        forEach(shaderObjects, glDeleteShader);


        // Clear current pipeline's data.
        shaderObjects.clear();
        shaderProgram = 0;
    }


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("loadShaderPipelines");
}


void loadVertexData(
    const GLvoid * vertexData,
    const GLsizeiptr vertexDataSize,
    const GLuint * indexData,
    const GLsizeiptr indexDataSize)
{
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
    glGenBuffers(INDEX_BUFFER_COUNT, indexBufferObjects);


    // Bind the Vertex Array Object first, then bind and set vertex & index buffer data.
    glBindVertexArray(vertexArrayObjects[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjects[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObjects[0]);

    glBufferData(
        GL_ARRAY_BUFFER, // Target buffer to load data into
        vertexDataSize,  // Size of data
        vertexData,      // Pointer to data
        GL_STATIC_DRAW); // Usage

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, // Target buffer to load data into
        indexDataSize,           // Size of data
        indexData,               // Pointer to data
        GL_STATIC_DRAW);         // Usage


    // Define pointers to vertex attributes.
    size_t previousAttributeSize = 0;

    for (GLuint index = 0u; index < vertexAttributes.size(); index++) {
        const VertexAttribute & vertexAttribute = vertexAttributes[index];
        glEnableVertexAttribArray(index);

        glVertexAttribPointer(
            index,
            vertexAttribute.elementCount,
            vertexAttribute.type.glType,
            vertexAttribute.isNormalized,
            vertexStride,
            (GLvoid *)previousAttributeSize);

        previousAttributeSize = vertexAttribute.size;
    }


    // Unbind vertex array first, that way unbinding GL_ELEMENT_ARRAY_BUFFER doesn't remove the index data from the
    // vertex array. GL_ARRAY_BUFFER can be unbound before or after the vertex array, but for consistency's sake we'll
    // unbind it after.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("loadVertexData");
}


void renderGraphics() {
    static const GLenum   MODE        = GL_TRIANGLES;
    static const GLsizei  INDEX_COUNT = 6;
    static const GLenum   TYPE        = GL_UNSIGNED_INT;
    static const GLvoid * FIRST       = 0;


    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);


    // Use shader program and set its uniforms.
    const GLuint shaderProgram = shaderPrograms[0];
    glUseProgram(shaderProgram);
    setUniform(shaderProgram, "uniformColor", { (sin(glfwGetTime()) / 2) + 0.5, 0.0f, 0.0f, 1.0f });


    // Bind and draw vertex array.
    glBindVertexArray(vertexArrayObjects[0]);
    glDrawElements(MODE, INDEX_COUNT, TYPE, FIRST);


    // Unbind shader program and vertex array.
    glBindVertexArray(0);
    glUseProgram(0);


    // !!! SHOULD ONLY BE UNCOMMENTED FOR DEBUGGING, AS THIS FUNCTION RUNS EVERY FRAME. !!!
    // Validate no OpenGL errors occurred.
    // validateNoOpenGLError("renderGraphics");
}


void destroyGraphics() {
    // Delete vertex data.
    glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertexArrayObjects);
    glDeleteBuffers(VERTEX_BUFFER_COUNT, vertexBufferObjects);
    glDeleteBuffers(INDEX_BUFFER_COUNT, indexBufferObjects);


    // Delete shader pipelines.
    forEach(shaderPrograms, glDeleteProgram);
    shaderPrograms.clear();


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("destroyGraphics");
}


} // namespace Nito
