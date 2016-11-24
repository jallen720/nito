#pragma once


#include <string>
#include <vector>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_scene(const std::string & name, const std::string & path);
bool scene_exists(const std::string & name);
void set_scene_to_load(const std::string & name);
void check_load_scene();
void set_system_requirements(const std::string & system_name, const std::vector<std::string> & components);
void set_component_requirements(const std::string & component_name, const std::vector<std::string> & systems);


} // namespace Nito
