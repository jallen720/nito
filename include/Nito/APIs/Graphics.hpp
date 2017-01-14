#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Nito/APIs/Resources.hpp"


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
    const std::string default_vertex_container_id;
    const std::vector<std::string> capabilities;
    const std::vector<std::string> clear_flags;
    const glm::vec4 clear_color;

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
        VEC4,
        MAT4,
    }
    type;

    const void * data;
};


enum class Render_Modes
{
    TRIANGLES,
    LINE_STRIP,
    LINES,
};


struct Render_Data
{
    using Uniforms = std::map<std::string, Uniform>;

    const Render_Modes render_mode;
    const std::string * layer_name;
    const std::string * texture_path;
    const std::string * shader_pipeline_name;
    const std::string * vertex_container_id;
    const Uniforms * uniforms;
    const glm::mat4 model_matrix;
};


struct Render_Canvas
{
    const float width;
    const float height;
    const float z_near;
    const float z_far;
    const glm::mat4 view_matrix;
};


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
    const std::string & id,
    const std::vector<GLfloat> & vertex_data,
    const std::vector<GLuint> & index_data);

void load_render_layer(const std::string & name, const std::string & render_space);
void load_render_data(const Render_Data & render_data);
void render(const Render_Canvas & render_canvas);
void cleanup_rendering();
void destroy_graphics();
float get_pixels_per_unit();
const std::string & get_default_vertex_container_id();


} // namespace Nito
