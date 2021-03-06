#pragma once


#include <functional>

#include "Nito/APIs/ECS.hpp"


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NITO_SYSTEM_ENTITY_HANDLERS(SYSTEM_NAME) \
    {                                            \
        #SYSTEM_NAME,                            \
        {                                        \
            SYSTEM_NAME ## _subscribe,           \
            SYSTEM_NAME ## _unsubscribe,         \
        },                                       \
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Update_Handler = std::function<void()>;


struct Component_Handlers
{
    const Component_Allocator allocator;
    const Component_Deallocator deallocator;
};


struct System_Entity_Handlers
{
    const System_Entity_Handler subscriber;
    const System_Entity_Handler unsubscriber;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void add_update_handler(const Update_Handler & update_handler);
int run_engine();
float get_time_scale();
void set_time_scale(float value);

template<typename T>
Component_Allocator get_component_allocator();

template<typename T>
Component_Deallocator get_component_deallocator();


} // namespace Nito


#include "Nito/Engine.ipp"
