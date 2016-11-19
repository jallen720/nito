#pragma once


#include <string>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_openal();
void load_audio_file(const std::string & path);
void create_audio_source(const std::string & identifier, bool is_looping, float volume, const std::string & data_path);
void play_audio_source(const std::string & identifier);
void stop_audio_source(const std::string & identifier);
void clean_openal();


} // namespace Nito
