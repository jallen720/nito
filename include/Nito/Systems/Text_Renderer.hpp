#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void text_renderer_subscribe(Entity entity);
void text_renderer_unsubscribe(Entity entity);
void text_renderer_update();


} // namespace Nito
