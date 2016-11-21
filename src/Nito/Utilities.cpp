#include "Nito/Utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Nito/APIs/Graphics.hpp"


// glm/glm.hpp
using glm::mat4;
using glm::vec3;

// glm/gtc/matrix_transform.hpp
using glm::translate;
using glm::rotate;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mat4 calculate_model_matrix(
    const float width,
    const float height,
    const vec3 & origin,
    const vec3 & position,
    const vec3 & scale,
    const float rotation)
{
    static const vec3 ROTATION_AXIS(0.0f, 0.0f, 1.0f);

    mat4 model_matrix;
    const vec3 origin_offset = origin * vec3(width, height, 0.0f) * scale;
    model_matrix = translate(model_matrix, position * get_unit_scale());
    model_matrix = rotate(model_matrix, rotation, ROTATION_AXIS);
    model_matrix = translate(model_matrix, -origin_offset);
    model_matrix = glm::scale(model_matrix, scale);
    model_matrix = glm::scale(model_matrix, vec3(width, height, 1.0f));
    return model_matrix;
}


} // namespace Nito
