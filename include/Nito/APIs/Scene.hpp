#pragma once


#include <string>
#include <vector>
#include "Cpp_Utils/JSON.hpp"

#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_scene(const std::string & name, const std::string & path);
void set_blueprints(const Cpp_Utils::JSON & blueprints_data);
bool scene_exists(const std::string & name);
void set_scene_to_load(const std::string & name);
void check_load_scene();
void set_system_requirements(const std::string & system_name, const std::vector<std::string> & components);
void set_component_requirements(const std::string & component_name, const std::vector<std::string> & systems);
Entity load_blueprint(const std::string & name);


} // namespace Nito
