#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <string>

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"


using std::vector;
using std::string;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<string *> entity_render_layers;
static vector<Sprite *> entity_sprites;
static vector<Transform *> entity_transforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void renderer_subscribe(const Entity entity)
{
    entity_render_layers.push_back((string *)get_component(entity, "render_layer"));
    entity_sprites.push_back((Sprite *)get_component(entity, "sprite"));
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));
}


void renderer_update()
{
    for (auto i = 0u; i < entity_sprites.size(); i++)
    {
        load_render_data(
            entity_render_layers[i],
            entity_sprites[i],
            entity_transforms[i]);
    }
}


} // namespace Nito
