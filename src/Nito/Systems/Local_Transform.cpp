#include "Nito/Systems/Local_Transform.hpp"

#include <map>
#include <string>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"


using std::map;
using std::string;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::remove;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Entity_State
{
    const string * parent_id;
    Transform * transform;
    const Local_Transform * local_transform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calculate_transform(
    const Entity entity,
    const map<Entity, Entity> & entity_parents,
    map<Entity, const Transform *> & calculated_transforms)
{
    // This entity has no parent, meaning it is a root transform, so no calculations need to be done on its transform.
    if (!contains_key(entity_parents, entity))
    {
        calculated_transforms[entity] = (Transform *)get_component(entity, "transform");
        return;
    }

    Entity_State & entity_state = entity_states[entity];
    Entity entity_parent = entity_parents.at(entity);

    // If entity's parent's transform has not already been calculated, calculate its transform before calculating
    // entity's transform.
    if (!contains_key(calculated_transforms, entity_parent))
    {
        calculate_transform(entity_parent, entity_parents, calculated_transforms);
    }


    // Calculate entity's transform based on its parent's transform and its local_transform.
    const Transform * parent_transform = calculated_transforms[entity_parent];
    Transform * entity_transform = entity_state.transform;
    const Local_Transform * entity_local_transform = entity_state.local_transform;
    entity_transform->position = get_child_world_position(parent_transform, entity_local_transform->position);
    entity_transform->scale = parent_transform->scale * entity_local_transform->scale;
    entity_transform->rotation = parent_transform->rotation + entity_local_transform->rotation;
    calculated_transforms[entity] = entity_transform;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void local_transform_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (string *)get_component(entity, "parent_id"),
        (Transform *)get_component(entity, "transform"),
        (Local_Transform *)get_component(entity, "local_transform"),
    };
}


void local_transform_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void local_transform_update()
{
    map<Entity, Entity> entity_parents;
    map<Entity, const Transform *> calculated_transforms;

    for_each(entity_states, [&](const Entity entity, const Entity_State & state) -> void
    {
        entity_parents[entity] = get_entity(*state.parent_id);
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
