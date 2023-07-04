#include "first_app.h"

#include "render_system.h"
#include "coral_camera.h"

// STD
#include <stdexcept>
#include <array>

#include "vk_initializers.h"

using namespace coral_3d;

first_app::first_app()
{
	load_gameobjects();
}

first_app::~first_app()
{
}

void first_app::run()
{
	render_system render_system{ device_, renderer_.get_swapchain_render_pass() };
    coral_camera camera{};
   
    // camera.set_view_direction(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});

	while (!window_.should_close())
	{
		glfwPollEvents();

        float aspect{ renderer_.get_aspect_ratio() };
        // camera.set_ortographic_projection(-aspect, aspect, -1, 1, -1, 1);
        camera.set_perspective_projection(glm::radians(60.f), aspect, 0.1f, 10.f);

		if (auto command_buffer = renderer_.begin_frame())
		{
			renderer_.begin_swapchain_render_pass(command_buffer);
			render_system.render_gameobjects(command_buffer, gameobjects_, camera);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

std::unique_ptr<coral_mesh> create_cube_model(coral_device& device, glm::vec3 offset) {
    std::vector<Vertex> vertices
    {
        // left face (white)
        {{-.5f, -.5f, -.5f}, { .9f, .9f, .9f }},
        { {-.5f, .5f, .5f}, {.9f, .9f, .9f} },
        { {-.5f, -.5f, .5f}, {.9f, .9f, .9f} },
        { {-.5f, -.5f, -.5f}, {.9f, .9f, .9f} },
        { {-.5f, .5f, -.5f}, {.9f, .9f, .9f} },
        { {-.5f, .5f, .5f}, {.9f, .9f, .9f} },

            // right face (yellow)
        { {.5f, -.5f, -.5f}, {.8f, .8f, .1f} },
        { {.5f, .5f, .5f}, {.8f, .8f, .1f} },
        { {.5f, -.5f, .5f}, {.8f, .8f, .1f} },
        { {.5f, -.5f, -.5f}, {.8f, .8f, .1f} },
        { {.5f, .5f, -.5f}, {.8f, .8f, .1f} },
        { {.5f, .5f, .5f}, {.8f, .8f, .1f} },

            // top face (orange, remember y axis points down)
        { {-.5f, -.5f, -.5f}, {.9f, .6f, .1f} },
        { {.5f, -.5f, .5f}, {.9f, .6f, .1f} },
        { {-.5f, -.5f, .5f}, {.9f, .6f, .1f} },
        { {-.5f, -.5f, -.5f}, {.9f, .6f, .1f} },
        { {.5f, -.5f, -.5f}, {.9f, .6f, .1f} },
        { {.5f, -.5f, .5f}, {.9f, .6f, .1f} },

            // bottom face (red)
        { {-.5f, .5f, -.5f}, {.8f, .1f, .1f} },
        { {.5f, .5f, .5f}, {.8f, .1f, .1f} },
        { {-.5f, .5f, .5f}, {.8f, .1f, .1f} },
        { {-.5f, .5f, -.5f}, {.8f, .1f, .1f} },
        { {.5f, .5f, -.5f}, {.8f, .1f, .1f} },
        { {.5f, .5f, .5f}, {.8f, .1f, .1f} },

            // nose face (blue)
        { {-.5f, -.5f, 0.5f}, {.1f, .1f, .8f} },
        { {.5f, .5f, 0.5f}, {.1f, .1f, .8f} },
        { {-.5f, .5f, 0.5f}, {.1f, .1f, .8f} },
        { {-.5f, -.5f, 0.5f}, {.1f, .1f, .8f} },
        { {.5f, -.5f, 0.5f}, {.1f, .1f, .8f} },
        { {.5f, .5f, 0.5f}, {.1f, .1f, .8f} },

            // tail face (green)
        { {-.5f, -.5f, -0.5f}, {.1f, .8f, .1f} },
        { {.5f, .5f, -0.5f}, {.1f, .8f, .1f} },
        { {-.5f, .5f, -0.5f}, {.1f, .8f, .1f} },
        { {-.5f, -.5f, -0.5f}, {.1f, .8f, .1f} },
        { {.5f, -.5f, -0.5f}, {.1f, .8f, .1f} },
        { {.5f, .5f, -0.5f}, {.1f, .8f, .1f} },

    };
    for (auto& v : vertices)
        v.position += offset;

    return std::make_unique<coral_mesh>(device, vertices);
}

void first_app::load_gameobjects()
{
    std::shared_ptr<coral_mesh> mesh {create_cube_model(device_, { 0.f, 0.f, 0.f })};

    auto cube{ coral_gameobject::create_gameobject() };
    cube.mesh_ = mesh;
    cube.transform_.translation = { 0.f, 0.f, 2.5f };
    cube.transform_.scale = { 0.5f, 0.5f, 0.5f };

    gameobjects_.emplace_back(std::move(cube));
}