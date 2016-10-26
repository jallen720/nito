#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <string>
#include "Cpp_Utils/Vector.hpp"

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"


using std::vector;
using std::string;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::sort;


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
static vector<int> render_order;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool highest_y_position(const int index_a, const int index_b)
{
    return entity_transforms[index_a]->position.y > entity_transforms[index_b]->position.y;
}


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
    render_order.push_back(render_order.size());
}


void renderer_update()
{
    sort(render_order, highest_y_position);

    for (const int index : render_order)
    {
        load_render_data(
            entity_render_layers[index],
            entity_sprites[index],
            entity_transforms[index]);
    }
}


} // namespace Nito
