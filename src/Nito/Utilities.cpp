#include "Nito/Utilities.hpp"

#include <string>


using std::string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NITO_COMPONENT_HANDLER(string)
{
    return new string(component_data.get<string>());
}


NITO_COMPONENT_HANDLER(int)
{
    return new int(component_data);
}


NITO_COMPONENT_HANDLER(float)
{
    return new float(component_data);
}


} // namespace Nito
