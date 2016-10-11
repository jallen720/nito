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


struct Viewport
{
    GLint x;
    GLint y;
    float z_near;
    float z_far;
};


struct Dimensions
{
    float width;
    float height;
    glm::vec3 origin;
};


struct Sprite
{
    std::string texture_path;
    std::string shader_pipeline_name;
    Dimensions dimensions;
};


} // namespace Nito
