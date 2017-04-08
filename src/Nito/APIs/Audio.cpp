#include "Nito/APIs/Audio.hpp"

#include <stdexcept>
#include <map>
#include <cstring>
#include <AL/alc.h>
#include <AL/alut.h>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::runtime_error;
using std::string;
using std::map;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, ALuint> buffers;
static map<string, ALuint> audio_sources;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void validate_no_openal_error(const string & description)
{
    static const map<ALCenum, const string> OPENAL_ERROR_MESSAGES
    {
        { AL_INVALID_NAME      , "invalid name"      },
        { AL_INVALID_ENUM      , "invalid enum"      },
        { AL_INVALID_VALUE     , "invalid value"     },
        { AL_INVALID_OPERATION , "invalid operation" },
        { AL_OUT_OF_MEMORY     , "out of memory"     },
    };

    const ALCenum error = alGetError();

    if (error != AL_NO_ERROR)
    {
        const string error_message =
            contains_key(OPENAL_ERROR_MESSAGES, error)
            ? OPENAL_ERROR_MESSAGES.at(error)
            : "an unknown OpenAL error occurred";

        throw runtime_error("OPENAL ERROR: " + description + ": " + error_message + "!");
    }
}


static string get_alut_error_message(const string & description)
{
    static const string ALUT_ERROR_PREFIX("ALUT ERROR: ");

    return ALUT_ERROR_PREFIX + description + ": " + alutGetErrorString(alutGetError()) + "!";
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_openal()
{
    // Initialize ALUT.
    if (!alutInit(0, nullptr))
    {
        throw runtime_error(get_alut_error_message("init_openal(): alutInit()"));
    }
}


void load_audio_file(const string & path)
{
    const ALuint buffer = alutCreateBufferFromFile(path.c_str());

    if (buffer == AL_NONE)
    {
        throw runtime_error(get_alut_error_message("init_openal(): alutCreateBufferFromFile()"));
    }

    buffers[path] = buffer;
}


void create_audio_source(const string & id)
{
    alGenSources(1, &audio_sources[id]);
    validate_no_openal_error("create_audio_source(): alGenSources()");
}


void create_audio_source(const string & id, const string & path, bool looping, float volume)
{
    if (!contains_key(buffers, path))
    {
        throw runtime_error("ERROR: no audio file with path \"" + path + "\" was loaded in the Audio API!");
    }

    create_audio_source(id);
    set_audio_source_buffer(id, path);
    set_audio_source_looping(id, looping);
    set_audio_source_volume(id, volume);
}


void set_audio_source_buffer(const string & id, const string & path)
{
    alSourcei(audio_sources.at(id), AL_BUFFER, buffers.at(path));
    validate_no_openal_error("set_audio_source_buffer(): alSourcei()");
}


void set_audio_source_looping(const string & id, bool looping)
{
    alSourcei(audio_sources.at(id), AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    validate_no_openal_error("set_audio_source_looping(): alSourcei()");
}


void set_audio_source_volume(const string & id, float volume)
{
    alSourcef(audio_sources.at(id), AL_GAIN, volume);
    validate_no_openal_error("set_audio_source_volume(): alSourcef()");
}


void play_audio_source(const string & id)
{
    alSourcePlay(audio_sources.at(id));
    validate_no_openal_error("play_audio_source(): alSourcePlay()");
}


void stop_audio_source(const string & id)
{
    alSourceStop(audio_sources.at(id));
    validate_no_openal_error("stop_audio_source(): alSourcePlay()");
}


bool audio_source_playing(const string & id)
{
    ALint state;
    alGetSourcei(audio_sources.at(id), AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}


void clean_openal()
{
    // Delete audio-sources and buffers.
    for_each(audio_sources, [](const string & /*id*/, ALuint audio_source) -> void
    {
        // Ensure audio source is not playing before deleting.
        alSourceStop(audio_source);

        alDeleteSources(1, &audio_source);
    });

    for_each(buffers, [](const string & /*path*/, ALuint buffer) -> void
    {
        alDeleteBuffers(1, &buffer);
    });


    validate_no_openal_error("clean_openal(): deleting audio-sources and buffers");


    // Validate no errors occurred while exiting ALUT.
    if (!alutExit())
    {
        throw runtime_error(get_alut_error_message("clean_openal(): alutExit()"));
    }
}


} // namespace Nito
