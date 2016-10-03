#include "Nito/Engine.hpp"
#include "Nito/ECS.hpp"
#include "Nito/Systems/Controller.hpp"


// Nito/Engine.hpp
using Nito::get_window;
using Nito::add_update_handler;
using Nito::run_engine;

// Nito/ECS.hpp
using Nito::add_system_subscribe_handler;

// Nito/Systems/Controller.hpp
using Nito::controller_subscribe;
using Nito::controller_update;
using Nito::controller_init;


int main()
{
    // Setup custom systems.
    controller_init(get_window());
    add_system_subscribe_handler("controller", controller_subscribe);
    add_update_handler(controller_update);


    // Run Nito engine.
    return run_engine();
}
