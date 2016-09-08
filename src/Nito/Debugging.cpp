#include "Nito/Debugging.hpp"

#include <cstdio>


using std::string;
using glm::mat4;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print(const string & name, const mat4 & value)
{
    printf(
        "%s:\n"
        "    %f %f %f %f\n"
        "    %f %f %f %f\n"
        "    %f %f %f %f\n"
        "    %f %f %f %f\n",
        name.c_str(),
        value[0][0], value[1][0], value[2][0], value[3][0],
        value[0][1], value[1][1], value[2][1], value[3][1],
        value[0][2], value[1][2], value[2][2], value[3][2],
        value[0][3], value[1][3], value[2][3], value[3][3]);
}


} // namespace Nito
