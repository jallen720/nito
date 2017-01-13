#include "Nito/Collider_Component.hpp"


using std::string;

// glm/glm.hpp
using glm::vec4;
using glm::vec3;


namespace Nito
{


const vec4 Collider::COLOR(0.0f, 0.7f, 0.0f, 1.0f);

const Render_Data::Uniforms Collider::UNIFORMS
{
    { "color", Uniform { Uniform::Types::VEC4, &Collider::COLOR } },
};

const string Collider::LAYER_NAME("world");
const string Collider::SHADER_PIPELINE_NAME("color");
const vec3 Collider::ORIGIN(0.0f);


} // namespace Nito
