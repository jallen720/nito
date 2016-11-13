#include "Nito/APIs/Scene.hpp"

#include <map>
#include <vector>
#include <stdexcept>
#include "Cpp_Utils/File.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/JSON.hpp"
#include "Cpp_Utils/Fn.hpp"

#include "Nito/APIs/ECS.hpp"


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static string get_system_requirement_message(
    const Entity entity,
    const string & system_name,
    const string & requirement_name,
    const string & requirement_type)
{
    return "ERROR: entity " + to_string(entity) + " does not contain a " + requirement_name + " " + requirement_type +
           " required by the " + system_name + " system!";
}


void load_scene(const string & name)
{
    if (!scene_exists(name))
    {
        throw runtime_error("ERROR: no scene named \"" + name + "\" was set in the Scene API!");
    }


    // Delete any existing entity data before loading entity data from scene.
    delete_entity_data();


    // Load entities.
    const vector<JSON> scene_data = read_json_file(scenes.at(name));

    const vector<Entity> entities = transform<Entity>(scene_data, [](const JSON & /*entity_data*/) -> Entity
    {
        return create_entity();
    });


    // Add components to entities.
    for (auto i = 0u; i < entities.size(); i++)
    {
        for_each(scene_data[i]["components"], [&](const string & component, const JSON & data) -> void
        {
            add_component(entities[i], component, data);
        });
    }


    // Subscribe entities to systems.
    const JSON systems_data = read_json_file("resources/data/systems.json");

    for (auto i = 0u; i < entities.size(); i++)
    {
        const Entity entity = entities[i];
        const vector<string> & entity_systems = scene_data[i]["systems"];


        // Validate all system and component requirements are met for all entity systems, then subscribe entity to them.
        for (const string & system_name : entity_systems)
        {
            // Defining system and component requirements for systems is optional, so make sure requirements are defined
            // before checking them.
            if (contains_key(systems_data, system_name))
            {
                const JSON & system_data = systems_data[system_name];

                if (contains_key(system_data, "required_components"))
                {
                    for (const string & required_component : system_data["required_components"])
                    {
                        if (!has_component(entity, required_component))
                        {
                            throw runtime_error(
                                get_system_requirement_message(entity, system_name, required_component, "component"));
                        }
                    }
                }

                if (contains_key(system_data, "required_systems"))
                {
                    for (const string & required_system : system_data["required_systems"])
                    {
                        if (!contains(entity_systems, required_system))
                        {
                            throw runtime_error(
                                get_system_requirement_message(entity, system_name, required_system, "system"));
                        }
                    }
                }
            }


            // If entity meets all system and component requirements for this system, subscribe entity to it.
            subscribe_to_system(entity, system_name);
        }
    }
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


} // namespace Nito
