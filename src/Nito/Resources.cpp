#include <GL/glew.h>
#include "Nito/Resources.hpp"

#include <stdexcept>
#include <Magick++.h>
#include <glm/glm.hpp>
#include "Cpp_Utils/Collection.hpp"

#include "Nito/Graphics.hpp"


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
static map<string, Texture> textures;
static map<string, FT_Pos> glyph_advances;
static FT_Library ft;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_freetype()
{
    if (FT_Init_FreeType(&ft))
    {
        throw runtime_error("FREETYPE ERROR: could not initialize FreeType!");
    }
}


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
    texture.format = config["format"];

    for_each(config["options"], [&](const string & option_key, const string & option_value) -> void
    {
        texture.options[option_key] = option_value;
    });


    // Load texture data from image at path.
    const string texture_path = TEXTURES_RESOURCE_PATH + config["path"].get<string>();
    Image image;
    Blob blob;
    image.read(texture_path);
    image.flip();
    image.write(&blob, image_formats.at(texture.format));


    // Load texture dimensions from image.
    texture.dimensions =
    {
        (float)image.columns(),
        (float)image.rows(),
        vec3(),
    };


    // Track texture and pass its data to Graphics.
    textures[texture_path] = texture;
    load_texture_data(texture, blob.data(), texture_path);
}


void load_font(const JSON & config)
{
    static const map<string, string> font_texture_options
    {
        { "wrap_s"     , "clamp_to_edge" },
        { "wrap_t"     , "clamp_to_edge" },
        { "min_filter" , "linear"        },
        { "mag_filter" , "linear"        },
    };


    // Attempt to load font face.
    FT_Face face;
    string font_face_path = config["path"];

    if (FT_New_Face(ft, font_face_path.c_str(), 0, &face))
    {
        throw runtime_error("FREETYPE ERROR: failed to load font face from \"" + font_face_path + "\"!");
    }

    // Setting the width to 0 lets the face dynamically calculate the width based on the given height.
    FT_Set_Pixel_Sizes(face, 0, config["height"]);


    // Load ASCII characters.
    for (auto character = 0u; character < 128; character++)
    {
        if (FT_Load_Char(face, character, FT_LOAD_RENDER))
        {
            throw runtime_error("FREETYPE ERROR: failed to load glyph!");
        }


        // Create texture from font face.
        Texture texture;
        texture.format = "r";
        texture.options = font_texture_options;
        FT_GlyphSlot glyph = face->glyph;
        const FT_Bitmap & bitmap = glyph->bitmap;
        unsigned int width = bitmap.width;
        unsigned int height = bitmap.rows;


        // Calculate origin from glyph metrics.
        vec3 origin;

        if (glyph->bitmap_left != 0 && width != 0)
        {
            origin.x = -((float)glyph->bitmap_left / width);
        }

        if (glyph->bitmap_top != 0 && height != 0)
        {
            origin.y = -(((float)glyph->bitmap_top / height) - 1);
        }

        texture.dimensions =
        {
            (float)width,
            (float)height,
            origin,
        };


        // Data is loaded upside down, so invert data on the y-axis.
        unsigned char * buffer = bitmap.buffer;
        unsigned char * data = new unsigned char[width * height];

        for (unsigned char row = 0; row < height; row++)
        {
            memcpy(
                data + (width * row),
                buffer + (width * (height - row - 1)),
                sizeof(unsigned char) * width);
        }


        // Load texture data and use the path of the font face with the appended character as its identifier.
        string glyph_identifier = font_face_path + " : " + ((char)character);
        textures[glyph_identifier] = texture;
        glyph_advances[glyph_identifier] = face->glyph->advance.x >> 6;
        load_texture_data(texture, data, glyph_identifier);
    }
}


const Texture & get_loaded_texture(const string & path)
{
    return textures.at(path);
}


FT_Pos get_loaded_glyph_advance(const string & identifier)
{
    return glyph_advances.at(identifier);
}


} // namespace Nito
