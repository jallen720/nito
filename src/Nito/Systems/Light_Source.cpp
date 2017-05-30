#include "Nito/Systems/Light_Source.hpp"

#include <map>
#include "Cpp_Utils/Map.hpp"

#include "Nito/Components.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::map;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, int> entity_light_sources;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void light_source_subscribe(Entity entity)
{
    auto light_source = (Light_Source *)get_component(entity, "light_source");

    entity_light_sources[entity] = create_light_source(
        light_source->intensity,
        light_source->range,
        light_source->color,
        &((Transform *)get_component(entity, "transform"))->position,
        &light_source->enabled);
}


void light_source_unsubscribe(Entity entity)
{
    destroy_light_source(entity_light_sources.at(entity));
    remove(entity_light_sources, entity);
}


} // namespace Nito
