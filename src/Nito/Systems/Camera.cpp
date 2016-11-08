#include "Nito/Systems/Camera.hpp"

#include <vector>

#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::vector;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Dimensions *> entity_dimensions;
static vector<Transform *> entity_transforms;
static vector<Viewport *> entity_viewports;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void camera_subscribe(const Entity entity)
{
    entity_dimensions.push_back((Dimensions *)get_component(entity, "dimensions"));
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));
    entity_viewports.push_back((Viewport *)get_component(entity, "viewport"));
}


void camera_update()
{
    init_rendering();

    for (auto i = 0u; i < entity_dimensions.size(); i++)
    {
        const Viewport * entity_viewport = entity_viewports[i];
        const Dimensions * _entity_dimensions = entity_dimensions[i];
        const Transform * entity_transform = entity_transforms[i];

        render(
            {
                entity_viewport->x,
                entity_viewport->y,
                entity_viewport->z_near,
                entity_viewport->z_far,
                {
                    _entity_dimensions->width,
                    _entity_dimensions->height,
                    &_entity_dimensions->origin,
                    &entity_transform->position,
                    &entity_transform->scale,
                },
            });
    }

    cleanup_rendering();
}


} // namespace Nito
