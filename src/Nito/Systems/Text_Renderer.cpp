#include "Nito/Systems/Renderer.hpp"

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"
#include "Nito/Resources.hpp"


using std::vector;
using std::string;

// glm/glm.hpp
using glm::vec3;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string TEXT_SHADER_PIPELINE_NAME = "text";
static vector<string *> entity_render_layers;
static vector<Text *> entity_texts;
static vector<const vec3 *> text_scales;
static vector<vector<string>> character_texture_paths;
static vector<vector<vec3>> character_positions;
static vector<vector<const Dimensions *>> character_dimensions;
static vector<Shader_Pipeline_Uniforms> text_uniforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void text_renderer_subscribe(const Entity entity)
{
    auto entity_text = (Text *)get_component(entity, "text");
    entity_render_layers.push_back((string *)get_component(entity, "render_layer"));
    entity_texts.push_back(entity_text);


    // Load character data for entity.
    auto entity_transform = (Transform *)get_component(entity, "transform");
    const vec3 * text_scale = &entity_transform->scale;
    vector<string> entity_character_texture_paths;
    vector<vec3> entity_character_positions;
    vector<const Dimensions *> entity_character_dimensions;
    const string font_prefix = entity_text->font + " : ";
    vec3 character_position_offset = vec3(0.0f);
    const float unit_scale = get_unit_scale();

    for (const char character : entity_text->value)
    {
        const string character_texture_path = font_prefix + character;
        const Dimensions * character_dimensions = &get_loaded_texture(character_texture_path).dimensions;
        entity_character_texture_paths.push_back(character_texture_path);
        entity_character_positions.push_back(entity_transform->position + character_position_offset);
        entity_character_dimensions.push_back(character_dimensions);
        character_position_offset.x += get_loaded_glyph_advance(character_texture_path) / unit_scale * text_scale->x;
    }

    character_texture_paths.push_back(entity_character_texture_paths);
    character_positions.push_back(entity_character_positions);
    character_dimensions.push_back(entity_character_dimensions);
    text_scales.push_back(text_scale);


    // Set text color for this entity's shader pipeline uniforms.
    text_uniforms.push_back(
        {
            {
                "text_color",
                {
                    Uniform::Types::VEC3,
                    &entity_text->color,
                },
            }
        });
}


void text_renderer_update()
{
    for (auto entity = 0u; entity < entity_render_layers.size(); entity++)
    {
        const string * entity_render_layer = entity_render_layers[entity];
        const vec3 * text_scale = text_scales[entity];
        const vector<string> & entity_character_texture_paths = character_texture_paths[entity];
        const vector<vec3> & entity_character_positions = character_positions[entity];
        const vector<const Dimensions *> & entity_character_dimensions = character_dimensions[entity];
        const Shader_Pipeline_Uniforms * entity_text_uniforms = &text_uniforms[entity];

        for (auto character_index = 0u; character_index < entity_character_texture_paths.size(); character_index++)
        {
            load_render_data(
                entity_render_layer,
                &entity_character_texture_paths[character_index],
                &TEXT_SHADER_PIPELINE_NAME,
                entity_text_uniforms,
                entity_character_dimensions[character_index],
                &entity_character_positions[character_index],
                text_scale);
        }
    }
}


} // namespace Nito
