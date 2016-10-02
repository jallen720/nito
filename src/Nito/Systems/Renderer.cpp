#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <string>
#include <stdexcept>

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"


using std::vector;
using std::string;
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

const vector<string> required_components
{
    "sprite",
    "transform",
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void renderer_subscribe(const Entity entity)
{
    string error_message;

    for (const string & component : required_components)
    {
        if (!has_component(entity, component))
        {
            error_message += "Entity does not have a " + component + " component required by the renderer system!";
        }
    }

    if (!error_message.empty())
    {
        throw runtime_error(error_message);
    }

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
