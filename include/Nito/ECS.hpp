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
using Component_Handler = std::function<Component(const Cpp_Utils::JSON &)>;
using System_Subscribe_Handler = std::function<void(const Entity)>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Entity create_entity();
void add_component(const Entity entity, const std::string & type, const Cpp_Utils::JSON & config);
bool has_component(const Entity entity, const std::string & type);
Component get_component(const Entity entity, const std::string & type);
void set_component_handler(const std::string & type, const Component_Handler & component_handler);
void set_system_subscribe_handler(const std::string & name, const System_Subscribe_Handler & system_subscribe_handler);
void subscribe_to_system(const Entity entity, const std::string & system_name);


} // namespace Nito
