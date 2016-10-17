#pragma once


#include <string>
#include <map>
#include <GL/glew.h>
#include <Magick++.h>
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

    std::string path;
    std::string format;
    Options options;
    Magick::Blob blob;
    Dimensions dimensions;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_texture(const Cpp_Utils::JSON & config);
const Texture & get_loaded_texture(const std::string & path);
const std::vector<Texture> & get_loaded_textures();


} // namespace Nito
