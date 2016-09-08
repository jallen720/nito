#include "Nito/Graphics.hpp"

#include <stdexcept>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Magick++.h>
#include "CppUtils/Fn/accumulate.hpp"
#include "CppUtils/MapUtils/containsKey.hpp"
#include "CppUtils/ContainerUtils/forEach.hpp"

#include "Nito/Debugging.hpp"


using std::map;
using std::vector;
using std::string;
using std::runtime_error;
using std::size_t;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::translate;
using glm::value_ptr;
using glm::ortho;
using Magick::Blob;
using Magick::Image;
using CppUtils::accumulate;
using CppUtils::containsKey;
using CppUtils::forEach;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexAttribute
{
    struct Type
    {
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


struct TextureFormat
{
    const GLenum internal;
    const std::string image;
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
static vector<GLuint> textureObjects;
static vector<GLuint> shaderPrograms;
static mat4 projectionMatrix;
static vec3 unitScale;


VertexAttribute::Types VertexAttribute::types
{
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
    if (!containsKey(VertexAttribute::types, typeName))
    {
        throw runtime_error("ERROR: " + typeName + " is not a valid vertex attribute type!");
    }

    const VertexAttribute::Type & type = VertexAttribute::types.at(typeName);

    return
    {
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
    if (parameterValue != expectedParameterValue)
    {
        GLint infoLogLength;
        getParameter(shaderEntity, GL_INFO_LOG_LENGTH, &infoLogLength);
        vector<GLchar> infoLog(infoLogLength);
        getInfoLog(shaderEntity, infoLog.size(), nullptr, &infoLog[0]);
        throw runtime_error("ERROR: " + string(infoLog.begin(), infoLog.end()));
    }
}


static void compileShaderObject(const GLuint shaderObject, const vector<string> sources)
{
    // Attach sources and compile shaderObject.
    const size_t sourceCount = sources.size();
    const GLchar * sourceCode[sourceCount];

    for (size_t i = 0; i < sourceCount; i++)
    {
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


static void setUniform(const GLuint shaderProgram, const GLchar * uniformName, const vec4 & uniformValue)
{
    glUniform4f(
        glGetUniformLocation(shaderProgram, uniformName),
        uniformValue.x,
        uniformValue.y,
        uniformValue.z,
        uniformValue.w);
}


static void setUniform(const GLuint shaderProgram, const GLchar * uniformName, const GLint uniformValue)
{
    glUniform1i(glGetUniformLocation(shaderProgram, uniformName), uniformValue);
}


static void setUniform(const GLuint shaderProgram, const GLchar * uniformName, const GLfloat uniformValue)
{
    glUniform1f(glGetUniformLocation(shaderProgram, uniformName), uniformValue);
}


static void setUniform(
    const GLuint shaderProgram,
    const GLchar * uniformName,
    const mat4 & uniformValue,
    const GLboolean transpose = GL_FALSE)
{
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, uniformName), // Uniform location
        1,                                                // Matrices to be modified (1 if target is not an array)
        transpose,                                        // Transpose matric (must be GL_FALSE apparently?)
        value_ptr(uniformValue));                         // Pointer to uniform value
}


static void validateNoOpenGLError(const string & functionName)
{
    static const map<GLenum, const string> openGLErrorMessages
    {
        { GL_INVALID_ENUM                  , "Invalid enum"                  },
        { GL_INVALID_VALUE                 , "Invalid value"                 },
        { GL_INVALID_OPERATION             , "Invalid operation"             },
        { GL_INVALID_FRAMEBUFFER_OPERATION , "Invalid framebuffer operation" },
        { GL_OUT_OF_MEMORY                 , "Out of memory"                 },
        { GL_STACK_UNDERFLOW               , "Stack underflow"               },
        { GL_STACK_OVERFLOW                , "Stack overflow"                },
    };

    const GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        string errorMessage =
            containsKey(openGLErrorMessages, error)
            ? openGLErrorMessages.at(error)
            : "An unknown OpenGL error occurred!";

        throw runtime_error("OPENGL ERROR: in " + functionName + "(): " + errorMessage);
    }
}


static void bindTexture(const GLuint textureObject, const GLuint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureObject);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initGLEW()
{
    // Init GLEW in experimental mode.
    glewExperimental = GL_TRUE;


    // Validate GLEW initialized properly.
    if (glewInit() != GLEW_OK)
    {
        throw runtime_error("GLEW ERROR: Failed to initialize GLEW!");
    }
}


void configureOpenGL(const OpenGLConfig & openGLConfig)
{
    // Configure viewport.
    glViewport(0, 0, openGLConfig.windowWidth, openGLConfig.windowHeight);

    projectionMatrix =
        ortho(
            0.0f,                             // Left
            (float)openGLConfig.windowWidth,  // Right
            0.0f,                             // Top
            (float)openGLConfig.windowHeight, // Bottom
            0.1f,                             // Z near
            100.0f);                          // Z far


    // Configure clear color.
    const Color & clearColor = openGLConfig.clearColor;

    glClearColor(
        clearColor.red,
        clearColor.green,
        clearColor.blue,
        clearColor.alpha);


    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);


    // Enable transparency.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Set unit scale, which determines how many pixels an entity moves when moved 1 unit.
    unitScale = { openGLConfig.pixelsPerUnit, openGLConfig.pixelsPerUnit, 1.0f };


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("configureOpenGL");
}


void loadShaderPipelines(const vector<ShaderPipeline> & shaderPipelines)
{
    static const map<string, const GLenum> shaderTypes
    {
        { "vertex"   , GL_VERTEX_SHADER   },
        { "fragment" , GL_FRAGMENT_SHADER },
    };

    GLuint shaderProgram;
    vector<GLuint> shaderObjects;

    const auto processShaderObjects = [&](void (* processShaderObject)(GLuint, GLuint)) -> void
    {
        for (const GLuint shaderObject : shaderObjects)
        {
            processShaderObject(shaderProgram, shaderObject);
        }
    };


    // Process shader pipelines.
    for (const ShaderPipeline & shaderPipeline : shaderPipelines)
    {
        // Create and compile shader objects from sources.
        forEach(shaderPipeline, [&](const string & shaderType, const vector<string> & sources) -> void
        {
            if (!containsKey(shaderTypes, shaderType))
            {
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


void loadTextures(const vector<Texture> & textures)
{
    static const map<string, const GLint> textureOptionKeys
    {
        { "wrap-s"     , GL_TEXTURE_WRAP_S     },
        { "wrap-t"     , GL_TEXTURE_WRAP_T     },
        { "min-filter" , GL_TEXTURE_MIN_FILTER },
        { "mag-filter" , GL_TEXTURE_MAG_FILTER },
    };

    static const map<string, const GLint> textureOptionValues
    {
        // Wrap values
        { "repeat"          , GL_REPEAT          },
        { "mirrored-repeat" , GL_MIRRORED_REPEAT },
        { "clamp-to-edge"   , GL_CLAMP_TO_EDGE   },

        // Filter values
        { "linear"  , GL_LINEAR  },
        { "nearest" , GL_NEAREST },
    };

    static const map<string, const TextureFormat> textureFormats
    {
        { "rgba" , { GL_RGBA , "RGBA" } },
        { "rgb"  , { GL_RGB  , "RGB"  } },
    };


    // Allocate memory to hold texture objects.
    textureObjects.reserve(textures.size());
    glGenTextures(textureObjects.capacity(), &textureObjects[0]);


    // Load data and configure options for textures.
    for (auto i = 0u; i < textures.size(); i++)
    {
        const Texture & texture = textures[i];
        glBindTexture(GL_TEXTURE_2D, textureObjects[i]);


        // Configure options for the texture object.
        forEach(texture.options, [&](const string & optionKey, const string & optionValue) -> void
        {
            glTexParameteri(
                GL_TEXTURE_2D,
                textureOptionKeys.at(optionKey),
                textureOptionValues.at(optionValue));
        });


        // Load texture data from image at path.
        const TextureFormat textureFormat = textureFormats.at(texture.format);
        Blob  blob;
        Image image;
        image.read(texture.path);
        image.flip();
        image.write(&blob, textureFormat.image);

        glTexImage2D(
            GL_TEXTURE_2D,          // Target
            0,                      // Level of detail (0 is base image LOD)
            textureFormat.internal, // Internal format
            image.columns(),        // Image width
            image.rows(),           // Image height
            0,                      // Border width (must be 0 apparently?)
            textureFormat.internal, // Texel data format (must match internal format)
            GL_UNSIGNED_BYTE,       // Texel data type
            blob.data());           // Pointer to image data


        // Unbind texture now that its data has been loaded.
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("loadTextureData");
}


void loadVertexData(
    const GLvoid * vertexData,
    const GLsizeiptr vertexDataSize,
    const GLuint * indexData,
    const GLsizeiptr indexDataSize)
{
    // Vertex attribute specification
    static const vector<VertexAttribute> vertexAttributes
    {
        createVertexAttribute("float", 3, GL_FALSE), // Position
        createVertexAttribute("float", 2, GL_FALSE), // UV
    };

    static const GLsizei vertexStride =
        accumulate(
            (GLsizei)0,
            vertexAttributes,
            [](const GLsizei total, const VertexAttribute & vertexAttribute) -> GLsizei
            {
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

    for (GLuint index = 0u; index < vertexAttributes.size(); index++)
    {
        const VertexAttribute & vertexAttribute = vertexAttributes[index];
        glEnableVertexAttribArray(index);

        glVertexAttribPointer(
            index,                            // Index of attribute
            vertexAttribute.elementCount,     // Number of attribute elements
            vertexAttribute.type.glType,      // Type of attribute elements
            vertexAttribute.isNormalized,     // Should attribute elements be normalized?
            vertexStride,                     // Stride between attributes
            (GLvoid *)previousAttributeSize); // Pointer to first element of attribute

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


void renderGraphics()
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Bind vertex array, textures and shader program, then draw data.
    const GLuint shaderProgram = shaderPrograms[0];
    glBindVertexArray(vertexArrayObjects[0]);
    bindTexture(textureObjects[0], 0u);
    glUseProgram(shaderProgram);
    setUniform(shaderProgram, "texture0", 0);


    // Create matrix to scale model to texture's dimensions.
    int textureWidth;
    int textureHeight;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
    mat4 textureScaleMatrix = scale(mat4(), { textureWidth, textureHeight, 1.0f });


    // Setup matrices for current entity and bind them to shader program.
    mat4 viewMatrix;
    setUniform(shaderProgram, "projection", projectionMatrix);
    setUniform(shaderProgram, "view", viewMatrix);

    vec3 positions[]
    {
        vec3(0.0f, 0.0f, -0.5f),
        vec3(1.5f, 0.2f, -0.6f),
        vec3(1.3f, 0.0f, -0.5f),
        vec3(2.0f, 5.0f, -0.5f),
        vec3(9.5f, 6.2f, -0.5f),
        vec3(4.8f, 6.0f, -0.5f),
        vec3(2.4f, 1.4f, -0.5f),
        vec3(0.7f, 3.0f, -0.5f),
        vec3(1.5f, 2.0f, -0.5f),
        vec3(8.3f, 1.0f, -0.5f),
    };

    for (const vec3 & position : positions)
    {
        mat4 modelMatrix;
        modelMatrix = translate(modelMatrix, position * unitScale);
        setUniform(shaderProgram, "model", modelMatrix * textureScaleMatrix);

        // Draw data.
        glDrawElements(
            GL_TRIANGLES,    // Render mode
            6,               // Index count
            GL_UNSIGNED_INT, // Index type
            (GLvoid *)0);    // Pointer to start of index array
    }


    // Unbind vertex array, textures and shader program.
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);


    // !!! SHOULD ONLY BE UNCOMMENTED FOR DEBUGGING, AS renderGraphics() RUNS EVERY FRAME. !!!
    // Validate no OpenGL errors occurred.
    validateNoOpenGLError("renderGraphics");
}


void destroyGraphics()
{
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
