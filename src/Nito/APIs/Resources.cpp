#include <GL/glew.h>

#include "Nito/APIs/Resources.hpp"

#include <stdexcept>
#include <SOIL.h>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/File.hpp"

#include "Nito/APIs/Graphics.hpp"


using std::string;
using std::map;
using std::vector;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

// Cpp_Utils/File.hpp
using Cpp_Utils::platform_path;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, Texture> textures;
static map<string, Glyph> glyphs;
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


void load_textures(const JSON & texture_group)
{
    static const map<string, int> IMAGE_FORMATS
    {
        { "rgba" , SOIL_LOAD_RGBA },
        { "rgb"  , SOIL_LOAD_RGB  },
    };


    const string format = texture_group["format"];
    const vector<string> paths = texture_group["paths"];


    // Populate texture group options.
    map<string, string> options;

    for_each(texture_group["options"], [&](const string & option_key, const string & option_value) -> void
    {
        options[option_key] = option_value;
    });


    // Load each texture for the texture group.
    for (const string & path : paths)
    {
        // Create and configure new texture.
        Texture texture;
        texture.format = format;
        texture.options = options;


        // Load texture data from image at path.
        int image_width;
        int image_height;

        unsigned char * image_data = SOIL_load_image(
            platform_path(path).c_str(),
            &image_width,
            &image_height,
            nullptr,
            IMAGE_FORMATS.at(texture.format));


        // Load texture dimensions from image.
        texture.dimensions =
        {
            (float)image_width,
            (float)image_height,
            vec3(),
        };


        // Track texture and pass its data to Graphics API.
        textures[path] = texture;


        // Load then free image data.
        load_texture_data(texture, image_data, path);
        SOIL_free_image_data(image_data);
    }
}


void load_font(const JSON & config)
{
    static const map<string, string> FONT_TEXTURE_OPTIONS
    {
        { "wrap_s"     , "clamp_to_edge" },
        { "wrap_t"     , "clamp_to_edge" },
        { "min_filter" , "linear"        },
        { "mag_filter" , "linear"        },
    };


    // Attempt to load font face.
    FT_Face face;
    const string font_face_path = config["path"];

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
        texture.options = FONT_TEXTURE_OPTIONS;
        FT_GlyphSlot glyph = face->glyph;
        const FT_Bitmap & bitmap = glyph->bitmap;
        unsigned int width = bitmap.width;
        unsigned int height = bitmap.rows;


        // Calculate origin from glyph metrics.
        vec3 origin;

        if (width != 0)
        {
            origin.x = -((float)glyph->bitmap_left / width);
        }

        if (height != 0)
        {
            origin.y = -(((float)glyph->bitmap_top / height) - 1);
        }

        texture.dimensions =
        {
            (float)width,
            (float)height,
            origin,
        };


        // Load texture data and use the path of the font face with the appended character as its identifier.
        string glyph_identifier = font_face_path + " : " + ((char)character);
        textures[glyph_identifier] = texture;

        glyphs[glyph_identifier] =
        {
            glyph->advance.x >> 6,
            vec2(glyph->bitmap_left, glyph->bitmap_top),
        };

        load_texture_data(texture, bitmap.buffer, glyph_identifier);
    }
}


const Texture & get_loaded_texture(const string & path)
{
    if (!contains_key(textures, path))
    {
        throw runtime_error("ERROR: no texture with path \"" + path + "\" was loaded by Resources API!");
    }

    return textures.at(path);
}


const Glyph & get_loaded_glyph(const string & identifier)
{
    if (!contains_key(glyphs, identifier))
    {
        throw runtime_error("ERROR: no glyph with identifier \"" + identifier + "\" was loaded by Resources API!");
    }

    return glyphs.at(identifier);
}


} // namespace Nito
