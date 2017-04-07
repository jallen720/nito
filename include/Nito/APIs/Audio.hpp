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
void create_audio_source(const std::string & id, const std::string & path, bool is_looping, float volume);
void play_audio_source(const std::string & id);
void stop_audio_source(const std::string & id);
void clean_openal();


} // namespace Nito
