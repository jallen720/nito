#include "Nito/Utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Nito/APIs/Graphics.hpp"


// glm/glm.hpp
using glm::mat4;
using glm::vec3;

// glm/gtc/matrix_transform.hpp
using glm::translate;
using glm::rotate;
using glm::radians;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const vec3 ROTATION_AXIS(0.0f, 0.0f, 1.0f);


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
    mat4 model_matrix;
    const vec3 origin_offset = origin * vec3(width, height, 0.0f) * scale;
    model_matrix = translate(model_matrix, position * get_unit_scale());
    model_matrix = rotate(model_matrix, radians(rotation), ROTATION_AXIS);
    model_matrix = translate(model_matrix, -origin_offset);
    model_matrix = glm::scale(model_matrix, scale);
    model_matrix = glm::scale(model_matrix, vec3(width, height, 1.0f));
    return model_matrix;
}


mat4 calculate_view_matrix(
    const float width,
    const float height,
    const vec3 & origin,
    const vec3 & position,
    const vec3 & scale,
    const float rotation)
{
    mat4 view_matrix;
    const vec3 view_origin_offset = origin * vec3(width, height, 0.0f);
    const vec3 view_position = (position * scale * get_unit_scale());
    view_matrix = translate(view_matrix, -view_position);
    view_matrix = translate(view_matrix, view_origin_offset);
    view_matrix = rotate(view_matrix, radians(rotation), ROTATION_AXIS);
    view_matrix = glm::scale(view_matrix, scale);
    return view_matrix;
}


} // namespace Nito
