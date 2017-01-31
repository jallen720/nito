#include "Nito/Systems/Polygon_Collider.hpp"

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Physics.hpp"


using std::map;
using std::string;

// glm/glm.hpp
using glm::distance;
using glm::degrees;
using glm::normalize;
using glm::vec3;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Polygon_Collider_State
{
    Transform * transform;
    const Collider * collider;
    const Polygon_Collider * polygon_collider;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Polygon_Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void polygon_collider_subscribe(Entity entity)
{
    auto collider = (Collider *)get_component(entity, "collider");
    auto transform = (Transform *)get_component(entity, "transform");
    auto polygon_collider = (Polygon_Collider *)get_component(entity, "polygon_collider");
    Polygon_Collider_State & polygon_collider_state = entity_states[entity];
    polygon_collider_state.transform = transform;
    polygon_collider_state.collider = collider;
    polygon_collider_state.polygon_collider = polygon_collider;

    // load_polygon_collider_data(
    //     entity,
    //     &collider->collision_handler,
    //     &collider->sends_collision,
    //     &collider->receives_collision,
    //     &polygon_collider->width,
    //     &polygon_collider->height,
    //     &transform->position);
}


void polygon_collider_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
    // remove_polygon_collider_data(entity);
}


void polygon_collider_update()
{
    const float pixels_per_unit = get_pixels_per_unit();

    for_each(entity_states, [=](Entity /*entity*/, Polygon_Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Polygon_Collider * entity_polygon_collider = entity_state.polygon_collider;
        const float entity_polygon_collider_width = entity_polygon_collider->width;
        const float entity_polygon_collider_height = entity_polygon_collider->height;


        // Render collider if flagged.
        if (entity_state.collider->render)
        {
            static const string VERTEX_CONTAINER_ID("line_collider");
            static const vec3 ORIGIN(0.5f, 0.5f, 0.0f);

            vec3 position = entity_transform->position;
            position.z = -1.0f;

            load_render_data(
                {
                    Render_Modes::LINES,
                    &Collider::LAYER_NAME,
                    nullptr,
                    &Collider::SHADER_PIPELINE_NAME,
                    &VERTEX_CONTAINER_ID,
                    &Collider::UNIFORMS,
                    calculate_model_matrix(
                        entity_polygon_collider_width * pixels_per_unit,
                        entity_polygon_collider_height * pixels_per_unit,
                        ORIGIN,
                        position,
                        entity_transform->scale,
                        entity_transform->rotation)
                });
        }
    });
}


} // namespace Nito
