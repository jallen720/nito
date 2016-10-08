#include "Nito/Systems/Camera.hpp"

#include <vector>

#include "Nito/Components.hpp"
#include "Nito/ECS.hpp"
#include "Nito/Graphics.hpp"


using std::vector;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Transform *> entity_transforms;
static vector<Viewport *> entity_viewports;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void camera_subscribe(const Entity entity)
{
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));
    entity_viewports.push_back((Viewport *)get_component(entity, "viewport"));
}


void camera_update(const float /*delta_time*/)
{
    init_rendering();

    for (auto i = 0u; i < entity_transforms.size(); i++)
    {
        render(entity_transforms[i], entity_viewports[i]);
    }

    cleanup_rendering();
}


} // namespace Nito
