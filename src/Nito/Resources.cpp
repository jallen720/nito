#include "Nito/Resources.hpp"

#include <stdexcept>
#include <glm/glm.hpp>
#include "Cpp_Utils/Collection.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;

// Magick++.h
using Magick::Blob;
using Magick::Image;

// glm/glm.hpp
using glm::vec3;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Texture> textures;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_texture(const JSON & config)
{
    static const map<string, const string> image_formats
    {
        { "rgba" , "RGBA" },
        { "rgb"  , "RGB"  },
    };

    static const string TEXTURES_RESOURCE_PATH = "resources/textures/";


    // Create and configure new texture.
    Texture texture;
    texture.path = TEXTURES_RESOURCE_PATH + config["path"].get<string>();
    texture.format = config["format"];

    for_each(config["options"], [&](const string & option_key, const string & option_value) -> void
    {
        texture.options[option_key] = option_value;
    });


    // Load texture data from image at path.
    Image image;
    image.read(texture.path);
    image.flip();
    image.write(&texture.blob, image_formats.at(texture.format));


    // Load texture dimensions from image.
    texture.dimensions =
    {
        (float)image.columns(),
        (float)image.rows(),
        vec3(),
    };


    // Track texture.
    textures.push_back(texture);
}


const Texture & get_loaded_texture(const string & path)
{
    for (const Texture & texture : textures)
    {
        if (texture.path == path)
        {
            return texture;
        }
    }

    throw runtime_error("ERROR: could not find loaded texture for path \"" + path + "\"!");
}


const vector<Texture> & get_loaded_textures()
{
    return textures;
}


} // namespace Nito
