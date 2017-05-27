#include "Nito/APIs/Audio.hpp"

#include <stdexcept>
#include <map>
#include <cstring>


#if _WIN32
#include <Windows.h>
#include <mmsystem.h>
#elif __gnu_linux__
#include <AL/alc.h>
#include <AL/alut.h>
#endif


#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/File.hpp"


using std::runtime_error;
using std::string;
using std::map;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/File.hpp
using Cpp_Utils::platform_path;


namespace Nito
{


#if _WIN32
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Strucutres
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Audio_Source
{
    string path;
    bool looping;
};
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if _WIN32
static map<string, Audio_Source> audio_sources;
#elif __gnu_linux__
static map<string, ALuint> buffers;
static map<string, ALuint> audio_sources;
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if __gnu_linux__
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
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_openal()
{
#if __gnu_linux__
    // Initialize ALUT.
    if (!alutInit(0, nullptr))
    {
        throw runtime_error(get_alut_error_message("init_openal(): alutInit()"));
    }
#endif
}


void load_audio_file(const string & path)
{
#if __gnu_linux__
    const ALuint buffer = alutCreateBufferFromFile(path.c_str());

    if (buffer == AL_NONE)
    {
        throw runtime_error(get_alut_error_message("init_openal(): alutCreateBufferFromFile()"));
    }

    buffers[path] = buffer;
#endif
}


void create_audio_source(const string & id)
{
#if __gnu_linux__
    alGenSources(1, &audio_sources[id]);
    validate_no_openal_error("create_audio_source(): alGenSources()");
#endif
}


void create_audio_source(const string & id, const string & path, bool looping, float volume)
{
#if _WIN32
    audio_sources[id] =
    {
        platform_path(path),
        looping,
    };
#elif __gnu_linux__
    if (!contains_key(buffers, path))
    {
        throw runtime_error("ERROR: no audio file with path \"" + path + "\" was loaded in the Audio API!");
    }

    create_audio_source(id);
    set_audio_source_buffer(id, path);
    set_audio_source_looping(id, looping);
    set_audio_source_volume(id, volume);
#endif
}


void set_audio_source_buffer(const string & id, const string & path)
{
#if __gnu_linux__
    alSourcei(audio_sources.at(id), AL_BUFFER, buffers.at(path));
    validate_no_openal_error("set_audio_source_buffer(): alSourcei()");
#endif
}


void set_audio_source_looping(const string & id, bool looping)
{
#if __gnu_linux__
    alSourcei(audio_sources.at(id), AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    validate_no_openal_error("set_audio_source_looping(): alSourcei()");
#endif
}


void set_audio_source_volume(const string & id, float volume)
{
#if __gnu_linux__
    alSourcef(audio_sources.at(id), AL_GAIN, volume);
    validate_no_openal_error("set_audio_source_volume(): alSourcef()");
#endif
}


void play_audio_source(const string & id)
{
#if _WIN32
    const Audio_Source & audio_source = audio_sources.at(id);
    DWORD flags = SND_FILENAME | SND_ASYNC;

    if (audio_source.looping)
    {
        flags |= SND_LOOP;
    }

    PlaySound(TEXT(audio_source.path.c_str()), NULL, flags);
#elif __gnu_linux__
    alSourcePlay(audio_sources.at(id));
    validate_no_openal_error("play_audio_source(): alSourcePlay()");
#endif
}


void stop_audio_source(const string & id)
{
#if __gnu_linux__
    alSourceStop(audio_sources.at(id));
    validate_no_openal_error("stop_audio_source(): alSourcePlay()");
#endif
}


bool audio_source_playing(const string & id)
{
#if _WIN32
    return false;
#elif __gnu_linux__
    ALint state;
    alGetSourcei(audio_sources.at(id), AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
#endif
}


void clean_openal()
{
#if __gnu_linux__
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
#endif
}


} // namespace Nito
