#pragma once


#include <string>


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


} // namespace Nito
