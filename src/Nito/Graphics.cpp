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
struct Vertex_Attribute
{
    struct Type
    {
        const GLenum gl_type;
        const size_t size;
    };

    using Types = const map<string, const Type>;

    static Types types;

    const Type & type;
    const GLint element_count;
    const GLboolean is_normalize;
    const size_t size;
};


struct Texture_Format
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
static GLuint vertex_array_objects[VERTEX_ARRAY_COUNT];
static GLuint vertex_buffer_objects[VERTEX_BUFFER_COUNT];
static GLuint index_buffer_objects[INDEX_BUFFER_COUNT];
static vector<GLuint> texture_objects;
static vector<GLuint> shader_programs;
static mat4 projection_matrix;
static vec3 unit_scale;


Vertex_Attribute::Types Vertex_Attribute::types
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
static Vertex_Attribute create_vertex_attribute(
    const string & type_name,
    const GLint element_count,
    const GLboolean is_normalize)
{
    if (!containsKey(Vertex_Attribute::types, type_name))
    {
        throw runtime_error("ERROR: " + type_name + " is not a valid vertex attribute type!");
    }

    const Vertex_Attribute::Type & type = Vertex_Attribute::types.at(type_name);

    return
    {
        type,
        element_count,
        is_normalize,
        type.size * element_count,
    };
}


static void validate_parameter_is(
    const GLuint shader_entity,
    const GLenum parameter,
    const GLint expected_parameter_value,
    void (* get_parameter)(GLuint, GLenum, GLint *),
    void (* get_info_log)(GLuint, GLsizei, GLsizei *, GLchar *))
{
    GLint parameter_value;
    get_parameter(shader_entity, parameter, &parameter_value);


    // Throw shader entity info log if parameter_value is not as expected.
    if (parameter_value != expected_parameter_value)
    {
        GLint info_log_length;
        get_parameter(shader_entity, GL_INFO_LOG_LENGTH, &info_log_length);
        vector<GLchar> info_log(info_log_length);
        get_info_log(shader_entity, info_log.size(), nullptr, &info_log[0]);
        throw runtime_error("ERROR: " + string(info_log.begin(), info_log.end()));
    }
}


static void compile_shader_object(const GLuint shader_object, const vector<string> sources)
{
    // Attach sources and compile shader_object.
    const size_t source_count = sources.size();
    const GLchar * source_code[source_count];

    for (size_t i = 0; i < source_count; i++)
    {
        source_code[i] = sources[i].c_str();
    }

    glShaderSource(shader_object, source_count, source_code, nullptr);
    glCompileShader(shader_object);


    // Check for compile time errors.
    validate_parameter_is(
        shader_object,
        GL_COMPILE_STATUS,
        GL_TRUE,
        glGetShaderiv,
        glGetShaderInfoLog);
}


static void set_uniform(const GLuint shader_program, const GLchar * uniform_name, const vec4 & uniform_value)
{
    glUniform4f(
        glGetUniformLocation(shader_program, uniform_name),
        uniform_value.x,
        uniform_value.y,
        uniform_value.z,
        uniform_value.w);
}


static void set_uniform(const GLuint shader_program, const GLchar * uniform_name, const GLint uniform_value)
{
    glUniform1i(glGetUniformLocation(shader_program, uniform_name), uniform_value);
}


static void set_uniform(const GLuint shader_program, const GLchar * uniform_name, const GLfloat uniform_value)
{
    glUniform1f(glGetUniformLocation(shader_program, uniform_name), uniform_value);
}


static void set_uniform(
    const GLuint shader_program,
    const GLchar * uniform_name,
    const mat4 & uniform_value,
    const GLboolean transpose = GL_FALSE)
{
    glUniformMatrix4fv(
        glGetUniformLocation(shader_program, uniform_name), // Uniform location
        1,                                                // Matrices to be modified (1 if target is not an array)
        transpose,                                        // Transpose matric (must be GL_FALSE apparently?)
        value_ptr(uniform_value));                         // Pointer to uniform value
}


static void validate_no_opengl_error(const string & function_name)
{
    static const map<GLenum, const string> opengl_error_messages
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
        string error_message =
            containsKey(opengl_error_messages, error)
            ? opengl_error_messages.at(error)
            : "An unknown OpenGL error occurred!";

        throw runtime_error("OPENGL ERROR: in " + function_name + "(): " + error_message);
    }
}


static void bind_texture(const GLuint texture_object, const GLuint texture_unit)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_object);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_glew()
{
    // Init GLEW in experimental mode.
    glewExperimental = GL_TRUE;


    // Validate GLEW initialized properly.
    if (glewInit() != GLEW_OK)
    {
        throw runtime_error("GLEW ERROR: Failed to initialize GLEW!");
    }
}


void configure_opengl(const OpenGL_Config & opengl_config)
{
    // Configure viewport.
    glViewport(0, 0, opengl_config.window_width, opengl_config.window_height);

    projection_matrix =
        ortho(
            0.0f,                               // Left
            (float)opengl_config.window_width,  // Right
            0.0f,                               // Top
            (float)opengl_config.window_height, // Bottom
            0.1f,                               // Z near
            100.0f);                            // Z far


    // Configure clear color.
    const Color & clear_color = opengl_config.clear_color;

    glClearColor(
        clear_color.red,
        clear_color.green,
        clear_color.blue,
        clear_color.alpha);


    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);


    // Enable transparency.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Set unit scale, which determines how many pixels an entity moves when moved 1 unit.
    unit_scale = { opengl_config.pixels_per_unit, opengl_config.pixels_per_unit, 1.0f };


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("configure_opengl");
}


void load_shader_pipelines(const vector<Shader_Pipeline> & shader_pipelines)
{
    static const map<string, const GLenum> shader_types
    {
        { "vertex"   , GL_VERTEX_SHADER   },
        { "fragment" , GL_FRAGMENT_SHADER },
    };

    GLuint shader_program;
    vector<GLuint> shader_objects;

    const auto process_shader_objects = [&](void (* processShaderObject)(GLuint, GLuint)) -> void
    {
        for (const GLuint shader_object : shader_objects)
        {
            processShaderObject(shader_program, shader_object);
        }
    };


    // Process shader pipelines.
    for (const Shader_Pipeline & shader_pipeline : shader_pipelines)
    {
        // Create and compile shader objects from sources.
        forEach(shader_pipeline, [&](const string & shader_type, const vector<string> & sources) -> void
        {
            if (!containsKey(shader_types, shader_type))
            {
                throw runtime_error("ERROR: " + shader_type + " is not a valid shader type!");
            }

            GLuint shader_object = glCreateShader(shader_types.at(shader_type));
            compile_shader_object(shader_object, sources);
            shader_objects.push_back(shader_object);
        });


        // Create, attach shader objects to and link shader program.
        shader_program = glCreateProgram();
        process_shader_objects(glAttachShader);
        glLinkProgram(shader_program);


        // Track shader program if it linked successfully.
        validate_parameter_is(
            shader_program,
            GL_LINK_STATUS,
            GL_TRUE,
            glGetProgramiv,
            glGetProgramInfoLog);

        shader_programs.push_back(shader_program);


        // Detach and delete shaders, as they are no longer needed by anything.
        process_shader_objects(glDetachShader);
        forEach(shader_objects, glDeleteShader);


        // Clear current pipeline's data.
        shader_objects.clear();
        shader_program = 0;
    }


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_shader_pipelines");
}


void load_textures(const vector<Texture> & textures)
{
    static const map<string, const GLint> texture_option_keys
    {
        { "wrap-s"     , GL_TEXTURE_WRAP_S     },
        { "wrap-t"     , GL_TEXTURE_WRAP_T     },
        { "min-filter" , GL_TEXTURE_MIN_FILTER },
        { "mag-filter" , GL_TEXTURE_MAG_FILTER },
    };

    static const map<string, const GLint> texture_option_values
    {
        // Wrap values
        { "repeat"          , GL_REPEAT          },
        { "mirrored-repeat" , GL_MIRRORED_REPEAT },
        { "clamp-to-edge"   , GL_CLAMP_TO_EDGE   },

        // Filter values
        { "linear"  , GL_LINEAR  },
        { "nearest" , GL_NEAREST },
    };

    static const map<string, const Texture_Format> texture_formats
    {
        { "rgba" , { GL_RGBA , "RGBA" } },
        { "rgb"  , { GL_RGB  , "RGB"  } },
    };


    // Allocate memory to hold texture objects.
    texture_objects.reserve(textures.size());
    glGenTextures(texture_objects.capacity(), &texture_objects[0]);


    // Load data and configure options for textures.
    for (auto i = 0u; i < textures.size(); i++)
    {
        const Texture & texture = textures[i];
        glBindTexture(GL_TEXTURE_2D, texture_objects[i]);


        // Configure options for the texture object.
        forEach(texture.options, [&](const string & option_key, const string & option_value) -> void
        {
            glTexParameteri(
                GL_TEXTURE_2D,
                texture_option_keys.at(option_key),
                texture_option_values.at(option_value));
        });


        // Load texture data from image at path.
        const Texture_Format texture_format = texture_formats.at(texture.format);
        Blob  blob;
        Image image;
        image.read(texture.path);
        image.flip();
        image.write(&blob, texture_format.image);

        glTexImage2D(
            GL_TEXTURE_2D,           // Target
            0,                       // Level of detail (0 is base image LOD)
            texture_format.internal, // Internal format
            image.columns(),         // Image width
            image.rows(),            // Image height
            0,                       // Border width (must be 0 apparently?)
            texture_format.internal, // Texel data format (must match internal format)
            GL_UNSIGNED_BYTE,        // Texel data type
            blob.data());            // Pointer to image data


        // Unbind texture now that its data has been loaded.
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_textures");
}


void load_vertex_data(
    const GLvoid * vertex_data,
    const GLsizeiptr vertex_data_size,
    const GLuint * index_data,
    const GLsizeiptr index_data_size)
{
    // Vertex attribute specification
    static const vector<Vertex_Attribute> vertex_attributes
    {
        create_vertex_attribute("float", 3, GL_FALSE), // Position
        create_vertex_attribute("float", 2, GL_FALSE), // UV
    };

    static const GLsizei vertex_stride =
        accumulate(
            (GLsizei)0,
            vertex_attributes,
            [](const GLsizei total, const Vertex_Attribute & vertex_attribute) -> GLsizei
            {
                return total + vertex_attribute.size;
            });


    // Generate containers for vertex data.
    glGenVertexArrays(VERTEX_ARRAY_COUNT, vertex_array_objects);
    glGenBuffers(VERTEX_BUFFER_COUNT, vertex_buffer_objects);
    glGenBuffers(INDEX_BUFFER_COUNT, index_buffer_objects);


    // Bind the Vertex Array Object first, then bind and set vertex & index buffer data.
    glBindVertexArray(vertex_array_objects[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_objects[0]);

    glBufferData(
        GL_ARRAY_BUFFER,  // Target buffer to load data into
        vertex_data_size, // Size of data
        vertex_data,      // Pointer to data
        GL_STATIC_DRAW);  // Usage

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, // Target buffer to load data into
        index_data_size,         // Size of data
        index_data,              // Pointer to data
        GL_STATIC_DRAW);         // Usage


    // Define pointers to vertex attributes.
    size_t previous_attribute_size = 0;

    for (GLuint attribute_index = 0u; attribute_index < vertex_attributes.size(); attribute_index++)
    {
        const Vertex_Attribute & vertex_attribute = vertex_attributes[attribute_index];
        glEnableVertexAttribArray(attribute_index);

        glVertexAttribPointer(
            attribute_index,                    // Index of attribute
            vertex_attribute.element_count,     // Number of attribute elements
            vertex_attribute.type.gl_type,      // Type of attribute elements
            vertex_attribute.is_normalize,      // Should attribute elements be normalized?
            vertex_stride,                      // Stride between attributes
            (GLvoid *)previous_attribute_size); // Pointer to first element of attribute

        previous_attribute_size = vertex_attribute.size;
    }


    // Unbind vertex array first, that way unbinding GL_ELEMENT_ARRAY_BUFFER doesn't remove the index data from the
    // vertex array. GL_ARRAY_BUFFER can be unbound before or after the vertex array, but for consistency's sake we'll
    // unbind it after.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_vertex_data");
}


void render_graphics()
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Bind vertex array, textures and shader program, then draw data.
    const GLuint shader_program = shader_programs[0];
    glBindVertexArray(vertex_array_objects[0]);
    bind_texture(texture_objects[0], 0u);
    glUseProgram(shader_program);
    set_uniform(shader_program, "texture0", 0);


    // Create matrix to scale model to texture's dimensions.
    int texture_width;
    int texture_height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture_height);
    mat4 texture_scale_matrix = scale(mat4(), { texture_width, texture_height, 1.0f });


    // Setup matrices for current entity and bind them to shader program.
    mat4 view_matrix;
    set_uniform(shader_program, "projection", projection_matrix);
    set_uniform(shader_program, "view", view_matrix);

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
        mat4 model_matrix;
        model_matrix = translate(model_matrix, position * unit_scale);
        set_uniform(shader_program, "model", model_matrix * texture_scale_matrix);


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


    // !!! SHOULD ONLY BE UNCOMMENTED FOR DEBUGGING, AS render_graphics() RUNS EVERY FRAME. !!!
    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("render_graphics");
}


void destroy_graphics()
{
    // Delete vertex data.
    glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertex_array_objects);
    glDeleteBuffers(VERTEX_BUFFER_COUNT, vertex_buffer_objects);
    glDeleteBuffers(INDEX_BUFFER_COUNT, index_buffer_objects);


    // Delete shader pipelines.
    forEach(shader_programs, glDeleteProgram);
    shader_programs.clear();


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("destroy_graphics");
}


} // namespace Nito
