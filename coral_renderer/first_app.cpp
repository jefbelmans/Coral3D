#include "first_app.h"

#include "render_system.h"

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

	while (!window_.should_close())
	{
		glfwPollEvents();

		if (auto command_buffer = renderer_.begin_frame())
		{
			renderer_.begin_swapchain_render_pass(command_buffer);
			render_system.render_gameobjects(command_buffer, gameobjects_);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::load_gameobjects()
{
	std::vector<Vertex> vertices
	{
		{{0.0f, -0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f }},
		{{-0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f }},
		{{0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }},
	};

	auto mesh{ std::make_shared<coral_mesh>(device_, vertices) };
	auto triangle = coral_gameobject::create_gameobject();
	triangle.mesh_ = mesh;
	triangle.color_ = { 0.1f, 0.8f, 0.1f };
	triangle.transform_.translation.x = -.2f;

	gameobjects_.emplace_back(std::move(triangle));
}