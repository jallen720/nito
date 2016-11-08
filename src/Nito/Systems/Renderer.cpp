#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <string>

#include "Nito/Components.hpp"
#include "Nito/APIs/Graphics.hpp"


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
    for (auto i = 0u; i < entity_render_layers.size(); i++)
    {
        const Sprite * entity_sprite = entity_sprites[i];
        const Transform * entity_transform = entity_transforms[i];
        const Dimensions & entity_sprite_dimensions = entity_sprite->dimensions;

        load_render_data(
            {
                entity_render_layers[i],
                &entity_sprite->texture_path,
                &entity_sprite->shader_pipeline_name,
                nullptr,
                entity_sprite_dimensions.width,
                entity_sprite_dimensions.height,
                &entity_sprite_dimensions.origin,
                &entity_transform->position,
                &entity_transform->scale
            });
    }
}


} // namespace Nito
