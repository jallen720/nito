#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Nito/Resources.hpp"
#include "Nito/Components.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OpenGL_Config
{
    const unsigned int pixels_per_unit;
    const std::vector<std::string> capabilities;
    const std::vector<std::string> clear_flags;

    const struct Clear_Color
    {
        const GLfloat red;
        const GLfloat green;
        const GLfloat blue;
        const GLfloat alpha;
    }
    clear_color;

    const struct Blending
    {
        const std::string source_factor;
        const std::string destination_factor;
    }
    blending;
};


struct Shader_Pipeline
{
    std::string name;
    std::map<std::string, std::vector<std::string>> shader_sources;
};


struct Uniform
{
    enum class Types
    {
        INT,
        VEC3,
        MAT4,
    }
    type;

    void * data;
};


using Shader_Pipeline_Uniforms = std::map<std::string, Uniform>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_glew();
void configure_opengl(const OpenGL_Config & opengl_config);
void load_shader_pipelines(const std::vector<Shader_Pipeline> & shader_pipelines);
void load_texture_data(const Texture & texture, const void * data, const std::string & identifier);

void load_vertex_data(
    const GLvoid * vertex_data,
    const GLsizeiptr vertex_data_size,
    const GLuint * index_data,
    const GLsizeiptr index_data_size);

void load_render_layer(const std::string & name, const std::string & render_space);

void load_render_data(
    const std::string * layer_name,
    const std::string * texture_path,
    const std::string * shader_pipeline_name,
    const Shader_Pipeline_Uniforms * shader_pipeline_uniforms,
    const Dimensions * dimensions,
    const glm::vec3 * position,
    const glm::vec3 * scale);

void init_rendering();
void render(const Dimensions * view_dimensions, const Viewport * viewport, const Transform * view_transform);
void cleanup_rendering();
void destroy_graphics();
const glm::vec3 & get_unit_scale();


} // namespace Nito
