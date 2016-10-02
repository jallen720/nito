#include <GL/glew.h>

#include "Nito/Systems/Controller.hpp"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Cpp_Utils/Fn.hpp"

#include "Nito/Components.hpp"
#include "Nito/Graphics.hpp"


using std::map;
using std::pair;
using std::vector;
using std::string;
using std::runtime_error;
using glm::vec3;
using Cpp_Utils::accumulate;


namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<Entity> entities;
static vector<Transform *> entity_transforms;
static GLFWwindow * window;

const vector<string> required_components
{
    "transform",
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vec3 key_direction_accumulator(const vec3 & accumulation, const pair<int, const vec3> & key_direction)
{
    int key_state = glfwGetKey(window, key_direction.first);

    return (key_state == GLFW_PRESS || key_state == GLFW_REPEAT)
           ? accumulation + key_direction.second
           : accumulation;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void controller_subscribe(const Entity entity)
{
    string error_message;

    for (const string & component : required_components)
    {
        if (!has_component(entity, component))
        {
            error_message += "Entity does not have a " + component + " component required by the controller system!";
        }
    }

    if (!error_message.empty())
    {
        throw runtime_error(error_message);
    }

    entities.push_back(entity);
    entity_transforms.push_back((Transform *)get_component(entity, "transform"));
}


void controller_update()
{
    static const map<int, const vec3> key_directions
    {
        { GLFW_KEY_W, vec3( 0.0f,  1.0f, 0.0f) },
        { GLFW_KEY_S, vec3( 0.0f, -1.0f, 0.0f) },
        { GLFW_KEY_D, vec3( 1.0f,  0.0f, 0.0f) },
        { GLFW_KEY_A, vec3(-1.0f,  0.0f, 0.0f) },
    };

    const auto direction = accumulate(vec3(), key_directions, key_direction_accumulator);

    for (Transform * entity_transform : entity_transforms)
    {
        entity_transform->position += direction * 0.02f;
    }
}


void controller_init(GLFWwindow * _window)
{
    window = _window;
}


} // namespace Nito
