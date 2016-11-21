#pragma once


#include <glm/glm.hpp>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::mat4 calculate_model_matrix(
    const float width,
    const float height,
    const glm::vec3 & origin,
    const glm::vec3 & position,
    const glm::vec3 & scale,
    const float rotation);

glm::mat4 calculate_view_matrix(
    const float width,
    const float height,
    const glm::vec3 & origin,
    const glm::vec3 & position,
    const glm::vec3 & scale,
    const float rotation);


} // namespace Nito
