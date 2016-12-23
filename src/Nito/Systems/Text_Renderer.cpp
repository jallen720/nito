#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Resources.hpp"


using std::vector;
using std::map;
using std::string;

// glm/glm.hpp
using glm::vec3;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
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
    const string * render_layer;
    const Transform * transform;
    Dimensions * dimensions;
    const Text * text;
    Render_Data::Uniforms uniforms;
    vector<string> character_texture_paths;
    vector<const Dimensions *> character_dimensions;
    vector<vec3> character_positions;
    vector<float> character_advances;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string TEXT_SHADER_PIPELINE_NAME = "text";
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void text_renderer_subscribe(const Entity entity)
{
    Entity_State & entity_state = entity_states[entity];
    auto entity_dimensions = (Dimensions *)get_component(entity, "dimensions");
    auto entity_text = (Text *)get_component(entity, "text");
    entity_state.render_layer = (string *)get_component(entity, "render_layer");
    entity_state.transform = (Transform *)get_component(entity, "transform");
    entity_state.dimensions = entity_dimensions;
    entity_state.text = entity_text;


    // Entity's width and height are calculated based on the width and height of its characters, so ensure width and
    // height are 0 in case user specified different values.
    entity_dimensions->width = 0.0f;
    entity_dimensions->height = 0.0f;


    // Load character data for entity.
    vector<string> & entity_character_texture_paths = entity_state.character_texture_paths;
    vector<const Dimensions *> & entity_character_dimensions = entity_state.character_dimensions;
    vector<vec3> & entity_character_positions = entity_state.character_positions;
    vector<float> & entity_character_advances = entity_state.character_advances;
    const string font_prefix = entity_text->font + " : ";
    const float pixels_per_unit = get_pixels_per_unit();

    for (const char character : entity_text->value)
    {
        const string character_texture_path = font_prefix + character;
        const Glyph & character_glyph = get_loaded_glyph(character_texture_path);
        float character_advance =  character_glyph.advance / pixels_per_unit;
        entity_character_texture_paths.push_back(character_texture_path);
        entity_character_dimensions.push_back(&get_loaded_texture(character_texture_path).dimensions);
        entity_character_positions.push_back(vec3());
        entity_character_advances.push_back(character_advance);


        // Update text entity's width & height.
        float character_bearing_y = character_glyph.bearing.y / pixels_per_unit;
        entity_state.dimensions->width += character_advance;

        if (character_bearing_y > entity_state.dimensions->height)
        {
            entity_state.dimensions->height = character_bearing_y;
        }
    }


    // Set text color for this entity's shader pipeline uniforms.
    entity_state.uniforms["text_color"] =
    {
        Uniform::Types::VEC3,
        &entity_text->color,
    };
}


void text_renderer_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void text_renderer_update()
{
    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Dimensions * entity_dimensions = entity_state.dimensions;
        const vector<string> & entity_character_texture_paths = entity_state.character_texture_paths;
        const vector<const Dimensions *> & entity_character_dimensions = entity_state.character_dimensions;
        vector<vec3> & entity_character_positions = entity_state.character_positions;
        const vector<float> & entity_character_advances = entity_state.character_advances;
        vec3 character_position_offset(0.0f);

        vec3 entity_origin_offset =
            vec3(entity_dimensions->width, entity_dimensions->height, 0.0f) * entity_dimensions->origin;


        // Calculate character positions and load character rendering data.
        for (auto character_index = 0u; character_index < entity_character_texture_paths.size(); character_index++)
        {
            const Dimensions * character_dimensions = entity_character_dimensions[character_index];
            vec3 & character_position = entity_character_positions[character_index];

            character_position = get_child_world_position(
                entity_transform,
                character_position_offset - entity_origin_offset);

            load_render_data(
                {
                    Render_Modes::TRIANGLES,
                    entity_state.render_layer,
                    &entity_character_texture_paths[character_index],
                    &TEXT_SHADER_PIPELINE_NAME,
                    &entity_state.uniforms,
                    calculate_model_matrix(
                        character_dimensions->width,
                        character_dimensions->height,
                        character_dimensions->origin,
                        character_position,
                        entity_transform->scale,
                        entity_transform->rotation),
                });

            character_position_offset.x += entity_character_advances[character_index];
        }
    });
}


} // namespace Nito
