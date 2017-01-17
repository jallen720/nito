#pragma once


#include <glm/glm.hpp>

#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_circle_collider_data(
    Entity entity,
    const Transform * transform,
    const Collider * collider,
    const Circle_Collider * circle_collider);

void load_line_collider_data(Entity entity, const glm::vec3 * line_start, const glm::vec3 * line_end);
void remove_circle_collider_data(Entity entity);
void remove_line_collider_data(Entity entity);
void physics_api_update();


} // namespace Nito
