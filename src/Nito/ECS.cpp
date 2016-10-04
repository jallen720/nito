#include "Nito/ECS.hpp"

#include <map>
#include <vector>
#include <stdexcept>
#include "Cpp_Utils/Map.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;
using Cpp_Utils::JSON;
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
static map<string, Component_Handler> component_handlers;
static map<string, System_Subscribe_Handler> system_subscribe_handlers;


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


void add_component(const Entity entity, const string & type, const JSON & data)
{
    if (!contains_key(component_handlers, type))
    {
        throw runtime_error("\"" + type + "\" is not a supported component type!");
    }

    components[type][entity] = component_handlers.at(type)(data);
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


void set_component_handler(const string & type, const Component_Handler & component_handler)
{
    component_handlers[type] = component_handler;
}


void set_system_subscribe_handler(const string & name, const System_Subscribe_Handler & system_subscribe_handler)
{
    system_subscribe_handlers[name] = system_subscribe_handler;
}


void subscribe_to_system(const Entity entity, const string & system_name)
{
    system_subscribe_handlers.at(system_name)(entity);
}


} // namespace Nito
