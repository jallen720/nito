#pragma once


#include "Cpp_Utils/JSON.hpp"

#include "Nito/ECS.hpp"


#define NITO_COMPONENT_HANDLER(TYPE) Component TYPE ## _component_handler(const Cpp_Utils::JSON & component_data)


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NITO_COMPONENT_HANDLER(string);
NITO_COMPONENT_HANDLER(int);
NITO_COMPONENT_HANDLER(float);



} // namespace Nito