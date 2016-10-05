#include "Nito/Graphics.hpp"

#include <stdexcept>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Magick++.h>
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Container.hpp"


using std::map;
using std::vector;
using std::string;
using std::runtime_error;
using std::size_t;

// glm/glm.hpp
using glm::vec3;
using glm::vec4;
using glm::mat4;

// glm/gtc/matrix_transform.hpp
using glm::translate;
using glm::ortho;

// glm/gtc/type_ptr.hpp
using glm::value_ptr;

// Magick++.h
using Magick::Blob;
using Magick::Image;

// Cpp_Utils/Fn.hpp
using Cpp_Utils::accumulate;
using Cpp_Utils::transform;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::get_values;

// Cpp_Utils/Container.hpp
using Cpp_Utils::for_each;


#define DEBUG


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
    const GLboolean is_normalized;
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
static map<string, GLuint> texture_objects;
static map<string, GLuint> shader_programs;
static mat4 projection_matrix;
static mat4 view_matrix;
static vec3 unit_scale;


Vertex_Attribute::Types Vertex_Attribute::types
{
    { "float", { GL_FLOAT, sizeof(GLfloat) } },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static Vertex_Attribute create_vertex_attribute(
    const string & type_name,
    const GLint element_count,
    const GLboolean is_normalized)
{
    if (!contains_key(Vertex_Attribute::types, type_name))
    {
        throw runtime_error("ERROR: " + type_name + " is not a valid vertex attribute type!");
    }

    const Vertex_Attribute::Type & type = Vertex_Attribute::types.at(type_name);

    return
    {
        type,
        element_count,
        is_normalized,
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

    const vector<const GLchar *> source_code =
        transform<const GLchar *>(sources, [](const string & source) -> const GLchar *
        {
            return source.c_str();
        });

    glShaderSource(shader_object, source_count, &source_code[0], nullptr);
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
        glGetUniformLocation(shader_program, uniform_name),
        1,                         // Matrices to be modified (1 if target is not an array)
        transpose,                 // Transpose matrix (must be GL_FALSE apparently?)
        value_ptr(uniform_value)); // Pointer to uniform value
}


static void validate_no_opengl_error(const string & description)
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
            contains_key(opengl_error_messages, error)
            ? opengl_error_messages.at(error)
            : "An unknown OpenGL error occurred!";

        throw runtime_error("OPENGL ERROR: " + description + ": " + error_message);
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
    static const map<string, const GLenum> blending_factors
    {
        { "zero"                     , GL_ZERO                     },
        { "one"                      , GL_ONE                      },
        { "src_color"                , GL_SRC_COLOR                },
        { "one_minus_src_color"      , GL_ONE_MINUS_SRC_COLOR      },
        { "dst_color"                , GL_DST_COLOR                },
        { "one_minus_dst_color"      , GL_ONE_MINUS_DST_COLOR      },
        { "src_alpha"                , GL_SRC_ALPHA                },
        { "one_minus_src_alpha"      , GL_ONE_MINUS_SRC_ALPHA      },
        { "dst_alpha"                , GL_DST_ALPHA                },
        { "one_minus_dst_alpha"      , GL_ONE_MINUS_DST_ALPHA      },
        { "constant_color"           , GL_CONSTANT_COLOR           },
        { "one_minus_constant_color" , GL_ONE_MINUS_CONSTANT_COLOR },
        { "constant_alpha"           , GL_CONSTANT_ALPHA           },
        { "one_minus_constant_alpha" , GL_ONE_MINUS_CONSTANT_ALPHA },
        { "src_alpha_saturate"       , GL_SRC_ALPHA_SATURATE       },
        { "src1_color"               , GL_SRC1_COLOR               },
        { "one_minus_src1_color"     , GL_ONE_MINUS_SRC1_COLOR     },
        { "src1_alpha"               , GL_SRC1_ALPHA               },
        { "one_minus_src1_alpha"     , GL_ONE_MINUS_SRC1_ALPHA     },
    };


    // Configure viewport.
    glViewport(0, 0, opengl_config.window_width, opengl_config.window_height);

    projection_matrix =
        ortho(
            0.0f,                               // Left
            (float)opengl_config.window_width,  // Right
            0.0f,                               // Top
            (float)opengl_config.window_height, // Bottom
            opengl_config.z_near,               // Z near
            opengl_config.z_far);               // Z far


    // Configure clear color.
    const OpenGL_Config::Clear_Color & clear_color = opengl_config.clear_color;

    glClearColor(
        clear_color.red,
        clear_color.green,
        clear_color.blue,
        clear_color.alpha);


    // Configure blending.
    if (opengl_config.blending.is_enabled)
    {
        const OpenGL_Config::Blending & blending = opengl_config.blending;


        // Validate source & destination blending factor options.
        if (!contains_key(blending_factors, blending.source_factor))
        {
            throw runtime_error("\"" + blending.source_factor + "\" is not a valid source blending factor!");
        }

        if (!contains_key(blending_factors, blending.destination_factor))
        {
            throw runtime_error("\"" + blending.destination_factor + "\" is not a valid destination blending factor!");
        }


        glEnable(GL_BLEND);

        glBlendFunc(
            blending_factors.at(blending.source_factor),
            blending_factors.at(blending.destination_factor));
    }


    // Set unit scale, which determines how many pixels an entity moves when moved 1 unit.
    unit_scale = vec3(opengl_config.pixels_per_unit, opengl_config.pixels_per_unit, 1.0f);


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("configure_opengl()");
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


    // Process shader pipelines.
    for (const Shader_Pipeline & shader_pipeline : shader_pipelines)
    {
        // Create and compile shader objects from sources.
        for_each(shader_pipeline.shader_sources, [&](const string & shader_type, const vector<string> & sources) -> void
        {
            if (!contains_key(shader_types, shader_type))
            {
                throw runtime_error("ERROR: " + shader_type + " is not a valid shader type!");
            }

            GLuint shader_object = glCreateShader(shader_types.at(shader_type));
            compile_shader_object(shader_object, sources);
            shader_objects.push_back(shader_object);
        });


        // Create, attach shader objects to and link shader program.
        shader_program = glCreateProgram();

        for (const GLuint shader_object : shader_objects)
        {
            glAttachShader(shader_program, shader_object);
        }

        glLinkProgram(shader_program);


        // Validate linking was successful, then track shader program.
        validate_parameter_is(
            shader_program,
            GL_LINK_STATUS,
            GL_TRUE,
            glGetProgramiv,
            glGetProgramInfoLog);

        shader_programs[shader_pipeline.name] = shader_program;


        // Detach and delete shaders, as they are no longer needed by anything.
        for (const GLuint shader_object : shader_objects)
        {
            glDetachShader(shader_program, shader_object);
        }

        for_each(shader_objects, glDeleteShader);


        // Clear current pipeline's data.
        shader_objects.clear();
        shader_program = 0;
    }


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_shader_pipelines()");
}


void load_textures(const vector<Texture> & textures)
{
    static const map<string, const GLint> texture_option_keys
    {
        { "wrap_s"     , GL_TEXTURE_WRAP_S     },
        { "wrap_t"     , GL_TEXTURE_WRAP_T     },
        { "min_filter" , GL_TEXTURE_MIN_FILTER },
        { "mag_filter" , GL_TEXTURE_MAG_FILTER },
    };

    static const map<string, const GLint> texture_option_values
    {
        // Wrap values
        { "repeat"          , GL_REPEAT          },
        { "mirrored_repeat" , GL_MIRRORED_REPEAT },
        { "clamp_to_edge"   , GL_CLAMP_TO_EDGE   },

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
    const size_t texture_count = textures.size();
    vector<GLuint> texture_object_ids;
    texture_object_ids.reserve(texture_count);
    glGenTextures(texture_object_ids.capacity(), &texture_object_ids[0]);


    // Load data and configure options for textures.
    for (auto i = 0u; i < texture_count; i++)
    {
        const Texture & texture = textures[i];
        const GLuint texture_object = texture_object_ids[i];
        glBindTexture(GL_TEXTURE_2D, texture_object);


        // Configure options for the texture object.
        for_each(texture.options, [&](const string & option_key, const string & option_value) -> void
        {
            glTexParameteri(
                GL_TEXTURE_2D,
                texture_option_keys.at(option_key),
                texture_option_values.at(option_value));
        });


        // Load texture data from image at path.
        const Texture_Format texture_format = texture_formats.at(texture.format);
        Blob blob;
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


        // Track texture by its path.
        texture_objects[texture.path] = texture_object;
    }


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_textures()");
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
    size_t current_attribute_offset = 0;

    for (GLuint attribute_index = 0u; attribute_index < vertex_attributes.size(); attribute_index++)
    {
        const Vertex_Attribute & vertex_attribute = vertex_attributes[attribute_index];
        glEnableVertexAttribArray(attribute_index);

        glVertexAttribPointer(
            attribute_index,                     // Index of attribute
            vertex_attribute.element_count,      // Number of attribute elements
            vertex_attribute.type.gl_type,       // Type of attribute elements
            vertex_attribute.is_normalized,      // Should attribute elements be normalized?
            vertex_stride,                       // Stride between attributes
            (GLvoid *)current_attribute_offset); // Pointer offset to first element of attribute

        current_attribute_offset += vertex_attribute.size;
    }


    // Unbind vertex array first, that way unbinding GL_ELEMENT_ARRAY_BUFFER doesn't remove the index data from the
    // vertex array. GL_ARRAY_BUFFER can be unbound before or after the vertex array, but for consistency's sake we'll
    // unbind it after.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_vertex_data()");
}


void init_rendering()
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);


    // Bind sprite vertex array.
    glBindVertexArray(vertex_array_objects[0]);


#ifdef DEBUG
    validate_no_opengl_error("init_rendering()");
#endif
}


void render(const Sprite * sprite, const Transform * transform)
{
    // Create model matrix from renderable's transformations.
    mat4 model_matrix = scale(translate(mat4(),
        transform->position * unit_scale),
        transform->scale);


    // Bind texture to texture unit 0.
    bind_texture(texture_objects.at(sprite->texture_path), 0u);


    // Create matrix to scale model matrix to texture's dimensions.
    int texture_width;
    int texture_height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture_height);
    mat4 texture_scale_matrix = scale(mat4(), vec3(texture_width, texture_height, 1.0f));


    // Bind shader pipeline and set its uniforms.
    const GLuint shader_program = shader_programs.at(sprite->shader_pipeline_name);
    glUseProgram(shader_program);
    set_uniform(shader_program, "texture0", 0);
    set_uniform(shader_program, "projection", projection_matrix);
    set_uniform(shader_program, "view", view_matrix);
    set_uniform(shader_program, "model", model_matrix * texture_scale_matrix);


    // Draw data.
    glDrawElements(
        GL_TRIANGLES,    // Render mode
        6,               // Index count
        GL_UNSIGNED_INT, // Index type
        (GLvoid *)0);    // Pointer to start of index array


#ifdef DEBUG
    validate_no_opengl_error("render_graphics()");
#endif
}


void cleanup_rendering()
{
    // Unbind vertex array, textures and shader program.
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);


#ifdef DEBUG
    validate_no_opengl_error("cleanup_rendering()");
#endif
}


void destroy_graphics()
{
    // Delete vertex data.
    glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertex_array_objects);
    glDeleteBuffers(VERTEX_BUFFER_COUNT, vertex_buffer_objects);
    glDeleteBuffers(INDEX_BUFFER_COUNT, index_buffer_objects);


    // Delete shader pipelines.
    for_each(get_values(shader_programs), glDeleteProgram);
    shader_programs.clear();


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("destroy_graphics()");
}


} // namespace Nito
