#include "Nito/Utilities.hpp"

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::string;

// glm/glm.hpp
using glm::mat4;
using glm::vec3;
using glm::distance;
using glm::degrees;
using glm::normalize;

// glm/gtc/matrix_transform.hpp
using glm::translate;
using glm::rotate;
using glm::scale;
using glm::radians;

// glm/gtx/vector_angle.hpp
using glm::orientedAngle;


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
    float model_width,
    float model_height,
    const vec3 & model_origin,
    const vec3 & model_position,
    const vec3 & model_scale,
    float model_rotation)
{
    mat4 model_matrix;
    const vec3 model_origin_offset = model_origin * vec3(model_width, model_height, 0.0f) * model_scale;
    const vec3 model_scaled_position = model_position * get_pixels_per_unit();
    model_matrix = translate(model_matrix, model_scaled_position);
    model_matrix = rotate(model_matrix, radians(model_rotation), ROTATION_AXIS);
    model_matrix = translate(model_matrix, -model_origin_offset);
    model_matrix = scale(model_matrix, model_scale);
    model_matrix = scale(model_matrix, vec3(model_width, model_height, 1.0f));
    return model_matrix;
}


mat4 calculate_view_matrix(
    float view_width,
    float view_height,
    const vec3 & view_origin,
    const vec3 & view_position,
    const vec3 & view_scale,
    float view_rotation)
{
    mat4 view_matrix;
    const vec3 view_origin_offset = view_origin * vec3(view_width, view_height, 0.0f);
    const vec3 view_scaled_position = view_position * get_pixels_per_unit() * view_scale;
    view_matrix = translate(view_matrix, view_origin_offset);
    view_matrix = rotate(view_matrix, radians(-view_rotation), ROTATION_AXIS);
    view_matrix = translate(view_matrix, -view_scaled_position);
    view_matrix = scale(view_matrix, view_scale);
    return view_matrix;
}


vec3 get_child_world_position(const Transform * parent_transform, const vec3 & child_local_position)
{
    mat4 position;
    position = translate(position, parent_transform->position);
    position = rotate(position, radians(parent_transform->rotation), ROTATION_AXIS);
    position = scale(position, parent_transform->scale);
    position = translate(position, child_local_position);
    return vec3(position[3][0], position[3][1], position[3][2]);
}


void draw_line_collider(const vec3 & line_begin, const vec3 & line_end, const vec3 & scale)
{
    static const string VERTEX_CONTAINER_ID("line_collider");
    static const vec3 BASE_ANGLE_VECTOR(1.0f, 0.0f, 0.0f);
    static const vec3 ANGLE_AXIS(0.0f, 0.0f, 1.0f);

    const float pixels_per_unit = get_pixels_per_unit();
    vec3 position = line_begin;
    position.z = -1.0f;

    const float line_orientation =
        degrees(orientedAngle(BASE_ANGLE_VECTOR, normalize(line_end - line_begin), ANGLE_AXIS));

    load_render_data(
        {
            Render_Modes::LINES,
            &Collider::LAYER_NAME,
            nullptr,
            &Collider::SHADER_PIPELINE_NAME,
            &VERTEX_CONTAINER_ID,
            &Collider::UNIFORMS,
            calculate_model_matrix(
                distance(line_begin, line_end) * pixels_per_unit,
                pixels_per_unit,
                Collider::ORIGIN,
                position,
                scale,
                line_orientation)
        });
}


} // namespace Nito
