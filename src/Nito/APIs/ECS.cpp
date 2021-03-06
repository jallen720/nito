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
using Cpp_Utils::contains;

// Cpp_Utils/Vector.hpp & Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

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
static vector<Entity> used_entities;
static vector<Entity> unused_entities;
static vector<Entity> flagged_entities;
static map<Entity, Components> entity_components;
static map<Entity, vector<string>> entity_subscriptions;

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


static void validate_component_has_handlers(const string & type)
{
    if (!contains_key(component_allocators, type))
    {
        throw runtime_error("ERROR: \"" + type + "\" is not a supported component type!");
    }
}


static void delete_entities(const vector<Entity> & entities)
{
    // Unsubscribe entities from systems first, that way if a system's unsubscribe handler references a component in
    // another entity that is going to be deleted, that can be handled as components won't be deleted until after all
    // systems have been unsubscribed from.
    for (const Entity entity : entities)
    {
        vector<string> & subscriptions = entity_subscriptions[entity];

        while (subscriptions.size() > 0)
        {
            unsubscribe_from_system(entity, subscriptions[0]);
        }

        remove(entity_subscriptions, entity);
    }


    // Deallocate components after all systems have been unsubscribed from.
    for (const Entity entity : entities)
    {
        Components & components = entity_components[entity];

        for_each(components, [&](const string & type, Component component) -> void
        {
            component_deallocators.at(type)(component);
        });

        remove(entity_components, entity);
    }


    // Move now unused entity IDs back to unused_entities.
    for (const Entity entity : entities)
    {
        remove(used_entities, entity);
        unused_entities.push_back(entity);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity()
{
    Entity entity;

    if (unused_entities.size() > 0)
    {
        entity = unused_entities.back();
        unused_entities.pop_back();
    }
    else
    {
        entity = entity_index++;
    }

    used_entities.push_back(entity);
    return entity;
}


Entity generate_entity(const map<string, Component> & components, const vector<string> & systems)
{
    const Entity entity = create_entity();

    for_each(components, [=](const string & type, Component component) -> void
    {
        add_component(entity, type, component);
    });

    for (const string & system : systems)
    {
        subscribe_to_system(entity, system);
    }

    return entity;
}


void add_component(Entity entity, const string & type, Component component)
{
    if (component == nullptr)
    {
        throw runtime_error("ERROR: cannot add null component to entity!");
    }

    validate_component_has_handlers(type);
    entity_components[entity][type] = component;
}


void add_component(Entity entity, const string & type, const JSON & data)
{
    validate_component_has_handlers(type);
    add_component(entity, type, component_allocators.at(type)(data));
}


Component get_component(Entity entity, const string & type)
{
    if (!has_component(entity, type))
    {
        throw runtime_error(
            "ERROR: entity " + to_string(entity) + " does not have a component of type \"" + type + "\"!");
    }

    return entity_components.at(entity).at(type);
}


bool has_component(Entity entity, const string & type)
{
    return contains_key(entity_components, entity) &&
           contains_key(entity_components.at(entity), type);
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


void subscribe_to_system(Entity entity, const string & system_name)
{
    if (!contains_key(system_subscribers, system_name))
    {
        throw runtime_error(get_system_entity_handler_error_message(system_name));
    }

    vector<string> & subscriptions = entity_subscriptions[entity];

    if (contains(subscriptions, system_name))
    {
        throw runtime_error(
            "ERROR: entity " + to_string(entity) + " is already subscribed to the \"" + system_name + "\" system!");
    }

    system_subscribers.at(system_name)(entity);
    subscriptions.push_back(system_name);
}


void unsubscribe_from_system(Entity entity, const string & system_name)
{
    if (!contains_key(system_unsubscribers, system_name))
    {
        throw runtime_error(get_system_entity_handler_error_message(system_name));
    }

    vector<string> & subscriptions = entity_subscriptions[entity];

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
    const vector<Entity> entities_with_ids = filter(used_entities, [](Entity entity) -> bool
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


void flag_entity_for_deletion(Entity entity)
{
    // Only flag entity if it hasn't already been flagged.
    if (!contains(flagged_entities, entity))
    {
        flagged_entities.push_back(entity);
    }
}


void delete_flagged_entities()
{
    if (flagged_entities.size() == 0)
    {
        return;
    }


    // Deleting entities could result in new entities being flagged for deletion in system unsubscribe handlers, so make
    // a copy of the currently flagged entities.
    const vector<Entity> current_flagged_entities = flagged_entities;
    flagged_entities.clear();
    delete_entities(current_flagged_entities);
}


void delete_all_entities()
{
    delete_entities(used_entities);


    // All entities are deleted so any entities that were still flagged for deletion no longer need to be handled.
    flagged_entities.clear();


    unused_entities.clear();
    entity_components.clear();
    entity_subscriptions.clear();
    entity_index = 0u;
}


} // namespace Nito
