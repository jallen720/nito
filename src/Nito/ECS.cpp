#include "Nito/ECS.hpp"

#include <map>
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


Component get_component(const Entity entity, const string & type)
{
    if (!contains_key(components, type))
    {
        throw runtime_error("No components found for type \"" + type + "\"!");
    }

    const Components & type_components = components.at(type);

    if (!contains_key(type_components, entity))
    {
        throw runtime_error("Entity does not have a component of type \"" + type + "\"!");
    }

    return type_components.at(entity);
}


} // namespace Nito
