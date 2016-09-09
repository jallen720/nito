#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Color
{
    const GLfloat red;
    const GLfloat green;
    const GLfloat blue;
    const GLfloat alpha;
};


struct OpenGL_Config
{
    struct Blending
    {
        const bool is_enabled;
        const std::string s_factor;
        const std::string d_factor;
    };

    const GLsizei window_width;
    const GLsizei window_height;
    const unsigned int pixels_per_unit;
    const Color clear_color;
    const Blending blending;
};


struct Shader_Pipeline
{
    std::string name;
    std::map<std::string, std::vector<std::string>> shader_sources;
};


struct Texture
{
    using Options = std::map<std::string, std::string>;

    std::string path;
    std::string format;
    Options options;
};


struct Entity
{
    const std::string shader_pipeline_name;
    const glm::vec3 position;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_glew();
void configure_opengl(const OpenGL_Config & opengl_config);
void load_shader_pipelines(const std::vector<Shader_Pipeline> & shader_pipelines);
void load_textures(const std::vector<Texture> & textures);

void load_vertex_data(
    const GLvoid * vertex_data,
    const GLsizeiptr vertex_data_size,
    const GLuint * index_data,
    const GLsizeiptr index_data_size);

void add_entity(const Entity & entity);
void render_graphics();
void destroy_graphics();


} // namespace Nito
