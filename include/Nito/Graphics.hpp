#pragma once


#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>

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
    const GLsizei window_width;
    const GLsizei window_height;
    const float z_near;
    const float z_far;
    const unsigned int pixels_per_unit;
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
        const bool is_enabled;
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


struct Texture
{
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
void init_glew();
void configure_opengl(const OpenGL_Config & opengl_config);
void load_shader_pipelines(const std::vector<Shader_Pipeline> & shader_pipelines);
void load_textures(const std::vector<Texture> & textures);

void load_vertex_data(
    const GLvoid * vertex_data,
    const GLsizeiptr vertex_data_size,
    const GLuint * index_data,
    const GLsizeiptr index_data_size);

void init_rendering();
void render(const Sprite * sprite, const Transform * transform);
void cleanup_rendering();
void destroy_graphics();


} // namespace Nito
