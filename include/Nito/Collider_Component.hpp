#pragma once


#include <string>
#include <functional>
#include <glm/glm.hpp>

#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"


namespace Nito
{


struct Collider
{
    // Common rendering data for colliders
    static const glm::vec4 COLOR;
    static const Render_Data::Uniforms UNIFORMS;
    static const std::string LAYER_NAME;
    static const std::string SHADER_PIPELINE_NAME;
    static const glm::vec3 ORIGIN;


    bool render;
    std::function<void(Entity)> collision_handler;
};


} // namespace Nito
