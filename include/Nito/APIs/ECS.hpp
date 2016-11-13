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
using System_Entity_Handler = std::function<void(const Entity)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity();
void add_component(const Entity entity, const std::string & type, const Cpp_Utils::JSON & data);
bool has_component(const Entity entity, const std::string & type);
Component get_component(const Entity entity, const std::string & type);

void set_component_handlers(
    const std::string & type,
    const Component_Allocator & component_allocator,
    const Component_Deallocator & component_deallocator);

void set_system_entity_handlers(
    const std::string & name,
    const System_Entity_Handler & system_subscriber,
    const System_Entity_Handler & system_unsubscriber);

void subscribe_to_system(const Entity entity, const std::string & system_name);
void unsubscribe_from_system(const Entity entity, const std::string & system_name);
Entity get_entity(const std::string & id);
void delete_entity_data();


} // namespace Nito
