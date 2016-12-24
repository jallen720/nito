#include "Nito/APIs/Graphics.hpp"

#include <unordered_map>
#include <stdexcept>
#include <functional>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Vector.hpp"

#include "Nito/APIs/Resources.hpp"


using std::map;
using std::unordered_map;
using std::vector;
using std::string;
using std::runtime_error;
using std::function;
using std::size_t;

// glm/glm.hpp
using glm::vec3;
using glm::vec4;
using glm::mat4;

// glm/gtc/matrix_transform.hpp
using glm::translate;
using glm::scale;
using glm::rotate;
using glm::ortho;

// glm/gtc/type_ptr.hpp
using glm::value_ptr;

// Cpp_Utils/Fn.hpp
using Cpp_Utils::accumulate;
using Cpp_Utils::transform;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::get_values;

// Cpp_Utils/Container.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::sort;


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


struct Vertex_Container
{
    GLuint vertex_array;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLsizei index_count;
};


struct Texture_Format
{
    const GLenum internal;
    const std::string image;
};


struct Render_Layer
{
    vector<Render_Data> render_datas;
    vector<unsigned int> order;

    enum class Space
    {
        WORLD,
        SCREEN,
    }
    space;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, Vertex_Container> vertex_containers;
static map<string, GLuint> texture_objects;
static map<string, GLuint> shader_programs;
static float pixels_per_unit;
static GLbitfield clear_flags;
static unordered_map<string, Render_Layer> render_layers;
static string default_vertex_container_id;


Vertex_Attribute::Types Vertex_Attribute::types
{
    { "float", { GL_FLOAT, sizeof(GLfloat) } },
};


static const map<string, const GLenum> capabilities
{
    { "blend"        , GL_BLEND        },
    { "depth_test"   , GL_DEPTH_TEST   },
    { "scissor_test" , GL_SCISSOR_TEST },
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


static void set_uniform(const GLuint shader_program, const GLchar * uniform_name, const vec3 & uniform_value)
{
    glUniform3f(
        glGetUniformLocation(shader_program, uniform_name),
        uniform_value.x,
        uniform_value.y,
        uniform_value.z);
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
        { GL_INVALID_ENUM                  , "invalid enum"                  },
        { GL_INVALID_VALUE                 , "invalid value"                 },
        { GL_INVALID_OPERATION             , "invalid operation"             },
        { GL_INVALID_FRAMEBUFFER_OPERATION , "invalid framebuffer operation" },
        { GL_OUT_OF_MEMORY                 , "out of memory"                 },
        { GL_STACK_UNDERFLOW               , "stack underflow"               },
        { GL_STACK_OVERFLOW                , "stack overflow"                },
    };

    const GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        const string error_message =
            contains_key(opengl_error_messages, error)
            ? opengl_error_messages.at(error)
            : "an unknown OpenGL error occurred";

        throw runtime_error("OPENGL ERROR: " + description + ": " + error_message + "!");
    }
}


static void bind_texture(const GLuint texture_object, const GLuint texture_unit)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_object);
}


static void set_shader_pipeline_uniforms(const GLuint shader_program, const Render_Data::Uniforms * uniforms)
{
    for_each(*uniforms, [&](const string & uniform_name, const Uniform & uniform) -> void
    {
        switch (uniform.type)
        {
            case Uniform::Types::INT:
            {
                set_uniform(shader_program, uniform_name.c_str(), *((GLint *)uniform.data));
                break;
            }
            case Uniform::Types::VEC3:
            {
                set_uniform(shader_program, uniform_name.c_str(), *((vec3 *)uniform.data));
                break;
            }
            case Uniform::Types::VEC4:
            {
                set_uniform(shader_program, uniform_name.c_str(), *((vec4 *)uniform.data));
                break;
            }
            case Uniform::Types::MAT4:
            {
                set_uniform(shader_program, uniform_name.c_str(), *((mat4 *)uniform.data));
                break;
            }
        };
    });
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
        throw runtime_error("GLEW ERROR: failed to initialize GLEW!");
    }
}


void configure_opengl(const OpenGL_Config & opengl_config)
{
    static const map<string, const GLbitfield> clear_flag_masks
    {
        { "color_buffer_bit"   , GL_COLOR_BUFFER_BIT   },
        { "depth_buffer_bit"   , GL_DEPTH_BUFFER_BIT   },
        { "accum_buffer_bit"   , GL_ACCUM_BUFFER_BIT   },
        { "stencil_buffer_bit" , GL_STENCIL_BUFFER_BIT },
    };

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


    // Configure capabilities.
    for (const string & capability : opengl_config.capabilities)
    {
        glEnable(capabilities.at(capability));
    }


    // Validate and configure clear flags.
    if (opengl_config.clear_flags.size() == 0)
    {
        throw runtime_error("ERROR: no clear flags set in OpenGL config!");
    }

    for (const string & clear_flag : opengl_config.clear_flags)
    {
        clear_flags |= clear_flag_masks.at(clear_flag);
    }


    // Configure blending if enabled.
    if (glIsEnabled(capabilities.at("blend")))
    {
        const OpenGL_Config::Blending & blending = opengl_config.blending;


        // Validate source & destination blending factor options.
        if (!contains_key(blending_factors, blending.source_factor))
        {
            throw runtime_error(
                "ERROR: source blending factor \"" + blending.source_factor + "\" is not a valid blending factor!");
        }

        if (!contains_key(blending_factors, blending.destination_factor))
        {
            throw runtime_error(
                "ERROR: destination blending factor \"" + blending.destination_factor + "\" is not a valid blending " +
                "factor!");
        }


        glBlendFunc(
            blending_factors.at(blending.source_factor),
            blending_factors.at(blending.destination_factor));
    }


    // Configure clear color.
    const vec4 & clear_color = opengl_config.clear_color;

    glClearColor(
        clear_color.x,
        clear_color.y,
        clear_color.z,
        clear_color.w);


    // Disable byte-alignment restriction (required for font rendering).
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    // Set unit scale, which determines how many pixels an entity moves when moved 1 unit.
    pixels_per_unit = opengl_config.pixels_per_unit;


    // Set default vertex_container_id to be used when rendering if no id was specified.
    default_vertex_container_id = opengl_config.default_vertex_container_id;


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


void load_texture_data(const Texture & texture, const void * data, const string & identifier)
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

    static const map<string, const GLint> internal_formats
    {
        { "rgba" , GL_RGBA },
        { "rgb"  , GL_RGB  },
        { "r"    , GL_RED  },
    };


    // Create new texture object that will be used to load texture data.
    const Dimensions & dimensions = texture.dimensions;
    const GLuint internal_format = internal_formats.at(texture.format);
    GLuint texture_object;
    glGenTextures(1, &texture_object);
    glBindTexture(GL_TEXTURE_2D, texture_object);


    // Configure options for the texture object.
    for_each(texture.options, [&](const string & option_key, const string & option_value) -> void
    {
        glTexParameteri(
            GL_TEXTURE_2D,
            texture_option_keys.at(option_key),
            texture_option_values.at(option_value));
    });


    // Load texture data.
    glTexImage2D(
        GL_TEXTURE_2D,     // Target
        0,                 // Level of detail (0 is base image LOD)
        internal_format,   // Internal format
        dimensions.width,  // Image width
        dimensions.height, // Image height
        0,                 // Border width (must be 0 apparently?)
        internal_format,   // Texel data format (must match internal format)
        GL_UNSIGNED_BYTE,  // Texel data type
        data);             // Pointer to image data


    // Unbind texture object now that its data has been loaded.
    glBindTexture(GL_TEXTURE_2D, 0);


    // Track texture object by its identifier.
    texture_objects[identifier] = texture_object;


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("load_textures()");
}


void load_vertex_data(const string & id, const vector<GLfloat> & vertex_data, const vector<GLuint> & index_data)
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


    Vertex_Container & vertex_container = vertex_containers[id];
    GLuint & vertex_array = vertex_container.vertex_array;
    GLuint & vertex_buffer = vertex_container.vertex_buffer;
    GLuint & index_buffer = vertex_container.index_buffer;
    vertex_container.index_count = index_data.size();


    // Generate containers for vertex data.
    glGenVertexArrays(1, &vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);


    // Bind the vertex array object first, then bind and set vertex & index buffer data.
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    glBufferData(
        GL_ARRAY_BUFFER,                      // Target buffer to load data into
        vertex_data.size() * sizeof(GLfloat), // Size of data
        &vertex_data[0],                      // Pointer to data
        GL_STATIC_DRAW);                      // Usage

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,            // Target buffer to load data into
        index_data.size() * sizeof(GLuint), // Size of data
        &index_data[0],                     // Pointer to data
        GL_STATIC_DRAW);                    // Usage


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


void load_render_layer(const string & name, const string & render_space)
{
    static const map<string, const Render_Layer::Space> render_spaces
    {
        { "world"  , Render_Layer::Space::WORLD  },
        { "screen" , Render_Layer::Space::SCREEN },
    };

    Render_Layer & render_layer = render_layers[name];
    render_layer.space = render_spaces.at(render_space);
}


void load_render_data(const Render_Data & render_data)
{
    Render_Layer & render_layer = render_layers[*render_data.layer_name];
    render_layer.render_datas.push_back(render_data);
    render_layer.order.push_back(render_layer.order.size());
}


void render(const Render_Canvas & render_canvas)
{
    static const map<Render_Modes, const GLenum> GL_RENDER_MODES
    {
        { Render_Modes::TRIANGLES  , GL_TRIANGLES  },
        { Render_Modes::LINE_STRIP , GL_LINE_STRIP },
    };

    const float canvas_x = render_canvas.x;
    const float canvas_y = render_canvas.y;
    const float canvas_width = render_canvas.width;
    const float canvas_height = render_canvas.height;


    // Set orthographic projection based on render_canvas dimensions.
    mat4 projection_matrix = ortho(
        0.0f,                                   // Left
        canvas_width,                           // Right
        0.0f,                                   // Top
        canvas_height,                          // Bottom
        render_canvas.z_near * pixels_per_unit, // Z near
        render_canvas.z_far * pixels_per_unit); // Z far


    // Configure OpenGL viewport.
    glViewport(
        canvas_x,
        canvas_y,
        canvas_width,
        canvas_height);


    // Configure scissor test if enabled.
    if (glIsEnabled(capabilities.at("scissor_test")))
    {
        glScissor(
            canvas_x,
            canvas_y,
            canvas_width,
            canvas_height);
    }


    // Clear buffers specified by clear_flags.
    glClear(clear_flags);


    // Set projection matrices for all shader programs.
    for_each(shader_programs, [&](const string & /*shader_pipeline_name*/, const GLuint shader_program) -> void
    {
        glUseProgram(shader_program);
        set_uniform(shader_program, "projection", projection_matrix);
    });


    // Render all layers.
    for_each(render_layers, [&](const string & /*layer_name*/, Render_Layer & render_layer) -> void
    {
        const vector<Render_Data> & render_datas = render_layer.render_datas;


        // Sort render layer order.
        sort(render_layer.order, [&](const unsigned int a, const unsigned int b) -> bool
        {
            return render_datas[a].model_matrix[3][2] >
                   render_datas[b].model_matrix[3][2];
        });


        // Set view matrices for all shader programs.
        auto layer_view_matrix =
            render_layer.space == Render_Layer::Space::WORLD
            ? render_canvas.view_matrix
            : mat4();

        for_each(shader_programs, [&](const string & /*shader_pipeline_name*/, const GLuint shader_program) -> void
        {
            glUseProgram(shader_program);
            set_uniform(shader_program, "view", layer_view_matrix);
        });


        // Render all data in layer.
        for (unsigned int index : render_layer.order)
        {
            const Render_Data & render_data = render_datas[index];
            const Render_Data::Uniforms * uniforms = render_data.uniforms;
            const string * vertex_container_id = render_data.vertex_container_id;
            const string * texture_path = render_data.texture_path;

            const Vertex_Container & vertex_container = vertex_containers.at(
                vertex_container_id == nullptr
                ? default_vertex_container_id
                : *vertex_container_id);


            // Bind vertex array containing vertex data to be rendered.
            glBindVertexArray(vertex_container.vertex_array);


            // Bind texture to texture unit 0.
            if (texture_path != nullptr)
            {
                bind_texture(texture_objects.at(*texture_path), 0u);
            }


            // Bind shader pipeline and set its uniforms.
            const GLuint shader_program = shader_programs.at(*render_data.shader_pipeline_name);
            glUseProgram(shader_program);
            set_uniform(shader_program, "model", render_data.model_matrix);

            if (texture_path != nullptr)
            {
                set_uniform(shader_program, "texture0", 0);
            }


            // Set custom shader pipeline uniforms if any were passed.
            if (uniforms != nullptr)
            {
                set_shader_pipeline_uniforms(shader_program, uniforms);
            }


            // Draw data.
            glDrawElements(
                GL_RENDER_MODES.at(render_data.render_mode), // Render mode
                vertex_container.index_count,                // Index count
                GL_UNSIGNED_INT,                             // Index type
                (GLvoid *)0);                                // Pointer to start of index array
        }
    });


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


    // Clear rendering data.
    for_each(render_layers, [](const string & /*layer_name*/, Render_Layer & render_layer) -> void
    {
        render_layer.render_datas.clear();
        render_layer.order.clear();
    });


#ifdef DEBUG
    validate_no_opengl_error("cleanup_rendering()");
#endif
}


void destroy_graphics()
{
    // Delete vertex data.
    for_each(vertex_containers, [](const string & /*id*/, const Vertex_Container & vertex_container) -> void
    {
        glDeleteVertexArrays(1, &vertex_container.vertex_array);
        glDeleteBuffers(1, &vertex_container.vertex_buffer);
        glDeleteBuffers(1, &vertex_container.index_buffer);
    });

    vertex_containers.clear();


    // Delete shader pipelines.
    for_each(get_values(shader_programs), glDeleteProgram);
    shader_programs.clear();


    // Validate no OpenGL errors occurred.
    validate_no_opengl_error("destroy_graphics()");
}


float get_pixels_per_unit()
{
    return pixels_per_unit;
}


const string & get_default_vertex_container_id()
{
    return default_vertex_container_id;
}


} // namespace Nito
