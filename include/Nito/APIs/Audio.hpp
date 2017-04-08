#pragma once


#include <string>
#include <AL/al.h>


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_openal();
void load_audio_file(const std::string & path);
void set_audio_source_buffer(const std::string & id, const std::string & path);
void set_audio_source_looping(const std::string & id, bool looping);
void set_audio_source_volume(const std::string & id, float volume);
void create_audio_source(const std::string & id);
void create_audio_source(const std::string & id, const std::string & path, bool looping, float volume);
void play_audio_source(const std::string & id);
void stop_audio_source(const std::string & id);
bool audio_source_playing(const std::string & id);
void clean_openal();


} // namespace Nito
