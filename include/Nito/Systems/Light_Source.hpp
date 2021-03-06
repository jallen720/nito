#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void light_source_subscribe(Entity entity);
void light_source_unsubscribe(Entity entity);


} // namespace Nito
