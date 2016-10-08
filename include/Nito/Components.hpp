#pragma once


#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace Nito
{


struct Transform
{
    glm::vec3 position;
    glm::vec3 scale;
};


struct Sprite
{
    std::string texture_path;
    std::string shader_pipeline_name;
};


struct Viewport
{
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    float z_near;
    float z_far;
};


} // namespace Nito
