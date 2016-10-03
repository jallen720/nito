#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <stdexcept>

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"


using std::vector;
using std::runtime_error;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Entity> entities;
static vector<Sprite *> entity_sprites;
static vector<Transform *> entity_transforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void renderer_subscribe(const Entity entity)
{
    entities.push_back(entity);
    entity_sprites.push_back((Sprite *)get_component(entity, "sprite"));
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));
}


void renderer_update()
{
    init_rendering();

    for (auto i = 0u; i < entities.size(); i++)
    {
        render(entity_sprites[i], entity_transforms[i]);
    }

    cleanup_rendering();
}


} // namespace Nito
