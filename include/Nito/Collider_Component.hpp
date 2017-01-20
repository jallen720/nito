#pragma once


#include <string>
#include <glm/glm.hpp>

#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Physics.hpp"


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
    bool sends_collision;
    bool receives_collision;
    Collision_Handler collision_handler;
};


} // namespace Nito
