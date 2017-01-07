#pragma once


#include <string>
#include <functional>
#include "Cpp_Utils/JSON.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Entity = unsigned int;
using Component = void *;
using Component_Allocator = std::function<Component(const Cpp_Utils::JSON &)>;
using Component_Deallocator = std::function<void(Component)>;
using System_Entity_Handler = std::function<void(Entity)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity();
Entity generate_entity(const std::map<std::string, Component> & components, const std::vector<std::string> & systems);
void add_component(Entity entity, const std::string & type, Component component);
void add_component(Entity entity, const std::string & type, const Cpp_Utils::JSON & data);
Component get_component(Entity entity, const std::string & type);
bool has_component(Entity entity, const std::string & type);

void set_component_handlers(
    const std::string & type,
    const Component_Allocator & component_allocator,
    const Component_Deallocator & component_deallocator);

void set_system_entity_handlers(
    const std::string & name,
    const System_Entity_Handler & system_subscriber,
    const System_Entity_Handler & system_unsubscriber);

void subscribe_to_system(Entity entity, const std::string & system_name);
void unsubscribe_from_system(Entity entity, const std::string & system_name);
Entity get_entity(const std::string & id);
void flag_entity_for_deletion(Entity entity);
void delete_flagged_entities();
void delete_entity_data();


} // namespace Nito
