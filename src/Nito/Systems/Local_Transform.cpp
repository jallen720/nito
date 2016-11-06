#include "Nito/Systems/Local_Transform.hpp"

#include <map>
#include <string>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"

#include "Nito/Components.hpp"


using std::map;
using std::string;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, string *> entity_parent_ids;
static map<Entity, Transform *> entity_transforms;
static map<Entity, Local_Transform *> entity_local_transforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calculate_transform(
    const Entity entity,
    const map<Entity, Entity> & entity_parents,
    map<Entity, Transform *> & calculated_transforms)
{
    // This entity has no parent, meaning it is a root transform, so no calculations need to be done on its transform.
    if (!contains_key(entity_parents, entity))
    {
        calculated_transforms[entity] = (Transform *)get_component(entity, "transform");
        return;
    }

    Entity entity_parent = entity_parents.at(entity);

    // If entity's parent's transform has not already been calculated, calculate its transform before calculating
    // entity's transform.
    if (!contains_key(calculated_transforms, entity_parent))
    {
        calculate_transform(entity_parent, entity_parents, calculated_transforms);
    }


    // Calculate entity's transform based on its parent's transform and its local_transform.
    Transform * parent_transform = calculated_transforms[entity_parent];
    Transform * entity_transform = entity_transforms[entity];
    Local_Transform * entity_local_transform = entity_local_transforms[entity];
    entity_transform->position = parent_transform->position + entity_local_transform->position;
    entity_transform->scale = parent_transform->scale * entity_local_transform->scale;
    calculated_transforms[entity] = entity_transform;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void local_transform_subscribe(const Entity entity)
{
    entity_parent_ids[entity] = (string *)get_component(entity, "parent_id");
    entity_transforms[entity] = (Transform *)get_component(entity, "transform");
    entity_local_transforms[entity] = (Local_Transform *)get_component(entity, "local_transform");
}


void local_transform_update()
{
    map<Entity, Entity> entity_parents;
    map<Entity, Transform *> calculated_transforms;

    for_each(entity_parent_ids, [&](const Entity entity, const string * parent_id) -> void
    {
        entity_parents[entity] = get_entity(*parent_id);
    });

    for_each(entity_parents, [&](const Entity entity, const Entity /*parent*/) -> void
    {
        if (!contains_key(calculated_transforms, entity))
        {
            calculate_transform(entity, entity_parents, calculated_transforms);
        }
    });
}


} // namespace Nito
