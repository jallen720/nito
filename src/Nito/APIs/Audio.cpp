#include "Nito/APIs/Audio.hpp"

#include <stdexcept>
#include <map>
#include <cstring>
#include <AL/al.h>
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


void create_audio_source(const string & identifier, bool is_looping, float volume, const string & data_path)
{
    if (!contains_key(buffers, data_path))
    {
        throw runtime_error("ERROR: no audio file with path \"" + data_path + "\" was loaded in the Audio API!");
    }

    ALuint & audio_source = audio_sources[identifier];
    const ALuint buffer = buffers.at(data_path);

    alGenSources(1, &audio_source);
    validate_no_openal_error("create_audio_source(): alGenSources()");

    alSourcei(audio_source, AL_BUFFER, buffer);
    validate_no_openal_error("create_audio_source(): alSourcei()");

    alSourcei(audio_source, AL_LOOPING, is_looping ? AL_TRUE : AL_FALSE);
    validate_no_openal_error("create_audio_source(): alSourcei()");

    alSourcef(audio_source, AL_GAIN, volume);
    validate_no_openal_error("create_audio_source(): alSourcef()");
}


void play_audio_source(const string & identifier)
{
    alSourcePlay(audio_sources.at(identifier));
    validate_no_openal_error("play_audio_source(): alSourcePlay()");
}


void stop_audio_source(const string & identifier)
{
    alSourceStop(audio_sources.at(identifier));
    validate_no_openal_error("stop_audio_source(): alSourcePlay()");
}


void clean_openal()
{
    for_each(audio_sources, [](const string & /*identifier*/, ALuint audio_source) -> void
    {
        // Ensure audio source is not playing before deleting.
        alSourceStop(audio_source);

        alDeleteSources(1, &audio_source);
    });

    for_each(buffers, [](const string & /*path*/, ALuint buffer) -> void
    {
        alDeleteBuffers(1, &buffer);
    });

    validate_no_openal_error("clean_openal(): deleting buffers");

    if (!alutExit())
    {
        throw runtime_error(get_alut_error_message("clean_openal(): alutExit()"));
    }
}


} // namespace Nito
