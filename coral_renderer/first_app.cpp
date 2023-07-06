#include "first_app.h"

#include "KeyboardController.h"
#include "render_system.h"
#include "coral_camera.h"

// STD
#include <stdexcept>
#include <array>
#include <chrono>

#include "vk_initializers.h"
#include "coral_texture.h"

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
  
    auto camera_object{ coral_gameobject::create_gameobject() };
    KeyboardController camera_controller{};

    auto last_time{ std::chrono::high_resolution_clock::now() };

    vkutil::load_image_from_file(device_, "textures/uv_checker.jpg", test_texture);

	while (!window_.should_close())
	{
		glfwPollEvents();

        auto current_time{ std::chrono::high_resolution_clock::now() };
        auto frame_time{ std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count() };
        last_time = current_time;

        // MOVE CAMERA
        camera_controller.move_in_plane_xz(window_.get_glfw_window(), frame_time, camera_object);
        camera.set_view_yxz(camera_object.transform_.translation, camera_object.transform_.rotation);

        float aspect{ renderer_.get_aspect_ratio() };
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

void first_app::load_gameobjects()
{
    std::shared_ptr<coral_mesh> mesh {coral_mesh::create_mesh_from_file(device_, "meshes/teapot.obj")};

    auto gameobject { coral_gameobject::create_gameobject() };
    gameobject.mesh_ = mesh;
    gameobject.transform_.translation = { 0.f, 0.f, 2.5f };
    gameobject.transform_.scale = { .5f, .5f, .5f };

    gameobjects_.emplace_back(std::move(gameobject));
}