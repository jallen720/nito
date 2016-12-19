#pragma once


#include <string>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Cpp_Utils/JSON.hpp"

#include "Nito/Components.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Texture
{
    using Options = std::map<std::string, std::string>;

    std::string format;
    Options options;
    Dimensions dimensions;
};


struct Glyph
{
    FT_Pos advance;
    glm::vec2 bearing;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_freetype();
void load_textures(const Cpp_Utils::JSON & texture_group);
void load_font(const Cpp_Utils::JSON & config);
const Texture & get_loaded_texture(const std::string & path);
const Glyph & get_loaded_glyph(const std::string & identifier);


} // namespace Nito
