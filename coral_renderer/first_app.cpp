#include "first_app.h"

#include "render_system.h"
#include "coral_camera.h"
#include "coral_buffer.h"

// STD
#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "vk_initializers.h"
#include "coral_texture.h"

using namespace coral_3d;

struct GlobalUBO
{
    glm::mat4 view_projeciton{1.f};

    // GLOBAL LIGHT
    glm::vec4 global_light_direction{ glm::normalize(glm::vec4{ 0.577f, 0.377f, -0.577f, 0.f})}; // w is ignored
    glm::vec4 ambient_light_color{.4f, .1f, .1f, 0.05f}; // w is intensity

    // POINT LIGHT
    glm::vec4 light_position{0.f, -0.85f, 0.f, 0.f}; // w is ignored
    glm::vec4 light_color{1.f}; // w is intensity
};

first_app::first_app()
{
    global_descriptor_pool_ = coral_descriptor_pool::Builder(device_)
        .set_max_sets(coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .build();

	load_gameobjects();
}

first_app::~first_app()
{}

void first_app::run()
{
    coral_buffer global_ubo
    {
        device_,
		sizeof(GlobalUBO),
		coral_swapchain::MAX_FRAMES_IN_FLIGHT,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
        0,
        device_.properties.limits.minUniformBufferOffsetAlignment
	};
    global_ubo.map();

    auto global_set_layout = coral_descriptor_set_layout::Builder(device_)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(); 

    std::vector<VkDescriptorSet> global_descriptor_sets{coral_swapchain::MAX_FRAMES_IN_FLIGHT};
    for (int i = 0; i < global_descriptor_sets.size(); i++)
    {
        auto buffer_info = global_ubo.descriptor_info_index(i);
        auto image_info = test_texture->get_descriptor_info();

        coral_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
            .write_buffer(0, &buffer_info)
            .write_image(1, &image_info)
            .build(global_descriptor_sets[i]);
    }

	render_system render_system{ device_, renderer_.get_swapchain_render_pass(), global_set_layout->get_descriptor_set_layout() };
    coral_camera camera{ {-1.5f, -1.5f, -1.5f} };

    auto last_time{ std::chrono::high_resolution_clock::now() };

	while (!window_.should_close())
	{
		glfwPollEvents();

        auto current_time{ std::chrono::high_resolution_clock::now() };
        auto frame_time{ std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count() };
        last_time = current_time;

        // MOVE CAMERA
        camera.update_input(window_.get_glfw_window(), frame_time);

        float aspect{ renderer_.get_aspect_ratio() };
        camera.set_perspective_projection(glm::radians(60.f), aspect, 0.1f, 1000.f);

		if (auto command_buffer = renderer_.begin_frame())
		{
            const int frame_index{ renderer_.get_frame_index() };

            FrameInfo frame_info
            {
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index],
                gameobjects_
            };

            // UPDATE GLOBAL UBO
            GlobalUBO ubo{};
            ubo.view_projeciton = camera.get_projection() * camera.get_view();
            global_ubo.write_to_index(&ubo, frame_index);
            global_ubo.flush_index(frame_index);

            // RENDER
			renderer_.begin_swapchain_render_pass(command_buffer);
			render_system.render_gameobjects(frame_info);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::load_gameobjects()
{
    constexpr uint32_t NUM_INSTANCES{ 125 };

    std::shared_ptr<coral_mesh> smooth_mesh {coral_mesh::create_mesh_from_file(device_, "assets/meshes/smooth_vase.obj")};
    std::shared_ptr<coral_mesh> flat_mesh {coral_mesh::create_mesh_from_file(device_, "assets/meshes/flat_vase.obj")};

    for (size_t x = 0; x < NUM_INSTANCES * 0.5f; x++)
    {
        for (size_t z = 0; z < NUM_INSTANCES * 0.5f; z++)
        {
            auto vase{ coral_gameobject::create_gameobject() };
            vase.mesh_ = z < NUM_INSTANCES * 0.25f ? smooth_mesh : flat_mesh;
            vase.transform_.translation = { 0.6f * x, 0.75f, 0.6f * z };
            vase.transform_.scale = { 3.f, 3.f, 3.f };
            gameobjects_.emplace(vase.get_id(), std::move(vase));
        }
    }
    
    std::cout << "\nNumber of instances: " << gameobjects_.size() << std::endl;
    std::cout << std::setprecision(8) << "Number of vertices: " << smooth_mesh->get_vertex_count() * gameobjects_.size() * 0.5f + flat_mesh->get_vertex_count() * gameobjects_.size() * 0.5f << std::endl;
    std::cout << std::setprecision(8) << "Number of indices: " << smooth_mesh->get_index_count() * gameobjects_.size() + flat_mesh->get_index_count() * gameobjects_.size() * 0.5f << std::endl;
    
    // LOAD TEXTURES
    test_texture = coral_texture::create_texture_from_file(
        device_,
        "assets/textures/uv_checker.jpg",
        VK_FORMAT_R8G8B8A8_SRGB
    );
}