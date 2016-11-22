#pragma once


#include <glm/glm.hpp>

#include "Nito/Components.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::mat4 calculate_model_matrix(
    const float model_width,
    const float model_height,
    const glm::vec3 & model_origin,
    const glm::vec3 & model_position,
    const glm::vec3 & model_scale,
    const float model_rotation);

glm::mat4 calculate_view_matrix(
    const float view_width,
    const float view_height,
    const glm::vec3 & view_origin,
    const glm::vec3 & view_position,
    const glm::vec3 & view_scale,
    const float view_rotation);

glm::vec3 get_child_world_position(const Transform * parent_transform, const glm::vec3 & child_local_position);


} // namespace Nito
