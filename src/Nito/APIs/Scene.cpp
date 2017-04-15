#include "Nito/APIs/Scene.hpp"

#include <map>
#include <stdexcept>
#include "Cpp_Utils/File.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Fn.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;

// Cpp_Utils/File.hpp
using Cpp_Utils::read_json_file;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;
using Cpp_Utils::merge;

// Cpp_Utils/Fn.hpp
using Cpp_Utils::transform;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static string scene_to_load = "";
static map<string, string> scenes;
static map<string, JSON> blueprints;
static map<string, vector<string>> system_requirements;
static map<string, vector<string>> component_requirements;
static map<string, Scene_Load_Handler> scene_load_handlers;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static string get_system_requirement_message(
    Entity entity,
    const string & system_name,
    const string & requirement_name,
    const string & requirement_type)
{
    return "ERROR: entity " + to_string(entity) + " does not contain a " + requirement_name + " " + requirement_type +
           " required by the " + system_name + " system!";
}


static void add_components(Entity entity, const JSON & entity_data, vector<string> & entity_component_list)
{
    // Defining components for an entity is optional.
    if (!contains_key(entity_data, "components"))
    {
        return;
    }


    for_each(entity_data["components"], [&](const string & component_name, const JSON & data) -> void
    {
        add_component(entity, component_name, data);
        entity_component_list.push_back(component_name);
    });
}


static void subscribe_to_systems(Entity entity, const JSON & entity_data, const vector<string> & entity_component_list)
{
    vector<string> entity_systems;


    // Defining systems for an entity is optional.
    if (contains_key(entity_data, "systems"))
    {
        entity_systems = entity_data["systems"].get<vector<string>>();
    }


    // Populate entity_systems with systems required by entity's components.
    for (const string & component_name : entity_component_list)
    {
        // Defining systems required by a component is optional, so make sure requirements are defined before
        // checking them.
        if (contains_key(component_requirements, component_name))
        {
            for (const string & component_required_system : component_requirements[component_name])
            {
                if (!contains(entity_systems, component_required_system))
                {
                    entity_systems.push_back(component_required_system);
                }
            }
        }
    }


    // Validate all system and component requirements are met for all entity systems, then subscribe entity to them.
    for (const string & system_name : entity_systems)
    {
        // Defining components required by a system is optional, so make sure requirements are defined before
        // checking them.
        if (contains_key(system_requirements, system_name))
        {
            for (const string & required_component : system_requirements[system_name])
            {
                if (!has_component(entity, required_component))
                {
                    throw runtime_error(
                        get_system_requirement_message(entity, system_name, required_component, "component"));
                }
            }
        }


        // If entity meets all system and component requirements for this system, subscribe entity to it.
        subscribe_to_system(entity, system_name);
    }
}


static void load_scene(const string & name)
{
    if (!scene_exists(name))
    {
        throw runtime_error("ERROR: no scene named \"" + name + "\" was set in the Scene API!");
    }


    // Delete any existing entity data before loading entity data from scene.
    delete_all_entities();


    // Load entities.
    const vector<JSON> scene_data = read_json_file(scenes.at(name));

    const vector<Entity> entities = transform<Entity>(scene_data, [](const JSON & /*entity_data*/) -> Entity
    {
        return create_entity();
    });


    // Add components to entities.
    map<Entity, vector<string>> entity_component_lists;

    for (auto i = 0u; i < entities.size(); i++)
    {
        const Entity entity = entities[i];
        add_components(entity, scene_data[i], entity_component_lists[entity]);
    }


    // Subscribe entities to systems.
    for (auto i = 0u; i < entities.size(); i++)
    {
        const Entity entity = entities[i];
        subscribe_to_systems(entity, scene_data[i], entity_component_lists.at(entity));
    }


    // Trigger scene-load handlers.
    for_each(scene_load_handlers, [&](const string & /*id*/, const Scene_Load_Handler & handler) -> void
    {
        handler(name);
    });
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_scene(const string & name, const string & path)
{
    scenes[name] = path;
}


void set_blueprints(const JSON & blueprints_data)
{
    static const string INHERITANCE_KEY = "inherits";

    for_each(blueprints_data, [&](const string & name, const JSON & blueprint) -> void
    {
        blueprints[name] = blueprint;
    });


    // Satisfy dependencies for loaded blueprints.
    for_each(blueprints, [&](const string & name, JSON & blueprint) -> void
    {
        if (!contains_key(blueprint, INHERITANCE_KEY))
        {
            return;
        }

        for (const string & dependency : blueprint.at(INHERITANCE_KEY).get<vector<string>>())
        {
            if (!contains_key(blueprints, dependency))
            {
                throw runtime_error(
                    "ERROR: dependency \"" + dependency + "\" for blueprint \"" + name + "\" does not refer to an "
                    "existing blueprint!");
            }

            blueprint = merge(blueprints.at(dependency), blueprint);
        }
    });
}


bool scene_exists(const string & name)
{
    return contains_key(scenes, name);
}


void set_scene_to_load(const string & name)
{
    scene_to_load = name;
}


void check_load_scene()
{
    if (scene_to_load != "")
    {
        load_scene(scene_to_load);
        scene_to_load = "";
    }
}


void set_component_requirements(const string & component_name, const vector<string> & systems)
{
    component_requirements[component_name] = systems;
}


void set_system_requirements(const string & system_name, const vector<string> & components)
{
    system_requirements[system_name] = components;
}


Entity load_blueprint(const string & name)
{
    if (!contains_key(blueprints, name))
    {
        throw runtime_error("ERROR: no blueprint named \"" + name + "\" was set in the Scene API!");
    }

    Entity entity = create_entity();
    vector<string> entity_component_list;
    add_components(entity, blueprints.at(name), entity_component_list);
    subscribe_to_systems(entity, blueprints.at(name), entity_component_list);
    return entity;
}


void set_scene_load_handler(const string & id, const Scene_Load_Handler & handler)
{
    scene_load_handlers[id] = handler;
}


} // namespace Nito
