#pragma once


#include <glm/glm.hpp>

#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Collision_Handler = std::function<void(Entity)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_circle_collider_data(
    Entity entity,
    const Collision_Handler * collision_handler,
    const bool * sends_collision,
    const bool * receives_collision,
    const float * radius,
    glm::vec3 * position,
    const glm::vec3 * scale);

void load_line_collider_data(
    Entity entity,
    const Collision_Handler * collision_handler,
    const bool * sends_collision,
    const bool * receives_collision,
    const glm::vec3 * line_begin,
    const glm::vec3 * line_end);

void load_polygon_collider_data(
    Entity entity,
    const Collision_Handler * collision_handler,
    const bool * sends_collision,
    const bool * receives_collision,
    const std::vector<glm::vec3> * line_begins,
    const std::vector<glm::vec3> * line_ends,
    glm::vec3 * position);

void remove_circle_collider_data(Entity entity);
void remove_line_collider_data(Entity entity);
void remove_polygon_collider_data(Entity entity);
void physics_api_update();


} // namespace Nito
