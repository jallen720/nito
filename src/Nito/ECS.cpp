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
static Component_Dependency_Data component_dependency_data;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_component_dependency_data(const Component_Dependency_Data & _component_dependency_data)
{
    component_dependency_data = _component_dependency_data;
}


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
    // Validate all component dependencies have been met.
    for (const string & dependency : component_dependency_data.at(type))
    {
        if (!has_component(entity, dependency))
        {
            throw runtime_error(
                "Entity is missing \"" + dependency + "\" component required by \"" + type + "\" component!");
        }
    }


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
