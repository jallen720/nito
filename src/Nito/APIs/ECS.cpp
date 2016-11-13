#include "Nito/APIs/ECS.hpp"

#include <map>
#include <vector>
#include <stdexcept>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/Fn.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/String.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains;

// Cpp_Utils/Fn.hpp
using Cpp_Utils::filter;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Components = map<string, Component>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static Entity entity_index = 0u;
static vector<Entity> entities;
static map<Entity, Components> components;
static map<Entity, vector<string>> entity_system_subscriptions;

// Handlers
static map<string, Component_Allocator> component_allocators;
static map<string, Component_Deallocator> component_deallocators;
static map<string, System_Entity_Handler> system_subscribers;
static map<string, System_Entity_Handler> system_unsubscribers;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static string get_system_entity_handler_error_message(const string & system_name)
{
    return "ERROR: no system entity handlers loaded for system named \"" + system_name + "\"!";
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity()
{
    Entity entity = entity_index++;
    entities.push_back(entity);
    return entity;
}


void add_component(const Entity entity, const string & type, const JSON & data)
{
    if (!contains_key(component_allocators, type))
    {
        throw runtime_error("ERROR: \"" + type + "\" is not a supported component type!");
    }

    components[entity][type] = component_allocators.at(type)(data);
}


bool has_component(const Entity entity, const string & type)
{
    return contains_key(components, entity) &&
           contains_key(components.at(entity), type);
}


Component get_component(const Entity entity, const string & type)
{
    if (!has_component(entity, type))
    {
        throw runtime_error(
            "ERROR: entity " + to_string(entity) + " does not have a component of type \"" + type + "\"!");
    }

    return components.at(entity).at(type);
}


void set_component_handlers(
    const string & type,
    const Component_Allocator & component_allocator,
    const Component_Deallocator & component_deallocator)
{
    component_allocators[type] = component_allocator;
    component_deallocators[type] = component_deallocator;
}


void set_system_entity_handlers(
    const string & name,
    const System_Entity_Handler & system_subscriber,
    const System_Entity_Handler & system_unsubscriber)
{
    system_subscribers[name] = system_subscriber;
    system_unsubscribers[name] = system_unsubscriber;
}


void subscribe_to_system(const Entity entity, const string & system_name)
{
    if (!contains_key(system_subscribers, system_name))
    {
        throw runtime_error(get_system_entity_handler_error_message(system_name));
    }

    vector<string> & subscriptions = entity_system_subscriptions[entity];

    if (contains(subscriptions, system_name))
    {
        throw runtime_error(
            "ERROR: entity " + to_string(entity) + " is already subscribed to the \"" + system_name + "\" system!");
    }

    system_subscribers.at(system_name)(entity);
    subscriptions.push_back(system_name);
}


void unsubscribe_from_system(const Entity entity, const string & system_name)
{
    if (!contains_key(system_unsubscribers, system_name))
    {
        throw runtime_error(get_system_entity_handler_error_message(system_name));
    }

    vector<string> & subscriptions = entity_system_subscriptions[entity];

    if (!contains(subscriptions, system_name))
    {
        throw runtime_error(
            "ERROR: entity " + to_string(entity) + " is not subscribed to the \"" + system_name + "\" system!");
    }

    system_unsubscribers.at(system_name)(entity);
    remove(subscriptions, system_name);
}


Entity get_entity(const string & id)
{
    const vector<Entity> entities_with_ids = filter(entities, [](const Entity entity) -> bool
    {
        return has_component(entity, "id");
    });

    for (const Entity entity : entities_with_ids)
    {
        const auto entity_id = (string *)get_component(entity, "id");

        if (*entity_id == id)
        {
            return entity;
        }
    }

    throw runtime_error("ERROR: no entity found with id \"" + id + "\"!");
}


void delete_entity_data()
{
    // Unsubscribe from systems.
    for_each(entity_system_subscriptions, [&](const Entity entity, vector<string> & subscriptions) -> void
    {
        while (subscriptions.size() > 0)
        {
            unsubscribe_from_system(entity, subscriptions[0]);
        }
    });


    // Delete components.
    for_each(components, [&](const Entity /*entity*/, Components & entity_components) -> void
    {
        for_each(entity_components, [&](const string & type, Component component) -> void
        {
            component_deallocators.at(type)(component);
        });
    });


    // Clear entities.
    entities.clear();
    components.clear();
    entity_system_subscriptions.clear();
    entity_index = 0u;
}


} // namespace Nito
