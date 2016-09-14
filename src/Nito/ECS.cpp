#include "Nito/ECS.hpp"

#include <stdexcept>
#include "Cpp_Utils/Map.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;
using Cpp_Utils::contains_key;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Components = map<Entity, void *>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Entity> entities;
static map<string, Components> components;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity()
{
    static Entity entity_index = 0u;

    Entity entity = entity_index++;
    entities.push_back(entity);
    return entity;
}


const vector<Entity> & get_entities()
{
    return entities;
}


void add_component(const Entity entity, const string & type, Component component)
{
    components[type][entity] = component;
}


bool has_component(const Entity entity, const string & type)
{
    return contains_key(components, type) &&
           contains_key(components.at(type), entity);
}


Component get_component(const Entity entity, const string & type)
{
    if (!has_component(entity, type))
    {
        throw runtime_error("Entity does not have a component of type \"" + type + "\"!");
    }

    return components.at(type).at(entity);
}


} // namespace Nito
