#pragma once


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
void load_collider_data(
    Entity entity,
    const Transform * transform,
    const Collider * collider,
    const Circle_Collider * circle_collider);

void remove_collider_data(Entity entity);
void physics_api_update();


} // namespace Nito
