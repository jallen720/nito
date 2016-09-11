#pragma once


#include <string>
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


} // namespace Nito
